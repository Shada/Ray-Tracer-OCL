#pragma once
#include <iostream>
#include <fstream>
#include <string>

class OBJImporter
{
public:
	OBJImporter();
	~OBJImporter();
	int nV;
	int nN;
	int nT;
	int nF;
	bool import(char* filename, float*& modelData); // add material data

	void parsePos(std::fstream& f, float*& tmpdata, int indexV);
	void parseTex(std::fstream& f, float*& tmpdata, int indexT);
	void parseNorm(std::fstream& f, float*& tmpdata, int indexN);
	void parseFace(std::fstream& f, float*& tmpdata, float*& modeldata, int indexF);
	void parseComment(std::fstream& f);
};

