// NewObjParser - 
// a more functional, clean, less confusing way to have an parser of .obj files

#include <fstream>
#include <string>
#include <map>
#include <sstream>
#include <iostream>
#include "NewParser.h";
using namespace std;
using namespace DirectX;

// ReadObjFile - is void as IO is impure. Do all work of parsing - spit out a vector of meshes at the end
// returns vector of meshes
// arguments:
// char* fileName - fileName of the file to parse - can be trimmed to determine folder later



void ReadObjFile(char* fileName)
{
	vector<XMFLOAT3> positions, normals;
	vector<XMFLOAT2> UVs;
	vector<unsigned int> indices;

	vector<MeshData> meshes;

	map<string, unsigned int> faceIndices;

	vector<materialInfo> materials;

	MeshData currentMesh;

	unsigned int currentTotals[3] = { 1, 1, 1 }; 

	ifstream inFile;
	inFile.open(fileName);

	STATE currentState = READING_VERTS;

	while (!inFile.eof())
	{
		char firstToken, secondToken;
		inFile >> firstToken;

		// using the value of the first character proceed appropriately
		string line;
		unsigned int val;

		XMFLOAT3 position, normal;
		XMFLOAT2 uv;
		MeshVertex vertex(position, uv, normal);

		switch (firstToken)
		{
			// If it's a type of vertex - what type is it?
			// use a second switch and go from there
			case 'v':
				if (currentState == READING_FACES)
				{
					currentTotals[0] += positions.size();
					currentTotals[1] += UVs.size();
					currentTotals[2] += normals.size();

					cout << faceIndices.size() << " " << indices.size() << endl;

					positions.clear();
					UVs.clear();
					normals.clear();

					currentMesh.indices = indices;
					indices.clear();
					faceIndices.clear();

					meshes.emplace_back(currentMesh);
					currentMesh = NewMesh();

					currentState = READING_VERTS;
				}

				// get the second token
				inFile >> secondToken;

				switch (secondToken)
				{
					case 't':
						uv = ReadUV(inFile);
						UVs.emplace_back(uv);
						break;
					case 'n':
						normal = ReadPosition(inFile);
						normals.emplace_back(normal);
						break;
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
					case '-':
						inFile.seekg(-1, ios_base::cur);
						position = ReadPosition(inFile);
						positions.emplace_back(position);
						break;
				}

				break;

			// If it's a face
			case 'f':
			{
				inFile.seekg(-1, ios_base::cur);
				getline(inFile, line);
				stringstream input(line);
				input.seekg(1, ios_base::cur);
				// check for number of spaces as 4 means triangle but 5 means quad, 
				// and for a quad we will need to break into two triangles
				val = count(line.begin(), line.end(), ' ');
				if (val == 4)
				{
					for (int i = 0; i < 3; i++)
					{
						string data;
						input >> data;
						unsigned int sizeBefore = faceIndices.size();
						faceIndices = AddToFaceMap(data, std::move(faceIndices));
						unsigned int sizeAfter = faceIndices.size();
						indices.emplace_back(faceIndices[data]);

						if (sizeBefore != sizeAfter)
						{
							vertex = CreateMeshVertex(data, positions, UVs, normals, currentTotals);
							AddvertexToMesh(vertex, std::move(currentMesh));
						}
					}
				}
				else if (val == 5)
				{
					string data[4];
					input >> data[0];
					input >> data[1];
					input >> data[2];
					input >> data[3];

					unsigned int indexOrder[] = { 0, 1, 2, 0, 2, 3 };

					for (int i = 0; i < 6; i++)
					{
						unsigned int indx = indexOrder[i];
						unsigned int sizeBefore = faceIndices.size();
						faceIndices = AddToFaceMap(data[indx], std::move(faceIndices));
						unsigned int sizeAfter = faceIndices.size();
						indices.emplace_back(faceIndices[data[indx]]);

						if (sizeBefore != sizeAfter)
						{
							vertex = CreateMeshVertex(data[indx], positions, UVs, normals, currentTotals);
							currentMesh = AddvertexToMesh(vertex, std::move(currentMesh));
						}
					}
				}
			}

				break;

			// If it's a comment ignore
			case '#':
				inFile.ignore(INT_MAX, '\n');
				break;

			// This should be the beginning of the face info, end of vertex info
			case 'g':
				currentState = READING_FACES;
				break;

			// This should be 'u' for 'usemtl'
			case 'u':
				{
					inFile.ignore(INT_MAX, ' ');
					string matName;
					inFile >> matName;
					currentMesh.materialIndex = FindIndexInVector(mat)
				}
				break;

			// this should be info about a material library
			case 'm':
				{
					inFile.ignore(INT_MAX, ' ');
					string matFile;
					inFile >> matFile;

				}
				break;

			case 's':
				// This parser ingnores smoothing faces
				inFile.ignore(INT_MAX, '\n');
				break;

			default:
				inFile.ignore(INT_MAX, '\n');
		}
	}

	inFile.close();
}

