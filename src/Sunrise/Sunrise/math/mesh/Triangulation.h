#pragma once

#include "srpch.h"


#include "Mesh.h"
#include "../Box.h"

//#include "boost/polygon/polygon.hpp"
//#include "boost/geometry/geometry.hpp"
#include <cassert>
//
//namespace gtl = boost::polygon;
//namespace bgm = boost::geometry;

namespace sunrise::math::mesh
{
	typedef std::vector<glm::dvec2> Polygon2D;
	typedef std::vector<Polygon2D> HPolygon2D;

	//// boost poly
	//typedef   boost::geometry::model::d2::point_xy<double> bPoint;
	//// booleans mean ccw and open in order
	//typedef   boost::geometry::model::polygon<bPoint, false, false> bPolygon;
	//typedef   boost::geometry::model::multi_polygon<bPolygon> bMultiPolygon;

	////typedef gtl::polygon_data<double> bPolygon;
	////typedef gtl::polygon_traits<bPolygon>::point_type bPoint;

	//SUNRISE_API bMultiPolygon boostFromMesh(Polygon2D p);
	//SUNRISE_API Polygon2D meshFromBoost(bMultiPolygon p);

	SUNRISE_API Polygon2D bunion(Polygon2D p1, Polygon2D p2);

	SUNRISE_API Polygon2D binterseciton(Polygon2D p1, Polygon2D p2);
	
	SUNRISE_API bool bDoIntersect(Polygon2D p1, Polygon2D p2);
	
	SUNRISE_API Polygon2D bDifference(Polygon2D p1, Polygon2D p2);


	SUNRISE_API Polygon2D bunionSMDifference(Polygon2D p1, Polygon2D p2);
	

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


	SUNRISE_API std::vector<std::vector<glm::dvec2>> differenceBetween(const std::vector<glm::dvec2>& polygon1, const std::vector<glm::dvec2>& polygon2);
	SUNRISE_API std::vector<std::vector<glm::dvec2>> symmetricDifferenceBetween(const std::vector<glm::dvec2>& polygon1, const std::vector<glm::dvec2>& polygon2);
	SUNRISE_API std::vector<std::vector<glm::dvec2>> intersectionOf(const std::vector<glm::dvec2>& polygon1, const std::vector<glm::dvec2>& polygon2);
	SUNRISE_API std::vector<std::vector<glm::dvec2>> unionOf(const std::vector<glm::dvec2>& polygon1,const std::vector<glm::dvec2>& polygon2);

	SUNRISE_API bool overlap(const std::vector<glm::dvec2>& polygon1, const std::vector<glm::dvec2>& polygon2);

	/// <summary>
	/// if the polygon passed in has the first point repeated as the last point this will remove it
	/// </summary>
	/// <param name="polygon"></param>
	/// <returns></returns>
	SUNRISE_API void makeOpenIfClosedForCGAL(std::vector<glm::dvec2>& polygon);
	
	SUNRISE_API Box bounds(std::vector<glm::dvec2>& points);




};

