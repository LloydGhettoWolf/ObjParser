#pragma once

#include <string>
#include <DirectXMath.h>
#include <vector>
#include <map>
#include <fstream>

using namespace std;
using namespace DirectX;

struct materialInfo
{
	XMFLOAT3 diffuse;
	XMFLOAT3 ambient;
	XMFLOAT3 specular;
	float specFactor;
	unsigned int diffTexIndex;
	unsigned int specTexIndex;
	unsigned int normMapIndex;
	unsigned int maskIndex;
};

class MaterialParser
{
public:
	void ReadMaterials(string& fileName);
	void WriteOutMaterials(ofstream& outFile);

	unsigned int GetNumMaterials() const { return mMaterials.size(); }

	const materialInfo& GetMaterial(unsigned int index) { return mMaterials[index]; }
	unsigned int GetMaterialIndex(string material) { return mMaterialNames[material];  }
private:

	vector<materialInfo> mMaterials;
	map<string, unsigned int> mMaterialNames;
	vector<string> mTextures;
	unsigned int mTexIndex = 0;
};
