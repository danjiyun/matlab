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

void serialize_init( serialize_t *p_context, serialize_cb_t p_cb, void *pv_cb_context, void *ppv_output, uint32_t output_length, uint8_t start, uint8_t stop, uint8_t escape )
{
	p_context->p_output = (uint8_t*)ppv_output;
	p_context->output_length = output_length;
	p_context->output_ndx = 0;
	p_context->start = start;
	p_context->stop = stop;
	p_context->escape = escape;
	p_context->state = serialize_state_stopped;
	p_context->restart_count = 0;
	p_context->overrun = 0;
	p_context->pv_cb_context = pv_cb_context;
	p_context->p_cb = p_cb;
}

void* serialize_decode_buffer( const serialize_t *p_context )
{
	return p_context->p_output;
}

uint32_t serialize_decode_length( const serialize_t *p_context )
{
	return p_context->output_ndx;
}

uint16_t serialize_decode( serialize_t *p_context, const void * pv_input, uint16_t length )
{
	uint16_t i,o;
	uint8_t b;
	serialize_state_t state = p_context->state;
	const uint8_t * p_input = (const uint8_t *)pv_input;

	for(i=0,o=p_context->output_ndx;i<length && o<p_context->output_length;i++)
	{
		b = p_input[i];
		if( state == serialize_state_stopped )
		{
			if( b == p_context->start )
			{
				state = serialize_state_started;
				o = 0;
			}
		}
		else if( state == serialize_state_started)
		{
			if( b == p_context->stop )
			{
				// decoded complete packet
				state = serialize_state_stopped;
				if( p_context->p_cb(p_context->pv_cb_context, p_context->p_output, o) )
					break;
				o = 0;
			}
			else if( b == p_context->escape )
			{
				state = serialize_state_escaped;
			}
			else if( b == p_context->start )
			{
				o = 0; // restarted - dump partial
				p_context->restart_count++;
			}
			else
			{
				if( o >= p_context->output_length )
				{
					o = 0;	// buffer overrun - dump partial
					state = serialize_state_stopped;
					p_context->overrun++;
				}
				else
				{
					p_context->p_output[o++] = b;
				}
			}
		}
		else if( state == serialize_state_escaped)
		{
			if( o >= p_context->output_length )
			{
				o = 0; // buffer overrun - dump partial
				state = serialize_state_stopped;
				p_context->overrun++;
			}
			else
			{
				p_context->p_output[o++] = b;
				state = serialize_state_started;
			}
		}
	}
	p_context->output_ndx = o;
	p_context->state = state;
	return i;
}


uint32_t serialize_encode( const serialize_t * p_context, void *pv_output, uint32_t output_length, const void *pv_input, uint32_t input_length, bool start, bool stop )
{
	const uint8_t * p_in = (const uint8_t *)pv_input;
	uint8_t * p_out = (uint8_t *)pv_output;
	uint32_t i,o=0;
	uint8_t b;

	if( start  )
	{
		while( !output_length );
		p_out[o++] = p_context->start;
	}

	for(i=0;i<input_length;i++)
	{
		b = p_in[i];
		if( b == p_context->escape || b == p_context->start || b == p_context->stop )
		{
			while( o >= output_length );
			p_out[o++] = p_context->escape;
		}
		while( o >= output_length );
		p_out[o++] = b;
	}
	if( stop )
	{
		while( o >= output_length );
		p_out[o++] = p_context->stop;
	}
	return o;	// return size of encode data stored in pv_output
}
