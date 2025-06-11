#ifndef SIGCRAFT_CHUNK_MESH_H
#define SIGCRAFT_CHUNK_MESH_H

#include "imr/imr.h"

#include <cstddef>

struct ChunkNeighbors {
    const Chunk* neighbours[3][3];
};

typedef struct {
    std::unique_ptr<imr::Buffer> buf;
    size_t num_verts;
    int x, y, z;

    void update(imr::Device&, ChunkNeighbors& n, const Chunk*);
} ChunkMesh;

#endif
