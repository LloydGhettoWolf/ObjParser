#include "ObjParser.h"

 void ObjParser::CreateUVs(MeshData* data)
{
	// Use pre-collected UVs and index list 
	// to add correct uv information to the space
	// allocated to the UVs in the meshdata

	int numVerts = data->vertices.size();

	for (unsigned int i = 0; i < numVerts; i++)
	{
		data->vertices[i].uv = uvs[texIndices[i]];
	}

	// now data is assigned, clear the texIndices and
	// uv vectors
	uvs.clear();
	texIndices.clear();
}

 void ObjParser::CreateNormals(MeshData* data)
{
	// Go over each face
	// create a normal for that face
	// for each XMFLOAT3 create a list of face numbers
	// sum up the normals from each face for each XMFLOAT3
	// then normalize
	// hey vertices!


	unsigned int numFaces = data->indices.size() / 3;
	unsigned int numVerts = data->vertices.size();

	// A vector of vectors storing the faces associated with each XMFLOAT3
	vector<XMVECTOR> faceNormals;
	faceNormals.reserve(numVerts);

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
		XMVECTOR A = XMLoadFloat3(&data->vertices[indexA].position);
		XMVECTOR B = XMLoadFloat3(&data->vertices[indexB].position);
		XMVECTOR C = XMLoadFloat3(&data->vertices[indexC].position);

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
	for (unsigned int i = 0; i < numVerts; i++)
	{
		faceNormals[i] = XMVector3Normalize(faceNormals[i]);
		XMFLOAT3 destnorm;
		XMStoreFloat3(&destnorm, faceNormals[i]);
		data->vertices[i].normal = destnorm;
	}

}

 void ObjParser::FinishInfo(MeshData* data)
{
	// create normals and uvs for the mesh/submesh
	CreateNormals(data);
	texOffsetNum += uvs.size();
	CreateUVs(data);

	// adjust texoffset num
	offsetNum += data->vertices.size();

	// output the size of the vertice and indices to the standard out
	cout << data->vertices.size() << " " << data->indices.size() << " " << endl;

	// store the mesh for writing to the output file later
	meshes.emplace_back(*data);
	// now clear the current temp mesh data
	data->indices.clear();
	data->vertices.clear();

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
		data->vertices.emplace_back(vert);
		break;
	case 'f':
		// check for format of the face info
		// if it is like 1/1/1 then split up for use
		// otherwise just read in as is

		getline(ss, line);

		newSS << line;
		val = line.find('//');
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

					newSS >> texIndex;
					texIndices.emplace_back(texIndex - texOffsetNum);

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
		XMFLOAT2 uv;
		ss >> uv.x;
		ss >> uv.y;
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

		int len = firstToken.length();

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
				 fileContents >> info.materialName;
				 firstMaterial = false;
			 }
			 else
			 {
				 // Push prev material into vector
				 materials.emplace_back(info);
				 pair<string, unsigned int> matPair;
				 matPair.first = info.materialName;
				 matPair.second = materials.size() - 1;
				 materialNames.insert(matPair);

				 //clear info for new material and record name
				 memset(&info, 0, sizeof(materialInfo));
				 fileContents >> info.materialName;
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

			 if (texNames.count(texName) != 0)
			 {
				 pair<string, unsigned int> newPair;
				 newPair.first = texName;
				 newPair.second = texIndex++;
				 index = newPair.second;
				 texNames.insert(newPair);
			 }
			 else
			 {
				 index = texNames[texName];
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

 }

 void ObjParser::WriteOutData(string& outFile)
{
	// Now write out to a file in binary
	ofstream out;
	out.open(outFile, ios::binary);

	int size = meshes.size();
	out << size;
	for (int i = 0; i < size; i++)
	{
		MeshData mesh = meshes[i];

		// For each mesh/ submesh write a header that contains num of verts and indices
		headerInfo info;
		info.numverts = mesh.vertices.size();
		info.numIndices = mesh.indices.size();
		info.materialIndex = mesh.materialIndex;
		out.write((char*)&info, sizeof(headerInfo));

		// Now write out the raw data to the file
		out.write((char*)&mesh.vertices[0], sizeof(VertexType) * info.numverts);
		out.write((char*)&mesh.indices[0], sizeof(unsigned int) * info.numIndices);
	}

	//now write the material info out too
	size = materials.size();
	out << size;
	for (int i = 0; i < size; i++)
	{
		materialInfo info = materials[i];
		out.write((char*)&info, sizeof(materialInfo));
	}

	out.close();
}