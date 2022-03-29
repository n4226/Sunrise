#include "srpch.h"

//MARK: for earcut to work with glm


#include "Triangulation.h"

#include <mapbox/earcut.hpp>

namespace mapbox {
	namespace util {

		template <>
		struct nth<0, glm::dvec2> {
			inline static auto get(const glm::dvec2& t) {
				return t.x;
			};
		};
		template <>
		struct nth<1, glm::dvec2> {
			inline static auto get(const glm::dvec2& t) {
				return t.y;
			};
		};
	}
}

// until fix of triangulation with new lib
#if 0 // now using Geos of booean ops and earcut for triangulation
//#include <CGAL/draw_triangulation_2.h>
//CGAL_USE_BASIC_VIEWER

//#include "igl/triangle/triangulate.h"

//#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_2.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
//
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Triangulation_face_base_with_info_2.h>
#include <CGAL/Polygon_2.h>
#include <iostream>	
#include <limits>
	

#include <CGAL/Boolean_set_operations_2.h>
	

//using namespace boost::polygon::operators;

//TODO ----------------------------------------------------------------------------------- CGAL WILL HAVE TO BE LICENSED FOR PROFIT ----------------------------------------------------------------------------------------------------------

namespace sunrise::math::mesh {



	//	// must be ccw
	//bMultiPolygon boostFromMesh(Polygon2D p) {

	//	std::vector<bPoint> points = {};

	//	for (size_t i = 0; i < p.size(); i++)
	//	{
	//		points.push_back(bPoint(p[i].x, p[i].y));
	//	}

	//	bPolygon polygon;

	//	bgm::assign_points(polygon, points);
	// 
	//	return { polygon };
	//}

	//	// must be ccw
	//Polygon2D meshFromBoost(bMultiPolygon p) {
	//	Polygon2D poly;

	//	bgm::for_each_point(p, [&poly](bPoint point) {
	//		poly.push_back(glm::dvec2(point.x(), point.y()));
	//	});
	//	
	//	/*for each (auto point in p.)
	//	{
	//		poly.push_back(glm::dvec2(point.x(), point.y()));
	//	}*/

	//	return poly;
	//}

	//Polygon2D bunion(Polygon2D p1, Polygon2D p2) {
	//	auto bp1 = boostFromMesh(p1);
	//	auto bp2 = boostFromMesh(p2);

	//	bMultiPolygon result;

	//	bgm::union_(bp1, bp2, result);

	//	return meshFromBoost(result);
	//}
	//

	//Polygon2D binterseciton(Polygon2D p1, Polygon2D p2) {
	//	auto bp1 = boostFromMesh(p1);
	//	auto bp2 = boostFromMesh(p2);

	//	bMultiPolygon result;

	//	boost::geometry::intersection(bp1, bp2, result);

	//	return meshFromBoost(result);
	//}

	//bool bDoIntersect(Polygon2D p1, Polygon2D p2) {
	//	auto bp1 = boostFromMesh(p1);
	//	auto bp2 = boostFromMesh(p2);

	//	return boost::geometry::overlaps(bp1, bp2);
	//}

	//Polygon2D bDifference(Polygon2D p1, Polygon2D p2) {
	//	auto bp1 = boostFromMesh(p1);
	//	auto bp2 = boostFromMesh(p2);

	//	bMultiPolygon result;

	//	boost::geometry::difference(bp1, bp2, result);

	//	return meshFromBoost(result);
	//}

	//Polygon2D bunionSMDifference(Polygon2D p1, Polygon2D p2) {
	//	auto bp1 = boostFromMesh(p1);
	//	auto bp2 = boostFromMesh(p2);

	//	bMultiPolygon result;

	//	boost::geometry::sym_difference(bp1, bp2, result);

	//	return meshFromBoost(result);
	//}

	struct FaceInfo2
	{
		FaceInfo2() {}
		int nesting_level;
		bool in_domain() {
			return nesting_level % 2 == 1;
		}
	};


