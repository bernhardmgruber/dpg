#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include <noise/noise.h>
#include <iostream>

#include "marchingcubes.h"

#include "Chunk.h"

using namespace std;
using namespace noise;

const float Chunk::SIZE = 0.5;
const int Chunk::RESOLUTION = 8;

Chunk* Chunk::fromNoise(Vector3I center)
{
    Chunk* c = new Chunk();
    c->center = center;

    // gen noise cube
    /*module::RidgedMulti mountainTerrain;

    module::Billow baseFlatTerrain;
    baseFlatTerrain.SetFrequency (2.0);

    module::ScaleBias flatTerrain;
    flatTerrain.SetSourceModule (0, baseFlatTerrain);
    flatTerrain.SetScale (0.125);
    flatTerrain.SetBias (-0.75);

    module::Perlin terrainType;
    terrainType.SetFrequency (0.5);
    terrainType.SetPersistence (0.25);

    module::Select finalTerrain;
    finalTerrain.SetSourceModule (0, flatTerrain);
    finalTerrain.SetSourceModule (1, mountainTerrain);
    finalTerrain.SetControlModule (terrainType);
    finalTerrain.SetBounds (0.0, 1000.0);
    finalTerrain.SetEdgeFalloff (0.125);*/

    module::Perlin finalTerrain;

    const int size = RESOLUTION + 1 + 2; // + 1 for corners and + 2 for marging
    float* cube = new float[size * size * size];

    for(int x = 0; x < size; x++)
        for(int y = 0; y < size; y++)
            for(int z = 0; z < size; z++)
            {
                Vector3F world = c->ToWorld(x, y, z);
                //cout << world << endl;
                *(cube + x * size * size + y * size + z) = finalTerrain.GetValue(world.x, world.y, world.z);
            }

    // create geometry using marching cubes
    MarchChunk(*c, cube);

    delete cube;

    return c;
}

Vector3F Chunk::ToWorld(int x, int y, int z)
{
    Vector3F v;
    v.x = center.x - SIZE / 2.0 + SIZE / RESOLUTION * (x - 1);
    v.y = center.y - SIZE / 2.0 + SIZE / RESOLUTION * (y - 1);
    v.z = center.z - SIZE / 2.0 + SIZE / RESOLUTION * (z - 1);
    return v;
}

Vector3F Chunk::ToWorld(Vector3F in)
{
    Vector3F v;
    v.x = center.x - SIZE / 2.0 + SIZE / RESOLUTION * (in.x - 1);
    v.y = center.y - SIZE / 2.0 + SIZE / RESOLUTION * (in.y - 1);
    v.z = center.z - SIZE / 2.0 + SIZE / RESOLUTION * (in.z - 1);
    return v;
}

Vector3I Chunk::GetCenter()
{
    return center;
}

void Chunk::Render()
{
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_TRIANGLES);
    for(auto t : triangles)
    {
        glNormal3fv((float*)&t.normals[0]);
        glVertex3fv((float*)&t.vertices[0]);
        glNormal3fv((float*)&t.normals[1]);
        glVertex3fv((float*)&t.vertices[1]);
        glNormal3fv((float*)&t.normals[2]);
        glVertex3fv((float*)&t.vertices[2]);
    }
    glEnd();

    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_LINES);
    for(auto t : triangles)
    {
        Vector3F v;
        v = t.vertices[0];
        glVertex3fv((float*)&v);
        v = t.vertices[0] + t.normals[0] * 0.05;
        glVertex3fv((float*)&v);
        v = t.vertices[1];
        glVertex3fv((float*)&v);
        v = t.vertices[1] + t.normals[1] * 0.05;
        glVertex3fv((float*)&v);
        v = t.vertices[2];
        glVertex3fv((float*)&v);
        v = t.vertices[2] + t.normals[2] * 0.05;
        glVertex3fv((float*)&v);
    }
    glEnd();
}

Chunk::Chunk()
    : marked(false)
{

}
