//MaterialsParser.cpp
#include "MaterialsParser.h"

const vector<materialInfo>& ReadMaterialsFile(string& fileName)
{
	ifstream inFile;
	inFile.open(fileName);

	stringstream sstream;
	sstream << inFile.rdbuf();
	inFile.close();

	while (sstream.rdbuf()->in_avail() != 0)
	{
		string token;
		sstream >> token;

		if (token == "newmtl")
		{
			materialInfo newInfo;

			sstream >> newInfo.name;

			sstream >> token;

			if (token == "Ka") 
			{

			}

			if (token == "Ka")
			{

			}

		}
	}
}