	typedef CGAL::Exact_predicates_inexact_constructions_kernel       K;
	//typedef CGAL::Exact_predicates_exact_constructions_kernel       K;
	typedef CGAL::Triangulation_vertex_base_2<K>                      Vb;
	typedef CGAL::Triangulation_face_base_with_info_2<FaceInfo2, K>    Fbb;
	typedef CGAL::Constrained_triangulation_face_base_2<K, Fbb>        Fb;
	typedef CGAL::Triangulation_data_structure_2<Vb, Fb>               TDS;
	typedef CGAL::Exact_predicates_tag                                Itag;
	typedef CGAL::Constrained_Delaunay_triangulation_2<K, TDS, Itag>  CDT;
	typedef CDT::Point                                                Point;
	typedef CGAL::Polygon_2<K>                                        Polygon_2;
	typedef CGAL::Polygon_with_holes_2<K>                             Polygon_with_holes_2;
	typedef CDT::Face_handle                                          Face_handle;
	typedef std::list<Polygon_with_holes_2>                           Pwh_list_2;




	
	static void 
		mark_domains(CDT& ct,
			Face_handle start,
			int index,
			std::list<CDT::Edge>& border)
	{
		if (start->info().nesting_level != -1) {
			return;
		}
		std::list<Face_handle> queue;
		queue.push_back(start);
		while (!queue.empty()) {
			Face_handle fh = queue.front();
			queue.pop_front();
			if (fh->info().nesting_level == -1) {
				fh->info().nesting_level = index;
				for (int i = 0; i < 3; i++) {
					CDT::Edge e(fh, i);
					Face_handle n = fh->neighbor(i);
					if (n->info().nesting_level == -1) {
						if (ct.is_constrained(e)) border.push_back(e);
						else queue.push_back(n);
					}
				}
			}
		}
	}
	//explore set of facets connected with non constrained edges,
	//and attribute to each such set a nesting level.
	//We start from facets incident to the infinite vertex, with a nesting
	//level of 0. Then we recursively consider the non-explored facets incident
	//to constrained edges bounding the former set and increase the nesting level by 1.
	//Facets in the domain are those with an odd nesting level.
	static void
		mark_domains(CDT& cdt)
	{
		for (CDT::Face_handle f : cdt.all_face_handles()) {
			f->info().nesting_level = -1;
		}
		std::list<CDT::Edge> border;
		mark_domains(cdt, cdt.infinite_face(), 0, border);
		while (!border.empty()) {
			CDT::Edge e = border.front();
			border.pop_front();
			Face_handle n = e.first->neighbor(e.second);
			if (n->info().nesting_level == -1) {
				mark_domains(cdt, n, e.first->info().nesting_level + 1, border);
			}
		}
	}

	// poly gons should not have the start and end point be the same eg start and end should be different will close the path automatically
	//TODO - fix to export points in double not float
	std::pair<TriangulatedMesh*,bool> triangulate(std::vector<std::vector<glm::dvec2>>& polygon)
	{
		// see : https://doc.cgal.org/latest/Triangulation_2/index.html#title30
			// - File Triangulation_2 / polygon_triangulation.cpp

		auto mesh = new TriangulatedMesh{};


		std::vector<Polygon_2> cg_polygons = {};





		//std::vector<std::pair<Point,uint32_t>> points = {};
		//std::vector<Point> points = {};
		cg_polygons.resize(polygon.size());

		for (size_t i = 0; i < polygon.size(); i++)
		{
			for (size_t p = 0; p < polygon[i].size(); p++)
			{
				auto point = polygon[i][p];
				cg_polygons[i].push_back(Point(point.x, point.y));
			}//);
		}


		//Insert the polygons into a constrained triangulation
		CDT cdt;
		for (size_t i = 0; i < polygon.size(); i++)
			cdt.insert_constraint(cg_polygons[i].vertices_begin(), cg_polygons[i].vertices_end(), true);


		//Mark facets that are inside the domain bounded by the polygon
		mark_domains(cdt);

		std::set<Point> newVerts = {};

		for (Face_handle f : cdt.finite_face_handles())
		{
			if (f->info().in_domain()) {
				newVerts.insert(f->vertex(0)->point());
				newVerts.insert(f->vertex(1)->point());
				newVerts.insert(f->vertex(2)->point());
			}
		}

		for (auto& point : newVerts) {
			glm::float64 px = point.x();//.get_relative_precision_of_to_double();
			glm::float64 py = point.y();//.get_relative_precision_of_to_double();
			//glm::float64 px = to_double(point.x());
			//glm::float64 py = to_double(point.y());
			mesh->verts.emplace_back(px,py);
		}


		mesh->indicies.push_back({});

		for (Face_handle f : cdt.finite_face_handles())
		{
			if (f->info().in_domain()) {
				auto vPos1 = newVerts.find(f->vertex(0)->point());
				if (vPos1 == newVerts.end()) continue;
				auto index1 = std::distance(newVerts.begin(), vPos1);

				auto vPos2 = newVerts.find(f->vertex(1)->point());
				if (vPos2 == newVerts.end()) continue;
				auto index2 = std::distance(newVerts.begin(), vPos2);

				auto vPos3 = newVerts.find(f->vertex(2)->point());
				if (vPos3 == newVerts.end()) continue;
				auto index3 = std::distance(newVerts.begin(), vPos3);

				mesh->indicies[mesh->indicies.size() - 1].push_back(index1);
				mesh->indicies[mesh->indicies.size() - 1].push_back(index2);
				mesh->indicies[mesh->indicies.size() - 1].push_back(index3);
			}
		}

		bool isCLock = false;

		if (cg_polygons[0].is_simple())
			isCLock = cg_polygons[0].is_clockwise_oriented();

		return { mesh, isCLock };

	}

