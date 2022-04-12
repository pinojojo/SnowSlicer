#include "SnowSlicer.h"

#include <filesystem>


SnowSlicer::SnowSlicer(std::string stl_path, float layer_height, float scale)
{
	scaling_value_ = scale;

	// 1. load
	LoadMesh(stl_path);

	// 2. compute planes
	ComputePlanes(layer_height, scale);

	// 3. Slicing 
	Slicing();

	// 4. formatting
	Formatting();
	
	// 5. ordering
	Sorting();

}



void SnowSlicer::LoadMesh(std::string input_path)
{
	namespace PMP = CGAL::Polygon_mesh_processing;
	if (!PMP::IO::read_polygon_mesh(input_path, mesh_) || CGAL::is_empty(mesh_) || !CGAL::is_triangle_mesh(mesh_))
	{
		std::cerr << "Invalid input." << std::endl;
	}
	else
	{
		std::cout << "loaded mesh has " << mesh_.number_of_faces() << " faces ";
	}

	// Get bounding box
	using namespace snow;
	float min_x = 9999999;
	float min_y = 9999999;
	float min_z = 9999999;
	float max_x = -9999999;
	float max_y = -9999999;
	float max_z = -9999999;
	for (auto& point:mesh_.points())
	{
		//std::cout << point.x() << " " << point.y() << " " << point.z() << std::endl;
		if (point.x() < min_x) { min_x = point.x(); }
		if (point.y() < min_y) { min_y = point.y(); }
		if (point.z() < min_z) { min_z = point.z(); }

		if (point.x() > max_x) { max_x = point.x(); }
		if (point.y() > max_y) { max_y = point.y(); }
		if (point.z() > max_z) { max_z = point.z(); }
	}

	b_box_3d_.min_x = min_x;
	b_box_3d_.min_y = min_y;
	b_box_3d_.min_z = min_z;
	b_box_3d_.size_x = max_x - min_x;
	b_box_3d_.size_y = max_y - min_y;
	b_box_3d_.size_z = max_z - min_z;


}




void SnowSlicer::ComputePlanes(float layer_height, float scale)
{

	if (scale)
	{
		float layer_thickness_for_slcer = layer_height / scale;
		int plane_count =ceil(b_box_3d_.size_z / layer_thickness_for_slcer);
		
		for (size_t plane_id = 0; plane_id < plane_count; plane_id++)
		{
			float eps_start = b_box_3d_.size_z * 0.0001;
			float curr_plane_height = b_box_3d_.min_z + plane_id * layer_thickness_for_slcer + eps_start;
			planes_.push_back(curr_plane_height);
			std::cout << plane_id << " " << curr_plane_height << std::endl;
		}
		
	}
	else 
	{
		std::cout << "error: slice layer height incorrect! " << std::endl;
	}

}

void SnowSlicer::Slicing()
{
	// create cgal slicer
	CGAL::Polygon_mesh_slicer<snow::Mesh, snow::K> slicer(mesh_);
	snow::AABB_tree tree(edges(mesh_).first, edges(mesh_).second, mesh_);
	CGAL::Polygon_mesh_slicer<snow::Mesh, snow::K> slicer_aabb(mesh_, tree);

	// slice each plane to polylines
	for (size_t plane_id = 0; plane_id < planes_.size(); plane_id++)
	{
		snow::Polylines curr_polylines;

		slicer_aabb(snow::K::Plane_3(0, 0, 1, -planes_[plane_id]), std::back_inserter(curr_polylines));

		polylines_stack_.push_back(curr_polylines);
		curr_polylines.clear();

		std::cout << " plane " << plane_id << " sliced" << std::endl;
	}

	// info
	if (true)
	{
		for (size_t plane_id = 0; plane_id < planes_.size(); plane_id++)
		{
			std::cout << " plane "<<plane_id<<" has " << polylines_stack_[plane_id].size() << " polylines" << std::endl;

			

			for (auto& poly : polylines_stack_[plane_id])
			{
				std::cout << "      size : " << GetPolylineBoundingBox(poly).size_x <<" is " << IsCCW(poly) << std::endl;
			}
		}
	}


}

