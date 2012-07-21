#include <iostream>
#include <cstring>
#include <cmath>

#include "mathlib.h"
#include "Chunk.h"
#include "marchingcubes.h"

using namespace std;

#include "tables.inc"

Vector3F interpolate(float da, float db, Vector3F va, Vector3F vb)
{
    Vector3F result;

    float part = fabs(da) / (fabs(da) + fabs(db));

    result.x = va.x + (vb.x - va.x) * part;
    result.y = va.y + (vb.y - va.y) * part;
    result.z = va.z + (vb.z - va.z) * part;

    return result;
}

#define BLOCK_AT(x, y, z) (*(block + (x) * (Chunk::RESOLUTION + 1 + 2) * (Chunk::RESOLUTION + 1 + 2) + (y) * (Chunk::RESOLUTION + 1 + 2) + (z)))

float samplePoint(float* block, Vector3F v)
{
    // trilinear interpolation, http://en.wikipedia.org/wiki/Trilinear_interpolation
    int x0 = (int) v.x;
    int y0 = (int) v.y;
    int z0 = (int) v.z;
    int x1 = x0 + 1;
    int y1 = y0 + 1;
    int z1 = z0 + 1;

    if(x1 > 34) x1 = 34;
    if(y1 > 34) y1 = 34;
    if(z1 > 34) z1 = 34;

    float xd = (v.x - x0) / (x1 - x0);
    float yd = (v.y - y0) / (y1 - y0);
    float zd = (v.z - z0) / (z1 - z0);

    float c00 = BLOCK_AT(x0, y0, z0) * (1 - xd) + BLOCK_AT(x1, y0, z0) * xd;
    float c10 = BLOCK_AT(x0, y1, z0) * (1 - xd) + BLOCK_AT(x1, y1, z0) * xd;
    float c01 = BLOCK_AT(x0, y0, z1) * (1 - xd) + BLOCK_AT(x1, y0, z1) * xd;
    float c11 = BLOCK_AT(x0, y1, z1) * (1 - xd) + BLOCK_AT(x1, y1, z1) * xd;

    float c0 = c00 * (1 - yd) + c10 * yd;
    float c1 = c01 * (1 - yd) + c11 * yd;

    float c = c0 * (1 - zd) + c1 * zd;

    return c;
}

Vector3F sampleNormal(float* block, Vector3F& v)
{
    //cout << "Sample normal at " << v << endl;
    Vector3F grad;
    grad.x = samplePoint(block, Vector3F(v.x + 1, v.y, v.z)) + samplePoint(block, Vector3F(v.x - 1, v.y, v.z));
    grad.y = samplePoint(block, Vector3F(v.x, v.y + 1, v.z)) + samplePoint(block, Vector3F(v.x, v.y - 1, v.z));
    grad.z = samplePoint(block, Vector3F(v.x, v.y, v.z + 1)) + samplePoint(block, Vector3F(v.x, v.y, v.z - 1));
    return -1 * Normalize(grad);
}

void MarchChunk(Chunk& c, float* block)
{
    for(int x = 1; x < Chunk::RESOLUTION + 1; x++)
    {
        for(int y = 1; y < Chunk::RESOLUTION + 1; y++)
        {
            for(int z = 1; z < Chunk::RESOLUTION + 1; z++)
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

                        switch(edgeIndex)
                        {
                            case 0:  tri.vertices[e] = interpolate(v0, v1, Vector3F(x    , y    , z    ), Vector3F(x    , y    , z + 1)); break;
                            case 1:  tri.vertices[e] = interpolate(v1, v2, Vector3F(x    , y    , z + 1), Vector3F(x + 1, y    , z + 1)); break;
                            case 2:  tri.vertices[e] = interpolate(v2, v3, Vector3F(x + 1, y    , z + 1), Vector3F(x + 1, y    , z    )); break;
                            case 3:  tri.vertices[e] = interpolate(v3, v0, Vector3F(x + 1, y    , z    ), Vector3F(x    , y    , z    )); break;

                            case 4:  tri.vertices[e] = interpolate(v4, v5, Vector3F(x    , y + 1, z    ), Vector3F(x    , y + 1, z + 1)); break;
                            case 5:  tri.vertices[e] = interpolate(v5, v6, Vector3F(x    , y + 1, z + 1), Vector3F(x + 1, y + 1, z + 1)); break;
                            case 6:  tri.vertices[e] = interpolate(v6, v7, Vector3F(x + 1, y + 1, z + 1), Vector3F(x + 1, y + 1, z    )); break;
                            case 7:  tri.vertices[e] = interpolate(v7, v4, Vector3F(x + 1, y + 1, z    ), Vector3F(x    , y + 1, z    )); break;

                            case 8:  tri.vertices[e] = interpolate(v0, v4, Vector3F(x    , y    , z    ), Vector3F(x    , y + 1, z    )); break;
                            case 9:  tri.vertices[e] = interpolate(v1, v5, Vector3F(x    , y    , z + 1), Vector3F(x    , y + 1, z + 1)); break;
                            case 10: tri.vertices[e] = interpolate(v2, v6, Vector3F(x + 1, y    , z + 1), Vector3F(x + 1, y + 1, z + 1)); break;
                            case 11: tri.vertices[e] = interpolate(v3, v7, Vector3F(x + 1, y    , z    ), Vector3F(x + 1, y + 1, z    )); break;

                            default: cerr << "Invalid edge index: " << edgeIndex << endl; break;
                        }

                        tri.normals[e] = sampleNormal(block, tri.vertices[e]);
                        tri.vertices[e] = c.ToWorld(tri.vertices[e]);
                    }

                    c.triangles.push_back(tri);
                }
            }
        }
    }
}
