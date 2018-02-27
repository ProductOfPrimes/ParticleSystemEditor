#include "Transformable.h"
#include <GLM\gtc\quaternion.hpp>
#include <GLM\gtx\quaternion.hpp>
#include <GLM\gtc\matrix_transform.hpp>
#include <exception>

glm::mat4 Transform::s_identity = glm::mat4();

Transform::Transform() : m_transform(glm::mat4()), m_updated(true),
m_translate(glm::mat4()), m_rotate(glm::mat4()), m_scaling(glm::mat4()),
m_position(glm::vec3(0.0f)), m_rotation(glm::vec3(0.0f)), m_scale(glm::vec3(1.0f))
{
	//m_parentTransform = nullptr;
}

Transform::~Transform()
{
}

void Transform::update()
{
	if (!m_updated)
	{
		m_transform = m_translate * m_rotate * m_scaling;

		//if (isRoot())
		//{
		//	worldMatrix = m_transform;
		//} 
		//else
		//{
		//	worldMatrix = m_parentTransform->worldMatrix * m_transform;
		//	for (auto c : m_children)
		//	{
		//		c->m_updated = false;
		//		c->update();
		//	}
		//}
		m_updated = true;
	}



	//else
	//{
	//	worldMatrix = m_transform;
	//}
	//
	//// Update children
	//for (int i = 0; i < m_pChildren.size(); i++)
	//	m_pChildren[i]->update(dt);


	//if (!m_updated)
	//{
	//	//Better than before because we would have re-computed
	//	//ie if our translation and rotation matrices stayed the same but we scaled then we would still be calling translate() and rotate() in addition to scale()
	//	m_transform = m_translate * m_rotate * m_scaling;
	//	m_updated = true;
	//}

	//if (m_parentTransform)
	//{
	//	m_pLocalToWorldMatrix = m_pParent->getLocalToWorldMatrix() * m_pLocalTransformMatrix * scal;
	//}
	//else
	//{
	//	worldMatrix = m_transform;
	//}

	//// Update children
	//for (int i = 0; i < m_pChildren.size(); i++)
	//	m_pChildren[i]->update(dt);
}

//void Transform::addChild(Transform * const child)
//{
//	m_children.push_back(child);
//}
//
//void Transform::removeChild(Transform * const whoWantsToBeAnOrphan)
//{
//	auto found = std::find(m_children.begin(), m_children.end(), whoWantsToBeAnOrphan);
//	if (found != m_children.end())
//	{
//		m_children.erase(found);
//	}
//}
//
//void Transform::setParentTransform(Transform * parentTransform)
//{
//	m_parentTransform = parentTransform;
//}
//
//inline bool Transform::isRoot()
//{
//	return !(m_parentTransform);
//}
//
//std::vector<Transform*> Transform::getChildren()
//{
//	return m_children;
//}
//
//Transform * Transform::getParentTransform()
//{
//	return m_parentTransform;
//}
//
//glm::mat4 Transform::getWorldMatrix()
//{
//	return worldMatrix;
//}

const glm::mat4 & Transform::getTransform() const
{
	return m_transform;
}

const glm::mat4 & Transform::getRotationMatrix() const
{
	return m_rotate;
}

const glm::mat4 & Transform::getScaleMatrix() const
{
	return m_scaling;
}

const glm::vec3 & Transform::getPosition() const
{
	return m_position;
}

const glm::vec3 & Transform::getRotation() const
{
	return m_rotation;
}

const glm::vec3 & Transform::getScale() const
{
	return m_scale;
}

#if PRODUCTOFPRIMES_Y_FORWARD
glm::vec3 Transformable::getRight() const
{
	//return glm::vec3(m_rotate[0][0], m_rotate[1][0], m_rotate[2][0]);
	return glm::vec3(m_rotate[0].xyz);
}

glm::vec3 Transformable::getForward() const
{
	//return glm::vec3(m_rotate[0][2], m_rotate[1][2], m_rotate[2][2]);
	return glm::vec3(m_rotate[1].xyz);
}

glm::vec3 Transformable::getAbove() const
{
	//return glm::vec3(m_rotate[0][1], m_rotate[1][1], m_rotate[2][1]);
	return glm::vec3(m_rotate[2].xyz);
}
#else
glm::vec3 Transform::getRight() const
{
	return glm::vec3(m_rotate[0].xyz);
}

glm::vec3 Transform::getAbove() const
{
	return glm::vec3(m_rotate[1].xyz);
}

glm::vec3 Transform::getForward() const
{
	return glm::vec3(m_rotate[2].xyz);
}
#endif

Transform * Transform::rotateX(const float & delta)
{
	m_rotation.x += delta;
	return setRotation(m_rotation);
}

Transform * Transform::rotateY(const float & delta)
{
	m_rotation.y += delta;
	return setRotation(m_rotation);
}

Transform * Transform::rotateZ(const float & delta)
{
	m_rotation.z += delta;
	return setRotation(m_rotation);
}

