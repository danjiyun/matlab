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

#include "stdafx.h"
#include "serialize.h"
#include "com.h"
#include "svflow.h"

#define TIMEOUTS_ENABLED	// allows functions that rely on serial reads to timeout.  comment out for debugging.

typedef struct _context_t
{
	com_t						com;		// must be first member
	HANDLE						hComm;
	svflow_sample_t *			p_flow_sample;
	uint32_t					sample_ndx;
	uint32_t					sample_count;
	uint32_t					last_stamp;
	uint64_t					timestamp;
}
context_t;

static uint16_t uart_write(com_t *p_com, void *pv, uint16_t length)
{
	DWORD written;
	context_t *p_context = (context_t*)p_com;
	WriteFile(p_context->hComm, pv, length, &written, NULL);
	return (uint16_t)written;
}

static uint16_t uart_read(com_t *p_com, void *pv, uint16_t length)
{
	DWORD read;
	context_t *p_context = (context_t*)p_com;
	ReadFile(p_context->hComm, pv, length, &read, NULL);
	return (uint16_t)read;
}

static void direction( svflow_direction_t *p_dst, const com_device_flow_direction_t *p_src, uint32_t ndx )
{
	p_dst->p_average[ndx] = p_src->average;
	for( uint32_t i = 0; i < MAX_HITCOUNT; i++ )
		p_dst->p_hit[ndx*MAX_HITCOUNT + i] = p_src->hit[i];
	p_dst->p_t1_t2[ndx] = p_src->t1_t2;
	p_dst->p_t2_ideal[ndx] = p_src->t2_ideal;
}

static bool serialize_cb(void *pv_context, const void *pv_data, uint16_t length)
{
	context_t * p_context = (context_t *)pv_context;
	const com_union_t *p_packet = (const com_union_t*)pv_data;
	
	if (length != p_packet->hdr.size || com_checksum(pv_data, (uint8_t)length))
		return false;  // packet error detected -- simply drop the packet

	if (p_packet->hdr.id == COM_ID_DEVICE_FLOW_SAMPLE )
	{
		com_device_flow_sample_t *p_com_sample = (com_device_flow_sample_t*)&p_packet->flow_sample;
		if (!p_context->sample_ndx )
		{
			p_context->last_stamp = p_com_sample->timestamp;
			p_context->timestamp = 0;
		}

		svflow_sample_t *p_flow_sample = p_context->p_flow_sample;
		uint32_t ndx = p_context->sample_ndx;

		direction( &p_flow_sample->up, &p_com_sample->up, ndx );
		direction( &p_flow_sample->down, &p_com_sample->down, ndx );

		p_context->timestamp += (p_com_sample->timestamp-p_context->last_stamp);
		p_context->last_stamp = p_com_sample->timestamp;

		p_flow_sample->p_timestamp[ndx] = ((double_t)(p_context->timestamp )) / 96000000.0; // 96MHz is the timestamp frequency of the embedded target

		if (p_flow_sample->p_timestamp[ndx] > 10)
		{
		//	DebugBreak();
		}

		p_flow_sample->p_tof_diff[ndx] = p_com_sample->tof_diff;
		p_flow_sample->p_temp_sense[ndx] = p_com_sample->temp_sense;
		p_flow_sample->p_temp_ref[ndx] = p_com_sample->temp_ref;

		p_context->sample_ndx++;
		if( p_context->sample_ndx >= p_context->sample_count )
			return true;
	}
	return false;
}

uint32_t svflow_get_samples( void *pv_context, svflow_sample_t *p_flow_sample, uint32_t sample_count )
{
	context_t *p_context = (context_t*)pv_context;

	if (!p_flow_sample || !p_context)
		return 0;

	p_context->p_flow_sample = p_flow_sample;
	p_context->sample_count = sample_count;
	p_context->sample_ndx = 0;

	for( uint32_t i = 0; i < sample_count; i++ )
	{
#ifdef TIMEOUTS_ENABLED
		uint32_t tc, ltc = GetTickCount();
#endif
		while( p_context->sample_ndx < p_context->sample_count )
		{
#ifdef TIMEOUTS_ENABLED
			tc = GetTickCount();
			if( tc - ltc >= 1000 )
				goto timeout;
			if( com_read( &p_context->com ) )
				ltc = tc;
#else
			com_read( &p_context->com );
#endif
		}
	}
timeout:
	return p_context->sample_count;
}

void svflow_start( void *pv_context, float_t sample_rate_hz )
{
	context_t *p_context = (context_t*)pv_context;
	if( p_context )
	{
		com_host_start_sampling_t cmd;
		cmd.sample_rate_hz = sample_rate_hz;
		com_tx( &p_context->com, &cmd, COM_ID_HOST_START_SAMPLING, sizeof( com_host_start_sampling_t ) );
	}
}

void svflow_stop( void *pv_context )
{
	context_t *p_context = (context_t*)pv_context;
	if( p_context )
	{
		com_header_t cmd;
		com_tx( &p_context->com, &cmd, COM_ID_HOST_STOP_SAMPLING, sizeof( com_header_t ) );
	}
}

 void* svflow_open( uint32_t comport )
{
	 TCHAR temp[32] = { 0 };
	 HANDLE hComm;

	_stprintf_s(temp, ARRAY_COUNT(temp), _T("\\\\.\\COM%d"), comport);

	if (INVALID_HANDLE_VALUE == (hComm = CreateFile(temp, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL)))
	{
		return NULL;
	}
	context_t *p_context = (context_t*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(context_t));
	p_context->hComm = hComm;
	{
		DCB dcbSerialParams = { 0 };
		dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
		GetCommState(p_context->hComm, &dcbSerialParams);
		dcbSerialParams.BaudRate = CBR_115200;
		dcbSerialParams.ByteSize = 8;
		dcbSerialParams.StopBits = ONESTOPBIT;
		dcbSerialParams.Parity = NOPARITY;
		dcbSerialParams.fInX = FALSE;
		dcbSerialParams.fOutX = FALSE;
		SetCommState(p_context->hComm, &dcbSerialParams);
	}
	{
		COMMTIMEOUTS timeouts = { 0 };
		timeouts.ReadIntervalTimeout = 100; // in milliseconds
		timeouts.ReadTotalTimeoutConstant = 100;
		timeouts.ReadTotalTimeoutMultiplier = 1;
		timeouts.WriteTotalTimeoutConstant = 100; // in milliseconds
		timeouts.WriteTotalTimeoutMultiplier = 1;
		SetCommTimeouts(p_context->hComm, &timeouts);
	}
	{
		static uint8_t s_decode_buffer[COM_MAX_PACKET_SIZE * 2];
		static uint8_t s_encode_buffer[COM_MAX_PACKET_SIZE];
		com_init(&p_context->com, serialize_cb, p_context, s_decode_buffer, sizeof(s_decode_buffer), s_encode_buffer, sizeof(s_encode_buffer), uart_write, uart_read);
	}
	return p_context;
}

 void svflow_close(void *pv_context)
 {
	 context_t *p_context = (context_t*)pv_context;
	 if (p_context)
	 {
		 CloseHandle(p_context->hComm);
		 HeapFree(GetProcessHeap(), 0, p_context);
	 }
 }
