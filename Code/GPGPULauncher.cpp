#include "GPGPULauncher.h"

#define MEM_SIZE (512)
#define MAX_SOURCE_SIZE (0x100000)

GPGPULauncher::GPGPULauncher()
{
	context = nullptr;
	command_queue = nullptr;
	memobj = nullptr;
	program = nullptr;
	kernel = nullptr;
	raykernel = nullptr;
	imageBuffer = nullptr;
	image = nullptr;
	redRidingHoodImageDataBuffer = nullptr;
	redRidingHoodImageData = nullptr;
	elapsed_execution_time = 0;
	number_of_executions = 100;
}


GPGPULauncher::~GPGPULauncher()
{
	/* Finilization */
	clFlush(command_queue);
	clFinish(command_queue);
	clReleaseKernel(kernel);
	clReleaseKernel(raykernel);
	clReleaseProgram(program);
	clReleaseMemObject(memobj);
	clReleaseCommandQueue(command_queue);
	clReleaseContext(context);
	clReleaseMemObject(imageBuffer);
	clReleaseMemObject(redRidingHoodImageDataBuffer);
	delete(redRidingHoodImageData);
	redRidingHoodImageData = nullptr;
	delete(image);
	image = nullptr;
}
static const size_t testDataSize = 512 * 512;
void GPGPULauncher::launchProgram(Camera *camera)
{
	cl_int error = 0;
	// Prepare some test data
	
	cl_ulong time_start = 0, time_end = 0;

	const size_t globalWorkSize[] = { 512, 512, 0 };
	const size_t localWorkSize[] = { 16, 16, 0 };
	cl_event event;
	error = clEnqueueNDRangeKernel (command_queue, kernel, 2,
		nullptr,
		globalWorkSize,
		localWorkSize,
		0, nullptr, &event);
	if(error != 0)
	{
		printf("Error Queueing Kernel\n");
		printf("Error %i", error);
		printf("\nAborting...");
		printf(".......................\n");
		exit(error);
	}

	clWaitForEvents(1, &event);
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, nullptr);
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, nullptr);
	if (number_of_executions != 0)
	{
		elapsed_execution_time += (time_end - time_start) / 1000000;
	}
	else 
	{
		printf("Average time of execution: %f ms\n", elapsed_execution_time / 100);
	}
	number_of_executions--;


	error = clEnqueueReadBuffer(command_queue, imageBuffer, CL_FALSE, 0,
		sizeof (float) * testDataSize * 3,
		image,
		0, nullptr, nullptr);

	if(error != 0)
	{
		printf("Error Queueing Read Buffer to fetch data\n");
		printf("Error %i", error);
		printf("\nAborting...");
		printf(".......................\n");
		exit(error);
	}

}

