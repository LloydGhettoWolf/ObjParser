#pragma once
//objparser.h
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
	vector<XMFLOAT3> positions;
	vector<XMFLOAT3> normals;
	vector<XMFLOAT2> uvs;
	vector<XMFLOAT3> tangents;
	vector<unsigned int> indices;
	unsigned int materialIndex;
	bool isNormalMapped;
	XMFLOAT3 min, max, center;
};


struct headerInfo
{
	int numverts;
	int numIndices;
	unsigned int materialIndex;
};


class ObjParser
{
public:

	ObjParser() : mOffsetVert(1), mOffsetUv(1), mOffsetNorm(1) {};

	void Init();
	void CreateNormals();
	void CreateTangents();
	void FinishInfo(bool materialChange = false);
	void CreateVertex(string& faceDescription);

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

	bool finishedVertexInfo = false;
	bool mReadingFaces = false;
	bool hasNormalsInc = false;
	
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