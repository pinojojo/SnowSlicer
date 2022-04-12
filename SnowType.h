#pragma once
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Polygon_mesh_slicer.h>
#include <CGAL/Polygon_mesh_processing/transform.h>
#include <CGAL/Polygon_mesh_processing/IO/polygon_mesh_io.h>
#include <CGAL/AABB_halfedge_graph_segment_primitive.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>

#include "SimpleGraph.h"

#include <vector>

namespace snow
{
	typedef CGAL::Exact_predicates_inexact_constructions_kernel   K;
	typedef CGAL::Surface_mesh<K::Point_3>                        Mesh;
	typedef std::vector<K::Point_3>                               Polyline_type;
	typedef std::list<Polyline_type>                              Polylines;
	typedef CGAL::AABB_halfedge_graph_segment_primitive<Mesh>     HGSP;
	typedef CGAL::AABB_traits<K, HGSP>                            AABB_traits;
	typedef CGAL::AABB_tree<AABB_traits>                          AABB_tree;
	
	struct BoundingBox2D 
	{
		float min_x;
		float min_y;
		float size_x;
		float size_y;
	};

	struct BoundingBox3D
	{
		float min_x;
		float min_y;
		float min_z;
		float size_x;
		float size_y;
		float size_z;
	};

	struct Vec3 { float x; float y; float z; };
	struct Vec2 { float x; float y;};

	struct FormattedPolygon
	{
		int relative_id;
		
		std::vector<Vec2> points;
		BoundingBox2D b_box;
		bool is_ccw;
		bool is_root=false;

		
	};

	struct FormattedPlane
	{
		int relative_id;

		float z_value;
		std::vector<FormattedPolygon> polygons;
		BoundingBox2D b_box;
		std::vector<int> render_order;
		int largest_polygon_id = -1;
	};
	
	struct FormattedData
	{
		std::string name;

		BoundingBox2D b_box;
		std::vector<FormattedPlane> planes;
	};

}