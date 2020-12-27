#pragma once

#include "srpch.h"


class SUNRISE_API Scene
{
public:

	Scene();

	virtual void load() = 0;

	virtual void update() = 0;

	virtual void unload() = 0;



};

