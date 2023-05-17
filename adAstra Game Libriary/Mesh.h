#include <Windows.h>
#include <string>
#include <vector>
#include <xnamath.h>

#pragma once

using namespace std;

namespace Game
{
	struct Vertex
	{
		XMFLOAT4 pos;
		XMFLOAT4 normal;
	};

	string fileExtension(string& FileName);

	HRESULT loadMeshFromFile(string fileName, vector<Vertex> &vertices, vector<WORD> &indices);
	HRESULT loadMeshFromFile_ply(string fileName, vector<Vertex> &vertices, vector<WORD> &indices);

	void createSphere(Vertex* &vertices, WORD* &indices, int slices, int stacks);
	void loadStars(Vertex* &vertices, int count, float r);
}
