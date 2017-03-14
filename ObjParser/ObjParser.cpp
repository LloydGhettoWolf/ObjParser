#include "ObjParser.h"


 void ObjParser::CreateNormals()
{
	// Go over each face
	// create a normal for that face
	// for each XMFLOAT3 create a list of face numbers
	// sum up the normals from each face for each XMFLOAT3
	// then normalize
	// hey vertices!


	unsigned int numFaces = (unsigned int)mIndices.size() / 3;
	unsigned int numVerts = (unsigned int)mVertices.size();

	// A vector of vectors storing the faces associated with each XMFLOAT3
	vector<XMVECTOR> faceNormals;
	//faceNormals.reserve(numVerts);

	for (unsigned int i = 0; i < numVerts; i++)
	{
		faceNormals.emplace_back(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f));
	}

	for (unsigned int face = 0; face < numFaces; face++)
	{
		unsigned int index = face * 3;
		unsigned int indexA =mIndices[index];
		unsigned int indexB =mIndices[index + 1];
		unsigned int indexC =mIndices[index + 2];
		XMVECTOR A = XMLoadFloat4(&mVertices[indexA]);
		XMVECTOR B = XMLoadFloat4(&mVertices[indexB]);
		XMVECTOR C = XMLoadFloat4(&mVertices[indexC]);

		XMVECTOR first = B - A;
		XMVECTOR second = C - A;

		XMFLOAT4 faceNormal;
		XMStoreFloat4(&faceNormal, XMVector3Cross(first, second));
		XMVECTOR faceNormalVec = XMLoadFloat4(&faceNormal);
		faceNormals[indexA] += faceNormalVec;
		faceNormals[indexB] += faceNormalVec;
		faceNormals[indexC] += faceNormalVec;
	}

	// Now normalize all normals
	for (unsigned int i = 0; i < numVerts; i++)
	{
		faceNormals[i] = XMVector3Normalize(faceNormals[i]);
		XMStoreFloat4(&mNormals[i], faceNormals[i]);
	}

}

 void ObjParser::FinishInfo()
{
	static int meshNum = 0;
	meshNum++;
	
	// adjust offset nums
	mOffsetUv += (unsigned int)mUvs.size();
	mOffsetVert += (unsigned int)mVertices.size();
	mOffsetNorm += (unsigned int)mNormals.size();

	// output the size of the vertice and indices to the standard out
	cout << mFullVertices.size() << " " << mIndices.size() << " " << endl;

	mTempMesh.indices = mIndices;
	mTempMesh.vertices = mFullVertices;

	// store the mesh for writing to the output file later
	mMeshes.emplace_back(mTempMesh);

	// now clear the current temp mesh data
	mIndices.clear();
	mVertices.clear();
	mNormals.clear();
	mFullVertices.clear();
	mUvs.clear();
	faceIndices.clear();
	mCurrIndex = 0;

	// mark flag
	finishedVertexInfo = false;
}

 void ObjParser::CreateVertex(string& faceDescription)
 {
	 stringstream temp;
	 temp << faceDescription;
	 unsigned int index;
	 char waste;

	 temp >> index;
	 temp >> waste;

	 XMFLOAT4 pos = mVertices[index - mOffsetVert];

	 temp >> index;
	 temp >> waste;

	 XMFLOAT4 uv = mUvs[index - mOffsetUv];

	 temp >> index;
	 temp >> waste;

	 XMFLOAT4 normal = mNormals[index - mOffsetNorm];

	 VertexType newVert;
	 newVert.position = pos;
	 newVert.uv = uv;
	 newVert.normal = normal;

	 mFullVertices.emplace_back(newVert);
 }

 void ObjParser::ProcessFaceData(string& data)
 {
	 unsigned int val = faceIndices.count(data);

	 if (val == 0)
	 {
		 pair<string, unsigned int> newPair;
		 newPair.first = data;
		 newPair.second = mCurrIndex;
		 mIndices.emplace_back(mCurrIndex++);
		 CreateVertex(data);
	 }
	 else
	 {
		 unsigned int index = faceIndices[data];
		 mIndices.emplace_back(index);
	 }
 }

 void ObjParser::ProcessOneLenTokens(stringstream& ss, char firstToken)
{
	string line;
	stringstream newSS, subSS;
	size_t val;
	char waste;
	XMFLOAT4 vert;
	string vertData, prevVertData1, prevVertData2;

	switch (firstToken)
	{
	case 'v':

		//if faces have just been read in then
		//this is a new mesh
		if (finishedVertexInfo) {
			FinishInfo();
		}

		ss >> vert.x;
		ss >> vert.y;
		ss >> vert.z;
		vert.w = 1.0f;
		mVertices.emplace_back(vert);
		break;
	case 'f':
		// check for format of the face info
		// if it is like 1/1/1 then split up for use
		// otherwise just read in as is

		getline(ss, line);

		newSS << line;
		val = line.find('/');
		if (val != string::npos)
		{
			// is this face info containing 3 vertices (a triangle)
			// or 4 vertices for a quad? If a quad we need to split into two triangles
			// by repeating some of the vertices (see later)
			// count the spaces to determine this
			val = count(line.begin(), line.end(), ' ');

			if (val == 4)
			{
				for (unsigned int i = 0; i < 3; i++)
				{
					newSS >> vertData;
					ProcessFaceData(vertData);
				}

			}
			else if (val == 5)
			{
				// first three vertices are in a ccw direction like below, 1, 2 and 3
				// *1
				//
				// *2    *3

				// next three verts are like this
				// * 1   *3
				//
				//       *2

				// so will have to store the first index for use later, same for the third vertex of the
				// first tri which is the 2nd vertex of the second tri

				newSS >> vertData;
				ProcessFaceData(vertData);
				prevVertData1 = vertData;

				newSS >> vertData;
				ProcessFaceData(vertData);
				
				newSS >> vertData;
				ProcessFaceData(vertData);
				prevVertData2 = vertData;

				ProcessFaceData(prevVertData1);
				ProcessFaceData(prevVertData2);

				newSS >> vertData;
				ProcessFaceData(vertData);
			}
		}
		newSS.clear();
		break;
	case '#':
		ss.ignore(INT_MAX, '\n');
		break;
	case 'g':
		// If there is a g this is the beginning of the face info and the end of the vertex info
		finishedVertexInfo = true;
		string name;
		ss >> name;
		mMeshNames.emplace_back(name);
		break;
	}
}

 void ObjParser::ProcessTwoLenTokens(stringstream& ss,
									string& firstToken)
{
	XMFLOAT4 uv;
	XMFLOAT4 normal;

	switch (firstToken[1])
	{
	case 't':
		ss >> uv.x;
		ss >> uv.y;
		uv.z = 0.0f;
		uv.w = 0.0f;
		mUvs.emplace_back(uv);
		break;
	case 'n':
		ss >> normal.x;
		ss >> normal.y;
		ss >> normal.z;
		normal.w = 0.0f;
		mNormals.emplace_back(normal);
		break;
	}
}

 void ObjParser::ProcessLongerTokens( stringstream& ss, 
									 string& firstToken)
 {
	 if (firstToken == "usemtl")
	 {
		 string matName;
		 ss >> matName;
		 unsigned int matIndex = mMaterialNames[matName];
		 mTempMesh.materialIndex = matIndex;
	 }
	 else if (firstToken == "mtllib")
	 {
		 string matFile;
		 ss >> matFile;
		 ReadMaterials(matFile);
	 }
 }