	std::vector<std::vector<glm::dvec2>> differenceBetween(const std::vector<glm::dvec2>& _polygon1, const std::vector<glm::dvec2>& _polygon2)
	{
		typedef CGAL::Exact_predicates_exact_constructions_kernel      K;
		typedef CGAL::Triangulation_vertex_base_2<K>                      Vb;
		typedef CGAL::Triangulation_face_base_with_info_2<FaceInfo2, K>    Fbb;
		typedef CGAL::Constrained_triangulation_face_base_2<K, Fbb>        Fb;
		typedef CGAL::Triangulation_data_structure_2<Vb, Fb>               TDS;
		typedef CGAL::Exact_predicates_tag                                Itag;
		typedef CGAL::Constrained_Delaunay_triangulation_2<K, TDS, Itag>  CDT;
		typedef CDT::Point                                                Point;
		typedef CGAL::Polygon_2<K>                                        Polygon_2;
		typedef CGAL::Polygon_with_holes_2<K>                             Polygon_with_holes_2;
		typedef CDT::Face_handle                                          Face_handle;
		typedef std::list<Polygon_with_holes_2>                           Pwh_list_2;

		auto polygon1 = _polygon1;
		auto polygon2 = _polygon2;

		makeOpenIfClosedForCGAL(polygon1);
		makeOpenIfClosedForCGAL(polygon2);

		Polygon_2 p1;
		Polygon_2 p2;


		p1.resize(polygon1.size());
		p2.resize(polygon2.size());

		for (size_t i = 0; i < polygon1.size(); i++)
		{
			p1[i] = Point(polygon1[i].x, polygon1[i].y);
		}
		for (size_t i = 0; i < polygon2.size(); i++)
		{
			p2[i] = Point(polygon2[i].x, polygon2[i].y);
		}

		auto ds = p1.is_convex();
		auto ds2 = p2.is_simple();

		auto as = p1.is_convex();
		auto as2 = p2.is_simple();
		SR_ASSERT(p1.is_clockwise_oriented() == p2.is_clockwise_oriented());
		SR_CORE_TRACE("difference of two {} oriented polygons (true == clockwise)", p1.is_clockwise_oriented());

		if (p1.is_clockwise_oriented()) {
			p1.reverse_orientation();
			p2.reverse_orientation();
		}

		// Compute the intersection of P and Q.
		Polygon_with_holes_2                  UR;
		std::vector<Polygon_with_holes_2>                  intR;


		if (!CGAL::join(p1, p2, UR)) {
			// difference will also be trivial
			return { polygon1 };
		}


		// this function require4s both polygons to be counter clockwise oriented (may be a bug)
		CGAL::difference(p1, p2, std::back_inserter(intR));
		/*CGAL::join(p1, p2, UR);
		intR.resize(UR.size());
		intR.assign(UR.begin(),UR.end());*/

		auto outPolygon = std::vector<std::vector<glm::dvec2>>();

		if (intR.size() == 0)
			return outPolygon;

		if (intR[0].is_unbounded()) {
			throw std::runtime_error("not supported yet");
		}

		outPolygon.push_back({});

		for (auto vert = intR[0].outer_boundary().vertices_begin(); vert != intR[0].outer_boundary().vertices_end(); vert = std::next(vert)) {
			glm::float64 px = to_double(vert->x());
			glm::float64 py = to_double(vert->y());
			outPolygon[0].emplace_back(px, py);

			//outPolygon[0].emplace_back(vert->x(),vert->y());
		}

		for (auto hit = intR[0].holes_begin(); hit != intR[0].holes_end(); ++hit) {

		}

		return outPolygon;

	}

	std::vector<std::vector<glm::dvec2>> symmetricDifferenceBetween(const std::vector<glm::dvec2>& _polygon1, const std::vector<glm::dvec2>& _polygon2)
	{
		typedef CGAL::Exact_predicates_exact_constructions_kernel      K;
		typedef CGAL::Triangulation_vertex_base_2<K>                      Vb;
		typedef CGAL::Triangulation_face_base_with_info_2<FaceInfo2, K>    Fbb;
		typedef CGAL::Constrained_triangulation_face_base_2<K, Fbb>        Fb;
		typedef CGAL::Triangulation_data_structure_2<Vb, Fb>               TDS;
		typedef CGAL::Exact_predicates_tag                                Itag;
		typedef CGAL::Constrained_Delaunay_triangulation_2<K, TDS, Itag>  CDT;
		typedef CDT::Point                                                Point;
		typedef CGAL::Polygon_2<K>                                        Polygon_2;
		typedef CGAL::Polygon_with_holes_2<K>                             Polygon_with_holes_2;
		typedef CDT::Face_handle                                          Face_handle;
		typedef std::list<Polygon_with_holes_2>                           Pwh_list_2;

		auto polygon1 = _polygon1;
		auto polygon2 = _polygon2;

		makeOpenIfClosedForCGAL(polygon1);
		makeOpenIfClosedForCGAL(polygon2);

		Polygon_2 p1;
		Polygon_2 p2;


		p1.resize(polygon1.size());
		p2.resize(polygon2.size());

		for (size_t i = 0; i < polygon1.size(); i++)
		{
			p1[i] = Point(polygon1[i].x, polygon1[i].y);
		}
		for (size_t i = 0; i < polygon2.size(); i++)
		{
			p2[i] = Point(polygon2[i].x, polygon2[i].y);
		}

		auto ds = p1.is_convex();
		auto ds2 = p2.is_simple();

		auto as = p1.is_convex();
		auto as2 = p2.is_simple();
		SR_ASSERT(p1.is_clockwise_oriented() == p2.is_clockwise_oriented());
		SR_CORE_TRACE("sym difference of two {} oriented polygons (true == clockwise)", p1.is_clockwise_oriented());

		if (p1.is_clockwise_oriented()) {
			p1.reverse_orientation();
			p2.reverse_orientation();
		}

		// Compute the intersection of P and Q.
		//Pwh_list_2                  UR;
		std::vector<Polygon_with_holes_2>                  intR;



		// this function require4s both polygons to be counter clockwise oriented (may be a bug)
		CGAL::symmetric_difference(p1, p2, std::back_inserter(intR));
		/*CGAL::join(p1, p2, UR);
		intR.resize(UR.size());
		intR.assign(UR.begin(),UR.end());*/

		auto outPolygon = std::vector<std::vector<glm::dvec2>>();

		if (intR.size() == 0)
			return outPolygon;

		if (intR[0].is_unbounded()) {
			throw std::runtime_error("not supported yet");
		}

		outPolygon.push_back({});

		for (auto vert = intR[0].outer_boundary().vertices_begin(); vert != intR[0].outer_boundary().vertices_end(); vert = std::next(vert)) {
			glm::float64 px = to_double(vert->x());
			glm::float64 py = to_double(vert->y());
			outPolygon[0].emplace_back(px, py);

			//outPolygon[0].emplace_back(vert->x(),vert->y());
		}

		for (auto hit = intR[0].holes_begin(); hit != intR[0].holes_end(); ++hit) {

		}

		return outPolygon;
	}

