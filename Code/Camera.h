#pragma once
#include "common.h"
#include "InputInteface.h"

class Camera
{
public:
	Camera();
	~Camera();

	void update(double dt);

	Vec4 getCameraPosition() { return cameraPosition; };
	Vec4 getViewDirection() { return viewDirection; };
	Vec4 getRightDirection() { return rightDirection; };

private:
	Vec4 cameraPosition;
	Vec4 viewDirection;
	Vec4 rightDirection;
	Vec4 upDirection;

	float movingSpeedZ;
	float movingSpeedX;
	float movingSpeedY;
};