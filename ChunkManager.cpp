#include <thread>
#include <future>
#include "globals.h"

#include "ChunkManager.h"

using namespace std;

ChunkManager::ChunkManager(unsigned int loaderThreads)
: serializer("chunks/")
{

}

ChunkManager::~ChunkManager()
{
    for (auto& pair : chunks)
    {
        serializer.storeChunk(pair.second);
        delete pair.second;
    }
}

Chunk* ChunkManager::get(const Vector3I& pos)
{
    // check if chunk is available
    auto it = chunks.find(pos);
    if (it != chunks.end())
        return it->second;

    // chunk was not found, ask the cache on disk
    if (serializer.hasChunk(pos))
    {
        Chunk* c = serializer.get(pos);
        if (c != nullptr)
            chunks[pos] = c;
        return c;
    }

    // not found on disk, create it
    Chunk* c = creator.get(pos);
    if (c != nullptr)
        chunks[pos] = c;
    return c;
}

const ChunkMemoryFootprint ChunkManager::getMemoryFootprint() const
{
    ChunkMemoryFootprint mem = { 0 };

    for (const auto& pair : chunks)
    {
        const ChunkMemoryFootprint& cmem = pair.second->getMemoryFootprint();
        mem.densityValues += cmem.densityValues;
        mem.densityValueSize = cmem.densityValueSize;
        mem.triangles += cmem.triangles;
        mem.triangleSize = cmem.triangleSize;
    }

    return mem;
}
