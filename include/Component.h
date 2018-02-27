#ifndef COMPONENT_H
#define COMPONENT_H
//#include <string>
class GameObject;
class Component
{
	friend GameObject;
public:
	//No need to pass in parent. GameObject constructor sets parent of Components to itself.
	Component();
	virtual ~Component();

	//init and terminate do nothing by default, and may continue to do nothing in derived classes. Only update is required to be re-defined.
	//Used to wire a dependant component from parent to itself and check that its contents have been assigned.
	virtual void init();
	//Used to dictate automated behaviour
	virtual void update() = 0;
	//Used to fire events without deleting the object. ie pause the component and throw some event, but don't actually delete the component until the parent is deleted so the component virtually dissapears
	virtual void terminate();

	Component* setActive(bool);
	void deleteParent();

protected:
	GameObject* parent;

private:
	bool isActive;
};
#endif // !COMPONENT_H