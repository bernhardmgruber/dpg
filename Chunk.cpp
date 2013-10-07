#include <noise/noise.h>
#include <iostream>

#include "Timer.h"
#include "marchingcubes.h"

#include "Chunk.h"

using namespace std;

const float Chunk::SIZE = 1.0;
const unsigned int Chunk::RESOLUTION = 16;

Chunk::Chunk(Vector3I position)
	: position(position)
{
	noise::module::Perlin perlin;
	perlin.SetOctaveCount(5);
	perlin.SetFrequency(0.3);

	const unsigned int size = RESOLUTION + 1 + 2; // + 1 for corners and + 2 for marging
	densities = new DensityType[size * size * size];

	//Timer timer;

	for(int x = 0; x < size; x++)
	{
		for(int y = 0; y < size; y++)
		{
			for(int z = 0; z < size; z++)
			{
				Vector3F world = toWorld(x, y, z);
				densities[x * size * size + y * size + z] = (DensityType)perlin.GetValue(world.x, world.y, world.z);
			}
		}
	}

	//timer.tick();
	//cout << "Noise took " << timer.interval << " seconds" << endl;

	// create geometry using marching cubes
	marchChunk(*this, densities);

	//timer.tick();
	//cout << "Marching took " << timer.interval << " seconds" << endl;

	createBuffers();

	//timer.tick();
	//cout << "Buffers took " << timer.interval << " seconds" << endl;
}

Chunk::~Chunk()
{
	delete[] densities;
	glDeleteBuffers(1, &vertexBuffer);
	glDeleteBuffers(1, &indexBuffer);
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
}
