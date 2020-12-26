#include "srpch.h"

#include "Triangulation.h"

//#include <CGAL/draw_triangulation_2.h>
//CGAL_USE_BASIC_VIEWER

//#include "igl/triangle/triangulate.h"

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_2.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Triangulation_face_base_with_info_2.h>
#include <CGAL/Polygon_2.h>
#include <iostream>
#include <limits>


#include <CGAL/Boolean_set_operations_2.h>

//TODO ----------------------------------------------------------------------------------- CGAL WILL HAVE TO BE LICENSED FOR PROFIT ----------------------------------------------------------------------------------------------------------

namespace sunrise::math::mesh {


	struct FaceInfo2
	{
		FaceInfo2() {}
		int nesting_level;
		bool in_domain() {
			return nesting_level % 2 == 1;
		}
	};


	typedef CGAL::Exact_predicates_inexact_constructions_kernel       K;
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
			mesh->verts.emplace_back(point.x(), point.y());
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

	std::vector<std::vector<glm::dvec2>>* intersectionOf(std::vector<glm::dvec2>& polygon1, std::vector<glm::dvec2>& polygon2)
	{
		
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


		// Compute the intersection of P and Q.
		//Pwh_list_2                  intR;
		std::vector<Polygon_with_holes_2>                  intR;

		CGAL::intersection(p1, p2, std::back_inserter(intR));
		//CGAL::join(p1, p2, unionR);


		auto outPolygon = new std::vector<std::vector<glm::dvec2>>();

		if (intR[0].is_unbounded()) {
			throw std::runtime_error("not supported yet");
		}

		outPolygon->push_back({});
		
		for (auto vert = intR[0].outer_boundary().vertices_begin(); vert != intR[0].outer_boundary().vertices_end(); vert = std::next(vert)) {
			(*outPolygon)[0].emplace_back(vert->x(),vert->y());
		}

		for (auto hit = intR[0].holes_begin(); hit != intR[0].holes_end(); ++hit) {

		}

		return outPolygon;
	}

	Box bounds(std::vector<glm::dvec2>& points)
	{
		glm::dvec2 min = glm::dvec2(std::numeric_limits<double>::max()); //glm::dvec2(90, 180);
		glm::dvec2 max = glm::dvec2(std::numeric_limits<double>::min()); //glm::dvec2(-90, -180);

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

		return Box(min, max - min);
	}

	


}
