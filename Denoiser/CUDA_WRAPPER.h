#pragma once

#include <string>

#include "common.h"

//Common
bool checkCudaErrorStatus(cudaError_t status, const std::string& functionName = "");

bool getCudaDeviceProperties(int device);

bool queryCUDADevices();

bool startCUDAApplication(int device);

bool quiteCUDAApplication();


//Memory
bool allocateCUDAFloatBuffer(float* ptr, float** dev_ptr, index_t numFloats);

bool freeCUDAFloatBuffer(float* dev_ptr);

bool copyMem2Device(float* dev_ptr, float* ptr, index_t numFloats);

bool copyMemFromDevice(float* ptr, float* dev_ptr, index_t numFloats);



