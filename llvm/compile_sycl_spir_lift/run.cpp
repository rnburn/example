#include <CL/cl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cassert>
#include <string>
#include <fstream>
using namespace std;

cl_int oclGetPlatformID(cl_platform_id* clSelectedPlatformID)
{
    char chBuffer[1024];
    cl_uint num_platforms;
    cl_platform_id* clPlatformIDs;
    cl_int ciErrNum;
    *clSelectedPlatformID = NULL;
    cl_uint i = 0;

    // Get OpenCL platform count
    ciErrNum = clGetPlatformIDs (0, NULL, &num_platforms);
    if (ciErrNum != CL_SUCCESS)
    {
        printf(" Error %i in clGetPlatformIDs Call !!!\n\n", ciErrNum);
        return -1000;
    }
    else
    {
        if(num_platforms == 0)
        {
            printf("No OpenCL platform found!\n\n");
            return -2000;
        }
        else
        {
            // if there's a platform or more, make space for ID's
            if ((clPlatformIDs = (cl_platform_id*)malloc(num_platforms * sizeof(cl_platform_id))) == NULL)
            {
                printf("Failed to allocate memory for cl_platform ID's!\n\n");
                return -3000;
            }

            // get platform info for each platform and trap the NVIDIA platform if found
            ciErrNum = clGetPlatformIDs (num_platforms, clPlatformIDs, NULL);
            printf("Available platforms:\n");
            for(i = 0; i < num_platforms; ++i)
            {
                ciErrNum = clGetPlatformInfo (clPlatformIDs[i], CL_PLATFORM_NAME, 1024, &chBuffer, NULL);
                if(ciErrNum == CL_SUCCESS)
                {
                    printf("platform %d: %s\n", i, chBuffer);
                    if(strstr(chBuffer, "AMD") != NULL)   //AMD's OPENCL implementation works fine
                    // if(strstr(chBuffer, "Intel") != NULL)   //Broken
                    {
                        printf("selected platform %d\n", i);
                        *clSelectedPlatformID = clPlatformIDs[i];
                        break;
                    }
                }
            }

            assert(clSelectedPlatformID);

            free(clPlatformIDs);
        }
    }

    return CL_SUCCESS;
}

int load(const char *filename, char*& str, size_t& size)
{
  std::fstream f(filename, (std::fstream::in | std::fstream::binary));

  if(f.is_open())
  {
    size_t fileSize;
    f.seekg(0, std::fstream::end);
    size = fileSize = (size_t)f.tellg();
    f.seekg(0, std::fstream::beg);
    str = new char[size+1];
    if(!str)
    {
      f.close();
      return 0;
    }
    f.read(str, fileSize);
    f.close();
    str[size] = '\0';
    return 0;
  }
  cout << "Error: cannot open file " << filename << endl;
  return -1;
}

#define CHK(t) \
{ \
  cl_int tmp = (t);\
  if (tmp != CL_SUCCESS) { \
    cout << __FILE__ << ":" << __LINE__ << ": Error " << tmp << endl; \
    exit(tmp); \
  }\
}

int main(int argc, char* argv[])
{
  cl_uint numPlatforms;
  cl_platform_id platform = NULL;
  CHK(clGetPlatformIDs(0, NULL, &numPlatforms));
  if(numPlatforms == 0) {
    cout << "No OpenCL platform\n";
    exit(-1);
  }
  CHK(oclGetPlatformID(&platform));

  cl_uint numDevices = 0;
  cl_device_id device;
  CHK(clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 0, NULL, &numDevices));
  if (numDevices == 0) {
    cout << "No OpenCL GPU device\n";
    exit(-1);
  }
  CHK(clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL));


  cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);
  cl_command_queue commandQueue = clCreateCommandQueue(context, device, 0, NULL);

  // Host data
  float *A = NULL;  // Input array
  float *B = NULL;  // Input array
  float *C = NULL;  // Output array

  // Elements in each array
  const int elements = 2048;   

  // Compute the size of the data 
  size_t datasize = sizeof(float)*elements;

  // Allocate space for input/output data
  A = (float*)malloc(datasize);
  B = (float*)malloc(datasize);
  C = (float*)malloc(datasize);
  // Initialize the input data
  for(int i = 0; i < elements; i++) {
    A[i] = i+1;
    B[i] = i+5;
  }  

  // Use clEnqueueWriteBuffer() to write input array A to
  // the device buffer bufferA
  cl_int status;

  cl_mem bufferA, bufferB, bufferC;
  bufferA = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float) * elements, NULL, NULL);
  bufferB = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float) * elements, NULL, NULL);
  bufferC = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float) * elements, NULL, NULL);
  status = clEnqueueWriteBuffer(
      commandQueue, 
      bufferA, 
      CL_FALSE, 
      0, 
      datasize,                         
      A, 
      0, 
      NULL, 
      NULL);

  // Use clEnqueueWriteBuffer() to write input array B to 
  // the device buffer bufferB
  status = clEnqueueWriteBuffer(
      commandQueue, 
      bufferB, 
      CL_FALSE, 
      0, 
      datasize,                                  
      B, 
      0, 
      NULL, 
      NULL);

  const char *filename = "device.bc";
  char* source;
  size_t size;
  CHK(load(filename, source, size));
  cl_int errcode;
  cl_program program = clCreateProgramWithBinary(context, 1, &device, &size, (const unsigned char**)&source, 0 , &errcode);
  CHK(errcode);
  errcode = clBuildProgram(program, 1, &device, " -x spir", NULL, NULL);


  if (errcode != CL_SUCCESS) {
    char buf[1000];
    CHK(clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 1000, buf, NULL));
    cout << "Build log:" << endl << buf << endl;
    exit(-1);
  }

  // cl_kernel kernel = clCreateKernel(program, "_Z11sum_squaredPKU3AS1fS0_PU3AS1f", &errcode);
  cl_kernel kernel = clCreateKernel(program, "mykernel", &errcode);
  CHK(errcode);

  errcode = clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufferA);
  errcode = clSetKernelArg(kernel, 1, sizeof(cl_mem), &bufferB);
  errcode = clSetKernelArg(kernel, 2, sizeof(cl_mem), &bufferC);
  // errcode = clSetKernelArg(kernel, 3, sizeof(elements), &elements);
  CHK(errcode);

  size_t global_work_size[1] = {10};
  CHK(clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL));

  clEnqueueReadBuffer(
      commandQueue, 
      bufferC, 
      CL_TRUE, 
      0, 
      datasize, 
      C, 
      0, 
      NULL, 
      NULL);

  for(int i=0; i<10; ++i)
    cout << C[i] << "\n";
      
  CHK(clReleaseKernel(kernel));
  CHK(clReleaseProgram(program));
  CHK(clReleaseCommandQueue(commandQueue));
  CHK(clReleaseContext(context));
  return 0;
}

