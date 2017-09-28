#pragma once
//New Parser.h
#include <DirectXMath.h>
#include <vector>
#include <map>
using namespace DirectX;
using namespace std;

enum STATE {READING_VERTS, READING_FACES};

struct MeshData
{
	vector<XMFLOAT3> positions;
	vector<XMFLOAT3> normals;
	vector<XMFLOAT2> uvs;
	vector<XMFLOAT3> tangents;
	vector<unsigned int> indices;
	unsigned int materialIndex;
	bool isNormalMapped;
	XMFLOAT3 min, max, center;
};

struct MeshVertex
{
	XMFLOAT3 position;
	XMFLOAT2 UV;
	XMFLOAT3 normal;

	MeshVertex(XMFLOAT3 pos, XMFLOAT2 uv, XMFLOAT3 norm) :
		position(pos), UV(uv), normal(norm) {};
};

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

void ReadObjFile(char* fileName);
XMFLOAT3 ReadPosition(ifstream& inFile);
XMFLOAT2 ReadUV(ifstream& inFile);
MeshVertex CreateMeshVertex(string& data, const vector<XMFLOAT3>& positions, const vector<XMFLOAT2>& uvs, const vector<XMFLOAT3>& normals, unsigned int* currentTotals);
map<string, unsigned int> AddToFaceMap(const string& data, map<string, unsigned int>&& faces);
MeshData AddvertexToMesh(const MeshVertex& vertex, MeshData&& mesh);
MeshData NewMesh();
vector<materialInfo> ReadMaterials(char* fileName);
materialInfo GetMaterialColor(ifstream& materialFile, materialInfo info);
vector<string> AddTexture(vector<string> textures, const string& textureName);
unsigned int FindIndexInVector(const vector<string>& textures, const string& textureName);
materialInfo SetIndices(const string& token, materialInfo info, unsigned int index);