	/// <summary>
	/// both must be the same orentation e.g. either clockwise or coutner clockwise
	/// </summary>
	/// <param name="_polygon1"></param>
	/// <param name="_polygon2"></param>
	/// <returns></returns>
	std::vector<std::vector<glm::dvec2>> intersectionOf(const std::vector<glm::dvec2>& _polygon1,const std::vector<glm::dvec2>& _polygon2)
	{
		//https://doc.cgal.org/latest/Algebraic_foundations/group__PkgAlgebraicFoundationsRef.html#ga1f1bcd74fce34fd532445590bbda5cd5
		//typedef CGAL::Exact_predicates_inexact_constructions_kernel       K;
		typedef CGAL::Exact_predicates_exact_constructions_kernel      K;
		typedef CGAL::Triangulation_vertex_base_2<K>                      Vb;
		typedef CGAL::Triangulation_face_base_with_info_2<FaceInfo2, K>    Fbb;
		typedef CGAL::Constrained_triangulation_face_base_2<K, Fbb>        Fb;
		typedef CGAL::Triangulation_data_structure_2<Vb, Fb>               TDS;
		typedef CGAL::Exact_predicates_tag                                Itag;
		typedef CGAL::Constrained_Delaunay_triangulation_2<K, TDS, Itag>  CDT;
		typedef CDT::Point                                                Point;
		typedef CGAL::Polygon_2<K>                                        Polygon_2;
		typedef CGAL::Polygon_with_holes_2<K>                             Polygon_with_holes_2;
		typedef CDT::Face_handle                                          Face_handle;
		typedef std::list<Polygon_with_holes_2>                           Pwh_list_2;
		


		auto polygon1 = _polygon1;
		auto polygon2 = _polygon2;
		
		makeOpenIfClosedForCGAL(polygon1);
		makeOpenIfClosedForCGAL(polygon2);

		Polygon_2 p1;
		Polygon_2 p2;


		p1.resize(polygon1.size());
		p2.resize(polygon2.size());

		for (size_t i = 0; i < polygon1.size(); i++)
		{
			p1[i] = Point(polygon1[i].x, polygon1[i].y);
		}
		for (size_t i = 0; i < polygon2.size(); i++)
		{
			p2[i] = Point(polygon2[i].x, polygon2[i].y);
		}

		auto ds  = p1.is_convex();
		auto ds2 = p2.is_simple();

		auto as  = p1.is_convex();
		auto as2 = p2.is_simple();
		SR_ASSERT(p1.is_clockwise_oriented() == p2.is_clockwise_oriented());
		SR_CORE_TRACE("Intersection of two {} oriented polygons (true == clockwise)",p1.is_clockwise_oriented());

		if (p1.is_clockwise_oriented()) {
			p1.reverse_orientation();
			p2.reverse_orientation();
		}

		// Compute the intersection of P and Q.
		//Pwh_list_2                  UR;
		std::vector<Polygon_with_holes_2>                  intR;



		// this function require4s both polygons to be counter clockwise oriented (may be a bug)
		CGAL::intersection(p1, p2, std::back_inserter(intR));
		/*CGAL::join(p1, p2, UR);
		intR.resize(UR.size());
		intR.assign(UR.begin(),UR.end());*/

		auto outPolygon = std::vector<std::vector<glm::dvec2>>();

		if (intR.size() == 0)
			return outPolygon;

		if (intR[0].is_unbounded()) {
			throw std::runtime_error("not supported yet");
		}

		outPolygon.push_back({});
		
		for (auto vert = intR[0].outer_boundary().vertices_begin(); vert != intR[0].outer_boundary().vertices_end(); vert = std::next(vert)) {
			glm::float64 px = to_double(vert->x());
			glm::float64 py = to_double(vert->y());
			outPolygon[0].emplace_back(px, py);
			
			//outPolygon[0].emplace_back(vert->x(),vert->y());
		}

		for (auto hit = intR[0].holes_begin(); hit != intR[0].holes_end(); ++hit) {

		}

		return outPolygon;
	}

