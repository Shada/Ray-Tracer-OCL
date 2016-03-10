#pragma once
	
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include "common.h"
#include "Camera.h"
#include "OBJImporter.h"

#ifdef __APPLE__
	#include <OpenCL/opencl.h>
#else
	#include <CL/cl.h>
#endif

#pragma comment(lib, "OpenCL.lib")

class GPGPULauncher
{
public:

	GPGPULauncher();
	~GPGPULauncher();

	void init(Camera *camera);
	void loadKernel(const char* name);
	void createProgram(const std::string& source);
	void launchProgram(Camera *camera);

	void updatebuffers(Camera* camera, float _dt);

	void printPlatformNames();
	void printDeviceNames();

	float* getImage() { return image; }
private:
	std::vector<cl_platform_id> platform_ids;
	std::vector<cl_device_id> device_ids;
	cl_context context;
	cl_command_queue command_queue;
	cl_mem memobj;
	cl_program program;
	cl_kernel kernel;
	cl_kernel raykernel;
	cl_mem imageBuffer;
	cl_mem ballBuffer;
	cl_mem triBuffer;
	cl_mem cameraBuffer;
	cl_mem lightBuffer;

	float* image;
	float* balls;
	float* triangles;
	float* lightPos;
	float* lightCol;
	float* lightSpeed;
	float* cameraData;

	double elapsed_execution_time;
	int number_of_executions;
	
	cl_mem redRidingHoodImageDataBuffer;
	float* redRidingHoodImageData;
	int numLights;

	std::string getPlatformName(cl_platform_id id);
	std::string getDeviceName(cl_device_id id);
};