void ObjParser::ReadObjFile(string& filePath, string& fileName)
{
	ifstream inFile;
	string line;

	MeshData* data = new MeshData();
	mCurrentPath = filePath;
	// Copy all file info into a stringstream
	inFile.open(filePath + fileName);
	stringstream ss;
	ss << inFile.rdbuf();
	inFile.close();

	//While there's still info in the stringstream
	while (ss.rdbuf()->in_avail() != 0)
	{
		string firstToken;
		ss >> firstToken;

		int len = (int)firstToken.length();

		// The first token of a line could potentially be a vertex 'v' or something
		// longer like vt - get the length and then act appropriately
		switch (len)
		{
			case 1:
				ProcessOneLenTokens(ss, firstToken[0]);
				break;
			case 2:
				ProcessTwoLenTokens(ss, firstToken);
				break;
			default:
				ProcessLongerTokens(ss, firstToken);
				break;
		}
	}

	//special case - last mesh always needs a call to finish the data
	FinishInfo();
}


 void ObjParser::ReadMaterials(string& fileName)
 {
	 ifstream materialFile;
	 materialFile.open(mCurrentPath + fileName);

	 stringstream fileContents;
	 fileContents << materialFile.rdbuf();

	 materialInfo info;
	 memset(&info, 0, sizeof(materialInfo));
	 bool firstMaterial = true;

	 while (fileContents.rdbuf()->in_avail() != 0)
	 {
		 string token;
		 float r, g, b;
		 
		 fileContents >> token;
		 if (token == "newmtl")
		 {

			 if (firstMaterial)
			 {
				 fileContents >> mName;
				 firstMaterial = false;
			 }
			 else
			 {
				 // Push prev material into vector
				 mMaterials.emplace_back(info);
				 pair<string, unsigned int> matPair;
				 matPair.first = mName;
				 matPair.second = (unsigned int)mMaterials.size() - 1;
				 mMaterialNames.insert(matPair);

				 //clear info for new material and record name
				 memset(&info, 0, sizeof(materialInfo));
				 fileContents >> mName;
			 }
		 }
		 else if (token[0] == 'K')
		 {
			 fileContents >> r;
			 fileContents >> g;
			 fileContents >> b;

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
		 else if (token.substr(0,3) == "map")
		 {
			 string texName;
			 fileContents >> texName;
			 
			 unsigned int index = 0;
			 auto result = find(mTextures.begin(), mTextures.end(), texName);
			 if (result == mTextures.end())
			 {
				 mTextures.emplace_back(texName);
				 index = texIndex++;
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
		 }


	 }

	 // Push prev material into vector
	 mMaterials.emplace_back(info);
	 pair<string, unsigned int> matPair;
	 matPair.first = mName;
	 matPair.second = (unsigned int)mMaterials.size() - 1;
	 mMaterialNames.insert(matPair);

 }

 void ObjParser::WriteOutData(string& outFile)
{
	// Now write out to a file in binary
	ofstream out;
	out.open(outFile, ios::binary);

	unsigned int size = (unsigned int)mMeshes.size();
	out.write((char*)&size, sizeof(unsigned int));
	unsigned int numMaterials = (unsigned int)mMaterials.size();
	out.write((char*)&numMaterials, sizeof(unsigned int));
	

	for (unsigned int i = 0; i < size; i++)
	{
		MeshData mesh = mMeshes[i];

		// For each mesh/ submesh write a header that contains num of verts and indices
		headerInfo info;
		info.numverts = (int)mesh.vertices.size();
		info.numIndices = (int)mesh.indices.size();
		info.materialIndex = mesh.materialIndex;
		out.write((char*)&info, sizeof(headerInfo));

		// Now write out the raw data to the file
		out.write((char*)&mesh.vertices[0], sizeof(VertexType) * info.numverts);
		out.write((char*)&mesh.indices[0], sizeof(unsigned int) * info.numIndices);
	}

	//now write the material info out too
	for (unsigned int i = 0; i < numMaterials; i++)
	{
		out.write((char*)&mMaterials[i], sizeof(materialInfo));
	}

	out.write((char*)&texIndex, sizeof(unsigned int));

	//now write out names of textures
	for (auto i = mTextures.begin(); i != mTextures.end(); i++)
	{
		string value = *i;
		size_t len = value.length();
		out.write((char*)&len, sizeof(unsigned int));
		out.write(value.c_str(), sizeof(char) * len);
	}

	out.close();
}