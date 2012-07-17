#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include <noise/noise.h>
#include <iostream>

#include "marchingcubes.h"

#include "Chunk.h"

using namespace std;

const float Chunk::SIZE = 2;
const int Chunk::RESOLUTION = 32;

Chunk* Chunk::fromNoise(Vector3D center)
{
    Chunk* c = new Chunk();
    c->center = center;

    // gen noise cube
    noise::module::Perlin perlin;

    const int size = RESOLUTION + 1 + 2; // + 1 for corners and + 2 for marging
    float* cube = new float[size * size * size];

    for(int x = 0; x < size; x++)
        for(int y = 0; y < size; y++)
            for(int z = 0; z < size; z++)
            {
                Vector3D world = c->ToWorld(x, y, z);
                //cout << world << endl;
                *(cube + x * size * size + y * size + z) = perlin.GetValue(world.x, world.y, world.z);
            }

    // create geometry using marching cubes
    MarchChunk(*c, cube);

    delete cube;

    return c;
}

Vector3D Chunk::ToWorld(int x, int y, int z)
{
    Vector3D v;
    v.x = center.x - SIZE / 2.0 + SIZE / RESOLUTION * (x - 1);
    v.y = center.y - SIZE / 2.0 + SIZE / RESOLUTION * (y - 1);
    v.z = center.z - SIZE / 2.0 + SIZE / RESOLUTION * (z - 1);
    return v;
}

Vector3D Chunk::GetCenter()
{
    return center;
}

void Chunk::Render()
{
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_TRIANGLES);
    for(auto t : triangles)
    {
        glVertex3fv((float*)&t.vertices[0]);
        glVertex3fv((float*)&t.vertices[1]);
        glVertex3fv((float*)&t.vertices[2]);

    }
    glEnd();
}

Chunk::Chunk()
{
}

