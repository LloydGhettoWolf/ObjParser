#include "MaterialParser.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

using namespace std;

void MaterialParser::ReadMaterials(string& filePath)
{
	ifstream materialFile;
	materialFile.open(filePath);

	materialInfo info;
	memset(&info, 0, sizeof(materialInfo));
	bool firstMaterial = true;

	string materialName;

	unsigned int matIndex;
	unsigned int texIndex;

	while (!materialFile.eof())
	{
		string token;
		float r, g, b;

		materialFile >> token;
		if (token == "newmtl")
		{

			if (firstMaterial)
			{
				materialFile >> materialName;
				firstMaterial = false;
			}
			else
			{
				// Push prev material into vector
				mMaterials.emplace_back(info);
				pair<string, unsigned int> matPair;
				matPair.first = materialName;
				matPair.second = (unsigned int)mMaterials.size() - 1;
				mMaterialNames.insert(matPair);

				//clear info for new material and record name
				memset(&info, 0, sizeof(materialInfo));
				materialFile >> materialName;
			}
		}
		else if (token[0] == 'K')
		{
			materialFile >> r;
			materialFile >> g;
			materialFile >> b;

			XMFLOAT3 val(r, g, b);

			switch (token[1])
			{
			case 's':
				info.specular = val;
				break;
			case 'd':
				info.diffuse = val;
				break;
			case 'a':
				info.ambient = val;
				break;
			}
		}
		else if (token[0] == 'N')
		{
			float specComponent;
			materialFile >> specComponent;

			switch (token[1])
			{
			case 's':
				info.specFactor = specComponent;
				break;
			}
		}
		else if (token.substr(0, 3) == "map")
		{
			string texName;
			materialFile >> texName;

			unsigned int index = 0;
			auto result = find(mTextures.begin(), mTextures.end(), texName);
			if (result == mTextures.end())
			{
				mTextures.emplace_back(texName);
				index = mTexIndex++;
			}
			else
			{
				index = (unsigned int)(result - mTextures.begin());
			}

			if (token == "map_Kd")
			{
				info.diffTexIndex = index;
			}
			else if (token == "map_Ks")
			{
				info.specTexIndex = index;
			}
			else if (token == "map_bump")
			{
				info.normMapIndex = index;
			}
			else if (token == "map_d")
			{
				info.maskIndex = index;
			}
		}
	}

	// Push prev material into vector
	mMaterials.emplace_back(info);
	pair<string, unsigned int> matPair;
	matPair.first = materialName;
	matPair.second = (unsigned int)mMaterials.size() - 1;
	mMaterialNames.insert(matPair);
}

void MaterialParser::WriteOutMaterials(ofstream& out)
{
	//now write the material info out too
	unsigned int numMaterials = mMaterials.size();
	for (unsigned int i = 0; i < numMaterials; i++)
	{
		out.write((char*)&mMaterials[i], sizeof(materialInfo));
	}

	unsigned int texNum = mTextures.size();
	out.write((char*)&texNum, sizeof(unsigned int));

	//now write out names of textures
	for (auto i = mTextures.begin(); i != mTextures.end(); i++)
	{
		string value = *i;
		size_t len = value.length();
		out.write((char*)&len, sizeof(unsigned int));
		out.write(value.c_str(), sizeof(char) * len);
	}
}