#include <noise/noise.h>
#include <iostream>
#include <unordered_map>

#include "Timer.h"
#include "tables.inc"

#include "Chunk.h"

using namespace std;

const float Chunk::SIZE = 1.0;
const unsigned int Chunk::RESOLUTION = 16;

const unsigned int CHUNK_TRIANGLE_MAP_INITIAL_SIZE = 3000;

Chunk::Chunk(Vector3I position)
	: position(position), buffersInitialized(false)
{
	noise::module::Perlin perlin;
	perlin.SetOctaveCount(5);
	perlin.SetFrequency(0.3);

	const unsigned int size = RESOLUTION + 1 + 2; // + 1 for corners and + 2 for marging
	densities = new DensityType[size * size * size];

	//Timer timer;

	for(unsigned int x = 0; x < size; x++)
	{
		for(unsigned int y = 0; y < size; y++)
		{
			for(unsigned int z = 0; z < size; z++)
			{
				Vector3F world = toWorld(x, y, z);
				densities[x * size * size + y * size + z] = (DensityType)perlin.GetValue(world.x, world.y, world.z);
			}
		}
	}

	//timer.tick();
	//cout << "Noise took " << timer.interval << " seconds" << endl;

	// create geometry using marching cubes
	marchChunk(densities);

	//timer.tick();
	//cout << "Marching took " << timer.interval << " seconds" << endl;
}

Chunk::~Chunk()
{
	delete[] densities;

	if(buffersInitialized)
	{
		glDeleteBuffers(1, &vertexBuffer);
		glDeleteBuffers(1, &indexBuffer);
	}
}

const Vector3I Chunk::getPosition() const
{
	return position;
}

void Chunk::render() const
{
	//glColor3f(1.0, 1.0, 1.0);
	//glBegin(GL_TRIANGLES);
	//for(auto t : triangles)
	//{
	//    glNormal3fv((float*)&t.normals[0]);
	//    glVertex3fv((float*)&t.vertices[0]);
	//    glNormal3fv((float*)&t.normals[1]);
	//    glVertex3fv((float* )&t.vertices[1]);
	//    glNormal3fv((float*)&t.normals[2]);
	//    glVertex3fv((float*)&t.vertices[2]);
	//}
	//glEnd();

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(Vector3F), (const GLvoid*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(Vector3F), (const GLvoid*)sizeof(Vector3F));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glDrawElements(GL_TRIANGLES, (GLsizei)triangles.size() * 3, GL_UNSIGNED_INT, (const GLvoid*)0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	//glColor3f(1.0, 0.0, 0.0);
	//glBegin(GL_LINES);
	//for(auto t : triangles)
	//{
	//    Vector3F v;
	//    v = t.vertices[0];
	//    glVertex3fv((float*)&v);
	//    v = t.vertices[0] + t.normals[0] * 0.05f;
	//    glVertex3fv((float*)&v);
	//    v = t.vertices[1];
	//    glVertex3fv((float*)&v);
	//    v = t.vertices[1] + t.normals[1] * 0.05f;
	//    glVertex3fv((float*)&v);
	//    v = t.vertices[2];
	//    glVertex3fv((float*)&v);
	//    v = t.vertices[2] + t.normals[2] * 0.05f;
	//    glVertex3fv((float*)&v);
	//}
	//glEnd();
}

const ChunkMemoryFootprint Chunk::getMemoryFootprint() const
{
	ChunkMemoryFootprint mem;

	const unsigned int size = RESOLUTION + 1 + 2; // + 1 for corners and + 2 for marging

	mem.densityValues = size * size * size;
	mem.densityValueSize = sizeof(DensityType);
	mem.triangles = triangles.size();
	mem.triangleSize = sizeof(Triangle);

	return mem;
}

void Chunk::createBuffers()
{
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), vertices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangles.size() * sizeof(triangles[0]), triangles.data(), GL_STATIC_DRAW);

	buffersInitialized = true;
}



inline float Chunk::blockAt(DensityType* block, unsigned int x, unsigned int y, unsigned int z) const
{
    return (*(block + (x) * (Chunk::RESOLUTION + 1 + 2) * (Chunk::RESOLUTION + 1 + 2) + (y) * (Chunk::RESOLUTION + 1 + 2) + (z)));
}

Vector3F Chunk::getNormal(DensityType* block, const Vector3I& v) const
{
    Vector3F grad;
    grad.x = blockAt(block, v.x + 1, v.y    , v.z    ) - blockAt(block, v.x - 1, v.y    , v.z    );
    grad.y = blockAt(block, v.x    , v.y + 1, v.z    ) - blockAt(block, v.x    , v.y - 1, v.z    );
    grad.z = blockAt(block, v.x    , v.y    , v.z + 1) - blockAt(block, v.x    , v.y    , v.z - 1);

    return normalize(grad);
}

array<Chunk::DensityType, 8> Chunk::getDensityBlockAt(DensityType* block, unsigned int x, unsigned int y, unsigned int z) const
{
    array<DensityType, 8> values;
    values[0] = blockAt(block, x    , y    , z    );
    values[1] = blockAt(block, x    , y    , z + 1);
    values[2] = blockAt(block, x + 1, y    , z + 1);
    values[3] = blockAt(block, x + 1, y    , z    );
    values[4] = blockAt(block, x    , y + 1, z    );
    values[5] = blockAt(block, x    , y + 1, z + 1);
    values[6] = blockAt(block, x + 1, y + 1, z + 1);
    values[7] = blockAt(block, x + 1, y + 1, z    );
    return values;
}