void SnowSlicer::Formatting()
{
	// traverse plane 
	for (size_t plane_id = 0; plane_id < planes_.size(); plane_id++)
	{
		snow::FormattedPlane curr_fmt_plane;
		curr_fmt_plane.relative_id = plane_id;


		curr_fmt_plane.z_value = planes_[plane_id]; // z start from 0

		int poly_id = 0;
		
		// traverse polygons
		float largest_size = 0;
		for (auto& poly : polylines_stack_[plane_id])
		{
			snow::FormattedPolygon curr_fmt_polygon;
			curr_fmt_polygon.relative_id = poly_id++;

			snow::BoundingBox2D polygon_bbox = GetPolylineBoundingBox(poly);
			
			curr_fmt_polygon.b_box = polygon_bbox;
			//std::cout << "polygon's bbox size" << curr_fmt_polygon.b_box.size_x << " " << curr_fmt_polygon.b_box.size_y << " " << std::endl;
			if ((curr_fmt_polygon.b_box.size_x > largest_size) || (curr_fmt_polygon.b_box.size_y > largest_size))
			{
				largest_size = std::max(curr_fmt_polygon.b_box.size_x, curr_fmt_polygon.b_box.size_y);
				curr_fmt_plane.largest_polygon_id = curr_fmt_polygon.relative_id;
			}

			curr_fmt_polygon.is_ccw = IsCCW(poly);

			// traverse points in polygon
			for (auto& point_type : poly)
			{
				snow::Vec2 point;
				point.x = point_type.x();
				point.y = point_type.y();

				curr_fmt_polygon.points.push_back(point);
			}

			curr_fmt_plane.polygons.push_back(curr_fmt_polygon);

		}
		
		
		snow::BoundingBox2D plane_bbox = GetPlaneBoundingBox(polylines_stack_[plane_id]);

		curr_fmt_plane.b_box = plane_bbox;
		//std::cout << "plane's bbox size" << curr_fmt_plane.b_box.size_x << " " << curr_fmt_plane.b_box.size_y << " " << std::endl;
		


		data_.planes.push_back(curr_fmt_plane);
	}


	data_.b_box = GetDataBoundingBox(data_);



	if (true)
	{
		std::cout << data_.planes.size() << std::endl;
	}


}

void SnowSlicer::Sorting()
{
	// Generate graph
	for (size_t plane_id = 0; plane_id < data_.planes.size(); plane_id++)
	{
		//std::cout << "under " << plane_id << std::endl;
		graphs_.push_back(GenerateGraph(data_.planes[plane_id]));
	}

	// Generate sorting result
	for (size_t g_id = 0; g_id < graphs_.size(); g_id++)
	{
		int root_node = data_.planes[g_id].largest_polygon_id;
		data_.planes[g_id].render_order = graphs_[g_id].RunBFS(root_node, false);

		std::cout << "ordering: ";
		for (int order_id = 0; order_id < data_.planes[g_id].render_order.size(); order_id++)
		{
			int order_id_value = data_.planes[g_id].render_order[order_id];
			
			std::cout << " (" << data_.planes[g_id].polygons[order_id_value].is_ccw <<")" << data_.planes[g_id].polygons[order_id_value].b_box.size_x;
		}
		std::cout << std::endl;
	}
	

}

void SnowSlicer::SaveAsSVG(std::string path)
{
	FILE* svg = NULL;

	svg = fopen(path.c_str(), "w");

	if (!svg)
	{
		std::cout << "ERROR: xml path creating failed ! " << std::endl;
	}
	else
	{
		fprintf(svg,"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>");
		fprintf(svg, "\n");
		fprintf(svg, "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.0//EN\" \"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\">");
		fprintf(svg, "\n");

		std::string svg_string = GetSVGString(data_);

		fprintf(svg, svg_string.c_str());
		
		fclose(svg);
	}

}

bool SnowSlicer::IsCCW(snow::Polyline_type& polyline)
{
	// refer to https://stackoverflow.com/a/1165943/9030636
	float total_edges = 0;
	for (size_t point_id = 0; point_id < polyline.size(); point_id++)
	{
		if (point_id != polyline.size() - 1) 
		{
			float x_incremented = polyline[point_id + 1].x() - polyline[point_id].x();
			float y_accumulated = polyline[point_id + 1].y() + polyline[point_id].x();

			total_edges += x_incremented * y_accumulated;
		}
		else 
		{
			float x_incremented = polyline[0].x() - polyline[point_id].x();
			float y_accumulated = polyline[0].y() + polyline[point_id].x();
			total_edges += x_incremented * y_accumulated;

		}
	}

	return total_edges < 0;


}

