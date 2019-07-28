#include "ObjParser.h"

#ifdef FAST
void ObjParser::ReadObjFile(char* filePath, char* fileName)
{
	ifstream inFile;
	string line;

	MeshData* data = new MeshData();
	mCurrentPath = filePath;
	// Copy all file info into a stringstream
	inFile.open(strcat(filePath, fileName), std::ios::in | std::ios::binary);
	stringstream ss;
	char* fileData = NULL;

	inFile.ignore(std::numeric_limits<std::streamsize>::max());
	std::streamsize length = inFile.gcount();
	inFile.clear();   //  Since ignore will have set eof.
	inFile.seekg(0, std::ios_base::beg);

	//copy to a char pointer
	fileData = new char[length];
	inFile.read(fileData, length);
	inFile.close();

	char* currentByte = fileData;

	//While there's still info in the stringstream
	while (currentByte < fileData + length)
	{
		//find length of first token
		char* spacePoint = (char*)memchr(currentByte, ' ', 100);

		int len = spacePoint - currentByte;

		// The first token of a line could potentially be a vertex 'v' or something
		// longer like vt - get the length and then act appropriately
		switch (len)
		{
		case 1:
			ProcessOneLenTokens(&currentByte);
			break;
		case 2:
			ProcessTwoLenTokens(&currentByte);
			break;
		default:
			ProcessLongerTokens(&currentByte);
			break;
		}
	}

	//special case - last mesh always needs a call to finish the data
	FinishInfo();
	delete[] fileData;
}

float ObjParser::ReadFloat(char** pointer)
{
	//find length of first token
	char* spacePoint = (char*)memchr(*pointer, ' ', 100);
	int len = spacePoint - *pointer;

	float value = atof(*pointer);
	*pointer += len + 1;

	return value;
}

