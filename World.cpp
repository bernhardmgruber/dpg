#include "World.h"

World::World()
{

}

World::~World()
{
    chunks.clear();
}

void World::Update()
{
    // check for chunks to load, unload, generate ...

}

void World::Render()
{
    for(Chunk* c : chunks)
        c->Render();
}
