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

#include "config.h"

// commands the device originates:

#define COM_ID_DEVICE_FLOW_SAMPLE			0x01
#define COM_ID_DEVICE_STATUS				0x02

// commands the host originates

#define COM_ID_HOST_STATUS					0x81
#define COM_ID_HOST_START_SAMPLING			0x82
#define COM_ID_HOST_STOP_SAMPLING			0x83

#pragma pack(1)

typedef struct _com_header_t
{
	// common preample to all packets

	uint8_t		id;			// identifies the packet
	uint8_t		size;		// number of bytes in this packet (including this header)
	uint8_t		checksum;  	// value makes entire packet byte sum equal zero
}
com_header_t;

// FROM HOST

typedef struct _com_host_start_sampling_t
{
	// command the device to start sampling with the given parameters

	com_header_t	hdr;
	float_t			sample_rate_hz;
}
com_host_start_sampling_t;

// FROM DEVICE


typedef struct _com_device_flow_direction_t
{
	// this is a mirror of the MAX35103's registers
	// See the MAX35103 documentation for details

	float_t				t2_ideal;
	float_t				t1_t2;
	float_t				hit[MAX_HITCOUNT];
	float_t				average;
}
com_device_flow_direction_t;

typedef struct _com_device_flow_sample_t
{
	// contains MAX35103 "up" and "down" measurments
	com_header_t				hdr;
	uint32_t 					timestamp;	// timestamp from 96MHz clock
	com_device_flow_direction_t	up;
	com_device_flow_direction_t	down;
	float_t						tof_diff;	// difference between up and down as computed by the MAX35103
	float_t						temp_ref;	// measurement time assoiated with the reference resistance
	float_t						temp_sense;	// measurement time associated with the sensing elemenent
}
com_device_flow_sample_t;

typedef union _com_union_t
{
	// union of all packets -- used to define static storage
	com_header_t					hdr;
	com_device_flow_sample_t		flow_sample;
	com_host_start_sampling_t		start_sampling;
}
com_union_t;

#pragma pack()

#define COM_MAX_SAMPLES_PER_PACKET	16
#define COM_MAX_PACKET_SIZE			(sizeof(com_union_t)+COM_MAX_SAMPLES_PER_PACKET*sizeof(float_t))

struct _com_t;

typedef uint16_t (*com_write_t)(struct _com_t *, void*,uint16_t);
typedef uint16_t (*com_read_t)(struct _com_t *, void*,uint16_t);

typedef struct _com_t
{
	// com instance struct
	// includes all data necessary to use the com_* functions
	com_union_t			com_union;
	serialize_t 		serialize;
	uint8_t	*			p_encode_buff;
	uint16_t			encode_buff_length;
	uint16_t			encode_ndx;
	com_write_t			write;
	com_read_t			read;
}
com_t;


void com_init(com_t *p_init, serialize_cb_t p_cb, void *pv_cb_context, void *pv_decode_buff, uint32_t decode_buff_length, void *pv_encode_buff, uint32_t encode_buff_length, com_write_t p_write_func, com_read_t p_read_func);
bool com_read( com_t *p_com );
void com_tx( com_t *p_com, void* pv_packet, uint8_t com_id, uint8_t size );
uint8_t com_checksum( const void *pv_packet, uint8_t size );

