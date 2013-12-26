#ifdef _WIN32
#include <windows.h>
#endif

#include "ChunkSerializer.h"


ChunkSerializer::ChunkSerializer(string chunkDir)
{
#ifdef _WIN32
    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile(chunkDir.c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE)
        return;
    else
    {
        do
        {
            static_assert(is_same<uint64_t, Chunk::IdType>::value, "Chunk::IdType is assumed to be uint64_t");
            Chunk::IdType id = strtoul(findData.cFileName, nullptr, 10);

            availableChunks.insert(id);
        }
        while (FindNextFile(hFind, &findData));
        FindClose(hFind);
    }

#endif
}

ChunkSerializer::~ChunkSerializer()
{

}

void ChunkSerializer::storeChunk(const Chunk* chunk)
{

}

Chunk* ChunkSerializer::loadChunk(Chunk::IdType chunkId)
{
    if (availableChunks.find(chunkId) == availableChunks.end())
        return nullptr; // this chunk is not available in the chunk directory


}
