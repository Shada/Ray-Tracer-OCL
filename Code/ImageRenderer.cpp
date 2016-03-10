#include "ImageRenderer.h"

#include "GLIncludes.h"
GLFWwindow *window;
GLuint textureID;

static const GLfloat uvBufferData[] =
{
	0, 0,
	1, 0,
	0, 1,
	1, 0,
	1, 1,
	0, 1
};

static const GLfloat vertexPositionData[] =
{
	0
};

ImageRenderer::ImageRenderer()
{
	window = nullptr;
	init();
}


ImageRenderer::~ImageRenderer()
{
	glDeleteTextures(1, &textureID);
	glfwDestroyWindow(window);
	glfwTerminate();
}
#include <vector>
void ImageRenderer::render(float* image)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0, 1, 0.0, 1, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	glDisable(GL_LIGHTING);

	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_NONE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_NONE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 512, 512, 0, GL_RGB, GL_FLOAT, image);
	glEnable(GL_TEXTURE_2D);

	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex3f(0, 0, 0);
	glTexCoord2f(0, 1); glVertex3f(0, 1, 0);
	glTexCoord2f(1, 1); glVertex3f(1, 1, 0);
	glTexCoord2f(1, 0); glVertex3f(1, 0, 0);
	glEnd();

	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);

	glfwSwapBuffers(window);
	glfwPollEvents();
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

	if( action == GLFW_PRESS)
		InputInterFace::getInstance()->setKeyState(key, true);
	else if( action == GLFW_RELEASE)
		InputInterFace::getInstance()->setKeyState(key, false);
}

static void cursorpos_callback(GLFWwindow* window, double posX, double posY)
{
	InputInterFace::getInstance()->setCursorPos(posX, posY);
}

void ImageRenderer::init()
{
	//initialize glfw
	if(!glfwInit())
	{
		fprintf(stderr, "Failed to initilize GLFW");
		exit(EXIT_FAILURE);
	}

	window = glfwCreateWindow(512, 512, "Title", nullptr, nullptr);
	if(!window)
	{
		fprintf(stderr, "Failed to create window");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);

	glfwSwapInterval(1);

	glfwSetKeyCallback(window, key_callback);

	glfwSetCursorPosCallback(window, cursorpos_callback);
	
	glGenTextures(1, &textureID);
}

bool ImageRenderer::isWindowClosed()
{
	if(glfwWindowShouldClose(window))
	{
		return true;
	}
	return false;
}