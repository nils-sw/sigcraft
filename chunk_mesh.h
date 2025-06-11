#ifndef SIGCRAFT_CHUNK_MESH_H
#define SIGCRAFT_CHUNK_MESH_H

#include "imr/imr.h"

#include <cstddef>

struct ChunkNeighbors {
    const ChunkData* neighbours[3][3];
};

struct ChunkMesh {
    std::unique_ptr<imr::Buffer> buf;
    size_t num_verts;

    ChunkMesh(imr::Device&, ChunkNeighbors& n);
};

#endif