	std::vector<std::vector<glm::dvec2>> unionOf(const std::vector<glm::dvec2>& _polygon1, const std::vector<glm::dvec2>& _polygon2)
	{
		if (_polygon1.size() == 0)
			return { _polygon2 };


		if (_polygon2.size() == 0)
			return { _polygon1 };

		//https://doc.cgal.org/latest/Algebraic_foundations/group__PkgAlgebraicFoundationsRef.html#ga1f1bcd74fce34fd532445590bbda5cd5
		//typedef CGAL::Exact_predicates_inexact_constructions_kernel       K;
		typedef CGAL::Exact_predicates_exact_constructions_kernel      K;
		typedef CGAL::Triangulation_vertex_base_2<K>                      Vb;
		typedef CGAL::Triangulation_face_base_with_info_2<FaceInfo2, K>    Fbb;
		typedef CGAL::Constrained_triangulation_face_base_2<K, Fbb>        Fb;
		typedef CGAL::Triangulation_data_structure_2<Vb, Fb>               TDS;
		typedef CGAL::Exact_predicates_tag                                Itag;
		typedef CGAL::Constrained_Delaunay_triangulation_2<K, TDS, Itag>  CDT;
		typedef CDT::Point                                                Point;
		typedef CGAL::Polygon_2<K>                                        Polygon_2;
		typedef CGAL::Polygon_with_holes_2<K>                             Polygon_with_holes_2;
		typedef CDT::Face_handle                                          Face_handle;
		typedef std::list<Polygon_with_holes_2>                           Pwh_list_2;



		auto polygon1 = _polygon1;
		auto polygon2 = _polygon2;

		makeOpenIfClosedForCGAL(polygon1);
		makeOpenIfClosedForCGAL(polygon2);

		Polygon_2 p1;
		Polygon_2 p2;


		p1.resize(polygon1.size());
		p2.resize(polygon2.size());

		for (size_t i = 0; i < polygon1.size(); i++)
		{
			p1[i] = Point(polygon1[i].x, polygon1[i].y);
		}
		for (size_t i = 0; i < polygon2.size(); i++)
		{
			p2[i] = Point(polygon2[i].x, polygon2[i].y);
		}

		auto ds = p1.is_convex();
		auto ds2 = p2.is_simple();

		auto as = p1.is_convex();
		auto as2 = p2.is_simple();
		SR_ASSERT(p1.is_clockwise_oriented() == p2.is_clockwise_oriented());
		SR_CORE_TRACE("union of two {} oriented polygons (true == clockwise)", p1.is_clockwise_oriented());

		if (p1.is_clockwise_oriented()) {
			p1.reverse_orientation();
			p2.reverse_orientation();
		}

		// Compute the intersection of P and Q.
		Polygon_with_holes_2                  UR;
		std::vector<Polygon_with_holes_2>                  intR;



		// this function require4s both polygons to be counter clockwise oriented (may be a bug)
		//CGAL::join(p1, p2, intR);
		if (!CGAL::join(p1, p2, UR)) {
			// union is jsut the two polygons
			return { polygon1, polygon2 };
		}
		/*intR.resize(UR.size());
		intR.assign(UR.begin(),UR.end());*/

		intR.push_back(UR);

		auto outPolygon = std::vector<std::vector<glm::dvec2>>();



		if (intR.size() == 0)
			return outPolygon;

		if (intR[0].is_unbounded()) {
			throw std::runtime_error("not supported yet");
		}

		outPolygon.push_back({});

		for (auto vert = intR[0].outer_boundary().vertices_begin(); vert != intR[0].outer_boundary().vertices_end(); vert = std::next(vert)) {
			glm::float64 px = to_double(vert->x());
			glm::float64 py = to_double(vert->y());
			outPolygon[0].emplace_back(px, py);

			//outPolygon[0].emplace_back(vert->x(),vert->y());
		}

		for (auto hit = intR[0].holes_begin(); hit != intR[0].holes_end(); ++hit) {

		}

		return outPolygon;
	}

	bool pointInPolygon(glm::dvec2 point, Polygon2D polygon) {
		auto points = polygon;
		int i, j, nvert = polygon.size();
		bool c = false;

		for (i = 0, j = nvert - 1; i < nvert; j = i++) {
			if (((points[i].y >= point.y) != (points[j].y >= point.y)) &&
				(point.x <= (points[j].x - points[i].x) * (point.y - points[i].y) / (points[j].y - points[i].y) + points[i].x)
				)
				c = !c;
		}

		return c;
	}

