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
#include "serialize.h"
#include "com.h"

#define COM_SERIALIZE_START		0x18	// start of packet token
#define COM_SERIALIZE_STOP		0x81	// end of packet token
#define COM_SERIALIZE_ESCAPE	0x99	// escape token


void com_init( com_t *p_init, serialize_cb_t p_cb, void *pv_cb_context, void *pv_decode_buff, uint32_t decode_buff_length, void *pv_encode_buff, uint32_t encode_buff_length, com_write_t write_func, com_read_t read_func)
{
	// creates a com instance

	serialize_init( &p_init->serialize, p_cb, pv_cb_context, pv_decode_buff, decode_buff_length, COM_SERIALIZE_START, COM_SERIALIZE_STOP, COM_SERIALIZE_ESCAPE );
	p_init->p_encode_buff = (uint8_t*)pv_encode_buff;
	p_init->encode_buff_length = encode_buff_length;
	p_init->read = read_func;
	p_init->write = write_func;
	p_init->encode_ndx = 0;
}

uint8_t com_checksum( const void *pv_packet, uint8_t size )
{
	const uint8_t * p_packet = (const uint8_t *)pv_packet;
	uint8_t i;
	uint8_t sum = 0;
	for(i=0;i<size;i++)
	{
		sum += p_packet[i];
	}
	return sum;
}

void com_tx( com_t *p_com, void* pv_packet, uint8_t com_id, uint8_t size )
{
	// sends data using a com instance object

	com_header_t *p_hdr = (com_header_t*)pv_packet;
	p_hdr->id = com_id;
	p_hdr->size = size;
	p_hdr->checksum = 0;
	p_hdr->checksum = -com_checksum( pv_packet, size );
	size = serialize_encode( &p_com->serialize, p_com->p_encode_buff, p_com->encode_buff_length, pv_packet, size, true, true );
	p_com->write( p_com, p_com->p_encode_buff, size );
}

bool com_read( com_t *p_com )
{
	// receives data using a com instance object

	uint16_t length = p_com->read( p_com, &p_com->p_encode_buff[p_com->encode_ndx], p_com->encode_buff_length-p_com->encode_ndx );
	serialize_decode(&p_com->serialize, &p_com->p_encode_buff[p_com->encode_ndx], length);
	return length > 0 ? true : false;
}