XMFLOAT3 ReadPosition(ifstream& inFile)
{
	float x, y, z;
	inFile >> x;
	inFile >> y;
	inFile >> z;

	return XMFLOAT3(x, y, z);
}

XMFLOAT2 ReadUV(ifstream& inFile)
{
	float u, v;
	inFile >> u;
	inFile >> v;

	return XMFLOAT2(u, v);
}

map<string, unsigned int> AddToFaceMap(const string& data,  map<string, unsigned int>&& faces)
{
	unsigned int index;
	// does this exist already in the map?
	auto iterator = faces.find(data);

	if (iterator == faces.end())
	{
		index = faces.size();
		faces[data] = index;
	}

	return faces;
}

MeshVertex CreateMeshVertex(string& data, const vector<XMFLOAT3>& positions, const vector<XMFLOAT2>& uvs, const vector<XMFLOAT3>& normals, unsigned int* currentTotals)
{
	stringstream temp;
	temp << data;
	unsigned int posIndex, uvIndex, normIndex;
	char waste;

	// parse the vert by reading in the index number of the vert, then read the / in and throw it away
	// by parsing into 'waste'
	temp >> posIndex;
	temp >> waste;

	//do the same as above, but for uvs and normals
	temp >> uvIndex;
	temp >> waste;

	temp >> normIndex;
	temp >> waste;

	return MeshVertex(positions[posIndex - currentTotals[0]], uvs[uvIndex - currentTotals[1]], normals[normIndex - currentTotals[2]]);
}

MeshData AddvertexToMesh(const MeshVertex& vertex, MeshData&& mesh)
{
	mesh.positions.emplace_back(vertex.position);
	mesh.uvs.emplace_back(vertex.UV);
	mesh.normals.emplace_back(vertex.normal);
	return mesh;
}

MeshData NewMesh()
{
	return MeshData();
}

vector<materialInfo> ReadMaterials(char* fileName)
{
	vector<materialInfo> materials;
	map<string, unsigned int> materialNames;
	materialInfo currentInfo;
	ifstream materialFile;
	materialFile.open(fileName);

	memset(&currentInfo, 0, sizeof(materialInfo));
	bool firstMaterial = true;

	string materialName;

	vector<string> textureNames;

	unsigned int matIndex;
	unsigned int texIndex;

	while (!materialFile.eof())
	{
		string token;
		float r, g, b;

		materialFile >> token;
		if (token == "newmtl")
		{

			if (firstMaterial)
			{
				materialFile >> materialName;
				firstMaterial = false;
			}
			else
			{
				// Push prev material into vector
				materials.emplace_back(currentInfo);
				pair<string, unsigned int> matPair;
				matPair.first = materialName;
				matPair.second = (unsigned int)materials.size() - 1;
				materialNames.insert(matPair);

				//clear info for new material and record name
				memset(&currentInfo, 0, sizeof(materialInfo));
				materialFile >> materialName;
			}
		}
		else if (token[0] == 'K')
		{
			currentInfo = GetMaterialColor(materialFile);
		}
		else if (token[0] == 'N')
		{
			float specComponent;
			materialFile >> specComponent;

			switch (token[1])
			{
			case 's':
				currentInfo.specFactor = specComponent;
				break;
			}
		}
		else if (token.substr(0, 3) == "map")
		{
			string texName;
			materialFile >> texName;

			textureNames = AddTexture(textureNames, texName);
			unsigned int index = FindIndexInVector(textureNames, texName);

			currentInfo = SetIndices(token, currentInfo, index);
		}
	}

	// Push prev material into vector
	materials.emplace_back(currentInfo);
	pair<string, unsigned int> matPair;
	matPair.first = materialName;
	matPair.second = (unsigned int)materials.size() - 1;
	materialNames.insert(matPair);
	return materials;
}

materialInfo GetMaterialColor(ifstream& materialFile, materialInfo material)
{
	float r, g, b;

	materialFile >> r;
	materialFile >> g;
	materialFile >> b;

	XMFLOAT3 val(r, g, b);

	switch (token[1])
	{
	case 's':
		material.specular = val;
		break;
	case 'd':
		material.diffuse = val;
		break;
	case 'a':
		material.ambient = val;
		break;
	}

	return material;
}

vector<string> AddTexture(vector<string> textures, const string& textureName)
{
	auto result = find(textures.begin(), textures.end(), textureName);
	if (result == textures.end())
	{
		textures.emplace_back(textureName);
	}

	return textures;
}

unsigned int FindIndexInVector(const vector<string>& values, const string& name)
{
	auto result = find(values.begin(), values.end(), name);
	return result - values.begin();
}

materialInfo SetIndices(const string& token, materialInfo info, unsigned int index)
{
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
	else if (token == "map_d")
	{
		info.maskIndex = index;
	}

	return info;
}