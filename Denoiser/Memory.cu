#include "CUDA_WRAPPER.h"

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

bool allocateCUDAFloatBuffer(float* ptr, float** dev_ptr, size_t numFloats)
{
	cudaError_t deviceStatus;

	deviceStatus = cudaMalloc((void**)dev_ptr, numFloats * sizeof(float));

	if (!checkCudaErrorStatus(deviceStatus, "cudaMalloc"))
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool freeCUDAFloatBuffer(float* dev_ptr)
{
	cudaError_t deviceStatus;

	deviceStatus = cudaFree(dev_ptr);

	if (!checkCudaErrorStatus(deviceStatus, "cudaMalloc"))
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool copyMem2Device(float* dev_ptr, float* ptr, size_t numFloats)
{
	cudaError_t deviceStatus;

	deviceStatus = cudaMemcpy(dev_ptr, ptr, numFloats * sizeof(float), cudaMemcpyHostToDevice);

	if (!checkCudaErrorStatus(deviceStatus, "cudaMemcpy (cudaMemcpyHostToDevice)"))
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool copyMemFromDevice(float* ptr, float* dev_ptr, size_t numFloats)
{
	cudaError_t deviceStatus;

	deviceStatus = cudaMemcpy(ptr, dev_ptr, numFloats * sizeof(float), cudaMemcpyDeviceToHost);

	if (!checkCudaErrorStatus(deviceStatus, "cudaMemcpy (cudaMemcpyDeviceToHost)"))
	{
		return false;
	}
	else
	{
		return true;
	}
}