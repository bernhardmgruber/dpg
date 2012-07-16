#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

#include <noise/noise.h>

#include "marchingcubes.h"

#include "Chunk.h"

const float Chunk::SIZE = 10;
const int Chunk::RESOLUTION = 32;

Chunk* Chunk::fromNoise(Vector3D center)
{
    // gen noise cube
    noise::module::Perlin perlin;

    const int size = RESOLUTION + 1;
    float cube[size][size][size];

    for(int x = 0; x < size; x++)
        for(int y = 0; y < size; y++)
            for(int z = 0; z < size; z++)
            {
                float wx = center.x - SIZE / 2.0 + SIZE / RESOLUTION * x;
                float wy = center.y - SIZE / 2.0 + SIZE / RESOLUTION * y;
                float wz = center.z - SIZE / 2.0 + SIZE / RESOLUTION * z;
                cube[x][y][z] = perlin.GetValue(wx, wy, wz);
            }

    // create geometry using marching cubes
    Chunk* c = new Chunk();
    c->center = center;
    c->triangles = MarchBlock((float*)cube, RESOLUTION);

    return c;
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