void ObjParser::ProcessOneLenTokens(char** pointer)
{
	size_t val = 4;
	char waste;
	XMFLOAT3 vert;
	char* spacePoint = NULL;
	char* newLinePoint = NULL;

	switch (**pointer)
	{
	case 'v':

		//if faces have just been read in then
		//this is a new mesh
		mReadingFaces = false;
		if (mFinishedVertexInfo) {
			FinishInfo();
		}
		*pointer += 3;
		vert.x = ReadFloat(pointer);
		vert.y = ReadFloat(pointer);
		vert.z = ReadFloat(pointer);
		mPositions.emplace_back(vert);
		IsMax(vert);
		IsMin(vert);
		break;
	case 'f':
		// check for format of the face info
		// if it is like 1/1/1 then split up for use
		// otherwise just read in as is
		mReadingFaces = true;
		//getline(ss, line);

		//newSS << line;
		//val = line.find('/');
		spacePoint = (char*)memchr(pointer, '/', 100);
		if (spacePoint != NULL)
		{
			// is this face info containing 3 vertices (a triangle)
			// or 4 vertices for a quad? If a quad we need to split into two triangles
			// by repeating some of the vertices (see later)
			// count the spaces to determine this
			//val = count(line.begin(), line.end(), ' ');

			if (val == 4)
			{
				for (unsigned int i = 0; i < 3; i++)
				{
					//newSS >> vertData;
					//ProcessFaceData(vertData);

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

				//newSS >> vertData;
				//ProcessFaceData(vertData);
				//prevVertData1 = vertData;

				//newSS >> vertData;
				//ProcessFaceData(vertData);

				//newSS >> vertData;
				//ProcessFaceData(vertData);
				//prevVertData2 = vertData;

				//ProcessFaceData(prevVertData1);
				//ProcessFaceData(prevVertData2);

				//newSS >> vertData;
				//ProcessFaceData(vertData);
			}
		}
		//newSS.clear();
		break;
	case '#':
		newLinePoint = (char*)memchr(*pointer, '\n', 100);
		*pointer = newLinePoint + 1;
		break;
	case 'g':
		// If there is a g this is the beginning of the face info and the end of the vertex info
		mFinishedVertexInfo = true;
		string name;
		//ss >> name;
		mMeshNames.emplace_back(name);
		break;
	}
}

void ObjParser::ProcessTwoLenTokens(char** pointer)
{
	XMFLOAT2 uv;
	XMFLOAT3 normal;

	switch (**pointer)
	{
	case 't':
		//ss >> uv.x;
		//ss >> uv.y;
		mUvs.emplace_back(uv);
		break;
	case 'n':
		//ss >> normal.x;
		//ss >> normal.y;
		//ss >> normal.z;
		mNormals.emplace_back(normal);
		break;
	}
}

void ObjParser::ProcessLongerTokens(char** pointer)
{
	/*if (firstToken == "usemtl")
	{
	if (mReadingFaces)
	{
	cout << "mid face material change" << endl;
	FinishInfo(false);
	}
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
	}*/

	char* newLinePoint = NULL;

	switch (**pointer)
	{
	case 'u':
		newLinePoint = (char*)memchr(*pointer, '\n', 100);
		*pointer = newLinePoint + 1;
		break;
	case 'm':
		newLinePoint = (char*)memchr(*pointer, '\n', 100);
		*pointer = newLinePoint + 1;
		break;
	default:
		newLinePoint = (char*)memchr(*pointer, '\n', 100);
		*pointer = newLinePoint + 1;
		break;
	}
}

#else
void ObjParser::Init()
{
	//reset max and min
	mTempMesh.max = XMFLOAT3(-9999.0f, -9999.0f, -9999.0f);
	mTempMesh.min = XMFLOAT3(9999.0f, 9999.0f, 9999.0f);
}

void ObjParser::CalcCenterAndSubtract()
{
	// make the min and max treat 0.0f as the center
	XMVECTOR min, max;
	min = XMLoadFloat3(&mTempMesh.min);
	max = XMLoadFloat3(&mTempMesh.max);
	XMVECTOR center = min + 0.5f * (max - min);
	min -= center;
	max -= center;
	XMStoreFloat3(&mTempMesh.min, min);
	XMStoreFloat3(&mTempMesh.max, max);

	// adjust all positions accordingly
	unsigned int size = mPositions.size();
	for (unsigned int i = 0; i < size; i++)
	{
		XMVECTOR pos = XMLoadFloat3(&mPositions[i]);
		pos -= center;
		XMStoreFloat3(&mPositions[i], pos);
	}

	XMStoreFloat3(&mTempMesh.center, center);
}

 void ObjParser::CreateNormals()
{
	// Go over each face - if normals aren't included in the .obj file
	// create a normal for that face
	// for each XMFLOAT3 create a list of face numbers
	// sum up the normals from each face for each XMFLOAT3
	// then normalize
	// hey vertices!


	unsigned int numFaces = (unsigned int)mTempMesh.indices.size() / 3;
	unsigned int numVerts = (unsigned int)mTempMesh.indices.size();

	// A vector of XMVECTORs storing the faces associated with each XMFLOAT3
	vector<XMVECTOR> faceNormals;

	// init the vector to the correct size by emplacing back the correct number of normals
	for (unsigned int i = 0; i < numVerts; i++)
	{
		faceNormals.emplace_back(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f));
	}

	// now go over each face and create the normals
	for (unsigned int face = 0; face < numFaces; face++)
	{
		unsigned int index = face * 3;
		unsigned int indexA = mTempMesh.indices[index];
		unsigned int indexB = mTempMesh.indices[index + 1];
		unsigned int indexC = mTempMesh.indices[index + 2];

		// because we are working through the complete mesh data put together from the different streams
		// in the .obj file look up from the fully constructed tempmesh, not the class member vectors
		XMVECTOR A = XMLoadFloat3(&mTempMesh.positions[indexA]);
		XMVECTOR B = XMLoadFloat3(&mTempMesh.positions[indexB]);
		XMVECTOR C = XMLoadFloat3(&mTempMesh.positions[indexC]);

		XMVECTOR first = B - A;
		XMVECTOR second = C - A;

		XMFLOAT3 faceNormal;
		XMStoreFloat3(&faceNormal, XMVector3Cross(first, second));
		XMVECTOR faceNormalVec = XMLoadFloat3(&faceNormal);
		faceNormals[indexA] += faceNormalVec;
		faceNormals[indexB] += faceNormalVec;
		faceNormals[indexC] += faceNormalVec;
	}

	// Now normalize all normals
	mNormals.reserve(numVerts);
	for (unsigned int i = 0; i < numVerts; i++)
	{
		faceNormals[i] = XMVector3Normalize(faceNormals[i]);
		XMStoreFloat3(&mNormals[i], faceNormals[i]);
	}

}

 void ObjParser::CreateTangents()
 {
	 // go over each face
	 // create tangent for the face
	 // for each XMFLOAT3 create a list of face numbers
	 // sum up tangents
	 // normalize

	 unsigned int numFaces = (unsigned int)mTempMesh.indices.size() / 3;
	 unsigned int numVerts = (unsigned int)mTempMesh.indices.size();

	 // A vector of vectors storing the faces associated with each XMFLOAT3
	 vector<XMVECTOR> faceTangents1;
	 vector<XMVECTOR> faceTangents2;
	 //faceNormals.reserve(numVerts);

	 for (unsigned int i = 0; i < numVerts; i++)
	 {
		 faceTangents1.emplace_back(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f));
		 faceTangents2.emplace_back(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f));
	 }

	 for (unsigned int face = 0; face < numFaces; face++)
	 {
		 unsigned int index = face * 3;
		 unsigned int indexA = mTempMesh.indices[index];
		 unsigned int indexB = mTempMesh.indices[index + 1];
		 unsigned int indexC = mTempMesh.indices[index + 2];
		 XMVECTOR A = XMLoadFloat3(&mTempMesh.positions[indexA]);
		 XMVECTOR B = XMLoadFloat3(&mTempMesh.positions[indexB]);
		 XMVECTOR C = XMLoadFloat3(&mTempMesh.positions[indexC]);

		 XMVECTOR first = B - A;
		 XMVECTOR second = C - A;

		 XMVECTOR S = XMLoadFloat2(&mTempMesh.uvs[indexA]);
		 XMVECTOR U = XMLoadFloat2(&mTempMesh.uvs[indexB]);
		 XMVECTOR V = XMLoadFloat2(&mTempMesh.uvs[indexC]);

		 XMVECTOR UV1 = U - S;
		 XMVECTOR UV2 = V - S;

		 XMFLOAT2 s, t;
		 XMStoreFloat2(&s, UV1);
		 XMStoreFloat2(&t, UV2);

		 XMFLOAT3 x, y;
		 XMStoreFloat3(&x, first);
		 XMStoreFloat3(&y, second);

		 float r = 1.0f / (s.x * t.y - t.x * s.y);

		 XMFLOAT3 sDir(r * (t.y * x.x - s.y * y.x),
						 r * (t.y * x.y - s.y * y.y),
						 r * (t.y * x.z - s.y * y.z));

		 XMFLOAT3 tDir(r * (s.x * y.x - t.x * x.x),
						r * (s.x * y.y - t.x * x.y),
						r * (s.x * y.z - t.x * x.z));


		 XMVECTOR SDIR;
		 SDIR = XMLoadFloat3(&sDir);
		 faceTangents1[indexA] += SDIR;
		 faceTangents1[indexB] += SDIR;
		 faceTangents1[indexC] += SDIR;

		 XMVECTOR TDIR;
		 TDIR = XMLoadFloat3(&tDir);
		 faceTangents1[indexA] += TDIR;
		 faceTangents1[indexB] += TDIR;
		 faceTangents1[indexC] += TDIR;
	 }

	 for (unsigned int i = 0; i < numVerts; i++)
	 {
		 unsigned int index = mTempMesh.indices[i];
		 XMVECTOR n =  XMLoadFloat3(&mTempMesh.normals[index]);
		 XMVECTOR t1 = faceTangents1[index];
		 XMVECTOR t2 = faceTangents2[index];

		 XMVECTOR tangent;
		 XMFLOAT3 finishedTangent;
		 tangent = XMVector3Normalize(t1 - n * XMVector3Dot(n, t1));
		 XMStoreFloat3(&finishedTangent, tangent);
		 mTangents.emplace_back(finishedTangent);
	 }
 }

 void ObjParser::FinishInfo(bool materialChange)
{
	// there may be more than one mesh in an .obj, classic example being the sponza .obj
	// every time the finishing step for parsing mesh data is reached we increment total
	static unsigned int meshNum = 0;
	meshNum++;
	
	// adjust offset nums only if this is the end of a mesh and not a material change
	// halfway through a face listing
	if (!materialChange)
	{
		mOffsetUv += (unsigned int)mUvs.size();
		mOffsetVert += (unsigned int)mPositions.size();
		mOffsetNorm += (unsigned int)mNormals.size();
	}

	// copy the indices over, and clear as
	// there will be different indices for any following mesh material
	// change or not
	mTempMesh.indices = mIndices;
	mCurrIndex = 0;
	mIndices.clear();

	if (mMatParser.GetMaterial(mTempMesh.materialIndex).normMapIndex != 0)
	{
		CreateTangents();

		mTempMesh.tangents = mTangents;
		mTempMesh.isNormalMapped = true;
	}
	
	if (!mHasNormalsInc)
		CreateNormals();
	
	// store the mesh for writing to the output file later
	mMeshes.emplace_back(mTempMesh);
	
	// output the size of the vertice and indices to the standard out
	cout << mPositions.size() << " " << mTempMesh.indices.size() << " " << endl;

	// now clear stored positions if needed - 
	// in the case of meshes in an .obj that change material halfway through a listing of faces - 
	// so we will need the stored postions, normals etc untouched if this is the case
	if (!materialChange)
	{
		mPositions.clear();
		mNormals.clear();
		mUvs.clear();
		mTangents.clear();
		faceIndices.clear();
		// mark flag
		mFinishedVertexInfo = false;
	}

	// clear the data stored in the tempMesh as if starting from fresh
	// reset max and min
	mTempMesh.max = XMFLOAT3(-9999.0f, -9999.0f, -9999.0f);
	mTempMesh.min = XMFLOAT3(9999.0f, 9999.0f, 9999.0f);

	// reset all vectors holding vertex related data
	mTempMesh.positions.clear();
	mTempMesh.normals.clear();
	mTempMesh.uvs.clear();
	mTempMesh.tangents.clear();

	// reset so isn't normal mapped to start with
	mTempMesh.isNormalMapped = false;
}

 void ObjParser::CreateVertex(string& faceDescription)
 {
	 stringstream temp;
	 temp << faceDescription;
	 unsigned int index;
	 char waste;

	 // parse the vert by reading in the index number of the vert, then read the / in and throw it away
	 // by parsing into 'waste'
	 temp >> index;
	 temp >> waste;

	 // look up the relevant position in the read in positions for this mesh
	 XMFLOAT3 pos = mPositions[index - mOffsetVert];

	 //do the same as above, but for uvs and normals
	 temp >> index;
	 temp >> waste;

	 XMFLOAT2 uv = mUvs[index - mOffsetUv];

	 if (mHasNormalsInc)
	 {
		 temp >> index;
		 temp >> waste;

		 XMFLOAT3 normal = mNormals[index - mOffsetNorm];
		 mTempMesh.normals.emplace_back(normal);
	 }

	 // now place the extracted pos/norm/uv into the vector streams for the 
	 // temp mesh
	 mTempMesh.positions.emplace_back(pos);
	 mTempMesh.uvs.emplace_back(uv);
 }

 void ObjParser::ProcessFaceData(string& data)
 {

	 // does the string describing the face exist in the map?
	 size_t val = faceIndices.count(data);

	 if (val == 0)
	 {
		 // this vertex hasn't been created yet, so 
		 // will need to create entry in the map for it
		 pair<string, unsigned int> newPair;
		 // the key is the unparsed string describing the vertex 
		 newPair.first = data;
		 // the data is the current index number we have incremented to
		 newPair.second = mCurrIndex;
		 faceIndices.insert(newPair);
		 mIndices.emplace_back(mCurrIndex++);
		 CreateVertex(data);
	 }
	 else
	 {
		 // this vertex exists in the map, so just use the stored index
		 // with the vertex as the key
		 unsigned int index = faceIndices[data];
		 mIndices.emplace_back(index);
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

	//While there's still info in the stringstream
	while (!inFile.eof())
	{
		string firstToken;
		inFile >> firstToken;

		int len = (int)firstToken.length();

		// The first token of a line could potentially be a vertex 'v' or something
		// longer like vt - get the length and then act appropriately
		switch (len)
		{
		case 1:
			ProcessOneLenTokens(inFile, firstToken[0]);
			break;
		case 2:
			ProcessTwoLenTokens(inFile, firstToken);
			break;
		default:
			ProcessLongerTokens(inFile, firstToken);
			break;
		}
	}

	//special case - last mesh always needs a call to finish the data
	FinishInfo();
}

void ObjParser::ProcessOneLenTokens(ifstream& ss, char firstToken)
{
	string line;
	stringstream newSS, subSS;
	size_t val;
	XMFLOAT3 vert;
	string vertData, prevVertData1, prevVertData2;

	switch (firstToken)
	{
	case 'v':

		//if faces have just been read in then
		//this is a new mesh
		mReadingFaces = false;
		if (mFinishedVertexInfo) {
			FinishInfo();
		}

		ss >> vert.x;
		ss >> vert.y;
		ss >> vert.z;
		mPositions.emplace_back(vert);
		IsMax(vert);
		IsMin(vert);
		break;
	case 'f':
		// check for format of the face info
		// if it is like 1/1/1 then split up for use
		// otherwise just read in as is

		if (!mReadingFaces)
		{
			// calc center
			// subtract from all positions
			CalcCenterAndSubtract();
		}

		// mark reading faces bool as true, so if "usemtl" pops up we know its a mid-mesh
		// material change
		mReadingFaces = true;
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
		mFinishedVertexInfo = true;
		string name;
		ss >> name;
		mMeshNames.emplace_back(name);
		break;
	}
}

void ObjParser::ProcessTwoLenTokens(ifstream& ss,
	string& firstToken)
{
	XMFLOAT2 uv;
	XMFLOAT3 normal;

	switch (firstToken[1])
	{
	case 't':
		ss >> uv.x;
		ss >> uv.y;
		mUvs.emplace_back(uv);
		break;
	case 'n':
		mHasNormalsInc = true;
		ss >> normal.x;
		ss >> normal.y;
		ss >> normal.z;
		mNormals.emplace_back(normal);
		break;
	}
}

void ObjParser::ProcessLongerTokens(ifstream& ss,
	string& firstToken)
{
	if (firstToken == "usemtl")
	{
		if (mReadingFaces)
		{
			cout << "mid face material change" << endl;
			FinishInfo(true);
		}
		string matName;
		ss >> matName;
		unsigned int matIndex = mMatParser.GetMaterialIndex(matName);
		mTempMesh.materialIndex = matIndex;
	}
	else if (firstToken == "mtllib")
	{
		string matFile;
		ss >> matFile;
		mMatParser.ReadMaterials(mCurrentPath + matFile);
	}
}
#endif



 void ObjParser::IsMin(XMFLOAT3& vert)
 {
	 if (vert.x < mTempMesh.min.x)
	 {
		 mTempMesh.min.x = vert.x;
	 }

	 if (vert.y < mTempMesh.min.y)
	 {
		 mTempMesh.min.y = vert.y;
	 }

	 if (vert.z < mTempMesh.min.z)
	 {
		 mTempMesh.min.z = vert.z;
	 }
 }

 void ObjParser::IsMax(XMFLOAT3& vert)
 {
	 if (vert.x > mTempMesh.max.x)
	 {
		 mTempMesh.max.x = vert.x;
	 }

	 if (vert.y > mTempMesh.max.y)
	 {
		 mTempMesh.max.y = vert.y;
	 }

	 if (vert.z > mTempMesh.max.z)
	 {
		 mTempMesh.max.z = vert.z;
	 }
 }

 void ObjParser::WriteOutData(string& outFile)
{
	// Now write out to a file in binary
	ofstream out;
	out.open(outFile, ios::binary);

	unsigned int numMeshes = (unsigned int)mMeshes.size() - 2;
	out.write((char*)&numMeshes, sizeof(unsigned int));
	unsigned int numMaterials = (unsigned int)mMatParser.GetNumMaterials();
	out.write((char*)&numMaterials, sizeof(unsigned int));
	

	for (unsigned int i = 0; i < numMeshes; i++)
	{
		if (i == 4) continue;

		MeshData mesh = mMeshes[i];

		// For each mesh/ submesh write a header that contains num of verts and indices
		headerInfo info;
		info.numverts = (int)mesh.positions.size();
		info.numIndices = (int)mesh.indices.size();
		info.materialIndex = mesh.materialIndex;
		out.write((char*)&info, sizeof(headerInfo));

		out.write((char*)&mesh.isNormalMapped, sizeof(bool));

		out.write((char*)&mesh.positions[0], sizeof(XMFLOAT3) * info.numverts);
		out.write((char*)&mesh.normals[0], sizeof(XMFLOAT3) * info.numverts);
		out.write((char*)&mesh.uvs[0], sizeof(XMFLOAT2) * info.numverts);

		// Now write out the raw data to the file
		if (mesh.isNormalMapped)
		{
			out.write((char*)&mesh.tangents[0], sizeof(XMFLOAT3) * info.numverts);
		}

		out.write((char*)&mesh.indices[0], sizeof(unsigned int) * info.numIndices);
		
		//write out min and max
		out.write((char*)&mesh.min, sizeof(XMFLOAT3));
		out.write((char*)&mesh.max, sizeof(XMFLOAT3));
		out.write((char*)&mesh.center, sizeof(XMFLOAT3));
	}

	mMatParser.WriteOutMaterials(out);
	out.close();
}