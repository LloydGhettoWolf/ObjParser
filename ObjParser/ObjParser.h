#pragma once
//objparser.h = my obj parser that pre-processes into my "BOOM" file format
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <DirectXMath.h>
#include "MaterialParser.h"

using namespace std;
using namespace DirectX;

//#define FAST

struct MeshData
{
	// for the 'standard' mesh data that is common in .obj files
	vector<XMFLOAT3> positions;
	vector<XMFLOAT3> normals;
	vector<XMFLOAT2> uvs;
	vector<XMFLOAT3> tangents;
	vector<unsigned int> indices;

	// there are only so many materials in an .obj, enter all the info into an array of material infos and store an index into the array here
	unsigned int materialIndex;

	// is this normal mapped? This is checked in engine to choose right shader
	bool isNormalMapped;

	// info for AABB
	XMFLOAT3 min, max, center;
};

// each .BOOM file has a small header giving the info below
struct headerInfo
{
	int numverts;
	int numIndices;
	unsigned int materialIndex;
};

// The actual parser itself
class ObjParser
{
public:

	ObjParser() : mOffsetVert(1), mOffsetUv(1), mOffsetNorm(1) {};

	void Init();
	void CreateNormals();
	void CreateTangents();
	void FinishInfo(bool materialChange = false);
	void CreateVertex(string& faceDescription);

	// TODO finish FAST codepath
#ifdef FAST
	void ReadObjFile(char* filePath, char* fileName);

	void ProcessOneLenTokens(char** pointer);
	void ProcessTwoLenTokens(char** pointer);
	void ProcessLongerTokens(char** pointer);
	char* mCurrentPath;

	float ReadFloat(char** pointer);
#else
	void ReadObjFile(string& filePath, string& fileName);

	void ProcessOneLenTokens(ifstream& ss, char firstToken);
	void ProcessTwoLenTokens(ifstream& ss, string& firstToken);
	void ProcessLongerTokens(ifstream& ss, string& firstToken);
	string mCurrentPath;
#endif


	void WriteOutData(string& outFile);
	void ProcessFaceData(string& data);

	void IsMin(XMFLOAT3& vert);
	void IsMax(XMFLOAT3& vert);

	void CalcCenterAndSubtract();

private:

	bool mFinishedVertexInfo = false;
	bool mReadingFaces = false;
	bool mHasNormalsInc = false;
	
	unsigned int mOffsetVert;
	unsigned int mOffsetUv;
	unsigned int mOffsetNorm;

	unsigned int mCurrIndex = 0;

	MeshData mTempMesh;
	vector<MeshData> mMeshes;

	vector<XMFLOAT3> mPositions;
	vector<XMFLOAT2> mUvs;
	vector<XMFLOAT3> mNormals;
	vector<XMFLOAT3> mTangents;
	vector<unsigned int> mIndices;
	map<string, unsigned int> faceIndices;

	vector<string> mMeshNames;

	MaterialParser mMatParser;
};