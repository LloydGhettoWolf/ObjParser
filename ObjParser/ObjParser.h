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

	ObjParser() : mOffsetVert(1), mOffsetUv(1), mOffsetNorm(1), matIndex(0) {};

	void CreateNormals();
	void FinishInfo();
	void ProcessOneLenTokens(stringstream& ss, char firstToken);
	void ProcessTwoLenTokens(stringstream& ss, string& firstToken);
	void ProcessLongerTokens(stringstream& ss, string& firstToken);
	void CreateVertex(string& faceDescription);


	void ReadMaterials(string& fileName);

	void ReadObjFile(string& filePath, string& fileName);
	void WriteOutData(string& outFile);
	void ProcessFaceData(string& data);

private:

	bool finishedVertexInfo = false;
	
	unsigned int mOffsetVert;
	unsigned int mOffsetUv;
	unsigned int mOffsetNorm;
	unsigned int matIndex;
	unsigned int texIndex;

	unsigned int mCurrIndex = 0;

	MeshData mTempMesh;
	vector<MeshData> mMeshes;
	vector<VertexType> mFullVertices;

	vector<XMFLOAT4> mVertices;
	vector<XMFLOAT4> mUvs;
	vector<XMFLOAT4> mNormals;
	vector<unsigned int> mIndices;
	map<string, unsigned int> faceIndices;

	vector<string> mMeshNames;
	vector<materialInfo> mMaterials;
	vector<string> mTextures;
	map<string, unsigned int> mMaterialNames;
	string mName;

	string mCurrentPath;
};