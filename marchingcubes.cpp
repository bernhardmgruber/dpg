#include <iostream>
#include <cstring>
#include <cmath>

#include "mathlib.h"
#include "Chunk.h"
#include "marchingcubes.h"

using namespace std;

#include "tables.inc"

#define BLOCK_AT(x, y, z) (*(block + (x) * (Chunk::RESOLUTION + 1 + 2) * (Chunk::RESOLUTION + 1 + 2) + (y) * (Chunk::RESOLUTION + 1 + 2) + (z)))

template<typename T>
Vector3F interpolate(float da, float db, Vector3<T> va, Vector3<T> vb)
{
	Vector3F result;

	float part = fabs(da) / (fabs(da) + fabs(db));

	result.x = va.x + (vb.x - va.x) * part;
	result.y = va.y + (vb.y - va.y) * part;
	result.z = va.z + (vb.z - va.z) * part;

	return result;
}

Vector3F getNormal(float* block, const Vector3I& v)
{
	Vector3F grad;
	grad.x = BLOCK_AT(v.x + 1, v.y    , v.z    ) - BLOCK_AT(v.x - 1, v.y    , v.z    );
	grad.y = BLOCK_AT(v.x    , v.y + 1, v.z    ) - BLOCK_AT(v.x    , v.y - 1, v.z    );
	grad.z = BLOCK_AT(v.x    , v.y    , v.z + 1) - BLOCK_AT(v.x    , v.y    , v.z - 1);

	return normalize(grad);
}

void marchChunk(Chunk& c, float* block)
{
	for(unsigned int x = 1; x < Chunk::RESOLUTION + 1; x++)
	{
		for(unsigned int y = 1; y < Chunk::RESOLUTION + 1; y++)
		{
			for(unsigned int z = 1; z < Chunk::RESOLUTION + 1; z++)
			{
				float v0 = BLOCK_AT(x    , y    , z    );
				float v1 = BLOCK_AT(x    , y    , z + 1);
				float v2 = BLOCK_AT(x + 1, y    , z + 1);
				float v3 = BLOCK_AT(x + 1, y    , z    );
				float v4 = BLOCK_AT(x    , y + 1, z    );
				float v5 = BLOCK_AT(x    , y + 1, z + 1);
				float v6 = BLOCK_AT(x + 1, y + 1, z + 1);
				float v7 = BLOCK_AT(x + 1, y + 1, z    );

				int caseIndex = 0;

				if(v0 > 0) caseIndex |= 0x01;
				if(v1 > 0) caseIndex |= 0x02;
				if(v2 > 0) caseIndex |= 0x04;
				if(v3 > 0) caseIndex |= 0x08;
				if(v4 > 0) caseIndex |= 0x10;
				if(v5 > 0) caseIndex |= 0x20;
				if(v6 > 0) caseIndex |= 0x40;
				if(v7 > 0) caseIndex |= 0x80;

				if(caseIndex == 255)
					continue; // solid block
				if(caseIndex == 0)
					continue; // air block

				int numTriangles = case_to_numpolys[caseIndex];

				for(int t = 0; t < numTriangles; t++)
				{
					Triangle tri;

					// for each edge
					for(int e = 0; e < 3; e++)
					{
						int edgeIndex = edge_connect_list[caseIndex][t][e];

						float value1;
						float value2;
						Vector3I vec1;
						Vector3I vec2;

						switch(edgeIndex)
						{
						case 0:  value1 = v0; value2 = v1; vec1 = Vector3I(x    , y    , z    ); vec2 = Vector3I(x    , y    , z + 1); break;
						case 1:  value1 = v1; value2 = v2; vec1 = Vector3I(x    , y    , z + 1); vec2 = Vector3I(x + 1, y    , z + 1); break;
						case 2:  value1 = v2; value2 = v3; vec1 = Vector3I(x + 1, y    , z + 1); vec2 = Vector3I(x + 1, y    , z    ); break;
						case 3:  value1 = v3; value2 = v0; vec1 = Vector3I(x + 1, y    , z    ); vec2 = Vector3I(x    , y    , z    ); break;

						case 4:  value1 = v4; value2 = v5; vec1 = Vector3I(x    , y + 1, z    ); vec2 = Vector3I(x    , y + 1, z + 1); break;
						case 5:  value1 = v5; value2 = v6; vec1 = Vector3I(x    , y + 1, z + 1); vec2 = Vector3I(x + 1, y + 1, z + 1); break;
						case 6:  value1 = v6; value2 = v7; vec1 = Vector3I(x + 1, y + 1, z + 1); vec2 = Vector3I(x + 1, y + 1, z    ); break;
						case 7:  value1 = v7; value2 = v4; vec1 = Vector3I(x + 1, y + 1, z    ); vec2 = Vector3I(x    , y + 1, z    ); break;

						case 8:  value1 = v0; value2 = v4; vec1 = Vector3I(x    , y    , z    ); vec2 = Vector3I(x    , y + 1, z    ); break;
						case 9:  value1 = v1; value2 = v5; vec1 = Vector3I(x    , y    , z + 1); vec2 = Vector3I(x    , y + 1, z + 1); break;
						case 10: value1 = v2; value2 = v6; vec1 = Vector3I(x + 1, y    , z + 1); vec2 = Vector3I(x + 1, y + 1, z + 1); break;
						case 11: value1 = v3; value2 = v7; vec1 = Vector3I(x + 1, y    , z    ); vec2 = Vector3I(x + 1, y + 1, z    ); break;

						default: cerr << "Invalid edge index: " << edgeIndex << endl; break;
						}

						Vector3F vertex = interpolate(value1, value2, vec1, vec2);
						tri.vertices[e].position = c.toWorld(vertex);

						Vector3F normal1 = getNormal(block, vec1);
						Vector3F normal2 = getNormal(block, vec2);

						tri.vertices[e].normal = normalize(interpolate(value1, value2, normal1, normal2));
					}

					c.triangles.push_back(tri);
				}
			}
		}
	}
}
