#pragma once

#include <fftw3.h>

namespace Denoise
{
	/*
	typedef float DOMAIN_FORMAT;
	typedef fftwf_r2r_kind TRANSFORM_KIND;
	typedef fftwf_plan PLAN_TYPE;
	#define PLAN_CTOR fftwf_plan_many_r2r
	#define PLAN_DTOR fftwf_destroy_plan
	#define PLAN_EXECUTOR fftwf_execute_r2r
	*/

	typedef double DOMAIN_FORMAT;
	typedef fftw_r2r_kind TRANSFORM_KIND;
	typedef fftw_plan PLAN_TYPE;
	#define PLAN_CTOR fftw_plan_many_r2r
	#define PLAN_DTOR fftw_destroy_plan 
	#define PLAN_EXECUTOR fftw_execute_r2r

}