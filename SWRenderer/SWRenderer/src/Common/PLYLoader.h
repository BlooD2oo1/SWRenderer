#pragma once

#include <cstdio>
#include <cstring>
#include <cstdint>

inline bool PLY_LoadFromFile(const char* szFilename, SMesh& sMesh)
{
	FILE* pFile = nullptr;
#if defined(_MSC_VER) && _MSC_VER >= 1400
	fopen_s(&pFile, szFilename, "rb");
#else
	pFile = fopen(szFilename, "rb");
#endif

	if (!pFile) return false;

	char line[512];

	if (!fgets(line, sizeof(line), pFile) || strncmp(line, "ply", 3) != 0)
	{
		fclose(pFile);
		return false;
	}

	uint32_t vertexCount = 0;
	uint32_t edgeCount = 0;

	enum EElementType { ELEM_NONE, ELEM_VERTEX, ELEM_EDGE, ELEM_FACE };
	EElementType currentElement = ELEM_NONE;

	bool hasColor = false;
	bool colorIsUchar = true;
	bool hasAlpha = false;

	while (fgets(line, sizeof(line), pFile))
	{
		if (strncmp(line, "end_header", 10) == 0)
			break;

		char keyword[64] = { 0 };
		if ( sscanf_s( line, "%63s", keyword, (unsigned)_countof( keyword ) ) != 1 ) continue;

		if (strcmp(keyword, "element") == 0)
		{
			char elemName[64] = { 0 };
			uint32_t count = 0;
			if ( sscanf_s( line, "element %63s %u", elemName, (unsigned)_countof( elemName ), &count ) == 2 )
			{
				if (strcmp(elemName, "vertex") == 0)
				{
					currentElement = ELEM_VERTEX;
					vertexCount = count;
				}
				else if (strcmp(elemName, "edge") == 0)
				{
					currentElement = ELEM_EDGE;
					edgeCount = count;
				}
				else if (strcmp(elemName, "face") == 0)
				{
					currentElement = ELEM_FACE;
					edgeCount = count;
				}
				else
				{
					currentElement = ELEM_NONE;
				}
			}
		}
		else if (strcmp(keyword, "property") == 0 && currentElement == ELEM_VERTEX)
		{
			char typeStr[64] = { 0 };
			char propName[64] = { 0 };
			if ( sscanf_s( line, "property %63s %63s", typeStr, (unsigned)_countof( typeStr ), propName, (unsigned)_countof( propName ) ) == 2 )
			{
				if (strcmp(propName, "red") == 0 || strcmp(propName, "r") == 0 || strcmp(propName, "diffuse_red") == 0)
				{
					hasColor = true;
					if (strcmp(typeStr, "float") == 0 || strcmp(typeStr, "double") == 0)
					{
						colorIsUchar = false;
					}
				}
				if (strcmp(propName, "alpha") == 0 || strcmp(propName, "a") == 0)
				{
					hasAlpha = true;
				}
			}
		}
	}

	if (vertexCount == 0 || edgeCount == 0)
	{
		fclose(pFile);
		return false;
	}

	sMesh.Clear();
	sMesh.m_pVertices = new SVertexPC[vertexCount];
	sMesh.m_iVertexCount = vertexCount;

	sMesh.m_pIndices = new uint32_t[edgeCount * 2];
	sMesh.m_iIndexCount = edgeCount * 2;

	for (uint32_t i = 0; i < vertexCount; ++i)
	{
		if (!fgets(line, sizeof(line), pFile))
		{
			sMesh.Clear();
			fclose(pFile);
			return false;
		}

		SVertexPC& v = sMesh.m_pVertices[i];
		float x = 0.0f, y = 0.0f, z = 0.0f;

		if (hasColor)
		{
			if (colorIsUchar)
			{
				unsigned int r = 255, g = 255, b = 255, a = 255;
				if (hasAlpha)
					sscanf_s( line, "%f %f %f %u %u %u %u", &x, &y, &z, &r, &g, &b, &a );
				else
					sscanf_s(line, "%f %f %f %u %u %u", &x, &y, &z, &r, &g, &b);

				v.sAttribs.vColor.x = r / 255.0f;
				v.sAttribs.vColor.y = g / 255.0f;
				v.sAttribs.vColor.z = b / 255.0f;
				v.sAttribs.vColor.w = a / 255.0f;
			}
			else
			{
				float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;
				if (hasAlpha)
					sscanf_s(line, "%f %f %f %f %f %f %f", &x, &y, &z, &r, &g, &b, &a);
				else
					sscanf_s(line, "%f %f %f %f %f %f", &x, &y, &z, &r, &g, &b);

				v.sAttribs.vColor.x = r;
				v.sAttribs.vColor.y = g;
				v.sAttribs.vColor.z = b;
				v.sAttribs.vColor.w = a;
			}
		}
		else
		{
			sscanf_s(line, "%f %f %f", &x, &y, &z);

			v.sAttribs.vColor.x = 1.0f;
			v.sAttribs.vColor.y = 1.0f;
			v.sAttribs.vColor.z = 1.0f;
			v.sAttribs.vColor.w = 1.0f;
		}

		v.vPos.x = x;
		v.vPos.y = y;
		v.vPos.z = z;
	}

	uint32_t validIndexCount = 0;
	for (uint32_t i = 0; i < edgeCount; ++i)
	{
		if (!fgets(line, sizeof(line), pFile))
			break;

		unsigned int val1 = 0, val2 = 0, val3 = 0;
		int scanned = sscanf_s(line, "%u %u %u", &val1, &val2, &val3);

		uint32_t idx0 = 0, idx1 = 0;
		bool validLine = false;

		if (scanned == 3 && val1 == 2)
		{
			idx0 = val2;
			idx1 = val3;
			validLine = true;
		}
		else if (scanned >= 2)
		{
			idx0 = val1;
			idx1 = val2;
			validLine = true;
		}

		if (validLine && idx0 < vertexCount && idx1 < vertexCount)
		{
			sMesh.m_pIndices[validIndexCount++] = idx0;
			sMesh.m_pIndices[validIndexCount++] = idx1;
		}
	}

	sMesh.m_iIndexCount = validIndexCount;

	fclose(pFile);
	return true;
}
/*
inline bool PLY_SaveFromLineList(const char* szFilename, const SVertexPC* pLineList, uint32_t iLineListCount)
{
	if (!pLineList || iLineListCount < 2) return false;

	FILE* pFile = nullptr;
#if defined(_MSC_VER) && _MSC_VER >= 1400
	fopen_s(&pFile, szFilename, "w");
#else
	pFile = fopen(szFilename, "w");
#endif

	if (!pFile) return false;

	uint32_t edgeCount = iLineListCount / 2;

	fprintf(pFile, "ply\n");
	fprintf(pFile, "format ascii 1.0\n");
	fprintf(pFile, "element vertex %u\n", iLineListCount);
	fprintf(pFile, "property float x\n");
	fprintf(pFile, "property float y\n");
	fprintf(pFile, "property float z\n");
	fprintf(pFile, "property uchar red\n");
	fprintf(pFile, "property uchar green\n");
	fprintf(pFile, "property uchar blue\n");
	fprintf(pFile, "property uchar alpha\n");
	fprintf(pFile, "element edge %u\n", edgeCount);
	fprintf(pFile, "property int vertex1\n");
	fprintf(pFile, "property int vertex2\n");
	fprintf(pFile, "end_header\n");

	for (uint32_t i = 0; i < iLineListCount; ++i)
	{
		const SVertexPC& v = pLineList[i];

		unsigned char r = (unsigned char)(std::min(std::max(v.sAttribs.vColor.x, 0.0f), 1.0f) * 255.0f);
		unsigned char g = (unsigned char)(std::min(std::max(v.sAttribs.vColor.y, 0.0f), 1.0f) * 255.0f);
		unsigned char b = (unsigned char)(std::min(std::max(v.sAttribs.vColor.z, 0.0f), 1.0f) * 255.0f);
		unsigned char a = (unsigned char)(std::min(std::max(v.sAttribs.vColor.w, 0.0f), 1.0f) * 255.0f);

		fprintf(pFile, "%f %f %f %u %u %u %u\n",
			v.vPos.x, v.vPos.y, v.vPos.z,
			r, g, b, a);
	}

	for (uint32_t i = 0; i < edgeCount; ++i)
	{
		fprintf(pFile, "%u %u\n", i * 2, i * 2 + 1);
	}

	fclose(pFile);
	return true;
}
*/