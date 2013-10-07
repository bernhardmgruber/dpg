#include <SDL.h>
#include <SDL_opengl.h>
#include <noise/noise.h>
#include <iostream>

#include "marchingcubes.h"

#include "Chunk.h"

using namespace std;

const float Chunk::SIZE = 2.0;
const unsigned int Chunk::RESOLUTION = 32;

Chunk::Chunk(Vector3I position)
	: position(position), marked(false)
{
    noise::module::Perlin perlin;
	//perlin.SetOctaveCount(5);
	perlin.SetFrequency(0.3);

    const int size = RESOLUTION + 1 + 2; // + 1 for corners and + 2 for marging
    densities = new float[size * size * size];

    for(int x = 0; x < size; x++)
        for(int y = 0; y < size; y++)
            for(int z = 0; z < size; z++)
            {
                Vector3F world = toWorld(x, y, z);
                densities[x * size * size + y * size + z] = (float)perlin.GetValue(world.x, world.y, world.z);
            }

    // create geometry using marching cubes
    MarchChunk(*this, densities);
}

Chunk::~Chunk()
{
	delete[] densities;
}

Vector3I Chunk::getPosition()
{
    return position;
}

void Chunk::render()
{
    //glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_TRIANGLES);
    for(auto t : triangles)
    {
        glNormal3fv((float*)&t.normals[0]);
        glVertex3fv((float*)&t.vertices[0]);
        glNormal3fv((float*)&t.normals[1]);
        glVertex3fv((float* )&t.vertices[1]);
        glNormal3fv((float*)&t.normals[2]);
        glVertex3fv((float*)&t.vertices[2]);
    }
    glEnd();

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
