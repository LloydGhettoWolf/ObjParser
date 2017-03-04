// ObjParserScratchPad.cpp
// Testground for parsing wavefront .obj files
#include "ObjParser.h"


int main()
{
	ObjParser objparse;

	string file = "sponza.obj";
	string dir = "C://Users/GhettoFett/Documents/meshes/sponza_obj/";
	string output = "C://Users/GhettoFett/Documents/processedMeshes/sponza.boom";
	objparse.ReadObjFile(dir, file);
	objparse.WriteOutData(output);
	return 0;
}