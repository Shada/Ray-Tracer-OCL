#include "InputInteface.h"

InputInterFace* InputInterFace::instance = nullptr;

InputInterFace* InputInterFace::getInstance()
{
	if(!instance)
		instance = new InputInterFace();

	return instance;
}

InputInterFace::InputInterFace()
{
	for(int i = 0; i < 350; i++)
		keyStates[i] = false;
	cursorPosX = 0;
	cursorPosY = 0;
	
	prevCursorPosX = 0;
	prevCursorPosY = 0;
}

void InputInterFace::getCursorMove(double &cursorMoveX, double &cursorMoveY)
{
	cursorMoveX = this->prevCursorPosX - this->cursorPosX;
	cursorMoveY = this->prevCursorPosY - this->cursorPosY;

	this->prevCursorPosX = this->cursorPosX;
	this->prevCursorPosY = this->cursorPosY;
}

void InputInterFace::setCursorPos(double cursorPosX, double cursorPosY)
{
	this->cursorPosX = cursorPosX;
	this->cursorPosY = cursorPosY;
}
InputInterFace::~InputInterFace() 
{
	if(instance)
	{
		delete instance;
		instance = nullptr;
	}
}