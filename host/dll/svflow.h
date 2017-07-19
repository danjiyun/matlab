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

typedef struct _svflow_direction_t
{
	double_t *p_t2_ideal;
	double_t *p_t1_t2;
	double_t *p_hit;
	double_t *p_average;
}
svflow_direction_t;

typedef struct _svflow_sample_t
{
	double_t *					p_timestamp;
	svflow_direction_t			up;
	svflow_direction_t			down;
	double_t *					p_tof_diff;
	double_t *					p_temp_ref;
	double_t *					p_temp_sense;
}
svflow_sample_t;

void* svflow_open( uint32_t comport);	// open and returns a flow object corrosponding the given Widnows COM port
void svflow_close(void *pv_context);	// closes a flow object returned by flow_open()

uint32_t svflow_get_samples(void *pv_context, svflow_sample_t *p_flow_sample, uint32_t sample_count);  // returns samples collected by the target

void svflow_start( void *pv_context, float_t sample_rate_hz );	// tells the target to begin collecting samples
void svflow_stop( void *pv_context );								// tells the target to stop collecting samples

