#include <stdio.h>
#include <stdlib.h>
#include <ctime>

#include "InputInteface.h"
#include "ImageRenderer.h"
#include "GPGPULauncher.h"
#include "Camera.h"


#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL\cl.h>
#endif

#pragma comment(lib, "OpenCL.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glfw3dll.lib")

#define MEM_SIZE (512)
#define MAX_SOURCE_SIZE (0x100000)

ImageRenderer *renderer = nullptr;
GPGPULauncher *gpgpuLauncher = nullptr;

static clock_t prevTime = 0;
static clock_t currTime = 0;
static double passedTime = 0.0;
static int passedFrames = 0;
static int fps = 0;
static double deltaTime = 0;

Camera* camera = nullptr;

int main()
{
	
	renderer = new ImageRenderer();
	camera = new Camera();
	gpgpuLauncher = new GPGPULauncher();
	gpgpuLauncher->init(camera);

	prevTime = clock();

	while (!renderer->isWindowClosed())
	{
		currTime = clock();
		deltaTime = (currTime - prevTime) / 1000.0;
		camera->update(deltaTime);
		gpgpuLauncher->updatebuffers(camera, deltaTime);
		gpgpuLauncher->launchProgram(camera);
		renderer->render(gpgpuLauncher->getImage());
		passedFrames++;
		passedTime += deltaTime;
		prevTime = currTime;
		if(passedTime >= 1.0)
		{
			printf("FPS: %i\n", fps);
			fps = int(passedFrames / passedTime);
			passedFrames = 0;
			passedTime = 0.0;
		}
	}

	delete(renderer);
	renderer = nullptr;
	delete(gpgpuLauncher);
	gpgpuLauncher = nullptr;
	return 0;
}