	bool overlap(const std::vector<glm::dvec2>& _polygon1, const std::vector<glm::dvec2>& _polygon2)
	{

		


		//https://doc.cgal.org/latest/Algebraic_foundations/group__PkgAlgebraicFoundationsRef.html#ga1f1bcd74fce34fd532445590bbda5cd5
		//typedef CGAL::Exact_predicates_inexact_constructions_kernel       K;
		typedef CGAL::Exact_predicates_exact_constructions_kernel      K;
		typedef CGAL::Triangulation_vertex_base_2<K>                      Vb;
		typedef CGAL::Triangulation_face_base_with_info_2<FaceInfo2, K>    Fbb;
		typedef CGAL::Constrained_triangulation_face_base_2<K, Fbb>        Fb;
		typedef CGAL::Triangulation_data_structure_2<Vb, Fb>               TDS;
		typedef CGAL::Exact_predicates_tag                                Itag;
		typedef CGAL::Constrained_Delaunay_triangulation_2<K, TDS, Itag>  CDT;
		typedef CDT::Point                                                Point;
		typedef CGAL::Polygon_2<K>                                        Polygon_2;
		typedef CGAL::Polygon_with_holes_2<K>                             Polygon_with_holes_2;
		typedef CDT::Face_handle                                          Face_handle;
		typedef std::list<Polygon_with_holes_2>                           Pwh_list_2;



		auto polygon1 = _polygon1;
		auto polygon2 = _polygon2;

		makeOpenIfClosedForCGAL(polygon1);
		makeOpenIfClosedForCGAL(polygon2);


		//overide without cgal

		for (size_t i = 0; i < polygon1.size(); i++)
		{
			if (pointInPolygon(polygon1[i], polygon2)) {
				return true;
			}
		}
		return false;
		// exdone

		Polygon_2 p1;
		Polygon_2 p2;


		p1.resize(polygon1.size());
		p2.resize(polygon2.size());

		for (size_t i = 0; i < polygon1.size(); i++)
		{
			p1[i] = Point(polygon1[i].x, polygon1[i].y);
		}
		for (size_t i = 0; i < polygon2.size(); i++)
		{
			p2[i] = Point(polygon2[i].x, polygon2[i].y);
		}

		auto ds = p1.is_convex();
		auto ds2 = p2.is_simple();

		auto as = p1.is_convex();
		auto as2 = p2.is_simple();
		SR_ASSERT(p1.is_clockwise_oriented() == p2.is_clockwise_oriented());
		SR_CORE_TRACE("do intersect test of two {} oriented polygons (true == clockwise)", p1.is_clockwise_oriented());

		if (p1.is_clockwise_oriented()) {
			p1.reverse_orientation();
			p2.reverse_orientation();
		}

		//return CGAL::do_intersect(p1, p2);

		Polygon_with_holes_2                  UR;

		return CGAL::join(p1, p2, UR);
	}

	void makeOpenIfClosedForCGAL(std::vector<glm::dvec2>& polygon)
	{
		if (polygon.size() == 0) return;

		if (polygon[polygon.size() - 1] == polygon[0])
			polygon.pop_back();
	}

	Box bounds(std::vector<glm::dvec2>& points)
	{
		glm::dvec2 min = glm::dvec2(std::numeric_limits<double>::max()); //glm::dvec2(90, 180);
		glm::dvec2 max = glm::dvec2(-std::numeric_limits<double>::max()); //glm::dvec2(-90, -180);

		for (auto& pos : points) {
			if (pos.x < min.x)
				min.x = pos.x;
			if (pos.y < min.y)
				min.y = pos.y;

			if (pos.x > max.x)
				max.x = pos.x;
			if (pos.y > max.y)
				max.y = pos.y;
		}

		SR_CORE_ASSERT(min != glm::dvec2(std::numeric_limits<double>::max()) && max != glm::dvec2(-std::numeric_limits<double>::max()));

		auto result = Box(min, max - min);

#if SR_ENABLE_PRECONDITION_CHECKS

		for (auto& point : points) {
			SR_ASSERT(result.contains(point));
		}

#endif

		return result;
	}



	




}
#else
#include <geos_c.h>

namespace sunrise::math::mesh {

	Box bounds(std::vector<glm::dvec2>& points)
	{
		glm::dvec2 min = glm::dvec2(std::numeric_limits<double>::max()); //glm::dvec2(90, 180);
		glm::dvec2 max = glm::dvec2(-std::numeric_limits<double>::max()); //glm::dvec2(-90, -180);

		for (auto& pos : points) {
			if (pos.x < min.x)
				min.x = pos.x;
			if (pos.y < min.y)
				min.y = pos.y;

			if (pos.x > max.x)
				max.x = pos.x;
			if (pos.y > max.y)
				max.y = pos.y;
		}

		SR_CORE_ASSERT(points.size() == 0 || (min != glm::dvec2(std::numeric_limits<double>::max()) && max != glm::dvec2(-std::numeric_limits<double>::max())));

		auto result = Box(min, max - min);

#if SR_ENABLE_PRECONDITION_CHECKS

		for (auto& point : points) {
			SR_ASSERT(result.contains(point));
		}

#endif

		return result;
	}