void GPGPULauncher::init(Camera* camera)
{
	int tri = 300;
	numLights = 1;

	float* modeldata = nullptr;
	float* invcube = nullptr;
	OBJImporter *importer = new OBJImporter();
	importer->import("./Models/bthlogo/bth.obj", modeldata);	
	importer->import("./Models/cube/invcube.obj", invcube);
	delete(importer);
	importer = nullptr;

	printf(".......................\n");
	printf("Starting initialization of OpenCL\n\n");
	cl_int ret = 0;
	
	/* Get Platform IDs */
	
	printf("Getting available Platforms\n");
	cl_uint ret_num_platforms = 0;
	ret = clGetPlatformIDs(0, nullptr, &ret_num_platforms);
	if(ret != 0)
	{
		printf("Error getting Platform IDs\n");
		printf("Error %i", ret);
		printf("\nAborting...");
		printf(".......................\n");
		exit(ret);
	}
	platform_ids = std::vector<cl_platform_id>(ret_num_platforms);
	ret = clGetPlatformIDs(ret_num_platforms, platform_ids.data(), nullptr);
	if(ret != 0)
	{
		printf("Error getting Platform IDs\n");
		printf("Error %i", ret);
		printf("\nAborting...");
		printf(".......................\n");
		exit(ret);
	}
	printPlatformNames();
	
	/* Get Device IDs */
	printf("Getting available Devices\n");
	cl_uint ret_num_devices = 0;
	ret = clGetDeviceIDs(platform_ids[0], CL_DEVICE_TYPE_ALL, 0, nullptr, &ret_num_devices);
	if(ret != 0)
	{
		printf("Error getting Device IDs\n");
		printf("Error %i", ret);
		printf("\nAborting...");
		printf(".......................\n");
		exit(ret);
	}
	device_ids = std::vector<cl_device_id>(ret_num_devices);
	ret = clGetDeviceIDs(platform_ids[0], CL_DEVICE_TYPE_ALL, ret_num_devices, device_ids.data(), nullptr);
	if(ret != 0)
	{
		printf("Error getting Device IDs\n");
		printf("Error %i", ret);
		printf("\nAborting...");
		printf(".......................\n");
		exit(ret);
	}
	printDeviceNames();

	/* Create OpenCL context */
	
	printf("Creating OpenCL context\n");
	const cl_context_properties context_properties[] = 
	{
		CL_CONTEXT_PLATFORM,
		reinterpret_cast<cl_context_properties>(platform_ids[0]),
		0, 0
	};
	context = clCreateContext(context_properties, ret_num_devices, device_ids.data(), nullptr, nullptr, &ret);
	
	if(ret != 0)
	{
		printf("Error creating context\n");
		printf("Error %i", ret);
		printf("\nAborting...");
		printf(".......................\n");
		exit(ret);
	}

	/* Create command queue */
	printf("Creating Command queue\n");
	command_queue = clCreateCommandQueue(context, device_ids[0], CL_QUEUE_PROFILING_ENABLE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &ret);
	if(ret != 0)
	{
		printf("Error creating command queue\n");
		printf("Error %i", ret);
		printf("\nAborting...");
		printf(".......................\n");
		exit(ret);
	}

	/* Create and build shader program */
	printf("Creating and building shader program Shaders/hello.cl\n");
	loadKernel("Shaders/hello.cl");
	ret = clBuildProgram(program, ret_num_devices, device_ids.data(), nullptr, nullptr, nullptr);
	if(ret != 0)
	{
		char* build_log = nullptr;
		size_t ret_val_size;
		clGetProgramBuildInfo(program, device_ids[0], CL_PROGRAM_BUILD_LOG, 0, nullptr, &ret_val_size);
		build_log = new char[ret_val_size+1];
		clGetProgramBuildInfo(program, device_ids[0], CL_PROGRAM_BUILD_LOG, ret_val_size, build_log, nullptr);

		
		printf("Error building shader program\n");
		printf("Error %i", ret);
		printf("\nAborting...");
		printf(".......................\n");
		printf("build log\n\n");
		printf(build_log);
		printf(".......................\n");
		delete(build_log);
		exit(ret);
	}

	/* create kernel */
	printf("Creating kernel\n");
	kernel = clCreateKernel(program, "hello", &ret);
	if(ret != 0)
	{
		printf("Error creating kernel hello\n");
		printf("Error %i", ret);
		printf("\nAborting...");
		printf(".......................\n");
		exit(ret);
	}

	/* Create and build shader program */
	printf("Creating and building shader program Shaders/raycreate.cl\n");
	loadKernel("Shaders/raycreate.cl");
	ret = clBuildProgram(program, ret_num_devices, device_ids.data(), nullptr, nullptr, nullptr);
	if (ret != 0)
	{
		char* build_log = nullptr;
		size_t ret_val_size;
		clGetProgramBuildInfo(program, device_ids[0], CL_PROGRAM_BUILD_LOG, 0, nullptr, &ret_val_size);
		build_log = new char[ret_val_size + 1];
		clGetProgramBuildInfo(program, device_ids[0], CL_PROGRAM_BUILD_LOG, ret_val_size, build_log, nullptr);
	
	
		printf("Error building shader program\n");
		printf("Error %i", ret);
		printf("\nAborting...");
		printf(".......................\n");
		printf("build log\n\n");
		printf(build_log);
		printf(".......................\n");
		delete(build_log);
		exit(ret);
	}
	
	/* create kernel */
	printf("Creating kernel\n");
	raykernel = clCreateKernel(program, "createrays", &ret);
	if (ret != 0)
	{
		printf("Error creating kernel raykernel\n");
		printf("Error %i", ret);
		printf("\nAborting...");
		printf(".......................\n");
		exit(ret);
	}

	/* this is our image that we are drawing to. */
	image = (float*)calloc(3 * testDataSize, sizeof(float));
	
	cl_int error;

	imageBuffer = clCreateBuffer (context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
		sizeof (float) * (testDataSize * 3),
		image, &error);

	if(error != 0)
	{
		printf("Error Creating imageBuffer\n");
		printf("Error %i", error);
		printf("\nAborting...");
		printf(".......................\n");
		exit(error);
	}

	clSetKernelArg(kernel, 0, sizeof(cl_mem), &imageBuffer);//send in image buffer to kernel

	cameraData = (float*)malloc(sizeof(float) * 12);
	memcpy(&cameraData[0], &camera->getCameraPosition(), sizeof(float) * 4);
	memcpy(&cameraData[4], &camera->getRightDirection(), sizeof(float) * 4);
	memcpy(&cameraData[8], &camera->getViewDirection(), sizeof(float) * 4);

	cameraBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		sizeof(float) * 12, &cameraData[0], &error); 
	
	if (error != 0)
	{
		printf("Error Creating cameraBuffer\n");
		printf("Error %i", error);
		printf("\nAborting...");
		printf(".......................\n");
		exit(error);
	}
	clSetKernelArg(kernel, 1, sizeof(cl_mem), &cameraBuffer);
	
	triangles = (float*)malloc(sizeof(float)* (8 * 3 * tri + 1)); 

	triangles[0] = tri;
	for (int xxx = 0; xxx <= 12 * 24; xxx++)  // first model. the object.
	{
		triangles[xxx + 1] = invcube[xxx];
	}
	for (int yyy = 12 * 24; yyy <= tri * 24; yyy++) // second model. the "room"
	{
		triangles[yyy + 1] = modeldata[yyy - 12 * 24];
	}

	triBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		sizeof(float)* (8 * 3 * tri + 1),
		triangles, &error);

	if (error != 0)
	{
		printf("Error Creating triBuffer\n");
		printf("Error %i", error);
		printf("\nAborting...");
		printf(".......................\n");
		exit(error);
	}
	clSetKernelArg(kernel, 3, sizeof(cl_mem), &triBuffer);

	lightPos = (float*)malloc(sizeof(float) * (3 * numLights + 1));
	lightSpeed = (float*)malloc(sizeof(float) * numLights);

	lightPos[0] = numLights;

	srand(0);

	for (int i = 0; i < numLights; i++)
	{
		lightPos[i * 3 + 1] = rand() % 200 - 100; //x
		lightPos[i * 3 + 2] = rand() % 200 - 100; //y
		lightPos[i * 3 + 3] = rand() % 200 - 100; //z

		lightSpeed[i] = rand() % 20 - 10;
	}
	lightBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		sizeof(float) * (3 * numLights + 1),
		lightPos, &error);
	
	if (error != 0)
	{
		printf("Error Creating lightBuffer\n");
		printf("Error %i", error);
		printf("\nAborting...");
		printf(".......................\n");
		exit(error);
	}

	clSetKernelArg(kernel, 4, sizeof(cl_mem), &lightBuffer);


	redRidingHoodImageData = (float*)malloc(sizeof(float) * 3 * 22 * 18);
	{
		//1
		redRidingHoodImageData[0]  = 0.5f; redRidingHoodImageData[1]  = 0.3f; redRidingHoodImageData[2]  = 0.7f;
		redRidingHoodImageData[3]  = 1; redRidingHoodImageData[4]  = 1; redRidingHoodImageData[5]  = 1;
		redRidingHoodImageData[6]  = 1; redRidingHoodImageData[7]  = 1; redRidingHoodImageData[8]  = 1;
		redRidingHoodImageData[9]  = 1; redRidingHoodImageData[10] = 1; redRidingHoodImageData[11] = 1;
		redRidingHoodImageData[12] = 1; redRidingHoodImageData[13] = 1; redRidingHoodImageData[14] = 1;
		redRidingHoodImageData[15] = 1; redRidingHoodImageData[16] = 1; redRidingHoodImageData[17] = 1;
		redRidingHoodImageData[18] = 1; redRidingHoodImageData[19] = 1; redRidingHoodImageData[20] = 1;
		redRidingHoodImageData[21] = 1; redRidingHoodImageData[22] = 1; redRidingHoodImageData[23] = 1;
		redRidingHoodImageData[24] = 1; redRidingHoodImageData[25] = 1; redRidingHoodImageData[26] = 1;
		redRidingHoodImageData[27] = 1; redRidingHoodImageData[28] = 1; redRidingHoodImageData[29] = 1;
		redRidingHoodImageData[30] = 1; redRidingHoodImageData[31] = 1; redRidingHoodImageData[32] = 1; 
		redRidingHoodImageData[33] = 1; redRidingHoodImageData[34] = 1; redRidingHoodImageData[35] = 1; 
		redRidingHoodImageData[36] = 1; redRidingHoodImageData[37] = 1; redRidingHoodImageData[38] = 1; 
		redRidingHoodImageData[39] = 1; redRidingHoodImageData[40] = 1; redRidingHoodImageData[41] = 1; 
		redRidingHoodImageData[42] = 1; redRidingHoodImageData[43] = 1; redRidingHoodImageData[44] = 1; 
		redRidingHoodImageData[45] = 1; redRidingHoodImageData[46] = 1; redRidingHoodImageData[47] = 1; 
		redRidingHoodImageData[48] = 1; redRidingHoodImageData[49] = 1; redRidingHoodImageData[50] = 1; 
		redRidingHoodImageData[51] = .63f; redRidingHoodImageData[52] = .07f; redRidingHoodImageData[53] = .06f; 
		redRidingHoodImageData[54] = 1; redRidingHoodImageData[55] = 1; redRidingHoodImageData[56] = 1; 
		redRidingHoodImageData[57] = 1; redRidingHoodImageData[58] = 1; redRidingHoodImageData[59] = 1; 
		redRidingHoodImageData[60] = 1; redRidingHoodImageData[61] = 1; redRidingHoodImageData[62] = 1; 
		redRidingHoodImageData[63] = 1; redRidingHoodImageData[64] = 1; redRidingHoodImageData[65] = 1; 
		//2
		redRidingHoodImageData[66]  = 1; redRidingHoodImageData[67]  = 1; redRidingHoodImageData[68]  = 1; 
		redRidingHoodImageData[69]  = 1; redRidingHoodImageData[70]  = 1; redRidingHoodImageData[71]  = 1; 
		redRidingHoodImageData[72]  = 1; redRidingHoodImageData[73]  = 1; redRidingHoodImageData[74]  = 1; 
		redRidingHoodImageData[75]  = 1; redRidingHoodImageData[76]  = 1; redRidingHoodImageData[77]  = 1; 
		redRidingHoodImageData[78]  = 1; redRidingHoodImageData[79]  = .43f; redRidingHoodImageData[80]  = .06f; 
		redRidingHoodImageData[81]  = 1; redRidingHoodImageData[82]  = .43f; redRidingHoodImageData[83]  = .06f; 
		redRidingHoodImageData[84]  = 1; redRidingHoodImageData[85]  = .43f; redRidingHoodImageData[86]  = .06f; 
		redRidingHoodImageData[87]  = 1; redRidingHoodImageData[88]  = .43f; redRidingHoodImageData[89]  = .06f; 
		redRidingHoodImageData[90]  = 1; redRidingHoodImageData[91]  = 1; redRidingHoodImageData[92]  = 1; 
		redRidingHoodImageData[93]  = 1; redRidingHoodImageData[94]  = 1; redRidingHoodImageData[95]  = 1; 
		redRidingHoodImageData[96]  = 1; redRidingHoodImageData[97]  = 1; redRidingHoodImageData[98]  = 1; 
		redRidingHoodImageData[99]  = 1; redRidingHoodImageData[100] = 1; redRidingHoodImageData[101] = 1; 
		redRidingHoodImageData[102] = 1; redRidingHoodImageData[103] = 1; redRidingHoodImageData[104] = 1; 
		redRidingHoodImageData[105] = 1; redRidingHoodImageData[106] = 1; redRidingHoodImageData[107] = 1; 
		redRidingHoodImageData[108] = 1; redRidingHoodImageData[109] = 1; redRidingHoodImageData[110] = 1; 
		redRidingHoodImageData[111] = 1; redRidingHoodImageData[112] = 1; redRidingHoodImageData[113] = 1; 
		redRidingHoodImageData[114] = .63f; redRidingHoodImageData[115] = .07f; redRidingHoodImageData[116] = .06f; 
		redRidingHoodImageData[117] = 1; redRidingHoodImageData[118] = 0; redRidingHoodImageData[119] = 0; 
		redRidingHoodImageData[120] = .63f; redRidingHoodImageData[121] = .07f; redRidingHoodImageData[122] = .06f; 
		redRidingHoodImageData[123] = 1; redRidingHoodImageData[124] = 1; redRidingHoodImageData[125] = 1; 
		redRidingHoodImageData[126] = 1; redRidingHoodImageData[127] = 1; redRidingHoodImageData[128] = 1; 
		redRidingHoodImageData[129] = 1; redRidingHoodImageData[130] = 1; redRidingHoodImageData[131] = 1; 
		//3
		redRidingHoodImageData[132] = 1; redRidingHoodImageData[133] = 1; redRidingHoodImageData[134] = 1; 
		redRidingHoodImageData[135] = 1; redRidingHoodImageData[136] = 1; redRidingHoodImageData[137] = 1; 
		redRidingHoodImageData[138] = 1; redRidingHoodImageData[139] = 1; redRidingHoodImageData[140] = 1; 
		redRidingHoodImageData[141] = 1; redRidingHoodImageData[142] = .43f; redRidingHoodImageData[143] = .06f; 
		redRidingHoodImageData[144] = 1; redRidingHoodImageData[145] = .65f; redRidingHoodImageData[146] = .14f; 
		redRidingHoodImageData[147] = 1; redRidingHoodImageData[148] = .65f; redRidingHoodImageData[149] = .14f; 
		redRidingHoodImageData[150] = 1; redRidingHoodImageData[151] = .65f; redRidingHoodImageData[152] = .14f; 
		redRidingHoodImageData[153] = 1; redRidingHoodImageData[154] = .65f; redRidingHoodImageData[155] = .14f; 
		redRidingHoodImageData[156] = 1; redRidingHoodImageData[157] = .43f; redRidingHoodImageData[158] = .06f; 
		redRidingHoodImageData[159] = 1; redRidingHoodImageData[160] = 1; redRidingHoodImageData[161] = 1; 
		redRidingHoodImageData[162] = 1; redRidingHoodImageData[163] = 1; redRidingHoodImageData[164] = 1; 
		redRidingHoodImageData[165] = 1; redRidingHoodImageData[166] = 1; redRidingHoodImageData[167] = 1; 
		redRidingHoodImageData[168] = 1; redRidingHoodImageData[169] = 1; redRidingHoodImageData[170] = 1; 
		redRidingHoodImageData[171] = 1; redRidingHoodImageData[172] = 1; redRidingHoodImageData[173] = 1; 
		redRidingHoodImageData[174] = 1; redRidingHoodImageData[175] = 1; redRidingHoodImageData[176] = 1; 
		redRidingHoodImageData[177] = 1; redRidingHoodImageData[178] = 1; redRidingHoodImageData[179] = 1; 
		redRidingHoodImageData[180] = .63f; redRidingHoodImageData[181] = .07f; redRidingHoodImageData[182] = .06f; 
		redRidingHoodImageData[183] = 1; redRidingHoodImageData[184] = 0; redRidingHoodImageData[185] = 0; 
		redRidingHoodImageData[186] = 1; redRidingHoodImageData[187] = 0; redRidingHoodImageData[188] = 0; 
		redRidingHoodImageData[189] = .63f; redRidingHoodImageData[190] = .07f; redRidingHoodImageData[191] = .06f; 
		redRidingHoodImageData[192] = 1; redRidingHoodImageData[193] = 1; redRidingHoodImageData[194] = 1; 
		redRidingHoodImageData[195] = 1; redRidingHoodImageData[196] = 1; redRidingHoodImageData[197] = 1; 
		//4
		redRidingHoodImageData[198] = 1; redRidingHoodImageData[199] = 1; redRidingHoodImageData[200] = 1; 
		redRidingHoodImageData[201] = 1; redRidingHoodImageData[202] = 1; redRidingHoodImageData[203] = 1; 
		redRidingHoodImageData[204] = 1; redRidingHoodImageData[205] = .43f; redRidingHoodImageData[206] = .06f; 
		redRidingHoodImageData[207] = 1; redRidingHoodImageData[208] = .65f; redRidingHoodImageData[209] = .14f; 
		redRidingHoodImageData[210] = 1; redRidingHoodImageData[211] = .65f; redRidingHoodImageData[212] = .14f; 
		redRidingHoodImageData[213] = 1; redRidingHoodImageData[214] = .65f; redRidingHoodImageData[215] = .14f; 
		redRidingHoodImageData[216] = 1; redRidingHoodImageData[217] = .65f; redRidingHoodImageData[218] = .14f; 
		redRidingHoodImageData[219] = 1; redRidingHoodImageData[220] = .65f; redRidingHoodImageData[221] = .14f; 
		redRidingHoodImageData[222] = 1; redRidingHoodImageData[223] = .65f; redRidingHoodImageData[224] = .14f; 
		redRidingHoodImageData[225] = 1; redRidingHoodImageData[226] = .43f; redRidingHoodImageData[227] = .06f; 
		redRidingHoodImageData[228] = 1; redRidingHoodImageData[229] = 1; redRidingHoodImageData[230] = 1; 
		redRidingHoodImageData[231] = 1; redRidingHoodImageData[232] = 1; redRidingHoodImageData[233] = 1; 
		redRidingHoodImageData[234] = 1; redRidingHoodImageData[235] = 1; redRidingHoodImageData[236] = 1; 
		redRidingHoodImageData[237] = 1; redRidingHoodImageData[238] = 1; redRidingHoodImageData[239] = 1; 
		redRidingHoodImageData[240] = 1; redRidingHoodImageData[241] = 1; redRidingHoodImageData[242] = 1; 
		redRidingHoodImageData[243] = 1; redRidingHoodImageData[244] = 1; redRidingHoodImageData[245] = 1; 
		redRidingHoodImageData[246] = .63f; redRidingHoodImageData[247] = .07f; redRidingHoodImageData[248] = .06f; 
		redRidingHoodImageData[249] = 1; redRidingHoodImageData[250] = 0; redRidingHoodImageData[251] = 0; 
		redRidingHoodImageData[252] = 1; redRidingHoodImageData[253] = 0; redRidingHoodImageData[254] = 0; 
		redRidingHoodImageData[255] = .63f; redRidingHoodImageData[256] = .07f; redRidingHoodImageData[257] = .06f; 
		redRidingHoodImageData[258] = 1; redRidingHoodImageData[259] = 1; redRidingHoodImageData[260] = 1; 
		redRidingHoodImageData[261] = 1; redRidingHoodImageData[262] = 1; redRidingHoodImageData[263] = 1; 
		//5
		redRidingHoodImageData[264] = 1; redRidingHoodImageData[265] = 1; redRidingHoodImageData[266] = 1; 
		redRidingHoodImageData[267] = 1; redRidingHoodImageData[268] = 1; redRidingHoodImageData[269] = 1; 
		redRidingHoodImageData[270] = 1; redRidingHoodImageData[271] = .43f; redRidingHoodImageData[272] = .06f; 
		redRidingHoodImageData[273] = 1; redRidingHoodImageData[274] = .65f; redRidingHoodImageData[275] = .14f; 
		redRidingHoodImageData[276] = 1; redRidingHoodImageData[277] = .65f; redRidingHoodImageData[278] = .14f; 
		redRidingHoodImageData[279] = 1; redRidingHoodImageData[280] = .65f; redRidingHoodImageData[281] = .14f; 
		redRidingHoodImageData[282] = 1; redRidingHoodImageData[283] = .65f; redRidingHoodImageData[284] = .14f; 
		redRidingHoodImageData[285] = 1; redRidingHoodImageData[286] = .65f; redRidingHoodImageData[287] = .14f; 
		redRidingHoodImageData[288] = 1; redRidingHoodImageData[289] = .65f; redRidingHoodImageData[290] = .14f; 
		redRidingHoodImageData[291] = 1; redRidingHoodImageData[292] = .43f; redRidingHoodImageData[293] = .06f; 
		redRidingHoodImageData[294] = 1; redRidingHoodImageData[295] = 1; redRidingHoodImageData[296] = 1; 
		redRidingHoodImageData[297] = 1; redRidingHoodImageData[298] = 1; redRidingHoodImageData[299] = 1; 
		redRidingHoodImageData[300] = 1; redRidingHoodImageData[301] = 1; redRidingHoodImageData[302] = 1; 
		redRidingHoodImageData[303] = 1; redRidingHoodImageData[304] = 1; redRidingHoodImageData[305] = 1; 
		redRidingHoodImageData[306] = 1; redRidingHoodImageData[307] = 1; redRidingHoodImageData[308] = 1; 
		redRidingHoodImageData[309] = .63f; redRidingHoodImageData[310] = .07f; redRidingHoodImageData[311] = .06f; 
		redRidingHoodImageData[312] = 1; redRidingHoodImageData[313] = 0; redRidingHoodImageData[314] = 0; 
		redRidingHoodImageData[315] = 1; redRidingHoodImageData[316] = 0; redRidingHoodImageData[317] = 0; 
		redRidingHoodImageData[318] = 1; redRidingHoodImageData[319] = 0; redRidingHoodImageData[320] = 0; 
		redRidingHoodImageData[321] = 1; redRidingHoodImageData[322] = 0; redRidingHoodImageData[323] = 0; 
		redRidingHoodImageData[324] = .63f; redRidingHoodImageData[325] = .07f; redRidingHoodImageData[326] = .06f; 
		redRidingHoodImageData[327] = 1; redRidingHoodImageData[328] = 1; redRidingHoodImageData[329] = 1; 
		//6
		redRidingHoodImageData[330] = 1; redRidingHoodImageData[331] = 1; redRidingHoodImageData[332] = 1; 
		redRidingHoodImageData[333] = 1; redRidingHoodImageData[334] = .43f; redRidingHoodImageData[335] = .06f; 
		redRidingHoodImageData[336] = 1; redRidingHoodImageData[337] = .65f; redRidingHoodImageData[338] = .14f; 
		redRidingHoodImageData[339] = 1; redRidingHoodImageData[340] = .65f; redRidingHoodImageData[341] = .14f; 
		redRidingHoodImageData[342] = 1; redRidingHoodImageData[343] = .65f; redRidingHoodImageData[344] = .14f; 
		redRidingHoodImageData[345] = 1; redRidingHoodImageData[346] = 1; redRidingHoodImageData[347] = 1; 
		redRidingHoodImageData[348] = 0; redRidingHoodImageData[349] = 0; redRidingHoodImageData[350] = 0; 
		redRidingHoodImageData[351] = 1; redRidingHoodImageData[352] = .65f; redRidingHoodImageData[353] = .14f; 
		redRidingHoodImageData[354] = 1; redRidingHoodImageData[355] = .65f; redRidingHoodImageData[356] = .14f; 
		redRidingHoodImageData[357] = 1; redRidingHoodImageData[358] = .65f; redRidingHoodImageData[359] = .14f; 
		redRidingHoodImageData[360] = 1; redRidingHoodImageData[361] = .43f; redRidingHoodImageData[362] = .06f; 
		redRidingHoodImageData[363] = 1; redRidingHoodImageData[364] = 1; redRidingHoodImageData[365] = 1; 
		redRidingHoodImageData[366] = 1; redRidingHoodImageData[367] = 1; redRidingHoodImageData[368] = 1; 
		redRidingHoodImageData[369] = 1; redRidingHoodImageData[370] = 1; redRidingHoodImageData[371] = 1; 
		redRidingHoodImageData[372] = 1; redRidingHoodImageData[373] = 1; redRidingHoodImageData[374] = 1; 
		redRidingHoodImageData[375] = .63f; redRidingHoodImageData[376] = .07f; redRidingHoodImageData[377] = .06f; 
		redRidingHoodImageData[378] = 1; redRidingHoodImageData[379] = 0; redRidingHoodImageData[380] = 0; 
		redRidingHoodImageData[381] = 1; redRidingHoodImageData[382] = 0; redRidingHoodImageData[383] = 0; 
		redRidingHoodImageData[384] = 1; redRidingHoodImageData[385] = .93f; redRidingHoodImageData[386] = .7f; 
		redRidingHoodImageData[387] = 1; redRidingHoodImageData[388] = 0; redRidingHoodImageData[389] = 0;
		redRidingHoodImageData[390] = .63f; redRidingHoodImageData[391] = .07f; redRidingHoodImageData[392] = .06f; 
		redRidingHoodImageData[393] = 1; redRidingHoodImageData[394] = 1; redRidingHoodImageData[395] = 1; 
		//7
		redRidingHoodImageData[396] = 1; redRidingHoodImageData[397] = .43f; redRidingHoodImageData[398] = .06f; 
		redRidingHoodImageData[399] = 1; redRidingHoodImageData[400] = .65f; redRidingHoodImageData[401] = .14f; 
		redRidingHoodImageData[402] = 1; redRidingHoodImageData[403] = .65f; redRidingHoodImageData[404] = .14f; 
		redRidingHoodImageData[405] = 1; redRidingHoodImageData[406] = .65f; redRidingHoodImageData[407] = .14f; 
		redRidingHoodImageData[408] = 1; redRidingHoodImageData[409] = .65f; redRidingHoodImageData[410] = .14f; 
		redRidingHoodImageData[411] = 0; redRidingHoodImageData[412] = 0; redRidingHoodImageData[413] = 0; 
		redRidingHoodImageData[414] = 0; redRidingHoodImageData[415] = 0; redRidingHoodImageData[416] = 0; 
		redRidingHoodImageData[417] = 1; redRidingHoodImageData[418] = .65f; redRidingHoodImageData[419] = .14f; 
		redRidingHoodImageData[420] = 1; redRidingHoodImageData[421] = .65f; redRidingHoodImageData[422] = .14f; 
		redRidingHoodImageData[423] = 1; redRidingHoodImageData[424] = .65f; redRidingHoodImageData[425] = .14f; 
		redRidingHoodImageData[426] = 1; redRidingHoodImageData[427] = .65f; redRidingHoodImageData[428] = .14f; 
		redRidingHoodImageData[429] = 1; redRidingHoodImageData[430] = .43f; redRidingHoodImageData[431] = .06f; 
		redRidingHoodImageData[432] = 1; redRidingHoodImageData[433] = 1; redRidingHoodImageData[434] = 1; 
		redRidingHoodImageData[435] = 1; redRidingHoodImageData[436] = 1; redRidingHoodImageData[437] = 1; 
		redRidingHoodImageData[438] = 1; redRidingHoodImageData[439] = 1; redRidingHoodImageData[440] = 1; 
		redRidingHoodImageData[441] = .63f; redRidingHoodImageData[442] = .07f; redRidingHoodImageData[443] = 0.06f; 
		redRidingHoodImageData[444] = 1; redRidingHoodImageData[445] = 0; redRidingHoodImageData[446] = 0; 
		redRidingHoodImageData[447] = 1; redRidingHoodImageData[448] = .93f; redRidingHoodImageData[449] = .7f;
		redRidingHoodImageData[450] = 1; redRidingHoodImageData[451] = .93f; redRidingHoodImageData[452] = .7f; 
		redRidingHoodImageData[453] = 1; redRidingHoodImageData[454] = 0; redRidingHoodImageData[455] = 0; 
		redRidingHoodImageData[456] = .63f; redRidingHoodImageData[457] = .07f; redRidingHoodImageData[458] = .06f; 
		redRidingHoodImageData[459] = 1; redRidingHoodImageData[460] = 1; redRidingHoodImageData[461] = 1; 
		//8
		redRidingHoodImageData[462] = 1; redRidingHoodImageData[463] = .43f; redRidingHoodImageData[464] = .06f; 
		redRidingHoodImageData[465] = 1; redRidingHoodImageData[466] = .65f; redRidingHoodImageData[467] = .14f; 
		redRidingHoodImageData[468] = 1; redRidingHoodImageData[469] = .65f; redRidingHoodImageData[470] = .14f; 
		redRidingHoodImageData[471] = 1; redRidingHoodImageData[472] = .65f; redRidingHoodImageData[473] = .14f; 
		redRidingHoodImageData[474] = 1; redRidingHoodImageData[475] = .65f; redRidingHoodImageData[476] = .14f; 
		redRidingHoodImageData[477] = 0; redRidingHoodImageData[478] = 0; redRidingHoodImageData[479] =	0; 
		redRidingHoodImageData[480] = 0; redRidingHoodImageData[481] = 0; redRidingHoodImageData[482] =	0; 
		redRidingHoodImageData[483] = 1; redRidingHoodImageData[484] = .65f; redRidingHoodImageData[485] = .14f; 
		redRidingHoodImageData[486] = 1; redRidingHoodImageData[487] = .65f; redRidingHoodImageData[488] = .14f; 
		redRidingHoodImageData[489] = 1; redRidingHoodImageData[490] = .65f; redRidingHoodImageData[491] = .14f; 
		redRidingHoodImageData[492] = 1; redRidingHoodImageData[493] = .65f; redRidingHoodImageData[494] = .14f; 
		redRidingHoodImageData[495] = 1; redRidingHoodImageData[496] = .43f; redRidingHoodImageData[497] = .06f; 
		redRidingHoodImageData[498] = 1; redRidingHoodImageData[499] = 1; redRidingHoodImageData[500] = 1; 
		redRidingHoodImageData[501] = 1; redRidingHoodImageData[502] = 1; redRidingHoodImageData[503] = 1; 
		redRidingHoodImageData[504] = 1; redRidingHoodImageData[505] = 1; redRidingHoodImageData[506] = 1; 
		redRidingHoodImageData[507] = 1; redRidingHoodImageData[508] = 1; redRidingHoodImageData[509] = 1; 
		redRidingHoodImageData[510] = .63f; redRidingHoodImageData[511] = .07f; redRidingHoodImageData[512] = .06f; 
		redRidingHoodImageData[513] = 1; redRidingHoodImageData[514] = .93f; redRidingHoodImageData[515] = .7f; 
		redRidingHoodImageData[516] = 1; redRidingHoodImageData[517] = .43f; redRidingHoodImageData[518] = .06f;
		redRidingHoodImageData[519] = .63f; redRidingHoodImageData[520] = .07f; redRidingHoodImageData[521] = .06f; 
		redRidingHoodImageData[522] = 1; redRidingHoodImageData[523] = 1; redRidingHoodImageData[524] = 1; 
		redRidingHoodImageData[525] = 1; redRidingHoodImageData[526] = 1; redRidingHoodImageData[527] = 1; 
		//9
		redRidingHoodImageData[528] = 1; redRidingHoodImageData[529] = 1; redRidingHoodImageData[530] = 1; 
		redRidingHoodImageData[531] = 1; redRidingHoodImageData[532] = .43f; redRidingHoodImageData[533] = .06f; 
		redRidingHoodImageData[534] = 1; redRidingHoodImageData[535] = .65f; redRidingHoodImageData[536] = .14f;
		redRidingHoodImageData[537] = 1; redRidingHoodImageData[538] = .65f; redRidingHoodImageData[539] = .14f; 
		redRidingHoodImageData[540] = 1; redRidingHoodImageData[541] = .65f; redRidingHoodImageData[542] = .14f; 
		redRidingHoodImageData[543] = 1; redRidingHoodImageData[544] = .65f; redRidingHoodImageData[545] = .14f; 
		redRidingHoodImageData[546] = 1; redRidingHoodImageData[547] = .65f; redRidingHoodImageData[548] = .14f; 
		redRidingHoodImageData[549] = 1; redRidingHoodImageData[550] = .65f; redRidingHoodImageData[551] = .14f;
		redRidingHoodImageData[552] = 1; redRidingHoodImageData[553] = .65f; redRidingHoodImageData[554] = .14f; 
		redRidingHoodImageData[555] = 1; redRidingHoodImageData[556] = .65f; redRidingHoodImageData[557] = .14f; 
		redRidingHoodImageData[558] = 1; redRidingHoodImageData[559] = .65f; redRidingHoodImageData[560] = .14f; 
		redRidingHoodImageData[561] = 1; redRidingHoodImageData[562] = .65f; redRidingHoodImageData[563] = .14f; 
		redRidingHoodImageData[564] = 1; redRidingHoodImageData[565] = .43f; redRidingHoodImageData[566] = .06f; 
		redRidingHoodImageData[567] = 1; redRidingHoodImageData[568] = 1; redRidingHoodImageData[569] = 1; 
		redRidingHoodImageData[570] = 1; redRidingHoodImageData[571] = 1; redRidingHoodImageData[572] = 1; 
		redRidingHoodImageData[573] = 1; redRidingHoodImageData[574] = 1; redRidingHoodImageData[575] = 1; 
		redRidingHoodImageData[576] = 1; redRidingHoodImageData[577] = .43f; redRidingHoodImageData[578] = .06f; 
		redRidingHoodImageData[579] = 1; redRidingHoodImageData[580] = .65f; redRidingHoodImageData[581] = .14f; 
		redRidingHoodImageData[582] = 1; redRidingHoodImageData[583] = .43f; redRidingHoodImageData[584] = .06f; 
		redRidingHoodImageData[585] = 1; redRidingHoodImageData[586] = 1; redRidingHoodImageData[587] = 1; 
		redRidingHoodImageData[588] = 1; redRidingHoodImageData[589] = 1; redRidingHoodImageData[590] = 1; 
		redRidingHoodImageData[591] = 1; redRidingHoodImageData[592] = 1; redRidingHoodImageData[593] = 1;
		//10
		redRidingHoodImageData[594] = 1; redRidingHoodImageData[595] = 1; redRidingHoodImageData[596] = 1; 
		redRidingHoodImageData[597] = 1; redRidingHoodImageData[598] = 1; redRidingHoodImageData[599] = 1; 
		redRidingHoodImageData[600] = 1; redRidingHoodImageData[601] = .43f; redRidingHoodImageData[602] = .06f; 
		redRidingHoodImageData[603] = 1; redRidingHoodImageData[604] = .43f; redRidingHoodImageData[605] = .06f; 
		redRidingHoodImageData[606] = 1; redRidingHoodImageData[607] = .65f; redRidingHoodImageData[608] = .14f; 
		redRidingHoodImageData[609] = 1; redRidingHoodImageData[610] = .65f; redRidingHoodImageData[611] = .14f; 
		redRidingHoodImageData[612] = 1; redRidingHoodImageData[613] = .65f; redRidingHoodImageData[614] = .14f; 
		redRidingHoodImageData[615] = 1; redRidingHoodImageData[616] = .65f; redRidingHoodImageData[617] = .14f; 
		redRidingHoodImageData[618] = 1; redRidingHoodImageData[619] = .65f; redRidingHoodImageData[620] = .14f; 
		redRidingHoodImageData[621] = 1; redRidingHoodImageData[622] = .65f; redRidingHoodImageData[623] = .14f; 
		redRidingHoodImageData[624] = 1; redRidingHoodImageData[625] = .65f; redRidingHoodImageData[626] = .14f; 
		redRidingHoodImageData[627] = 1; redRidingHoodImageData[628] = .65f; redRidingHoodImageData[629] = .14f;
		redRidingHoodImageData[630] = 1; redRidingHoodImageData[631] = .65f; redRidingHoodImageData[632] = .14f; 
		redRidingHoodImageData[633] = 1; redRidingHoodImageData[634] = .43f; redRidingHoodImageData[635] = .06f; 
		redRidingHoodImageData[636] = 1; redRidingHoodImageData[637] = 1; redRidingHoodImageData[638] = 1; 
		redRidingHoodImageData[639] = 1; redRidingHoodImageData[640] = .43f; redRidingHoodImageData[641] = .06f; 
		redRidingHoodImageData[642] = 1; redRidingHoodImageData[643] = .65f; redRidingHoodImageData[644] = .14f; 
		redRidingHoodImageData[645] = 1; redRidingHoodImageData[646] = .65f; redRidingHoodImageData[647] = .14f; 
		redRidingHoodImageData[648] = 1; redRidingHoodImageData[649] = .43f; redRidingHoodImageData[650] = .06f; 
		redRidingHoodImageData[651] = 1; redRidingHoodImageData[652] = 1; redRidingHoodImageData[653] = 1; 
		redRidingHoodImageData[654] = 1; redRidingHoodImageData[655] = 1; redRidingHoodImageData[656] = 1; 
		redRidingHoodImageData[657] = 1; redRidingHoodImageData[658] = 1; redRidingHoodImageData[659] = 1; 
		//11		
		redRidingHoodImageData[660] = 1; redRidingHoodImageData[661] = 1; redRidingHoodImageData[662] = 1; 
		redRidingHoodImageData[663] = 1; redRidingHoodImageData[664] = 1; redRidingHoodImageData[665] = 1; 
		redRidingHoodImageData[666] = 1; redRidingHoodImageData[667] = 1; redRidingHoodImageData[668] = 1; 
		redRidingHoodImageData[669] = 1; redRidingHoodImageData[670] = 1; redRidingHoodImageData[671] = 1; 
		redRidingHoodImageData[672] = 1; redRidingHoodImageData[673] = .43f; redRidingHoodImageData[674] = .06f; 
		redRidingHoodImageData[675] = 1; redRidingHoodImageData[676] = .43f; redRidingHoodImageData[677] = .06f; 
		redRidingHoodImageData[678] = 1; redRidingHoodImageData[679] = .43f; redRidingHoodImageData[680] = .06f; 
		redRidingHoodImageData[681] = 1; redRidingHoodImageData[682] = .65f; redRidingHoodImageData[683] = .14f; 
		redRidingHoodImageData[684] = 1; redRidingHoodImageData[685] = .65f; redRidingHoodImageData[686] = .14f; 
		redRidingHoodImageData[687] = 1; redRidingHoodImageData[688] = .43f; redRidingHoodImageData[689] = .06f; 
		redRidingHoodImageData[690] = 1; redRidingHoodImageData[691] = .65f; redRidingHoodImageData[692] = .14f; 
		redRidingHoodImageData[693] = 1; redRidingHoodImageData[694] = .65f; redRidingHoodImageData[695] = .14f; 
		redRidingHoodImageData[696] = 1; redRidingHoodImageData[697] = .65f; redRidingHoodImageData[698] = .14f;
		redRidingHoodImageData[699] = 1; redRidingHoodImageData[700] = .43f; redRidingHoodImageData[701] = .06f; 
		redRidingHoodImageData[702] = 1; redRidingHoodImageData[703] = .43f; redRidingHoodImageData[704] = .06f; 
		redRidingHoodImageData[705] = 1; redRidingHoodImageData[706] = .65f; redRidingHoodImageData[707] = .14f; 
		redRidingHoodImageData[708] = 1; redRidingHoodImageData[709] = .65f; redRidingHoodImageData[710] = .14f; 
		redRidingHoodImageData[711] = 1; redRidingHoodImageData[712] = .43f; redRidingHoodImageData[713] = .06f; 
		redRidingHoodImageData[714] = 1; redRidingHoodImageData[715] = 1; redRidingHoodImageData[716] = 1; 
		redRidingHoodImageData[717] = 1; redRidingHoodImageData[718] = 1; redRidingHoodImageData[719] = 1; 
		redRidingHoodImageData[720] = 1; redRidingHoodImageData[721] = 1; redRidingHoodImageData[722] = 1; 
		redRidingHoodImageData[723] = 1; redRidingHoodImageData[724] = 1; redRidingHoodImageData[725] = 1; 
		//12	
		redRidingHoodImageData[726] = 1; redRidingHoodImageData[727] = 1; redRidingHoodImageData[728] = 1; 
		redRidingHoodImageData[729] = 1; redRidingHoodImageData[730] = 1; redRidingHoodImageData[731] = 1; 
		redRidingHoodImageData[732] = 1; redRidingHoodImageData[733] = 1; redRidingHoodImageData[734] = 1; 
		redRidingHoodImageData[735] = 1; redRidingHoodImageData[736] = 1; redRidingHoodImageData[737] = 1; 
		redRidingHoodImageData[738] = 1; redRidingHoodImageData[739] = 1; redRidingHoodImageData[740] = 1; 
		redRidingHoodImageData[741] = 1; redRidingHoodImageData[742] = .43f; redRidingHoodImageData[743] = .06f; 
		redRidingHoodImageData[744] = 1; redRidingHoodImageData[745] = .93f; redRidingHoodImageData[746] = .7f; 
		redRidingHoodImageData[747] = 1; redRidingHoodImageData[748] = .93f; redRidingHoodImageData[749] = .7f; 
		redRidingHoodImageData[750] = 1; redRidingHoodImageData[751] = .43f; redRidingHoodImageData[752] = .06f; 
		redRidingHoodImageData[753] = 1; redRidingHoodImageData[754] = .65f; redRidingHoodImageData[755] = .14f; 
		redRidingHoodImageData[756] = 1; redRidingHoodImageData[757] = .65f; redRidingHoodImageData[758] = .14f; 
		redRidingHoodImageData[759] = 1; redRidingHoodImageData[760] = .65f; redRidingHoodImageData[761] = .14f; 
		redRidingHoodImageData[762] = 1; redRidingHoodImageData[763] = .65f; redRidingHoodImageData[764] = .14f; 
		redRidingHoodImageData[765] = 1; redRidingHoodImageData[766] = .65f; redRidingHoodImageData[767] = .14f; 
		redRidingHoodImageData[768] = 1; redRidingHoodImageData[769] = .43f; redRidingHoodImageData[770] = .06f; 
		redRidingHoodImageData[771] = 1; redRidingHoodImageData[772] = .65f; redRidingHoodImageData[773] = .14f; 
		redRidingHoodImageData[774] = 1; redRidingHoodImageData[775] = .65f; redRidingHoodImageData[776] = .14f; 
		redRidingHoodImageData[777] = 1; redRidingHoodImageData[778] = .43f; redRidingHoodImageData[779] = .06f;
		redRidingHoodImageData[780] = 1; redRidingHoodImageData[781] = 1; redRidingHoodImageData[782] = 1; 
		redRidingHoodImageData[783] = 1; redRidingHoodImageData[784] = 1; redRidingHoodImageData[785] = 1; 
		redRidingHoodImageData[786] = 1; redRidingHoodImageData[787] = 1; redRidingHoodImageData[788] = 1; 
		redRidingHoodImageData[789] = 1; redRidingHoodImageData[790] = 1; redRidingHoodImageData[791] = 1; 
		//13
		redRidingHoodImageData[792] = 1; redRidingHoodImageData[793] = 1; redRidingHoodImageData[794] = 1; 
		redRidingHoodImageData[795] = 1; redRidingHoodImageData[796] = 1; redRidingHoodImageData[797] = 1; 
		redRidingHoodImageData[798] = 1; redRidingHoodImageData[799] = 1; redRidingHoodImageData[800] = 1; 
		redRidingHoodImageData[801] = 1; redRidingHoodImageData[802] = 1; redRidingHoodImageData[803] = 1; 
		redRidingHoodImageData[804] = 1; redRidingHoodImageData[805] = 1; redRidingHoodImageData[806] = 1; 
		redRidingHoodImageData[807] = 1; redRidingHoodImageData[808] = .43f; redRidingHoodImageData[809] = .06f; 
		redRidingHoodImageData[810] = 1; redRidingHoodImageData[811] = .93f; redRidingHoodImageData[812] = .7f; 
		redRidingHoodImageData[813] = 1; redRidingHoodImageData[814] = .93f; redRidingHoodImageData[815] = .7f; 
		redRidingHoodImageData[816] = 1; redRidingHoodImageData[817] = .93f; redRidingHoodImageData[818] = .7f; 
		redRidingHoodImageData[819] = 1; redRidingHoodImageData[820] = .43f; redRidingHoodImageData[821] = .06f; 
		redRidingHoodImageData[822] = 1; redRidingHoodImageData[823] = .43f; redRidingHoodImageData[824] = .06f; 
		redRidingHoodImageData[825] = 1; redRidingHoodImageData[826] = .65f; redRidingHoodImageData[827] = .14f; 
		redRidingHoodImageData[828] = 1; redRidingHoodImageData[829] = .65f; redRidingHoodImageData[830] = .14f; 
		redRidingHoodImageData[831] = 1; redRidingHoodImageData[832] = .65f; redRidingHoodImageData[833] = .14f; 
		redRidingHoodImageData[834] = 1; redRidingHoodImageData[835] = .43f; redRidingHoodImageData[836] = .06f; 
		redRidingHoodImageData[837] = 1; redRidingHoodImageData[838] = .65f; redRidingHoodImageData[839] = .14f;
		redRidingHoodImageData[840] = 1; redRidingHoodImageData[841] = .43f; redRidingHoodImageData[842] = .06f; 
		redRidingHoodImageData[843] = 1; redRidingHoodImageData[844] = 1; redRidingHoodImageData[845] = 1; 
		redRidingHoodImageData[846] = 1; redRidingHoodImageData[847] = 1; redRidingHoodImageData[848] = 1; 
		redRidingHoodImageData[849] = 1; redRidingHoodImageData[850] = 1; redRidingHoodImageData[851] = 1; 
		redRidingHoodImageData[852] = 1; redRidingHoodImageData[853] = 1; redRidingHoodImageData[854] = 1; 
		redRidingHoodImageData[855] = 1; redRidingHoodImageData[856] = 1; redRidingHoodImageData[857] = 1; 
		//14
		redRidingHoodImageData[858] = 1; redRidingHoodImageData[859] = 1; redRidingHoodImageData[860] = 1; 
		redRidingHoodImageData[861] = 1; redRidingHoodImageData[862] = 1; redRidingHoodImageData[863] = 1; 
		redRidingHoodImageData[864] = 1; redRidingHoodImageData[865] = 1; redRidingHoodImageData[866] = 1; 
		redRidingHoodImageData[867] = 1; redRidingHoodImageData[868] = 1; redRidingHoodImageData[869] = 1; 
		redRidingHoodImageData[870] = 1; redRidingHoodImageData[871] = .43f; redRidingHoodImageData[872] = .06f; 
		redRidingHoodImageData[873] = 1; redRidingHoodImageData[874] = 1; redRidingHoodImageData[875] = 1; 
		redRidingHoodImageData[876] = 1; redRidingHoodImageData[877] = .43f; redRidingHoodImageData[878] = .06f; 
		redRidingHoodImageData[879] = 1; redRidingHoodImageData[880] = .93f; redRidingHoodImageData[881] = .7f; 
		redRidingHoodImageData[882] = 1; redRidingHoodImageData[883] = .93f; redRidingHoodImageData[884] = .7f; 
		redRidingHoodImageData[885] = 1; redRidingHoodImageData[886] = .93f; redRidingHoodImageData[887] = .7f; 
		redRidingHoodImageData[888] = 1; redRidingHoodImageData[889] = .65f; redRidingHoodImageData[890] = .14f; 
		redRidingHoodImageData[891] = 1; redRidingHoodImageData[892] = .65f; redRidingHoodImageData[893] = .14f; 
		redRidingHoodImageData[894] = 1; redRidingHoodImageData[895] = .65f; redRidingHoodImageData[896] = .14f; 
		redRidingHoodImageData[897] = 1; redRidingHoodImageData[898] = .65f; redRidingHoodImageData[899] = .14f; 
		redRidingHoodImageData[900] = 1; redRidingHoodImageData[901] = .43f; redRidingHoodImageData[902] = .06f; 
		redRidingHoodImageData[903] = 1; redRidingHoodImageData[904] = .43f; redRidingHoodImageData[905] = .06f; 
		redRidingHoodImageData[906] = 1; redRidingHoodImageData[907] = 1; redRidingHoodImageData[908] = 1; 
		redRidingHoodImageData[909] = 1; redRidingHoodImageData[910] = 1; redRidingHoodImageData[911] = 1; 
		redRidingHoodImageData[912] = 1; redRidingHoodImageData[913] = 1; redRidingHoodImageData[914] = 1; 
		redRidingHoodImageData[915] = 1; redRidingHoodImageData[916] = 1; redRidingHoodImageData[917] = 1; 
		redRidingHoodImageData[918] = 1; redRidingHoodImageData[919] = 1; redRidingHoodImageData[920] = 1; 
		redRidingHoodImageData[921] = 1; redRidingHoodImageData[922] = 1; redRidingHoodImageData[923] = 1; 
		//15
		redRidingHoodImageData[924] = 1; redRidingHoodImageData[925] = 1; redRidingHoodImageData[926] = 1; 
		redRidingHoodImageData[927] = 1; redRidingHoodImageData[928] = 1; redRidingHoodImageData[929] = 1; 
		redRidingHoodImageData[930] = 1; redRidingHoodImageData[931] = 1; redRidingHoodImageData[932] = 1; 
		redRidingHoodImageData[933] = 1; redRidingHoodImageData[934] = 1; redRidingHoodImageData[935] = 1; 
		redRidingHoodImageData[936] = 1; redRidingHoodImageData[937] = 1; redRidingHoodImageData[938] = 1; 
		redRidingHoodImageData[939] = 1; redRidingHoodImageData[940] = .43f; redRidingHoodImageData[941] = .06f; 
		redRidingHoodImageData[942] = 1; redRidingHoodImageData[943] = .43f; redRidingHoodImageData[944] = .06f; 
		redRidingHoodImageData[945] = 1; redRidingHoodImageData[946] = .43f; redRidingHoodImageData[947] = .06f; 
		redRidingHoodImageData[948] = 1; redRidingHoodImageData[949] = .93f; redRidingHoodImageData[950] = .7f; 
		redRidingHoodImageData[951] = 1; redRidingHoodImageData[952] = .93f; redRidingHoodImageData[953] = .7f; 
		redRidingHoodImageData[954] = 1; redRidingHoodImageData[955] = .65f; redRidingHoodImageData[956] = .14f; 
		redRidingHoodImageData[957] = 1; redRidingHoodImageData[958] = .65f; redRidingHoodImageData[959] = .14f; 
		redRidingHoodImageData[960] = 1; redRidingHoodImageData[961] = .65f; redRidingHoodImageData[962] = .14f; 
		redRidingHoodImageData[963] = 1; redRidingHoodImageData[964] = .43f; redRidingHoodImageData[965] = .06f; 
		redRidingHoodImageData[966] = 1; redRidingHoodImageData[967] = .43f; redRidingHoodImageData[968] = .06f; 
		redRidingHoodImageData[969] = 1; redRidingHoodImageData[970] = 1; redRidingHoodImageData[971] = 1; 
		redRidingHoodImageData[972] = 1; redRidingHoodImageData[973] = 1; redRidingHoodImageData[974] = 1; 
		redRidingHoodImageData[975] = 1; redRidingHoodImageData[976] = 1; redRidingHoodImageData[977] = 1; 
		redRidingHoodImageData[978] = 1; redRidingHoodImageData[979] = 1; redRidingHoodImageData[980] = 1; 
		redRidingHoodImageData[981] = 1; redRidingHoodImageData[982] = 1; redRidingHoodImageData[983] = 1; 
		redRidingHoodImageData[984] = 1; redRidingHoodImageData[985] = 1; redRidingHoodImageData[986] = 1; 
		redRidingHoodImageData[987] = 1; redRidingHoodImageData[988] = 1; redRidingHoodImageData[989] = 1;
		//16
		redRidingHoodImageData[990] = 1;  redRidingHoodImageData[991] = 1; redRidingHoodImageData[992] = 1; 
		redRidingHoodImageData[993] = 1;  redRidingHoodImageData[994] = 1; redRidingHoodImageData[995] = 1; 
		redRidingHoodImageData[996] = 1;  redRidingHoodImageData[997] = 1; redRidingHoodImageData[998] = 1; 
		redRidingHoodImageData[999] = 1;  redRidingHoodImageData[1000] = 1; redRidingHoodImageData[1001] = 1; 
		redRidingHoodImageData[1002] = 1; redRidingHoodImageData[1003] = 1; redRidingHoodImageData[1004] = 1; 
		redRidingHoodImageData[1005] = 1; redRidingHoodImageData[1006] = 1; redRidingHoodImageData[1007] = 1; 
		redRidingHoodImageData[1008] = 1; redRidingHoodImageData[1009] = 1; redRidingHoodImageData[1010] = 1; 
		redRidingHoodImageData[1011] = 1; redRidingHoodImageData[1012] = 1; redRidingHoodImageData[1013] = 1; 
		redRidingHoodImageData[1014] = 1; redRidingHoodImageData[1015] = .43f; redRidingHoodImageData[1016] = .06f; 
		redRidingHoodImageData[1017] = 1; redRidingHoodImageData[1018] = .43f; redRidingHoodImageData[1019] = .06f; 
		redRidingHoodImageData[1020] = 1; redRidingHoodImageData[1021] = .43f; redRidingHoodImageData[1022] = .06f; 
		redRidingHoodImageData[1023] = 1; redRidingHoodImageData[1024] = .65f; redRidingHoodImageData[1025] = .14f; 
		redRidingHoodImageData[1026] = 1; redRidingHoodImageData[1027] = .43f; redRidingHoodImageData[1028] = .06f; 
		redRidingHoodImageData[1029] = 1; redRidingHoodImageData[1030] = .43f; redRidingHoodImageData[1031] = .06f; 
		redRidingHoodImageData[1032] = 1; redRidingHoodImageData[1033] = 1; redRidingHoodImageData[1034] = 1; 
		redRidingHoodImageData[1035] = 1; redRidingHoodImageData[1036] = 1; redRidingHoodImageData[1037] = 1; 
		redRidingHoodImageData[1038] = 1; redRidingHoodImageData[1039] = 1; redRidingHoodImageData[1040] = 1; 
		redRidingHoodImageData[1041] = 1; redRidingHoodImageData[1042] = 1; redRidingHoodImageData[1043] = 1; 
		redRidingHoodImageData[1044] = 1; redRidingHoodImageData[1045] = 1; redRidingHoodImageData[1046] = 1; 
		redRidingHoodImageData[1047] = 1; redRidingHoodImageData[1048] = 1; redRidingHoodImageData[1049] = 1;
		redRidingHoodImageData[1050] = 1; redRidingHoodImageData[1051] = 1; redRidingHoodImageData[1052] = 1; 
		redRidingHoodImageData[1053] = 1; redRidingHoodImageData[1054] = 1; redRidingHoodImageData[1055] = 1; 
		//17
		redRidingHoodImageData[1056] = 1; redRidingHoodImageData[1057] = 1; redRidingHoodImageData[1058] = 1; 
		redRidingHoodImageData[1059] = 1; redRidingHoodImageData[1060] = 1; redRidingHoodImageData[1061] = 1; 
		redRidingHoodImageData[1062] = 1; redRidingHoodImageData[1063] = 1; redRidingHoodImageData[1064] = 1; 
		redRidingHoodImageData[1065] = 1; redRidingHoodImageData[1066] = 1; redRidingHoodImageData[1067] = 1; 
		redRidingHoodImageData[1068] = 1; redRidingHoodImageData[1069] = 1; redRidingHoodImageData[1070] = 1; 
		redRidingHoodImageData[1071] = 1; redRidingHoodImageData[1072] = 1; redRidingHoodImageData[1073] = 1; 
		redRidingHoodImageData[1074] = 1; redRidingHoodImageData[1075] = 1; redRidingHoodImageData[1076] = 1; 
		redRidingHoodImageData[1077] = 1; redRidingHoodImageData[1078] = 1; redRidingHoodImageData[1079] = 1; 
		redRidingHoodImageData[1080] = 1; redRidingHoodImageData[1081] = 1; redRidingHoodImageData[1082] = 1; 
		redRidingHoodImageData[1083] = 1; redRidingHoodImageData[1084] = .43f; redRidingHoodImageData[1085] = .06f; 
		redRidingHoodImageData[1086] = 1; redRidingHoodImageData[1087] = 1; redRidingHoodImageData[1088] = 1; 
		redRidingHoodImageData[1089] = 1; redRidingHoodImageData[1090] = .65f; redRidingHoodImageData[1091] = .14f; 
		redRidingHoodImageData[1092] = 1; redRidingHoodImageData[1093] = 1; redRidingHoodImageData[1094] = 1; 
		redRidingHoodImageData[1095] = 1; redRidingHoodImageData[1096] = .43f; redRidingHoodImageData[1097] = .06f; 
		redRidingHoodImageData[1098] = 1; redRidingHoodImageData[1099] = 1; redRidingHoodImageData[1100] = 1; 
		redRidingHoodImageData[1101] = 1; redRidingHoodImageData[1102] = 1; redRidingHoodImageData[1103] = 1; 
		redRidingHoodImageData[1104] = 1; redRidingHoodImageData[1105] = 1; redRidingHoodImageData[1106] = 1; 
		redRidingHoodImageData[1107] = 1; redRidingHoodImageData[1108] = 1; redRidingHoodImageData[1109] = 1;
		redRidingHoodImageData[1110] = 1; redRidingHoodImageData[1111] = 1; redRidingHoodImageData[1112] = 1; 
		redRidingHoodImageData[1113] = 1; redRidingHoodImageData[1114] = 1; redRidingHoodImageData[1115] = 1; 
		redRidingHoodImageData[1116] = 1; redRidingHoodImageData[1117] = 1; redRidingHoodImageData[1118] = 1; 
		redRidingHoodImageData[1119] = 1; redRidingHoodImageData[1120] = 1; redRidingHoodImageData[1121] = 1; 
		//18
		redRidingHoodImageData[1122] = 1; redRidingHoodImageData[1123] = 1; redRidingHoodImageData[1124] = 1; 
		redRidingHoodImageData[1125] = 1; redRidingHoodImageData[1126] = 1; redRidingHoodImageData[1127] = 1; 
		redRidingHoodImageData[1128] = 1; redRidingHoodImageData[1129] = 1; redRidingHoodImageData[1130] = 1; 
		redRidingHoodImageData[1131] = 1; redRidingHoodImageData[1132] = 1; redRidingHoodImageData[1133] = 1; 
		redRidingHoodImageData[1134] = 1; redRidingHoodImageData[1135] = 1; redRidingHoodImageData[1136] = 1; 
		redRidingHoodImageData[1137] = 1; redRidingHoodImageData[1138] = 1; redRidingHoodImageData[1139] = 1; 
		redRidingHoodImageData[1140] = 1; redRidingHoodImageData[1141] = 1; redRidingHoodImageData[1142] = 1; 
		redRidingHoodImageData[1143] = 1; redRidingHoodImageData[1144] = 1; redRidingHoodImageData[1145] = 1; 
		redRidingHoodImageData[1146] = 1; redRidingHoodImageData[1147] = 1; redRidingHoodImageData[1148] = 1; 
		redRidingHoodImageData[1149] = 1; redRidingHoodImageData[1150] = 1; redRidingHoodImageData[1151] = 1; 
		redRidingHoodImageData[1152] = 1; redRidingHoodImageData[1153] = .43f; redRidingHoodImageData[1154] = .06f; 
		redRidingHoodImageData[1155] = 1; redRidingHoodImageData[1156] = .43f; redRidingHoodImageData[1157] = .06f; 
		redRidingHoodImageData[1158] = 1; redRidingHoodImageData[1159] = .43f; redRidingHoodImageData[1160] = .06f; 
		redRidingHoodImageData[1161] = 1; redRidingHoodImageData[1162] = .43f; redRidingHoodImageData[1163] = .06f; 
		redRidingHoodImageData[1164] = 1; redRidingHoodImageData[1165] = 1; redRidingHoodImageData[1166] = 1; 
		redRidingHoodImageData[1167] = 1; redRidingHoodImageData[1168] = 1; redRidingHoodImageData[1169] = 1;
		redRidingHoodImageData[1170] = 1; redRidingHoodImageData[1171] = 1; redRidingHoodImageData[1172] = 1; 
		redRidingHoodImageData[1173] = 1; redRidingHoodImageData[1174] = 1; redRidingHoodImageData[1175] = 1; 
		redRidingHoodImageData[1176] = 1; redRidingHoodImageData[1177] = 1; redRidingHoodImageData[1178] = 1; 
		redRidingHoodImageData[1179] = 1; redRidingHoodImageData[1180] = 1; redRidingHoodImageData[1181] = 1; 
		redRidingHoodImageData[1182] = 1; redRidingHoodImageData[1183] = 1; redRidingHoodImageData[1184] = 1; 
		redRidingHoodImageData[1185] = 1; redRidingHoodImageData[1186] = 1; redRidingHoodImageData[1187] = 1; 
		
	}
	redRidingHoodImageDataBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		sizeof(float) * (3 * 22 * 18),
		redRidingHoodImageData, &error);
	if(error != 0)
	{
		printf("Error Creating redRidingHoodImageDataBuffer\n");
		printf("Error %i", error);
		printf("\nAborting...");
		printf(".......................\n");
		exit(error);
	}

	clSetKernelArg(kernel, 2, sizeof(cl_mem), &redRidingHoodImageDataBuffer);
}

