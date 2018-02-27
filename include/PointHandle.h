#pragma once
#include <string>
#include "TTK\GraphicsUtils.h"

class PointHandle
{
public:
	// Point size is in Pixels
	PointHandle(float _pointSize, glm::vec3* _position, std::string _label = "");

	// Does a simple radius based intersection test
	bool isInside(glm::vec3 p);
	//same, but ignores z-distance
	bool isInside(glm::vec2 p);

	void draw();

	bool isValid(); // returns false if the pointer is null

	glm::vec3* position;
	std::string label;
	float pointSize;
};