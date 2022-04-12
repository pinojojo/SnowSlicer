#pragma once

#include "SnowType.h"


#include <string>
#include <iomanip>
#include <sstream>

class SnowSlicer
{
public:
	SnowSlicer(std::string stl_path, float layer_height, float scale);



	
	void SaveAsSVG(std::string path);
public:
	snow::FormattedData data_;
private:
	void LoadMesh(std::string input_path);

	void ComputePlanes(float layer_height,float scale);
	void Slicing();
	void Formatting();
	void Sorting();
	

	bool IsCCW(snow::Polyline_type& polyline);
	snow::BoundingBox2D GetPolylineBoundingBox(snow::Polyline_type& polyline);
	snow::BoundingBox2D GetPlaneBoundingBox(snow::Polylines& lines);
	snow::BoundingBox2D GetDataBoundingBox(snow::FormattedData& data);
	SimpleGraph GenerateGraph(snow::FormattedPlane& plane);
	bool IsPointInPolygon(snow::Vec2 point, snow::FormattedPolygon& polygon);

	std::string GetSVGString(snow::FormattedData& data);
	std::string GetLayerString(snow::FormattedPlane& layer);
	std::string GetPolygonString(snow::FormattedPolygon& polygon);

	std::string FixedFloat(float val);
	std::string FixedFloatTo3(float val);

	
	

	

private:
	float scaling_value_ = 1.0f;
	std::vector<snow::Polylines> polylines_stack_;
	std::vector<float> planes_;
	snow::Mesh mesh_;
	snow::BoundingBox3D b_box_3d_;
	std::vector<SimpleGraph> graphs_;

	
};



