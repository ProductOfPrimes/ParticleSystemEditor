// Authored By: Tom Tsiliopoulos - 100616336
// Authored By: Joss Moo-Young - 100586602

#pragma once

#include <boost/serialization/access.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/version.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <GLM/glm/vec2.hpp>
#include <GLM/glm/vec3.hpp>
#include <GLM/glm/vec4.hpp>
#include <GLM/glm/mat4x4.hpp>

namespace boost {
	namespace serialization {

		template<class Archive>
		void serialize(Archive & ar, glm::vec4& vec, const unsigned int version)
		{
			ar & boost::serialization::make_nvp("x", vec.x);
			ar & boost::serialization::make_nvp("y", vec.y);
			ar & boost::serialization::make_nvp("z", vec.z);
			ar & boost::serialization::make_nvp("w", vec.w);
		}

		template<class Archive>
		void serialize(Archive & ar, glm::vec3& vec, const unsigned int version)
		{
			ar & boost::serialization::make_nvp("x", vec.x);
			ar & boost::serialization::make_nvp("y", vec.y);
			ar & boost::serialization::make_nvp("z", vec.z);
		}

		template<class Archive>
		void serialize(Archive & ar, glm::vec2& vec, const unsigned int version)
		{
			ar & boost::serialization::make_nvp("x", vec.x);
			ar & boost::serialization::make_nvp("y", vec.y);
		}


		template<class Archive>
		void serialize(Archive & ar, glm::mat4& mat, const unsigned int version)
		{
			ar & mat[0];
			ar & mat[1];
			ar & mat[2];
			ar & mat[3];
		}

	}
}
