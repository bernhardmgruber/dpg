#ifndef WORLD_H
#define WORLD_H

#include <vector>

#include "Chunk.h"

class World
{
    public:
        World();
        virtual ~World();

        void Update();
        void Render();
    private:
        std::vector<Chunk*> chunks;
};

#endif // WORLD_H
