#include "Component.h"
#include "GameObject.h"

Component::Component() : parent(nullptr), isActive(true)
{
}

Component::~Component()
{
	//printf("component desturctor called!\n");
}

void Component::init()
{
	if (parent == nullptr)
		throw std::exception("component has no parent! :(\n");
}

void Component::terminate()
{
}

Component * Component::setActive(bool active)
{
	isActive = active;
	return this;
}

void Component::deleteParent()
{
	parent->setDeleteFlag(true);
}
