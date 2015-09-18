#include "CUDA_WRAPPER.h"

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <iostream>

bool checkCudaErrorStatus(cudaError_t status, const std::string& functionName)
{
	if (status != cudaSuccess)
	{
		std::cerr << "ERROR [ " << functionName << " ]: " << cudaGetErrorString(status) << std::endl;
		return false;
	}
	return true;
}

bool getCudaDeviceProperties(int device)
{
	cudaError_t deviceStatus;
	cudaDeviceProp properties;
	deviceStatus = cudaGetDeviceProperties(&properties, device);

	if (!checkCudaErrorStatus(deviceStatus, "cudaGetDeviceProperties"))
	{
		return false;
	}

	std::cout << "Compute Capabilities for " << properties.name << " : " << std::endl;
	std::cout << "Major: " << properties.major << ", Minor: " << properties.minor << std::endl;
	std::cout << "Details: " << std::endl;
	std::cout << "	Num of SM    : " << properties.multiProcessorCount << std::endl;
	std::cout << "	Mem per Block: " << properties.sharedMemPerBlock << std::endl;
	std::cout << "	Mem per SM   : " << properties.sharedMemPerMultiprocessor << std::endl;

	return true;
}

bool queryCUDADevices()
{
	cudaError_t deviceStatus;

	int deviceCount = 0;
	deviceStatus = cudaGetDeviceCount(&deviceCount);

	if (!checkCudaErrorStatus(deviceStatus, "cudaGetDeviceCount"))
	{
		return false;
	}

	std::cout << "Num CUDA Devices Found: " << deviceCount << std::endl;

	return true;
}

bool startCUDAApplication(int device)
{
	cudaError_t deviceStatus;

	deviceStatus = cudaSetDevice(device);
	
	if (!checkCudaErrorStatus(deviceStatus, "cudaSetDevice"))
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool quiteCUDAApplication()
{
	cudaError_t deviceStatus;

	deviceStatus = deviceStatus = cudaDeviceReset();

	if (!checkCudaErrorStatus(deviceStatus, "cudaDeviceReset"))
	{
		return false;
	}
	else
	{
		return true;
	}
}