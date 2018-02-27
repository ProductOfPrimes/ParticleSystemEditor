#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H
#define ExtremeSafety true
#include "Transformable.h"
#include <typeinfo>
#include <unordered_map>

class GameObject
{
public:
	//Could make it a vector of unique pointers but that is really just a lot of typing for next to no gain as composing GameObjects will be automated by a Factory.
	GameObject(const std::vector<Component*>&);
	virtual ~GameObject();

	void update();	//*Not polymorphic. GameObject is the highest level of abstraction!
	Transform* transformable;

	template<typename T>
	bool exists() { return m_components[&typeid(T)] != nullptr; }

	template<typename T>
	T* accessComponent() { return static_cast<T*>(m_components[&typeid(T)]); }
	
	GameObject* setActive(bool);
	GameObject* setDeleteFlag(bool);

	bool getActive();
	bool getDeleteFlag();

private:
	std::unordered_map<const std::type_info*, Component*> m_components;
	bool m_deleteFlag, m_activeFlag;
};

#endif