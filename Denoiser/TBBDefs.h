#pragma once

//#include <tbb\queuing_mutex.h>
#include <tbb\spin_mutex.h>

namespace Denoise
{

	typedef tbb::spin_mutex TBB_MUTEX_TYPE;

}