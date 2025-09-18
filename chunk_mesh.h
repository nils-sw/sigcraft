#ifndef SIGCRAFT_CHUNK_MESH_H
#define SIGCRAFT_CHUNK_MESH_H

#include "imr/imr.h"
#include "nasl/nasl.h"

#include <cstddef>

struct ChunkNeighbors {
    const ChunkData* neighbours[3][3];
};

struct ChunkMesh {
    std::unique_ptr<imr::Buffer> buf;
    size_t num_verts;
    size_t meshlet_count;

    ChunkMesh(imr::Device&, ChunkNeighbors& n);

    struct Meshlet {
        std::array<std::array<uint32_t, 3>, 32> vertex_positions = {};
        std::array<std::array<uint32_t, 3>, 32> triangle_indices = {};
        std::array<nasl::vec3, 32> triangle_colors = {};
        uint32_t num_verts = 0;
        uint32_t num_tris = 0;
    };

    struct Vertex {
        int32_t vx, vy, vz;
        uint32_t tt;
        uint32_t ss;
        uint32_t nnx, nny, nnz;
        // uint32_t pad;
        uint32_t br, bg, bb;
    };

    static_assert(sizeof(Vertex) == sizeof(uint32_t) * 11);
};

#endif
