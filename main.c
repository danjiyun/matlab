/*******************************************************************************
 * Copyright (C) 2017 Maxim Integrated Products, Inc., All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
 * OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of Maxim Integrated
 * Products, Inc. shall not be used except as stated in the Maxim Integrated
 * Products, Inc. Branding Policy.
 *
 * The mere transfer of this software does not imply any licenses
 * of trade secrets, proprietary technology, copyrights, patents,
 * trademarks, maskwork rights, or any other form of intellectual
 * property whatsoever. Maxim Integrated Products, Inc. retains all
 * ownership rights.
 *
 ******************************************************************************/

#include "global.h"

#include "tmr_utils.h"
#include "serialize.h"
#include "com.h"

#include "board.h"
#include "transducer.h"

#include <tmr.h>

#define RESULTS_COUNT		8

typedef enum _result_status_t
{
	result_status_invalid,	// a problem was detected with the data returned from the MAX3510x
	result_status_valid
}
result_status_t;

typedef struct _timestamped_results_t
{
	uint32_t			sequence;	// sequence number
	uint32_t			timestamp;	// 96MHz timestamp
	result_status_t		status;		// flags problems with the results field below
	max3510x_tof_results_t	tof_results;	// contains regiters values from the MAX3510x
	max3510x_temp_results_t	temp_results;	// contains regiters values from the MAX3510x
}
timestamped_results_t;

static timestamped_results_t 	s_ts_results[RESULTS_COUNT];    // circular buffer of results with timestamps
static uint8_t					s_results_read_ndx;
static uint8_t					s_results_count;
static uint8_t					s_results_write_ndx;

static uint32_t					s_sequence;

typedef enum _sample_state_t
{
	sample_state_idle,			// not expecting reponse from the MAX3501x
	sample_state_isr_pending,	// we've sent a command that should result in an interrupt
}
sample_state_t;


static sample_state_t s_sample_state;	// this variable serializes activity between max3510x_int_isr() and SysTick_Handler()
										// default NVIC priorities (==0) ensure serialized execution.

static uint32_t s_sampling_overrun;		// number of times max3510x_int_isr() executed before expected (non-zero indicates a fatal condition)
static uint32_t s_sampling_underflow;	// number of times a sample command was issued before the previous command returned a result.
										// >0 indicates that the max3510 can not support the desired sampling rate in it's current configuration.
static com_t s_com;						// com object
static bool s_send_samples;				// TRUE if the host has asked the target to send samples

void max3510x_int_isr(void * pv)
{
	// application-level interrupt service routine for the MAX3510x

	uint32_t ts = TMR32_GetCount(TIMESTAMP_TIMER);
	uint16_t status = max3510x_interrupt_status(&g_max35103);

	if( (status & (MAX3510X_REG_INTERRUPT_STATUS_TOF|MAX3510X_REG_INTERRUPT_STATUS_TO)) == MAX3510X_REG_INTERRUPT_STATUS_TOF )
	{
		max3510x_read_tof_results( &g_max35103, &s_ts_results[s_results_write_ndx].tof_results );
		max3510x_read_temp_results( &g_max35103, &s_ts_results[s_results_write_ndx].temp_results );
		if(  s_sample_state == sample_state_isr_pending )
		{
			// write into the circular buffer.  writes are guaranteed
			// oldest value overwritten

			// validate the measuremnt.
			// validation failures suggest SPI problems.
			if( max3510x_validate_measurement(&s_ts_results[s_results_write_ndx].tof_results.up, transducer_hit_count()) &&
				max3510x_validate_measurement(&s_ts_results[s_results_write_ndx].tof_results.down, transducer_hit_count()) )
			{
				s_ts_results[s_results_write_ndx].status = result_status_valid;
			} 
			else
			{
				s_ts_results[s_results_write_ndx].status = result_status_invalid;
			}

			s_ts_results[s_results_write_ndx].timestamp = ts;
			s_ts_results[s_results_write_ndx].sequence = s_sequence++;

			s_results_write_ndx++;
			s_results_count++;
			if( s_results_write_ndx >= RESULTS_COUNT )
			{
				s_results_write_ndx = 0;
			}
			if( s_results_count >= RESULTS_COUNT )
			{
				// we overwrote the oldest result
				s_results_count = RESULTS_COUNT;
				s_results_read_ndx++;
				if( s_results_read_ndx >= RESULTS_COUNT )
				{
					s_results_read_ndx = 0;
				}
			}
			s_sample_state = sample_state_idle;
		}
		else
		{
			s_sampling_overrun++;
		}
	}
	else if( status & MAX3510X_REG_INTERRUPT_STATUS_TE ) 
	{
		max3510x_tof_diff(&g_max35103);

	}
	else if( status & MAX3510X_REG_INTERRUPT_STATUS_TO ) 
	{
		s_sample_state = sample_state_idle;
	}
	board_max3510x_clear_interrupt();
}

