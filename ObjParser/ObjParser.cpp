#include "ObjParser.h"

 void ObjParser::CreateUVs(MeshData* data)
{
	// Use pre-collected UVs and index list 
	// to add correct uv information to the space
	// allocated to the UVs in the meshdata

	unsigned int numIndices = (unsigned int)data->indices.size();

	for (unsigned int i = 0; i < numIndices; i++)
	{
		unsigned int index = data->indices[i];
		data->vertices[index].uv = uvs[texIndices[i]];
	}

}

 void ObjParser::CreateNormals(MeshData* data)
{
	// Go over each face
	// create a normal for that face
	// for each XMFLOAT3 create a list of face numbers
	// sum up the normals from each face for each XMFLOAT3
	// then normalize
	// hey vertices!


	unsigned int numFaces = (unsigned int)data->indices.size() / 3;
	unsigned int numVerts = (unsigned int)data->vertices.size();

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
		unsigned int indexA = data->indices[index];
		unsigned int indexB = data->indices[index + 1];
		unsigned int indexC = data->indices[index + 2];
		XMVECTOR A = XMLoadFloat4(&data->vertices[indexA].position);
		XMVECTOR B = XMLoadFloat4(&data->vertices[indexB].position);
		XMVECTOR C = XMLoadFloat4(&data->vertices[indexC].position);

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
		XMStoreFloat4(&data->vertices[i].normal, faceNormals[i]);
	}

}

 void ObjParser::FinishInfo(MeshData* data)
{
	static int meshNum = 0;
	meshNum++;
	// create normals and uvs for the mesh/submesh
	CreateNormals(data);
	texOffsetNum += (unsigned int)uvs.size();
	CreateUVs(data);

	// adjust texoffset num
	offsetNum += (unsigned int)data->vertices.size();

	// output the size of the vertice and indices to the standard out
	cout << data->vertices.size() << " " << data->indices.size() << " " << endl;

	// store the mesh for writing to the output file later
	meshes.emplace_back(*data);
	// now clear the current temp mesh data
	data->indices.clear();
	data->vertices.clear();

	// now data is assigned, clear the texIndices and
	// uv vectors
	uvs.clear();
	texIndices.clear();

	finishedVertexInfo = false;
}

 void ObjParser::ProcessOneLenTokens(MeshData* data,
	stringstream& ss,
	char firstToken)
{
	VertexType vert;
	unsigned int index, prevIndex1, prevIndex2;
	unsigned int uvIndex, prevUvIndex1, prevUvIndex2;
	string line;
	stringstream newSS;
	size_t val;
	char waste;

	switch (firstToken)
	{
	case 'v':

		//if faces have just been read in then
		//this is a new mesh
		if (finishedVertexInfo) {
			FinishInfo(data);
		}

		ss >> vert.position.x;
		ss >> vert.position.y;
		ss >> vert.position.z;
		vert.position.w = 1.0f;
		data->vertices.emplace_back(vert);
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
				// simple, just iterate over this code 3 times
				for (int i = 0; i < 3; i++)
				{
					newSS >> index;
					data->indices.emplace_back(index - offsetNum);
					newSS >> waste;

					newSS >> uvIndex;
					texIndices.emplace_back(uvIndex - texOffsetNum);

					newSS.ignore(INT_MAX, ' ');
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

				newSS >> index;
				prevIndex1 = index;
				data->indices.emplace_back(index - offsetNum);
				newSS >> waste;
				newSS >> uvIndex;
				prevUvIndex1 = uvIndex;
				texIndices.emplace_back(uvIndex - texOffsetNum);
				newSS.ignore(INT_MAX, ' ');

				newSS >> index;
				data->indices.emplace_back(index - offsetNum);
				newSS >> waste;
				newSS >> uvIndex;
				texIndices.emplace_back(uvIndex - texOffsetNum);
				newSS.ignore(INT_MAX, ' ');

				newSS >> index;
				prevIndex2 = index;
				data->indices.emplace_back(index - offsetNum);
				newSS >> waste;
				newSS >> uvIndex;
				prevUvIndex2 = uvIndex;
				texIndices.emplace_back(uvIndex - texOffsetNum);

				data->indices.emplace_back(prevIndex1 - offsetNum);
				data->indices.emplace_back(prevIndex2 - offsetNum);
				texIndices.emplace_back(prevUvIndex1 - texOffsetNum);
				texIndices.emplace_back(prevUvIndex2 - texOffsetNum);
				newSS.ignore(INT_MAX, ' ');
				newSS >> index;
				data->indices.emplace_back(index - offsetNum);
				newSS >> waste;
				newSS >> uvIndex;
				texIndices.emplace_back(uvIndex - texOffsetNum);
			}
		}
		else
		{
			newSS >> index;
			data->indices.emplace_back(index - offsetNum);
			newSS >> index;
			data->indices.emplace_back(index - offsetNum);
			newSS >> index;
			data->indices.emplace_back(index - offsetNum);
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
		meshNames.emplace_back(name);
		break;
	}
}

 void ObjParser::ProcessTwoLenTokens(MeshData* data,
									stringstream& ss,
									string& firstToken)
{
	switch (firstToken[1])
	{
	case 't':
		XMFLOAT4 uv;
		ss >> uv.x;
		ss >> uv.y;
		uv.z = 0.0f;
		uv.w = 0.0f;
		uvs.emplace_back(uv);
		break;
	}
}

 void ObjParser::ProcessLongerTokens(MeshData* data, 
									 stringstream& ss, 
									 string& firstToken)
 {
	 if (firstToken == "usemtl")
	 {
		 string matName;
		 ss >> matName;
		 unsigned int matIndex = materialNames[matName];
		 data->materialIndex = matIndex;
	 }
	 else if (firstToken == "mtllib")
	 {
		 string matFile;
		 ss >> matFile;
		 ReadMaterials(matFile);
	 }
 }

