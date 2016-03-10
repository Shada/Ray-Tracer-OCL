#include "OBJImporter.h"

OBJImporter::OBJImporter()
{
}

bool OBJImporter::import(char* filename, float*& modeldata)
{
	nV = 0;
	nN = 0;
	nT = 0;
	nF = 0;
	std::fstream f(filename);
	std::string tmp;
	if (f)
	{
		while (f.eof() != true)
		{
			f >> tmp;
			if (tmp == "v") nV++;
			else if (tmp == "vt") nT++;
			else if (tmp == "vn") nN++;
			else if (tmp == "f") nF++;
			else if (tmp == "#") parseComment(f);
			
			std::getline(f, tmp);

			if (f.eof())
				break;
		}
	}
	std::cout << "nV: " << nV << std::endl;
	std::cout << "nT: " << nT << std::endl;
	std::cout << "nN: " << nN << std::endl;
	std::cout << "nF: " << nF << std::endl;

	modeldata = (float*)malloc(((3 + 3 + 2) * 3 * nF + 1) * sizeof(float));
	float* tmpdata = (float*)malloc(sizeof(float) * ((nV * 3) + (nT * 2) + (nN * 3)));

	int indexV = 0;
	int indexT = 0;
	int indexN = 0;
	int indexF = 0;
	std::fstream d(filename);
	if (d)
	{
		while (d.eof() != true)
		{
			d >> tmp;
			if (tmp == "v") { parsePos(d, tmpdata, indexV); indexV += 3; }
			else if (tmp == "vt") { parseTex(d, tmpdata, indexV + indexT); indexT += 2; }
			else if (tmp == "vn") { parseNorm(d, tmpdata, indexV + indexT + indexN); indexN += 3; }
			else if (tmp == "f") parseFace(d, tmpdata, modeldata, indexF++);
			else if (tmp == "#") parseComment(d);
			
			std::getline(d, tmp);

			if (d.eof())
				break;
		}
	}
	else
		return false;

	delete(tmpdata);
	tmpdata = nullptr;

	return true;
}

void OBJImporter::parsePos(std::fstream& f, float*& tmpdata, int indexV)
{
	f >> tmpdata[indexV] >> tmpdata[indexV + 1] >> tmpdata[indexV + 2];

	//std::cout << tmpdata[indexV] << " : " << tmpdata[indexV + 1] << " : " << tmpdata[indexV + 2] << std::endl;
}

void OBJImporter::parseTex(std::fstream& f, float*& tmpdata, int indexT)
{
	f >> tmpdata[indexT] >> tmpdata[indexT + 1];

	//std::cout << tmpdata[indexT] << " : " << tmpdata[indexT + 1] << std::endl;
}

void OBJImporter::parseNorm(std::fstream& f, float*& tmpdata, int indexN)
{
	f >> tmpdata[indexN] >> tmpdata[indexN + 1] >> tmpdata[indexN + 2];

	//std::cout << tmpdata[indexN] << " : " << tmpdata[indexN + 1] << " : " << tmpdata[indexN + 2] << std::endl;
}

void OBJImporter::parseFace(std::fstream& f, float*& tmpdata, float*& modeldata, int indexF)
{
	int v, vt, vn;
	char tmp;
	f >> v >> tmp >> vt >> tmp >> vn;
	v--; vt--; vn--;
	// V1
	modeldata[indexF * 3 * 8 + 0] = tmpdata[v * 3 + 0];
	modeldata[indexF * 3 * 8 + 1] = tmpdata[v * 3 + 1];
	modeldata[indexF * 3 * 8 + 2] = tmpdata[v * 3 + 2];
	// T1
	modeldata[indexF * 3 * 8 + 3] = tmpdata[nV * 3 + vt * 2 + 0];
	modeldata[indexF * 3 * 8 + 4] = tmpdata[nV * 3 + vt * 2 + 1];
	// N1
	modeldata[indexF * 3 * 8 + 5] = tmpdata[nV * 3 + nT * 2 + vn * 3 + 0];
	modeldata[indexF * 3 * 8 + 6] = tmpdata[nV * 3 + nT * 2 + vn * 3 + 1];
	modeldata[indexF * 3 * 8 + 7] = tmpdata[nV * 3 + nT * 2 + vn * 3 + 2];

	f >> v >> tmp >> vt >> tmp >> vn;
	v--; vt--; vn--;
	// V2
	modeldata[indexF * 3 * 8 + 8] = tmpdata[v * 3 + 0];
	modeldata[indexF * 3 * 8 + 9] = tmpdata[v * 3 + 1];
	modeldata[indexF * 3 * 8 + 10] = tmpdata[v * 3 + 2];
	// T2
	modeldata[indexF * 3 * 8 + 11] = tmpdata[nV * 3 + vt * 2 + 0];
	modeldata[indexF * 3 * 8 + 12] = tmpdata[nV * 3 + vt * 2 + 1];
	// N2
	modeldata[indexF * 3 * 8 + 13] = tmpdata[nV * 3 + nT * 2 + vn * 3 + 0];
	modeldata[indexF * 3 * 8 + 14] = tmpdata[nV * 3 + nT * 2 + vn * 3 + 1];
	modeldata[indexF * 3 * 8 + 15] = tmpdata[nV * 3 + nT * 2 + vn * 3 + 2];

	f >> v >> tmp >> vt >> tmp >> vn;
	v--; vt--; vn--;
	// V3
	modeldata[indexF * 3 * 8 + 16] = tmpdata[v * 3 + 0];
	modeldata[indexF * 3 * 8 + 17] = tmpdata[v * 3 + 1];
	modeldata[indexF * 3 * 8 + 18] = tmpdata[v * 3 + 2];
	// T3
	modeldata[indexF * 3 * 8 + 19] = tmpdata[nV * 3 + vt * 2 + 0];
	modeldata[indexF * 3 * 8 + 20] = tmpdata[nV * 3 + vt * 2 + 1];
	// N3
	modeldata[indexF * 3 * 8 + 21] = tmpdata[nV * 3 + nT * 2 + vn * 3 + 0];
	modeldata[indexF * 3 * 8 + 22] = tmpdata[nV * 3 + nT * 2 + vn * 3 + 1];
	modeldata[indexF * 3 * 8 + 23] = tmpdata[nV * 3 + nT * 2 + vn * 3 + 2];
}

void OBJImporter::parseComment(std::fstream& f)
{
	std::string tmp;
	std::getline(f, tmp);
	std::cout << "Comment: " << tmp << std::endl;
}

OBJImporter::~OBJImporter()
{
}
