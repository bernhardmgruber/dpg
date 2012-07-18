#include <iostream>

#include "Camera.h"

#include "World.h"

using namespace std;

World::World()
{

}

World::~World()
{
    chunks.clear();
}

#define CAMERA_CHUNK_RADIUS 2

void World::Update()
{
    cout << "BEGIN world update" << endl;

    // Get camera position
    Vector3F pos = Camera::GetInstance().GetPosition();

    // Determine the chunk the camera is in.
    Chunk* cameraChunk;

    static bool first = true;

    if(first)
    {
        first = false;

        cameraChunk = Chunk::fromNoise(Vector3I(0, 0, 0));
        chunks[cameraChunk->center] = cameraChunk;
    }
    else
        cameraChunk = chunks[Vector3I(0, 0, 0)];

    // Reset markers and clear renderList
    for(Chunk* c : renderList)
        c->marked = false;
    renderList.clear();

    // Check for chunks to load, unload, generate and build renderList
    RecursiveChunkCheck(cameraChunk, CAMERA_CHUNK_RADIUS);

    cout << "END world update" << endl;
}

void World::Render()
{
    for(Chunk* c : renderList)
        c->Render();
}

void World::RecursiveChunkCheck(Chunk* c, int level)
{
    if(level == 0 || c->marked)
        return;

    cout << "Checking chunk " << c->center << " level " << level << endl;

    renderList.push_back(c);
    c->marked = true;

    Vector3I offsets[] = {Vector3I(1, 0, 0), Vector3I(-1, 0, 0), Vector3I(0, 1, 0), Vector3I(0, -1, 0), Vector3I(0, 0, 1), Vector3I(0, 0, -1)};

    for(Vector3I& offset : offsets)
    {
        Vector3I neighborCenter = c->center + offset;
        Chunk* neighborChunk;

        auto it = chunks.find(neighborCenter);
        if(it == chunks.end())
        {
            // this neighbor chunk has not been loaded
            cout << "Chunk " << c->center << " has no neighbor at " << neighborCenter << endl;

            neighborChunk = Chunk::fromNoise(neighborCenter);
            chunks[neighborCenter] = neighborChunk;
        }
        else
        {
            // this neighbor chunk is loaded
            neighborChunk = it->second;
        }

        RecursiveChunkCheck(neighborChunk, level - 1);
    }
}