void GPGPULauncher::loadKernel(const char* name)
{
	std::ifstream in(name);
	std::string result((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

	createProgram(result);
}

void GPGPULauncher::createProgram(const std::string& source)
{
	size_t lengths[1] = { source.size() };
	const char* sources[1] = { source.data() };

	cl_int error = 0;
	program = clCreateProgramWithSource(context, 1, sources, lengths, &error);

	if(error != 0)
	{
		printf("Error creating Program\n");
		printf("Error %i", error);
		printf("Aborting\n");
		printf(".......................\n");
		exit(error);
	}
}

void GPGPULauncher::printPlatformNames()
{
	printf("*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n");
	printf("Printing platform names\n\n");
	int numPlatforms = platform_ids.size();
	for(int i = 0; i < numPlatforms; i++)
	{
		printf("%i. %s\n", i, getPlatformName(platform_ids[i]).c_str());
	}
	printf("*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n");
}

std::string GPGPULauncher::getPlatformName(cl_platform_id id)
{
	size_t size = 0;
	clGetPlatformInfo(id, CL_PLATFORM_NAME, 0, nullptr, &size);

	std::string result;
	result.resize(size);
	clGetPlatformInfo(id, CL_PLATFORM_NAME, size, const_cast<char*>(result.data()), nullptr);
	return result;
}

void GPGPULauncher::printDeviceNames()
{
	printf("*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n");
	printf("Printing device names\n");
	int numDevices = device_ids.size();
	for(int i = 0; i < numDevices; i++)
	{
		printf("%i. %s\n", i, getDeviceName(device_ids[i]).c_str());

	}
	printf("*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n");
}

std::string GPGPULauncher::getDeviceName(cl_device_id id)
{
	size_t size = 0;
	clGetDeviceInfo(id, CL_DEVICE_NAME, 0, nullptr, &size);

	std::string result;
	result.resize(size);
	clGetDeviceInfo(id, CL_DEVICE_NAME, size, const_cast<char*>(result.data()), nullptr);
	return result;
}

void GPGPULauncher::updatebuffers(Camera* _camera, float _dt)
{
	cl_int error = 0;
	// Prepare some test data

	memcpy(&cameraData[0], &_camera->getCameraPosition(), sizeof(float) * 4);
	memcpy(&cameraData[4], &_camera->getRightDirection(), sizeof(float) * 4);
	memcpy(&cameraData[8], &_camera->getViewDirection(), sizeof(float) * 4);

	error = clEnqueueWriteBuffer(command_queue, cameraBuffer, CL_FALSE, 0, sizeof(float) * 12, &cameraData[0], 0, nullptr, nullptr);
	if (error != 0)
	{
		printf("fail");
	}
	// update lights
	float newx, newz;
	for (int i = 0; i < numLights; i++)
	{
		newx = lightPos[i * 3 + 1] * cos(lightSpeed[i] * _dt) - lightPos[i * 3 + 3] * sin(lightSpeed[i] * _dt);
		newz = lightPos[i * 3 + 1] * sin(lightSpeed[i] * _dt) + lightPos[i * 3 + 3] * cos(lightSpeed[i] * _dt);

		lightPos[i * 3 + 1] = newx;
		lightPos[i * 3 + 3] = newz;
	}
	error = clEnqueueWriteBuffer(command_queue, lightBuffer, CL_FALSE, 0, sizeof(float) * (3 * numLights + 1), &lightPos[0], 0, nullptr, nullptr);
}