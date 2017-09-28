// ObjParserScratchPad.cpp
// Testground for parsing wavefront .obj files
//#include "ObjParser.h"
#include "NewParser.h"


int main()
{
	//ObjParser objparse;

	/*string file = "san-miguel.obj";
	string dir = "C://Users/GhettoFett/Documents/meshes/Sponza/";*/
	string output = "C://Users/GhettoFett/Documents/processedMeshes/sponza3.boom";

	string file = "sponza.obj";
	string dir = "C://Users/GhettoFett/Documents/meshes/sponza_obj/";

	//objparse.Init();
	//objparse.ReadObjFile(dir, file);
	//objparse.WriteOutData(output);

	ReadObjFile("C://Users/GhettoFett/Documents/meshes/sponza_obj/sponza.obj");
	return 0;
}