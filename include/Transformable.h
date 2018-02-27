#ifndef TRANSFORMABLE_H
#define TRANSFORMABLE_H
#include "Component.h"

#define GLM_FORCE_SWIZZLE

#include "custom_serialization.h"

#define PRODUCTOFPRIMES_Y_FORWARD false; // set this to true and forward will be Y, put PRODUCTOFPRIMES before it to tag it as one of ours

class Transform
{
	friend class boost::serialization::access;
public:
	Transform();
	virtual ~Transform();

	void update();

	////////////////////////////////
	// parenting system
	//void addChild(Transform* const child);
	//void removeChild(Transform* const whoWantsToBeAnOrphan);
	//void setParentTransform(Transform* parent);
	//inline bool isRoot();  // returns true if no parent, false if it has a parent
	//
	//std::vector<Transform*> getChildren();
	//Transform* getParentTransform();
	//
	//glm::mat4 getWorldMatrix();
	////////////////////////////////

	const glm::mat4& getTransform() const;
	const glm::mat4& getRotationMatrix() const;
	const glm::mat4& getScaleMatrix() const;

	const glm::vec3& getPosition() const;
	const glm::vec3& getRotation() const;
	const glm::vec3& getScale() const;

	glm::vec3 getRight() const;
	glm::vec3 getForward() const;
	glm::vec3 getAbove() const;

	Transform* setPosition(const glm::vec3&);
	Transform* setRotation(const glm::vec3&);
	Transform* setScale(const glm::vec3&);
	Transform* setScale(const float&);
	Transform* setDirection(const glm::vec3&); // will affect internal euler angles

	Transform* setTranslationMatrix(const glm::mat4&);
	Transform* setRotationMatrix(const glm::mat4&); // will affect internal euler angles
	Transform* setScaleMatrix(const glm::mat4&);

	Transform* rotateX(const float& deltaXrotation); //rotate by delta from current rotation
	Transform* rotateY(const float& deltaYrotation); //rotate by delta from current rotation
	Transform* rotateZ(const float& deltaZrotation); //rotate by delta from current rotation
	Transform* rotateXYZ(const glm::vec3& deltaXYZ); //rotate by delta from current rotation

	Transform* moveX(const float& deltaX); // translate by delta
	Transform* moveY(const float& deltaY); // translate by delta
	Transform* moveZ(const float& deltaZ); // translate by delta
	Transform* move(const glm::vec3& deltaXYZ); // translate by delta

	Transform operator* (const Transform& right); // returns a new Transformable representing the concatenation of the two transformables, with all extra members updated. Naturally slower than dealing with a single mat4

protected:
	glm::mat4 m_transform, m_translate, m_rotate, m_scaling;
	glm::vec3 m_position, m_rotation, m_scale;
	bool m_updated;
	static glm::mat4 s_identity;

	////////////////////////////////
	// parenting system

	//std::vector<Transform*> m_children;
	//Transform* m_parentTransform;
	//glm::mat4 worldMatrix;
	//glm::mat4 worldRotation;

	///////////////////////////////
	// boost
	
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & m_position;
		ar & m_rotation;
		ar & m_scale;

		ar & m_transform;
		ar & m_translate;
		ar & m_rotate;
		ar & m_scaling;
	}
};

class Transformable : public Component, public Transform
{
	friend class boost::serialization::access;
	//actually does literally nothing unique, just has basic Component functionality as well as Transform
	void update();
};


//namespace boost {
//	namespace serialization {
//		template<class Archive>
//		void serialize(Archive & ar, Transform& transform, const unsigned int version)
//		{
//			ar & transform.m_position;
//			ar & transform.m_rotation;
//			ar & transform.m_scale;
//
//			ar & transform.m_transform;
//			ar & transform.m_translate;
//			ar & transform.m_rotate;
//			ar & transform.m_scaling;
//		}
//	}
//}
#endif