snow::BoundingBox2D SnowSlicer::GetPolylineBoundingBox(snow::Polyline_type& polyline)
{
	float min_x = 9999999;
	float min_y = 9999999;
	float max_x = -9999999;
	float max_y = -9999999;
	

	for (auto& point:polyline)
	{
		if (point.x()<min_x){min_x = point.x();}
		if (point.y()<min_y){min_y = point.y();}
		if (point.x()>max_x){max_x = point.x();}
		if (point.y()>max_y){max_y = point.y();}
	}

	snow::BoundingBox2D ret;

	ret.min_x = min_x;
	ret.min_y = min_y;
	ret.size_x = max_x - min_x;
	ret.size_y = max_y - min_y;

	return ret;
}

snow::BoundingBox2D SnowSlicer::GetPlaneBoundingBox(snow::Polylines& lines)
{
	float min_x = 9999999;
	float min_y = 9999999;
	float max_x = -9999999;
	float max_y = -9999999;

	for (auto& poly:lines)
	{
		for (auto& point : poly)
		{
			if (point.x() < min_x) { min_x = point.x(); }
			if (point.y() < min_y) { min_y = point.y(); }
			if (point.x() > max_x) { max_x = point.x(); }
			if (point.y() > max_y) { max_y = point.y(); }
		}
	}

	snow::BoundingBox2D ret;

	ret.min_x = min_x;
	ret.min_y = min_y;
	ret.size_x = max_x - min_x;
	ret.size_y = max_y - min_y;

	return ret;
}

snow::BoundingBox2D SnowSlicer::GetDataBoundingBox(snow::FormattedData& data)
{
	float min_x = 9999999;
	float min_y = 9999999;
	float max_x = -9999999;
	float max_y = -9999999;

	for (auto& plane: data.planes)
	{
		for (auto& poly : plane.polygons)
		{
			for (auto& point : poly.points)
			{
				if (point.x < min_x) { min_x = point.x; }
				if (point.y < min_y) { min_y = point.y; }
				if (point.x > max_x) { max_x = point.x; }
				if (point.y > max_y) { max_y = point.y; }
			}
		}
	}

	snow::BoundingBox2D ret;

	ret.min_x = min_x;
	ret.min_y = min_y;
	ret.size_x = max_x - min_x;
	ret.size_y = max_y - min_y;

	return ret;
}

SimpleGraph SnowSlicer::GenerateGraph(snow::FormattedPlane& plane)
{
	int polygon_num = plane.polygons.size();
	SimpleGraph graph(polygon_num+1);
	
	// Create the Root polygon which contains all polygons
	snow::FormattedPolygon root_frame_polygon;
	root_frame_polygon.is_root = true;
	root_frame_polygon.relative_id = plane.polygons.size();
	snow::Vec2 root_poly_points[4];
	root_poly_points[0].x = plane.b_box.min_x-1;
	root_poly_points[0].y = plane.b_box.min_y-1;
	root_poly_points[1].x = plane.b_box.min_x+plane.b_box.size_x+1;
	root_poly_points[1].y = plane.b_box.min_y-1;
	root_poly_points[2].x = plane.b_box.min_x + plane.b_box.size_x+1;
	root_poly_points[2].y = plane.b_box.min_y + plane.b_box.size_y+1;
	root_poly_points[3].x = plane.b_box.min_x-1;
	root_poly_points[3].y = plane.b_box.min_y + plane.b_box.size_y+1;
	for (size_t i = 0; i < 4; i++)
	{
		root_frame_polygon.points.push_back(root_poly_points[i]);
	}

	plane.polygons.push_back(root_frame_polygon);
	plane.largest_polygon_id = root_frame_polygon.relative_id;

	// Add relations to graph
	for (size_t p_id = 0; p_id < polygon_num+1; p_id++)
	{
		for (size_t next_p_id = 0; next_p_id < polygon_num+1; next_p_id++)
		{
			if (next_p_id!=p_id)
			{
				// test next polyogn whether it is contained in current polygon
				snow::Vec2 test_point = plane.polygons[next_p_id].points[0];
				if (IsPointInPolygon(test_point, plane.polygons[p_id]))
				{
					std::cout << "polygon " << p_id  <<" contains " << next_p_id << " sizeA " << plane.polygons[p_id].b_box.size_x << " sizeB " << plane.polygons[next_p_id].b_box.size_x << std::endl;
					graph.AddRelation(plane.polygons[p_id].relative_id, plane.polygons[next_p_id].relative_id);
				}
			}
		}

	}
	// Remove conflits
	graph.RemoveConflicts();


	return graph;
}

