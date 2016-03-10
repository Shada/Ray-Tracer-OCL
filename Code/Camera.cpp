#include "Camera.h"

Camera::Camera()
{
	cameraPosition = Vec4(0, 0, -99, 0);
	viewDirection = Vec4(0, 0, 1, 0);
	rightDirection = Vec4(1, 0, 0, 0);
	upDirection = Vec4(0, 1, 0, 0);
	movingSpeedX = 0;
	movingSpeedY = 0;
	movingSpeedZ = 0;
}

#include <iostream>
void Camera::update(double dt)
{
	if(InputInterFace::getInstance()->getKeyState(TOBI_KEY_W))
		movingSpeedZ = 200;
	else if(InputInterFace::getInstance()->getKeyState(TOBI_KEY_S))
		movingSpeedZ = -200;
	else
		movingSpeedZ = 0;

	if(InputInterFace::getInstance()->getKeyState(TOBI_KEY_D))
		movingSpeedX = 200;
	else if(InputInterFace::getInstance()->getKeyState(TOBI_KEY_A))
		movingSpeedX = -200;
	else
		movingSpeedX  = 0;

	if(InputInterFace::getInstance()->getKeyState(TOBI_KEY_SPACE))
		movingSpeedY = 200;
	else if(InputInterFace::getInstance()->getKeyState(TOBI_KEY_LEFT_SHIFT))
		movingSpeedY = -200;
	else
		movingSpeedY  = 0;

	double mouseMoveX = 0;
	double mouseMoveY = 0;
	InputInterFace::getInstance()->getCursorMove(mouseMoveX, mouseMoveY);
	

	if(mouseMoveX > 0)
	{
		viewDirection = rotateVec4(viewDirection, upDirection, 0.07f);
		rightDirection = rotateVec4(rightDirection, upDirection, 0.07f);
	}
	if(mouseMoveX < 0)
	{
		viewDirection = rotateVec4(viewDirection, upDirection, -0.07f);
		rightDirection = rotateVec4(rightDirection, upDirection, -0.07f);
	}
	if(mouseMoveY > 0)
	{
		viewDirection = rotateVec4(viewDirection, rightDirection, -0.07f);
	}
	if(mouseMoveY < 0)
	{
		viewDirection = rotateVec4(viewDirection, rightDirection, 0.07f);
	}


	cameraPosition += viewDirection * movingSpeedZ * dt + rightDirection * movingSpeedX * dt + upDirection * movingSpeedY * dt;

	if (cameraPosition.x > 100)
		cameraPosition.x = 100;
	else if(cameraPosition.x < -100)
		cameraPosition.x = -100;
	if (cameraPosition.y > 100)
		cameraPosition.y = 100;
	else if (cameraPosition.y < -100)
		cameraPosition.y = -100;
	if (cameraPosition.z > 100)
		cameraPosition.z = 100;
	else if (cameraPosition.z < -100)
		cameraPosition.z = -100;
}

Camera::~Camera()
{
}