void SysTick_Handler(void)
{
	// this timer drives sampling
	if( s_sample_state == sample_state_idle )
	{
		s_sample_state = sample_state_isr_pending;
		max3510x_temperature(&g_max35103);
	}
	else
	{
		s_sampling_underflow++;
	}
}

static uint16_t uart_write(com_t * p_com, void * pv, uint16_t length)
{
	// UART write function used by the com module
	return board_uart_write( pv, length );
}

static uint16_t uart_read(com_t * p_com, void * pv, uint16_t length)
{
	return board_uart_read( pv, length );
}

static bool serialize_cb(void *pv_context, const void *pv_packet, uint16_t length)
{
	// callback to dispatch packets recieved from the host
	const com_union_t *p_com = (const com_union_t*)pv_packet;
	if( length != p_com->hdr.size || com_checksum(pv_packet,length) )
		return false; // packet error detected -- simply drop the packet
	switch( p_com->hdr.id )
	{
		case COM_ID_HOST_START_SAMPLING:
		{
			if( p_com->start_sampling.sample_rate_hz > 0.0F && p_com->start_sampling.sample_rate_hz <= 200.0F )
			{
                s_sampling_underflow = 0;
                s_sampling_overrun = 0;
                s_sample_state = sample_state_idle;
				s_send_samples = true;
				SYS_SysTick_Config( (uint32_t)((float_t)SYS_SysTick_GetFreq() / p_com->start_sampling.sample_rate_hz), 1);
			}
			break;
		}
		case COM_ID_HOST_STOP_SAMPLING:
		{
			SYS_SysTick_Config( (uint32_t)((float_t)SYS_SysTick_GetFreq() / 10.0F), 1);  // 10Hz sampling rate
			s_send_samples = false;
			break;
		}
	}
	return false;
}


int main(void)
{
	board_init(max3510x_int_isr);

	{
		static uint8_t s_decode_buffer[COM_MAX_PACKET_SIZE * 2];
		static uint8_t s_encode_buffer[COM_MAX_PACKET_SIZE];
		com_init(&s_com, serialize_cb, NULL, s_decode_buffer, sizeof(s_decode_buffer), s_encode_buffer, sizeof(s_encode_buffer), uart_write, uart_read);
	}

	SYS_SysTick_Config( (uint32_t)((float_t)SYS_SysTick_GetFreq() / 10.0F), 1);  // 100Hz sampling rate by default

	uint32_t last_ts = 0, ts;
	uint32_t sum_ts_delta;
	while( 1 )
	{
		// execute-bound loop to dispatch received packets and conditionally send
		// data sampled from the MAX35103
		com_read(&s_com);
		if( s_results_count )
		{
			ts = s_ts_results[s_results_read_ndx].timestamp;
			sum_ts_delta += (ts - last_ts);
			last_ts = ts;
			max3510x_float_tof_results_t f_tof_results;
			max3510x_float_temp_results_t f_temp_results;
			
			max3510x_convert_tof_results( &f_tof_results, &s_ts_results[s_results_read_ndx].tof_results );
			max3510x_convert_temp_results( &f_temp_results, &s_ts_results[s_results_read_ndx].temp_results );
			if( s_send_samples )
			{
				com_device_flow_sample_t sample;
				sample.up.t2_ideal = f_tof_results.up.t2_ideal;
				sample.up.t1_t2 = f_tof_results.up.t1_t2;
				for (uint32_t i=0;i<MIN(MAX_HITCOUNT,MAX3510X_MAX_HITCOUNT);i++)
					sample.up.hit[i] = f_tof_results.up.hit[i];
				sample.up.average =  f_tof_results.up.average;

				sample.down.t2_ideal = f_tof_results.down.t2_ideal;
				sample.down.t1_t2 = f_tof_results.down.t1_t2;
				for (uint32_t i=0;i<MIN(MAX_HITCOUNT,MAX3510X_MAX_HITCOUNT);i++)
					sample.down.hit[i] = f_tof_results.down.hit[i];
				sample.down.average =  f_tof_results.down.average;
				sample.tof_diff = f_tof_results.tof_diff;
				sample.timestamp = ts;
				sample.temp_ref = f_temp_results.temp[1];
				sample.temp_sense = f_temp_results.temp[0];
				com_tx( &s_com, &sample, COM_ID_DEVICE_FLOW_SAMPLE, sizeof(sample) );
			}

			s_results_read_ndx++;
			if( s_results_read_ndx >= RESULTS_COUNT )
			{
				s_results_read_ndx = 0;
			}
			s_results_count--;
		}
	}
}

