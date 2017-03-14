#pragma once
//objparser.h
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <DirectXMath.h>

using namespace std;
using namespace DirectX;

struct VertexType
{
	XMFLOAT4 position;
	XMFLOAT4 normal;
	XMFLOAT4 uv;
};

struct MeshData
{
	vector<VertexType> vertices;
	vector<unsigned int> indices;
	unsigned int materialIndex;
};

struct headerInfo
{
	int numverts;
	int numIndices;
	unsigned int materialIndex;
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
};


class ObjParser
{
public:

	ObjParser() : offsetNum(1), texOffsetNum(1), texIndex(0), matIndex(0) {};

	void CreateUVs(MeshData* data);
	void CreateNormals(MeshData* data);
	void FinishInfo(MeshData* data);
	void ProcessOneLenTokens(MeshData* data, stringstream& ss, char firstToken);
	void ProcessTwoLenTokens(MeshData* data, stringstream& ss, string& firstToken);
	void ProcessLongerTokens(MeshData* data, stringstream& ss, string& firstToken);

	void ReadMaterials(string& fileName);

	MeshData* ReadObjFile(string& filePath, string& fileName);
	void WriteOutData(string& outFile);

private:
	bool finishedVertexInfo = false;
	
	unsigned int offsetNum;
	unsigned int texOffsetNum;
	unsigned int texIndex;
	unsigned int matIndex;

	vector<MeshData> meshes;
	vector<XMFLOAT4> uvs;
	vector<unsigned int> texIndices;
	vector<string> meshNames;
	vector<materialInfo> materials;
	vector<string> textures;
	map<string, unsigned int> materialNames;
	string name;

	string currentPath;
};