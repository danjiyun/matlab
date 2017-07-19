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
#include "mex.h"	// C:\Program Files\MATLAB\R2016a\extern\include
#include "svflow.h"
#include "config.h"

typedef struct _function_table_t
{
	const char *p_name;
	void(*p_func)(int nlhs, mxArray *p_lhs[], int nrhs, const mxArray *p_rhs[]);
}
function_table_t;

static void mex_open(int nlhs, mxArray *p_lhs[], int nrhs, const mxArray *p_rhs[])
{
	uint32_t port;
	if (!nrhs || !mxIsScalar(p_rhs[0]) )
	{
		mexErrMsgTxt("Missing COM port number.\n");
		return;
	}
	port = (uint32_t)mxGetScalar(p_rhs[0]);
	void *pv_context = svflow_open(port);

	p_lhs[0] = mxCreateNumericMatrix(1, 1, mxINT64_CLASS, mxREAL);
	void **pp = (void*)mxGetData(p_lhs[0]);
	*pp = pv_context;
}

static void mex_start( int nlhs, mxArray *p_lhs[], int nrhs, const mxArray *p_rhs[] )
{
	if( !nrhs || !mxIsScalar( p_rhs[0] ) )
	{
		mexErrMsgTxt( "Missing handle.\n" );
		return;
	}
	if( nrhs < 2 || !mxIsScalar( p_rhs[1] ) )
	{
		mexErrMsgTxt( "Missing desired sampling frequency.\n" );
		return;
	}
	float_t sample_freq_hz = (float_t)mxGetScalar( p_rhs[1] );
	void **pp = (void*)mxGetData( p_rhs[0] );
	svflow_start( *pp, sample_freq_hz );
}

static void mex_stop( int nlhs, mxArray *p_lhs[], int nrhs, const mxArray *p_rhs[] )
{
	if( !nrhs || !mxIsScalar( p_rhs[0] ) )
	{
		mexErrMsgTxt( "Missing handle.\n" );
		return;
	}
	void **pp = (void*)mxGetData( p_rhs[0] );
	svflow_stop( *pp );
}

static void mex_close(int nlhs, mxArray *p_lhs[], int nrhs, const mxArray *p_rhs[])
{
	if (!nrhs || !mxIsScalar(p_rhs[0]))
	{
		mexErrMsgTxt("Missing handle.\n");
		return;
	}
	void **pp = (void*)mxGetData(p_rhs[0]);
	svflow_close(*pp);
}

static mxArray* create_direction_struct( uint32_t sample_count, svflow_direction_t *p_dir )
{
	static const char * direction_fieldnames[] =
	{
		"t2_ideal",
		"t1_t2",
		"hit",
		"average"
	};
	mxArray *p_direction_struct = mxCreateStructMatrix( 1, 1, ARRAY_COUNT( direction_fieldnames ), direction_fieldnames );
	mxArray *p_t2_ideal = mxCreateNumericMatrix( 1, sample_count, mxDOUBLE_CLASS, mxREAL );
	mxArray *p_t1_t2 = mxCreateNumericMatrix( 1, sample_count, mxDOUBLE_CLASS, mxREAL );
	mxArray *p_average = mxCreateNumericMatrix( 1, sample_count, mxDOUBLE_CLASS, mxREAL );
	mxArray *p_hit = mxCreateNumericMatrix( MAX_HITCOUNT, sample_count, mxDOUBLE_CLASS, mxREAL );
	mxSetField( p_direction_struct, 0, "t2_ideal", p_t2_ideal );
	mxSetField( p_direction_struct, 0, "t1_t2", p_t1_t2 );
	mxSetField( p_direction_struct, 0, "hit", p_hit );
	mxSetField( p_direction_struct, 0, "average", p_average );

	p_dir->p_t2_ideal = (double_t*)mxGetData( p_t2_ideal );
	p_dir->p_average = (double_t*)mxGetData( p_average );
	p_dir->p_hit = (double_t*)mxGetData( p_hit );
	p_dir->p_t1_t2 = (double_t*)mxGetData( p_t1_t2 );

	return p_direction_struct;
}


