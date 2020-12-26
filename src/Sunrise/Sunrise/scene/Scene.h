#pragma once




class Scene
{
public:

	Scene();

	virtual void load() = 0;

	virtual void update() = 0;

	virtual void unload() = 0;



};

