#pragma once
#include "common.h"
#include "InputInteface.h"

class ImageRenderer
{
public:
	ImageRenderer();
	~ImageRenderer();
	void init();
	void render(float*);

	bool isWindowClosed();
private:
};

