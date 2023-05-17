#include "Mesh.h"
#include <fstream>

#pragma optimize( "g", off )

void Game::loadStars(Vertex* &vertices, int count, float r)
{
	vertices = new Vertex[count];
	for (int i = 0; i < count; i++)
	{
		float phi = XM_PI * rand() / RAND_MAX;
		float ksi = XM_2PI * rand() / RAND_MAX;

		vertices[i] = { XMFLOAT4(r * sin(phi) * cos(ksi),
			r * sin(phi) * sin(ksi),
			r * cos(phi), 1.0f),
			XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f) };
	}
}

void Game::createSphere(Vertex* &vertices, WORD* &indices, int slices, int stacks)
{
	int vertices_count = slices * stacks + 2;
	int indices_count = slices * stacks * 6;
	vertices = new Vertex[vertices_count];
	indices = new WORD[indices_count];
	float delta = 1E-3;

	vertices[0] = { XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) };
	vertices[slices * stacks + 1] = { XMFLOAT4(0.0f, -1.0f, 0.0f, 1.0f), XMFLOAT4(0.0f, -1.0f, 0.0f, 1.0f) };
	int cc = 0;

	for (int m = 1; m <= slices; m++)
	for (int n = 1; n <= stacks; n++)
	{
		float x = sin(XM_PI * m / slices) * cos(XM_2PI * n / stacks);
		float z = sin(XM_PI * m / slices) * sin(XM_2PI * n / stacks);
		float y = cos(XM_PI * m / slices);

		XMVECTOR norm = XMVector3Normalize(XMVectorSet(x, y, z, 1.0f));

		vertices[(m - 1) * stacks + n] = { XMFLOAT4(x, y, z, 1.0f), XMFLOAT4(XMVectorGetX(norm), XMVectorGetY(norm), XMVectorGetZ(norm), 1.0f) };
	}

	int counter = 0;

	int first, second, third;

	for (int i = 1; i < stacks; i++)
	{
		indices[counter++] = 0;
		indices[counter++] = i;
		indices[counter++] = i + 1;
	}

	indices[counter++] = 0;
	indices[counter++] = stacks;
	indices[counter++] = 1;

	for (int i = 0; i < slices - 1; i++)
	{
		for (int j = 1; j < stacks; j++)
		{
			first = i * stacks + j;
			second = first + stacks;
			third = second + 1;

			indices[counter++] = first;
			indices[counter++] = second;
			indices[counter++] = third;

			second = third;
			third = first + 1;

			indices[counter++] = first;
			indices[counter++] = second;
			indices[counter++] = third;
		}

		first = i * stacks + stacks;
		second = first + stacks;
		third = first + 1;

		indices[counter++] = first;
		indices[counter++] = second;
		indices[counter++] = third;

		second = third;
		third = i * stacks + 1;

		indices[counter++] = first;
		indices[counter++] = second;
		indices[counter++] = third;

	}

	for (int i = 1; i < stacks; i++)
	{
		indices[counter++] = slices * stacks + 1;
		indices[counter++] = slices * stacks - i + 1;
		indices[counter++] = slices * stacks - i;
	}

	indices[counter++] = slices * stacks + 1;
	indices[counter++] = slices * stacks - stacks + 1;
	indices[counter++] = slices * stacks;
}

string Game::fileExtension(string& FileName)
{
	if (FileName.find_last_of(".") != std::string::npos)
		return FileName.substr(FileName.find_last_of(".") + 1);
	return "";
}

HRESULT Game::loadMeshFromFile_ply(string fileName, vector<Vertex> &vertices, vector<WORD> &indices)
{
	ifstream plyFile(fileName);

	vertices = vector<Vertex>();
	indices  = vector<WORD>();

	string str;
	while (str != "vertex")
		plyFile >> str;

	int vertices_count;
	plyFile >> vertices_count;

	while (str != "face")
		plyFile >> str;

	int faces_count;
	plyFile >> faces_count;

	while (str != "end_header")
		plyFile >> str;

	Vertex v;
	v.normal.w = v.pos.w = 1.0f;

	for (int i = 0; i < vertices_count; i++)
	{
		plyFile >> v.pos.x >> v.pos.y >> v.pos.z >> v.normal.x >> v.normal.y >> v.normal.z;
		vertices.push_back(v);
	}

	int n;
	WORD index;

	for (int i = 0; i < faces_count; i++)
	{
		plyFile >> n;
		for (int i = 0; i < n; i++)
		{
			plyFile >> index;
			indices.push_back(index);
		}
	}

	plyFile.close();
	return S_OK;
}

HRESULT Game::loadMeshFromFile(string fileName, vector<Vertex> &vertices, vector<WORD> &indices)
{
	if (fileExtension(fileName) == "ply")
		return loadMeshFromFile_ply(fileName, vertices, indices);
	else
		return E_FAIL;
}