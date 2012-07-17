#include "Camera.h"

#include "World.h"

World::World()
{

}

World::~World()
{
    chunks.clear();
}

#define CHUNK_RADIUS 2

void World::Update()
{
    Vector3D pos = Camera::GetInstance().GetPosition();

    // check for chunks to load, unload, generate ...
    static bool first = true;

    if(first)
    {
        first = false;

        chunks.push_back(Chunk::fromNoise(Vector3D(0, 0, 0)));
    }
}

void World::Render()
{
    for(Chunk* c : chunks)
        c->Render();
}
