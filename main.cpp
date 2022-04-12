#include "SnowSlicer.h"

int main(int argc,char** argv)
{
	std::string input_path;
	std::string output_path;
	float layer_height;
	float scale;

	if (argc==5)
	{
		input_path = argv[1];
		output_path = argv[2];
		layer_height = std::stof(argv[3]);
		scale = std::stof(argv[4]);

		SnowSlicer* slicer = new SnowSlicer(input_path, layer_height, scale);
		slicer->SaveAsSVG(output_path);
	}
	

	/*std::string AA = "C:\\Users\\lzx\\Desktop\\UMS5_Fresnel_lens.stl";
	std::string BB = "D:\\test_bin\\bin\\stl\\lambs.STL";
	SnowSlicer* slicer = new SnowSlicer(BB, 0.2, 10.5);

	slicer->SaveAsSVG("test_o.svg");*/

	return 0;
}