static void mex_get_samples(int nlhs, mxArray *p_lhs[], int nrhs, const mxArray *p_rhs[])
{
	char * sample_fieldnames[] =
	{
		"timestamp",
		"up",
		"down",
		"toff_diff",
		"temp_ref",
		"temp_sense"
	};
	
	if (!nrhs || !mxIsScalar(p_rhs[0]))
	{
		mexErrMsgTxt("Missing handle.\n");
		return;
	}
	if (nrhs < 2 || !mxIsScalar(p_rhs[1]))
	{
		mexErrMsgTxt("Missing number of samples to collect.\n");
		return;
	}

	svflow_sample_t sample;

	void **pp = (void*)mxGetData(p_rhs[0]);
	uint32_t sample_count = (uint32_t)mxGetScalar(p_rhs[1]);

	mxArray *p_sample_struct = mxCreateStructMatrix( 1, 1, ARRAY_COUNT(sample_fieldnames), sample_fieldnames );

	mxArray *p_timestamp = mxCreateNumericMatrix( 1, sample_count, mxDOUBLE_CLASS, mxREAL );
	mxSetField( p_sample_struct, 0, "timestamp", p_timestamp );

	mxSetField( p_sample_struct, 0, "up", create_direction_struct( sample_count, &sample.up ) );
	mxSetField( p_sample_struct, 0, "down", create_direction_struct( sample_count, &sample.down ) );

	mxArray *p_toff_diff = mxCreateNumericMatrix( 1, sample_count, mxDOUBLE_CLASS, mxREAL );
	mxSetField( p_sample_struct, 0, "toff_diff", p_toff_diff );
	
	mxArray *p_temp_ref = mxCreateNumericMatrix( 1, sample_count, mxDOUBLE_CLASS, mxREAL );
	mxSetField( p_sample_struct, 0, "temp_ref", p_temp_ref );

	mxArray *p_temp_sense = mxCreateNumericMatrix( 1, sample_count, mxDOUBLE_CLASS, mxREAL );
	mxSetField( p_sample_struct, 0, "temp_sense", p_temp_sense );

	sample.p_timestamp = (double_t*)mxGetData( p_timestamp );
	sample.p_tof_diff = (double_t*)mxGetData( p_toff_diff );
	sample.p_temp_ref = (double_t*)mxGetData( p_temp_ref );
	sample.p_temp_sense = (double_t*)mxGetData( p_temp_sense );

	svflow_get_samples( *pp, &sample, sample_count );

	p_lhs[0] = p_sample_struct;
}

static const function_table_t s_function_table[] =
{
	{ "get_samples", mex_get_samples },
	{ "open", mex_open },
	{ "close", mex_close },
	{ "start", mex_start },
	{ "stop", mex_stop }
};
 

static void function_error(void)
{
	size_t i;
	size_t ndx;
	const size_t buffer_size = 1024;

	char *p_msg = HeapAlloc(GetProcessHeap(), 0, buffer_size);
	StringCchCatA(p_msg, buffer_size, "Missing/Unknown function string. Expecting one of:");
	for (i = 0; i<ARRAY_COUNT(s_function_table); i++)
	{
		StringCbLengthA(p_msg, buffer_size, &ndx);
		StringCchPrintfA(&p_msg[ndx], buffer_size - ndx, "\n %s", s_function_table[i].p_name);
	}
	mexErrMsgTxt(p_msg);
	HeapFree(GetProcessHeap(), 0, p_msg);
}

void mexFunction(int nlhs, mxArray *p_lhs[], int nrhs, const mxArray *p_rhs[])
{
	uint32_t i;
	if (nrhs < 1)
	{
		function_error();
		return;
	}
	char func[32];
	if (!mxIsChar(p_rhs[0]) || mxGetString(p_rhs[0], func, sizeof(func)-1))
	{
		function_error();
		return;
	}
	for (i = 0; i < ARRAY_COUNT(s_function_table); i++)
	{
		if (!lstrcmpA(s_function_table[i].p_name, func))
		{
			s_function_table[i].p_func(nlhs, p_lhs, nrhs - 1, p_rhs + 1);
			return;
		}
	}
	function_error();
}