	void makeOpenIfClosedForCGAL(Polygon2D& polygon)
	{
		if (polygon.size() == 0) return;

		if (polygon[polygon.size() - 1] == polygon[0])
			polygon.pop_back();
	}

	
	//geos functions
	static void
		geos_msg_handler(const char* fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
		vprintf(fmt, ap);
		va_end(ap);
	}
	
	GEOSGeometry* SRToGEOS(const Polygon2D& p1) {
		SR_CORE_ASSERT(p1.size() > 0);
		//make open if closed
		auto count = p1.size();
		if (p1[p1.size() - 1] == p1[0]) {
			count -= 1;
		}
		auto coords = GEOSCoordSeq_create(count + 1, 2);


		for (size_t i = 0; i < count; i++)
		{
			GEOSCoordSeq_setOrdinate(coords, i, 0, p1[i].x);
			GEOSCoordSeq_setOrdinate(coords, i, 1, p1[i].y);
		}

		//TODO: asuming open polygo is given
		//if (p1.size() > 0) {
			GEOSCoordSeq_setOrdinate(coords, count, 0, p1[0].x);
			GEOSCoordSeq_setOrdinate(coords, count, 1, p1[0].y);
		//}

		auto ring = GEOSGeom_createLinearRing(coords);
		return ring;
	}

	GEOSGeometry* SRToGEOS(const HPolygon2D& p1) {

		SR_CORE_ASSERT(p1.size() > 0);
		std::vector<GEOSGeometry*> rings{};

		for (auto& ring : p1) {
			rings.push_back(SRToGEOS(ring));
		}
		
		// shell is first in rings array
		// holes are the remaining elements
		auto polygon = GEOSGeom_createPolygon(rings[0], rings.data() + 1,rings.size() - 1);

		return polygon;
	}

	GEOSGeometry* SRToGEOS(const MultiPolygon2D& p1) {

		std::vector<GEOSGeometry*> polies{};

		for (auto& poly : p1) {
			polies.push_back(SRToGEOS(poly));
		}

		auto group = GEOSGeom_createCollection(GEOS_MULTIPOLYGON, polies.data(), polies.size());

		return group;
	}

	std::vector<int> printOutGeometryCollectionContents(const GEOSGeometry* geometry) {

		auto count = GEOSGetNumGeometries(geometry);
		auto types = std::vector<int>(count);

		for (int i = 0; i < count ; i++)
		{
			auto geom = GEOSGetGeometryN(geometry, i);
			types[i] = GEOSGeomTypeId(geom);
			//SR_WARN("geometry group item {}, is of type {}", i, types[i]);
		}
		return types;
	}

	Polygon2D GEOSRingToSR(const GEOSGeometry* geometry) {
		SR_CORE_ASSERT(GEOSGeomTypeId(geometry) == GEOS_LINEARRING);

		Polygon2D result{};

		auto pCount = GEOSGeomGetNumPoints(geometry);
		auto seq = GEOSGeom_getCoordSeq(geometry);

		for (size_t i = 0; i < pCount; i++)
		{
			glm::dvec2 point{};
			GEOSCoordSeq_getXY(seq, i, &point.x, &point.y);
			result.push_back(point);
		}
		return result;
	}

	HPolygon2D GEOSPolyToSR(const GEOSGeometry* geometry) {
		if (GEOSGeomTypeId(geometry) == GEOS_LINEARRING)
			return { GEOSRingToSR(geometry) };

		// asuming geometry is a polygon
		SR_CORE_ASSERT(GEOSGeomTypeId(geometry) == GEOS_POLYGON);

		HPolygon2D result{};

		auto perimiterRing = GEOSGetExteriorRing(geometry);

		result.push_back(GEOSRingToSR(perimiterRing));

		auto numHoles = GEOSGetNumInteriorRings(geometry);
		result.resize(numHoles + 1);

		for (size_t i = 1; i < result.size(); i++)
		{
			result[i] = GEOSRingToSR(GEOSGetInteriorRingN(geometry,i - 1));
		}

		return result;
	}

	MultiPolygon2D GEOSMultiPolyToSR(const GEOSGeometry* geometry) {

		if (GEOSGeomTypeId(geometry) == GEOS_POLYGON)
			return { GEOSPolyToSR(geometry) };
		else if (GEOSGeomTypeId(geometry) == GEOS_POINT)
			//TODO: maybe make better
			return {};
		//auto type = GEOSGeomTypeId(geometry);
		//SR_INFO("possible unknown type id of {}", type);

		if (GEOSGeomTypeId(geometry) == GEOS_GEOMETRYCOLLECTION) {
			auto types = printOutGeometryCollectionContents(geometry);
			//find first polygon and use that instead
			auto poly = std::find_if(types.begin(), types.end(), [](auto type) {
				return type == GEOS_POLYGON;
			});
			if (poly != types.end()) {
				auto index = poly - types.begin();
				return GEOSMultiPolyToSR(GEOSGetGeometryN(geometry, index));
			}
			if (types.size() == 0)
				return { {} };
			SR_DEBUGBREAK();
		}

		SR_ASSERT(GEOSGeomTypeId(geometry) == GEOS_MULTIPOLYGON);
		/*if (GEOSGeomTypeId(geometry) != GEOS_MULTIPOLYGON) {
			throw std::runtime_error("wrong type");
		}*/

		MultiPolygon2D result{};

		auto count = GEOSGetNumGeometries(geometry);
		
		for (size_t i = 0; i < count; i++)
		{
			result.push_back(GEOSPolyToSR(GEOSGetGeometryN(geometry, i)));
		}

		return result;
	}


