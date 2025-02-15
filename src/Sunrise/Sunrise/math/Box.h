#pragma once

#include "srpch.h"

namespace sunrise::math {

	/// <summary>
	/// a 2d representation of a start and end point 
	/// </summary>
	struct SUNRISE_API Box
	{
		Box();
		Box(glm::dvec2 start, glm::dvec2 size);

		glm::dvec2 start, size = glm::dvec3(0);

		glm::dvec2 getEnd() const;
		glm::dvec2 getCenter() const;

		bool contains(glm::dvec2 point) const;
		bool containsAny(std::vector<glm::dvec2>& points) const;
		bool containsAny(std::vector<glm::dvec2>&& points) const;
		bool overlaps(Box other) const;
		//bool contains(Box other);

		std::vector<glm::dvec2> polygon() const;

		std::string toString() const;

		//static bool operator<(const Box& a, const Box& b) {
		//	return 
		//		(a.start.x < b.start.x && a.start.y < b.start.y)
		//		&&
		//		(a.size.x < b.size.x && a.size.y < b.size.y)
		//};

	};

	SUNRISE_API std::ostream& operator<<(std::ostream& strm, const Box& a);


}