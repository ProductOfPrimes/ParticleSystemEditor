#include "GameObject.h"
#include "Transformable.h"
#include <algorithm>

GameObject::GameObject(const std::vector<Component*>& components) :
	m_deleteFlag(false), m_activeFlag(true)
{
	//Assign the additional component(s)
	unsigned int size = components.size();
	for (unsigned int i = 0; i < size; i++) {
		const type_info* index = &typeid(*components[i]);	//alias to ith component type
#if ExtremeSafety
		//Throw if passed in components haven't been allocated correctly.
		if (!components[i])
			throw;

		//If this is the first component of its type, add it to the container. If not, throw because we must have only uniquely typed components. No components of equal type per GameObject.
		if (m_components.count(index) != 0)
			throw;
#endif
			//Store component by type id.
			m_components[index] = components[i];
			//Wire the component
			components[i]->parent = this;

			if (index == &typeid(Transformable))
				transformable = static_cast<Transformable*>(components[i]);
	}

	for (unsigned int i=0; i<size; i++)
		components[i]->init();
}

GameObject::~GameObject()
{
	//Says "cannot modify a constant value"...?
	//auto cleanup = [](std::pair<const std::type_info*, Component*> c) {
	//	if (c.second != nullptr)
	//		printf("lit\n");
	//	return true;
	//};
	//std::remove_if(m_components.begin(), m_components.end(), cleanup);

	for (auto itr = m_components.begin(); itr != m_components.end(); itr++) {
		delete itr->second;
		itr->second = nullptr;
	}
	m_components.clear();
}

void GameObject::update()
{
	if (!m_activeFlag)
		return;
	const auto itrBegin = m_components.begin();
	const auto itrEnd = m_components.end();
	for (auto itr = itrBegin; itr != itrEnd; itr++) {
		auto component = itr->second;
		if (component->isActive)
			component->update();
	}
}

bool GameObject::getActive()
{
	return m_activeFlag;
}

bool GameObject::getDeleteFlag()
{
	return m_deleteFlag;
}

GameObject * GameObject::setActive(bool active)
{
	m_activeFlag = active;
	return this;
}

GameObject * GameObject::setDeleteFlag(bool b)
{
	m_deleteFlag = b;
	return this;
}