	MultiPolygon2D bunion(const MultiPolygon2D& p1,const MultiPolygon2D& p2)
	{
		//TODO: stop or figure out how to abstract this geos globa stuff
		initGEOS(geos_msg_handler, geos_msg_handler);

		auto gp1 = SRToGEOS(p1);
		auto gp2 = SRToGEOS(p2);

		auto result = GEOSUnion(gp1,gp2);


		//TODO: this memory leaks becasue GEOSgeom type is
		//SR_CORE_INFO("just unioned two polygons and got a {}",GEOSGeomType(result));

		return GEOSMultiPolyToSR(result);
	}

	MultiPolygon2D bunionAll(const MultiPolygon2D& p)
	{
		//TODO: stop or figure out how to abstract this geos globa stuff
		initGEOS(geos_msg_handler, geos_msg_handler);


		auto gall = SRToGEOS(p);

		char* reason;
		GEOSGeometry* location;
		char valid = GEOSisValidDetail(gall, GEOSVALID_ALLOW_SELFTOUCHING_RING_FORMING_HOLE, &reason, &location);
		if (valid == 0)
			SR_CORE_INFO("valid: {}, reason: {}, locationType {}", (int)valid,reason,GEOSGeomTypeId(location));

		if (valid == 0) {
			auto gall2 = GEOSMakeValid(gall);
			GEOSFree(gall);
			gall = gall2;
		}


		//valid = GEOSisValidDetail(gall, GEOSVALID_ALLOW_SELFTOUCHING_RING_FORMING_HOLE, &reason, &location);
		//SR_CORE_INFO("valid: {}, reason: ?, locationType: ?", (int)valid);

		auto result = GEOSUnaryUnion(gall);
		
		//SR_CORE_INFO("after unary union type == {}", GEOSGeomTypeId(result));

		return GEOSMultiPolyToSR(result);
	}

	MultiPolygon2D binterseciton(const MultiPolygon2D& p1, const MultiPolygon2D& p2)
	{
		if (p2.size() == 0 || (p2.size() == 1 && p2[0].size() == 0) || (p2.size() == 1 && p2[0].size() == 1 && p2[0][0].size() == 0))
			return { {{}} };
		//TODO: stop or figure out how to abstract this geos globa stuff
		initGEOS(geos_msg_handler, geos_msg_handler);

		auto gp1 = SRToGEOS(p1);
		auto gp2 = SRToGEOS(p2);

		auto result = GEOSIntersection(gp1, gp2);

		//TODO: this memory leaks becasue GEOSgeom type is
		//SR_CORE_INFO("just intersected two polygons and got a {}", GEOSGeomType(result));

		
		return GEOSMultiPolyToSR(result);
	}

	MultiPolygon2D bDifference(const MultiPolygon2D& p1,const MultiPolygon2D& p2)
	{
		// if second is empty than just return first item
		if (p2.size() == 0 || (p2.size() == 1 && p2[0].size() == 0) || (p2.size() == 1 && p2[0].size() == 1 && p2[0][0].size() == 0))
			return p1;

		initGEOS(geos_msg_handler, geos_msg_handler);


		auto gp1 = SRToGEOS(p1);
		auto gp2 = SRToGEOS(p2);

		auto result = GEOSDifference(gp1, gp2);


		//TODO: this memory leaks becasue GEOSgeom type is
		//SR_CORE_INFO("just differenced two polygons and got a {}", GEOSGeomType(result));
		
		//TODO: make fail more robust
		if (!result)
			return p1;

		return GEOSMultiPolyToSR(result);
	}

	std::pair<TriangulatedMesh, bool> triangulate(const HPolygon2D& polygon) {

		TriangulatedMesh mesh;

		//TODO: make sure all sub polygons are open not closed

		mesh.indicies = { mapbox::earcut<uint32_t>(polygon) };
		
		for (auto subP : polygon)
			mesh.verts.insert(mesh.verts.end(), subP.begin(), subP.end());


		return std::make_pair(mesh, clockwiseOriented(polygon[0]));
	}

	
	bool clockwiseOriented(const Polygon2D& polygon)
	{
		//https://stackoverflow.com/questions/1165647/how-to-determine-if-a-list-of-polygon-points-are-in-clockwise-order
		double sum = 0;

		for (size_t i = 0; i < polygon.size(); i++)
		{
			auto& p1 = polygon[i];
			auto p2index = (i == polygon.size() - 1) ? 0 : i + 1;
			auto& p2 = polygon[p2index];

			sum += (p2.x - p1.x) * (p2.y + p1.y);
		}

		return sum > 0;
	}
}
#endif