Transform * Transform::rotateXYZ(const glm::vec3& deltaXYZ)
{
	m_rotation += deltaXYZ;
	return setRotation(m_rotation);
}

Transform * Transform::moveX(const float & deltaX)
{
	m_position.x += deltaX;
	m_translate = glm::translate(s_identity, m_position);
	m_updated = false;
	return this;
}

Transform * Transform::moveY(const float & deltaY)
{
	m_position.y += deltaY;
	m_translate = glm::translate(s_identity, m_position);
	m_updated = false;
	return this;
}

Transform * Transform::moveZ(const float & deltaZ)
{
	m_position.z += deltaZ;
	m_translate = glm::translate(s_identity, m_position);
	m_updated = false;
	return this;
}

Transform * Transform::move(const glm::vec3 & delta)
{
	m_position += delta;
	m_translate = glm::translate(s_identity, m_position);
	m_updated = false;
	return this;
}

Transform * Transform::setPosition(const glm::vec3 &p)
{
	m_position = p;
	m_translate = glm::translate(s_identity, p);
	m_updated = false;
	return this;
}

Transform * Transform::setRotation(const glm::vec3 &r)
{
	m_rotation = r;
	const glm::quat& q = glm::quat(r);		//Builds rotation quaternion from euler angles.
	const glm::vec3& axis = glm::axis(q);	//If 1 - w * w <= 0 (Occurs when euler angles are all 0 or q is not a rotation quaternion) returns (0, 0, 1), otherwise returns rotation axis.
	const float angle = glm::angle(q);		//acos 2 * w
	m_rotate = glm::rotate(s_identity, angle, axis);	//if the angle is 0, the axis doesn't matter. The only case in which axis() is 0 is when the angle is 0 so we're safe!
	m_updated = false;
	return this;
}

Transform * Transform::setScale(const glm::vec3 &s)
{
	m_scale = s;
	m_scaling = glm::scale(s_identity, s);
	m_updated = false;
	return this;
}

Transform * Transform::setScale(const float & s)
{
	m_scale = glm::vec3(s);
	m_scaling = glm::scale(s_identity, m_scale);
	m_updated = false;
	return this;
}

Transform * Transform::setDirection(const glm::vec3& direction)
{
#ifdef _DEBUG
	if (glm::length2(direction) == 0.0f)
	{
		throw std::exception("Component: Transformable: setDirection: passed direction was zero vector!\n");
		return this;
	}
#endif

#if PRODUCTOFPRIMES_Y_FORWARD
	glm::vec3 newForward = glm::normalize(direction);
	glm::vec3 newRight = glm::cross(newForward, glm::vec3(0.0f, 0.0f, 1.0f));
	glm::vec3 newUp = glm::normalize(glm::cross(newRight, newForward));

	m_rotate[0] = glm::vec4(newRight, 0.0f);
	m_rotate[1] = glm::vec4(newForward, 0.0f);
	m_rotate[2] = glm::vec4(newUp, 0.0f);

	glm::quat quat = glm::toQuat(m_rotate);

	m_rotation = eulerAngles(quat);
	m_updated = false;
	return this;
#else
	// z forward, x right, y up

	glm::vec3 newForward = glm::normalize(direction);
	glm::vec3 newRight = glm::cross(newForward, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::vec3 newUp = glm::normalize(glm::cross(newRight, newForward));

	m_rotate[0] = glm::vec4(newRight, 0.0f);
	m_rotate[1] = glm::vec4(newUp, 0.0f);
	m_rotate[2] = glm::vec4(newForward, 0.0f);

	glm::quat quat = glm::toQuat(m_rotate);

	m_rotation = eulerAngles(quat);
	m_updated = false;
	return this;
#endif
}

Transform * Transform::setTranslationMatrix(const glm::mat4 & translation)
{
	m_translate = translation;
	m_position = translation[3];
	m_updated = false;
	return this;
}

Transform * Transform::setRotationMatrix(const glm::mat4 & rotation)
{
	m_rotate = rotation;
	glm::quat quat = glm::toQuat(m_rotate);
	m_rotation = eulerAngles(quat);
	m_updated = false;
	return this;
}

Transform * Transform::setScaleMatrix(const glm::mat4 & scale)
{
	m_scaling = scale;
	m_scale = glm::vec3(m_scaling[0][0], m_scaling[1][1], m_scaling[2][2]);
	m_updated = false;
	return this;
}

Transform Transform::operator*(const Transform & right)
{
	Transform ret;
	ret.m_transform = m_transform * right.m_transform;
	ret.m_translate = m_translate * right.m_translate;
	ret.m_rotate = m_rotate * right.m_rotate;

	ret.m_scaling = m_scaling * right.m_scaling;

	//recalculate euler angles
	glm::quat quat = glm::toQuat(ret.m_rotate);
	ret.m_rotation = eulerAngles(quat);

	ret.m_position = m_position + right.m_position;
	ret.m_scale = m_scale * right.m_scale;

	ret.m_updated = false;

	return ret;
}

void Transformable::update()
{
	Transform::update();
}
