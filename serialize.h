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


typedef bool(*serialize_cb_t)(void *pv_context, const void *pv_packet, uint16_t length);

typedef enum _serialize_state_t
{
	serialize_state_stopped,
	serialize_state_started,
	serialize_state_escaped
}
serialize_state_t;

typedef struct _serialize_t
{
	serialize_cb_t		p_cb;
	void *				pv_cb_context;
	uint8_t * 			p_output;
	uint16_t			output_length;
	uint16_t			output_ndx;
	uint8_t				start, stop, escape;	// framing values
	serialize_state_t	state;					// decode state machine
	uint16_t			restart_count;			// number of times a start was encountered while already started.
	uint16_t			overrun;				// number of times serialization was topped to to insufficient destination buffer space.
}
serialize_t;

uint16_t serialize_decode(serialize_t * p_context, const void *pv_input, uint16_t length);
uint32_t serialize_decode_length( const serialize_t *p_context );
void serialize_init( serialize_t *p_context, serialize_cb_t p_cb, void *pv_cb_context, void *ppv_output, uint32_t output_length, uint8_t start, uint8_t stop, uint8_t escape );
uint32_t serialize_encode( const serialize_t * p_context, void *pv_output, uint32_t output_length, const void *pv_input, uint32_t input_length, bool start, bool stop );
void* serialize_decode_buffer( const serialize_t *p_context );
