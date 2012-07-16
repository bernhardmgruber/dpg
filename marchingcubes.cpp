#include <iostream>
#include <cstring>
#include <cmath>

#include "Chunk.h"
#include "marchingcubes.h"

using namespace std;

#include "tables.inc"

Vector3D position(int x, int y, int z)
{
    return Vector3D(x / 16.0, y / 16.0, z / 16.0);
}

Vector3D interpolate(float da, float db, Vector3D va, Vector3D vb)
{
    Vector3D result;

    float part = fabs(da) / (fabs(da) + fabs(db));

    result.x = va.x + (vb.x - va.x) * part;
    result.y = va.y + (vb.y - va.y) * part;
    result.z = va.z + (vb.z - va.z) * part;

    return result;
}

#define BLOCK_AT(x, y, z) (*(block + x * size * size + y * size + z))

vector<Triangle> MarchBlock(float* block, int size)
{
    vector<Triangle> triangles;

    for(int x = 0; x < size; x++)
    {
        for(int y = 0; y < size; y++)
        {
            for(int z = 0; z < size; z++)
            {
                /*
                float v0 = block[x][y][z];
                float v1 = block[x][y][z+1];
                float v2 = block[x+1][y][z+1];
                float v3 = block[x+1][y][z];
                float v4 = block[x][y+1][z];
                float v5 = block[x][y+1][z+1];
                float v6 = block[x+1][y+1][z+1];
                float v7 = block[x+1][y+1][z];
                */

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
                    vector<Vector3D> vertices;

                    // for each edge
                    for(int e = 0; e < 3; e++)
                    {
                        int edgeIndex = edge_connect_list[caseIndex][t][e];

                        switch(edgeIndex)
                        {
                        case 0:
                            vertices.push_back(interpolate(v0, v1, position(x, y, z), position(x, y, z+1)));
                            break;
                        case 1:
                            vertices.push_back(interpolate(v1, v2, position(x, y, z+1), position(x+1, y, z+1)));
                            break;
                        case 2:
                            vertices.push_back(interpolate(v2, v3, position(x+1, y, z+1), position(x+1, y, z)));
                            break;
                        case 3:
                            vertices.push_back(interpolate(v3, v0, position(x+1, y, z), position(x, y, z)));
                            break;

                        case 4:
                            vertices.push_back(interpolate(v4, v5, position(x, y+1, z), position(x, y+1, z+1)));
                            break;
                        case 5:
                            vertices.push_back(interpolate(v5, v6, position(x, y+1, z+1), position(x+1, y+1, z+1)));
                            break;
                        case 6:
                            vertices.push_back(interpolate(v6, v7, position(x+1, y+1, z+1), position(x+1, y+1, z)));
                            break;
                        case 7:
                            vertices.push_back(interpolate(v7, v4, position(x+1, y+1, z), position(x, y+1, z)));
                            break;

                        case 8:
                            vertices.push_back(interpolate(v0, v4, position(x, y, z), position(x, y+1, z)));
                            break;
                        case 9:
                            vertices.push_back(interpolate(v1, v5, position(x, y, z+1), position(x, y+1, z+1)));
                            break;
                        case 10:
                            vertices.push_back(interpolate(v2, v6, position(x+1, y, z+1), position(x+1, y+1, z+1)));
                            break;
                        case 11:
                            vertices.push_back(interpolate(v3, v7, position(x+1, y, z), position(x+1, y+1, z)));
                            break;
                        default:
                            cerr << "Invalid edge index: " << edgeIndex << endl;
                            break;
                        }
                    }

                    Triangle tri;
                    memcpy(tri.vertices, vertices.data(), sizeof(Vector3D) * 3);
                    triangles.push_back(tri);
                }
            }
        }
    }

    return triangles;
}