inline unsigned int Chunk::getCaseIndexFromDensityBlock(array<DensityType, 8> values) const
{
    int caseIndex = 0;

    if(values[0] > 0) caseIndex |= 0x01;
    if(values[1] > 0) caseIndex |= 0x02;
    if(values[2] > 0) caseIndex |= 0x04;
    if(values[3] > 0) caseIndex |= 0x08;
    if(values[4] > 0) caseIndex |= 0x10;
    if(values[5] > 0) caseIndex |= 0x20;
    if(values[6] > 0) caseIndex |= 0x40;
    if(values[7] > 0) caseIndex |= 0x80;

    return caseIndex;
}

struct Vector3UIHash
{
    size_t operator() (const Vector3UI& v) const { return v.x ^ (v.y << 11) ^ (v.z << 22); }
};

void Chunk::marchChunk(DensityType* block)
{
    unordered_map<Vector3F, unsigned int, Vector3UIHash> vertexMap(CHUNK_TRIANGLE_MAP_INITIAL_SIZE);

    for(unsigned int x = 1; x < Chunk::RESOLUTION + 1; x++)
    {
        for(unsigned int y = 1; y < Chunk::RESOLUTION + 1; y++)
        {
            for(unsigned int z = 1; z < Chunk::RESOLUTION + 1; z++)
            {
                array<DensityType, 8> values = getDensityBlockAt(block, x, y, z);

                unsigned int caseIndex = getCaseIndexFromDensityBlock(values);

                if(caseIndex == 255)
                    continue; // solid block
                if(caseIndex == 0)
                    continue; // air block

                int numTriangles = case_to_numpolys[caseIndex];

                // for each triangle of the cube
                for(int t = 0; t < numTriangles; t++)
                {
                    Vector3I tri;

                    // for each edge of the cube a triangle vertex is on
                    for(int e = 0; e < 3; e++)
                    {
                        int edgeIndex = edge_connect_list[caseIndex][t][e];

                        DensityType value1;
                        DensityType value2;
                        Vector3I vec1;
                        Vector3I vec2;

                        switch(edgeIndex)
                        {
                        case 0:  value1 = values[0]; value2 = values[1]; vec1 = Vector3I(x    , y    , z    ); vec2 = Vector3I(x    , y    , z + 1); break;
                        case 1:  value1 = values[1]; value2 = values[2]; vec1 = Vector3I(x    , y    , z + 1); vec2 = Vector3I(x + 1, y    , z + 1); break;
                        case 2:  value1 = values[2]; value2 = values[3]; vec1 = Vector3I(x + 1, y    , z + 1); vec2 = Vector3I(x + 1, y    , z    ); break;
                        case 3:  value1 = values[3]; value2 = values[0]; vec1 = Vector3I(x + 1, y    , z    ); vec2 = Vector3I(x    , y    , z    ); break;

                        case 4:  value1 = values[4]; value2 = values[5]; vec1 = Vector3I(x    , y + 1, z    ); vec2 = Vector3I(x    , y + 1, z + 1); break;
                        case 5:  value1 = values[5]; value2 = values[6]; vec1 = Vector3I(x    , y + 1, z + 1); vec2 = Vector3I(x + 1, y + 1, z + 1); break;
                        case 6:  value1 = values[6]; value2 = values[7]; vec1 = Vector3I(x + 1, y + 1, z + 1); vec2 = Vector3I(x + 1, y + 1, z    ); break;
                        case 7:  value1 = values[7]; value2 = values[4]; vec1 = Vector3I(x + 1, y + 1, z    ); vec2 = Vector3I(x    , y + 1, z    ); break;

                        case 8:  value1 = values[0]; value2 = values[4]; vec1 = Vector3I(x    , y    , z    ); vec2 = Vector3I(x    , y + 1, z    ); break;
                        case 9:  value1 = values[1]; value2 = values[5]; vec1 = Vector3I(x    , y    , z + 1); vec2 = Vector3I(x    , y + 1, z + 1); break;
                        case 10: value1 = values[2]; value2 = values[6]; vec1 = Vector3I(x + 1, y    , z + 1); vec2 = Vector3I(x + 1, y + 1, z + 1); break;
                        case 11: value1 = values[3]; value2 = values[7]; vec1 = Vector3I(x + 1, y    , z    ); vec2 = Vector3I(x + 1, y + 1, z    ); break;

                        default: cerr << "Invalid edge index: " << edgeIndex << endl; break;
                        }

                        Vector3F vertex = interpolate(value1, value2, vec1, vec2);

                        // lookup this vertex
                        auto it = vertexMap.find(vertex);
                        if(it != vertexMap.end()) // we found it
                            tri[e] = it->second;
                        else
                        {
                            // calculate a new one
                            Vertex v;
                            v.position = toWorld(vertex);

                            Vector3F normal1 = getNormal(block, vec1);
                            Vector3F normal2 = getNormal(block, vec2);
                            v.normal = normalize(interpolate(value1, value2, normal1, normal2));

                            tri[e] = (unsigned int)vertices.size();
                            vertices.push_back(v);

                            vertexMap[v.position] = tri[e];
                        }
                    }

                    triangles.push_back(tri);
                }
            }
        }
    }

    cout << "Vertex map size " << vertexMap.size() << endl;
}