MeshData* ObjParser::ReadObjFile(string& filePath, string& fileName)
{
	ifstream inFile;
	string line;

	MeshData* data = new MeshData();
	currentPath = filePath;
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
				ProcessOneLenTokens(data, ss, firstToken[0]);
				break;
			case 2:
				ProcessTwoLenTokens(data, ss, firstToken);
				break;
			default:
				ProcessLongerTokens(data, ss, firstToken);
				break;
		}
	}

	//special case - last mesh always needs a call to finish the data
	FinishInfo(data);
	return data;
}


 void ObjParser::ReadMaterials(string& fileName)
 {
	 ifstream materialFile;
	 materialFile.open(currentPath + fileName);

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
				 fileContents >> name;
				 firstMaterial = false;
			 }
			 else
			 {
				 // Push prev material into vector
				 materials.emplace_back(info);
				 pair<string, unsigned int> matPair;
				 matPair.first = name;
				 matPair.second = (unsigned int)materials.size() - 1;
				 materialNames.insert(matPair);

				 //clear info for new material and record name
				 memset(&info, 0, sizeof(materialInfo));
				 fileContents >> name;
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
			 auto result = find(textures.begin(), textures.end(), texName);
			 if (result == textures.end())
			 {
				 textures.emplace_back(texName);
				 index = texIndex++;
			 }
			 else
			 {
				 index = (unsigned int)(result - textures.begin());
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
	 materials.emplace_back(info);
	 pair<string, unsigned int> matPair;
	 matPair.first = name;
	 matPair.second = (unsigned int)materials.size() - 1;
	 materialNames.insert(matPair);

 }

 void ObjParser::WriteOutData(string& outFile)
{
	// Now write out to a file in binary
	ofstream out;
	out.open(outFile, ios::binary);

	unsigned int size = (unsigned int)meshes.size();
	out.write((char*)&size, sizeof(unsigned int));
	unsigned int numMaterials = (unsigned int)materials.size();
	out.write((char*)&numMaterials, sizeof(unsigned int));
	

	for (unsigned int i = 0; i < size; i++)
	{
		MeshData mesh = meshes[i];

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
		out.write((char*)&materials[i], sizeof(materialInfo));
	}

	out.write((char*)&texIndex, sizeof(unsigned int));

	//now write out names of textures
	for (auto i = textures.begin(); i != textures.end(); i++)
	{
		string value = *i;
		size_t len = value.length();
		out.write((char*)&len, sizeof(unsigned int));
		out.write(value.c_str(), sizeof(char) * len);
	}

	out.close();
}