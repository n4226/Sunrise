#pragma once

#include "srpch.h"


#include "Mesh.h"
#include "../Box.h"

namespace sunrise::math::mesh
{

	struct SUNRISE_API TriangulatedMesh
	{
		std::vector<glm::dvec2> verts;
		std::vector<std::vector<glm::uint32_t>> indicies;
	};

	/// <summary>
	/// array of ppolygons that make up shape
	/// first polygon is base shape 
	/// all other polygons are holes 
	/// 
	/// the polygon returned is the pol.ygon made up of the first polygon points given
	/// </summary>
	SUNRISE_API std::pair<TriangulatedMesh*, bool> triangulate(std::vector<std::vector<glm::dvec2>>& polygon);

	SUNRISE_API std::vector<std::vector<glm::dvec2>> intersectionOf(const std::vector<glm::dvec2>& polygon1,const std::vector<glm::dvec2>& polygon2);

	/// <summary>
	/// if the polygon passed in has the first point repeated as the last point this will remove it
	/// </summary>
	/// <param name="polygon"></param>
	/// <returns></returns>
	SUNRISE_API void makeOpenIfClosedForCGAL(std::vector<glm::dvec2>& polygon);
	
	SUNRISE_API Box bounds(std::vector<glm::dvec2>& points);



};

