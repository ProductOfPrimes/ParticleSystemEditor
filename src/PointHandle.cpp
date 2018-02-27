#include "PointHandle.h"

PointHandle::PointHandle(float _pointSize, glm::vec3* _position, std::string _label)
{
	pointSize = _pointSize;
	position = _position;
	label = _label;
}

bool PointHandle::isInside(glm::vec3 p)
{
	return glm::length((p - *position)) < pointSize;
}

bool PointHandle::isInside(glm::vec2 p)
{
	return glm::length((p - glm::vec2(*position))) < pointSize;
}

void PointHandle::draw()
{
	TTK::Graphics::DrawPoint(*position, pointSize);
	TTK::Graphics::DrawText2D(label, position->x, position->y);
}

bool PointHandle::isValid()
{
	return (position != NULL);
}