bool SnowSlicer::IsPointInPolygon(snow::Vec2 point, snow::FormattedPolygon& polygon)
{
	int i, j, c = 0;
	int nvert = polygon.points.size();
	for (i = 0, j = nvert - 1; i < nvert; j = i++) {
		if (((polygon.points[i].y > point.y) != (polygon.points[j].y > point.y)) &&
			(point.x < (polygon.points[j].x - polygon.points[i].x) * (point.y - polygon.points[i].y) / (polygon.points[j].y - polygon.points[i].y) + polygon.points[i].x))
			c = !c;
	}

	return c;
	
}

std::string SnowSlicer::GetSVGString(snow::FormattedData& data)
{
	float width = data.b_box.size_x * scaling_value_;
	float height = data.b_box.size_y * scaling_value_;

	std::string text_svg = "<svg width=\"" + FixedFloat(width) +"\" height =\""+FixedFloat(height)+"\" ";
	text_svg += "xmlns=\"http://www.w3.org/2000/svg\" xmlns:svg=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xmlns:slic3r=\"http://slic3r.org/namespaces/slic3r\">";
	text_svg += "\n";

	for (size_t p_id = 0; p_id < data.planes.size(); p_id++)
	{
		text_svg += "\n";
		text_svg += GetLayerString(data.planes[p_id]);

	}


	text_svg += "\n";
	text_svg += "</svg>";
	
	return text_svg;
}

std::string SnowSlicer::GetLayerString(snow::FormattedPlane& layer)
{
	std::string ret;

	float curr_z_value = (layer.z_value - b_box_3d_.min_z) * scaling_value_;

	ret = "  <g id=\"layer" + std::to_string(layer.relative_id) + "\" " + "slic3r:z=\"" + std::to_string(curr_z_value) + "\">";

	// polygons
	for (size_t poly_order_id = 0; poly_order_id < layer.render_order.size(); poly_order_id++)
	{

		ret += "\n";

		int actual_polygon_id = layer.render_order[poly_order_id];

		ret += GetPolygonString(layer.polygons[actual_polygon_id]);
	}

	ret += "\n";
	ret += "  </g>";
	

	return ret;
}

std::string SnowSlicer::GetPolygonString(snow::FormattedPolygon& polygon)
{
	

	std::string ret;

	ret += "    ";
	
	std::string type;
	std::string points;
	std::string style;

	if (polygon.is_ccw)
	{
		type = "contour";
		style = "fill: white";
	}
	else 
	{
		type = "hole";
		style = "fill: black";
	}

	for (auto& point : polygon.points)
	{
		points += FixedFloat((point.x-data_.b_box.min_x)*scaling_value_) + "," + FixedFloat((point.y-data_.b_box.min_y)*scaling_value_);
		if (&point!=&polygon.points.back())
		{
			points += " ";
		}
	}
	
	ret += "<polygon slic3r:type=\"" + type + "\" points=\"";
	ret += points;
	ret += "\" style=\"";
	ret += style;
	ret += "\" />";

	return ret;
}

std::string SnowSlicer::FixedFloat(float val)
{
	std::stringstream stream;
	stream << std::fixed << std::setprecision(6) << val;
	return stream.str();
}

std::string SnowSlicer::FixedFloatTo3(float val)
{
	std::stringstream stream;
	stream << std::fixed << std::setprecision(3) << val;
	return stream.str();
}
