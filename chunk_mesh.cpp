extern "C" {

#include "enklume/chunk.h"

}

#include "chunk_mesh.h"

#include <assert.h>
#include <vector>

#define MINUS_X_FACE(V) \
V(-1, -1, -1,   0, 0) \
V(-1,  1, -1,   0, 1) \
V(-1,  1,  1,   1, 1) \
V(-1, -1, -1,   0, 0) \
V(-1,  1,  1,   1, 1) \
V(-1, -1,  1,   1, 0)

#define PLUS_X_FACE(V) \
V(1,   1, -1,   1, 1) \
V(1,  -1, -1,   1, 0) \
V(1,   1,  1,   0, 1) \
V(1,   1,  1,   0, 1) \
V(1,  -1, -1,   1, 0) \
V(1,  -1,  1,   0, 0)

#define MINUS_Z_FACE(V) \
V(1,   1, -1,   0, 1) \
V(-1, -1, -1,   1, 0) \
V(1,  -1, -1,   0, 0) \
V(-1,  1, -1,   1, 1) \
V(-1, -1, -1,   1, 0) \
V(1,   1, -1,   0, 1)

#define PLUS_Z_FACE(V) \
V(1,  -1,  1,   1, 0) \
V(-1, -1,  1,   0, 0) \
V(1,   1,  1,   1, 1) \
V(1,   1,  1,   1, 1) \
V(-1, -1,  1,   0, 0) \
V(-1,  1,  1,   0, 1)

#define MINUS_Y_FACE(V) \
V(-1, -1, -1,   0, 0) \
V(1,  -1,  1,   1, 1) \
V(1,  -1, -1,   1, 0) \
V(1,  -1,  1,   1, 1) \
V(-1, -1, -1,   0, 0) \
V(-1, -1,  1,   0, 1)

#define PLUS_Y_FACE(V) \
V(1,   1,  1,   1, 0) \
V(-1,  1, -1,   0, 1) \
V(1,   1, -1,   1, 1) \
V(-1,  1,  1,   0, 0) \
V(-1,  1, -1,   0, 1) \
V(1,   1,  1,   1, 0)

#define CUBE(V) \
MINUS_X_FACE(V) \
PLUS_X_FACE(V)  \
MINUS_Z_FACE(V) \
PLUS_Z_FACE(V)  \
MINUS_Y_FACE(V) \
PLUS_Y_FACE(V)\

static int cubeData[] = {
#define V(x, y, z, t, s) x, y, z, t, s,
CUBE(V)
#undef V
};

#define V(cx, cy, cz, t, s) tmp[0] = (int) ((cx + 1) / 2) + (float) x; tmp[1] = (int) ((cy + 1) / 2) + (float) y; tmp[2] = (int) ((cz + 1) / 2) + (float) z; tmp[3] = t; tmp[4] = s; g.push_back(tmp[0]); g.push_back(tmp[1]); g.push_back(tmp[2]); g.push_back(tmp[3]); g.push_back(tmp[4]);

static void paste_cube(std::vector<float>& g, unsigned x, unsigned y, unsigned z) {
    float tmp[5];
    CUBE(V)
}

static void paste_minus_x_face(std::vector<float>& g, unsigned x, unsigned y, unsigned z) {
    float tmp[5];
    MINUS_X_FACE(V)
}

static void paste_plus_x_face(std::vector<float>& g, unsigned x, unsigned y, unsigned z) {
    float tmp[5];
    PLUS_X_FACE(V)
}

static void paste_minus_y_face(std::vector<float>& g, unsigned x, unsigned y, unsigned z) {
    float tmp[5];
    MINUS_Y_FACE(V)
}

static void paste_plus_y_face(std::vector<float>& g, unsigned x, unsigned y, unsigned z) {
    float tmp[5];
    PLUS_Y_FACE(V)
}

static void paste_minus_z_face(std::vector<float>& g, unsigned x, unsigned y, unsigned z) {
    float tmp[5];
    MINUS_Z_FACE(V)
}

static void paste_plus_z_face(std::vector<float>& g, unsigned x, unsigned y, unsigned z) {
    float tmp[5];
    PLUS_Z_FACE(V)
}

#undef V

static BlockData access_safe(const Chunk* chunk, ChunkNeighbors& neighbours, int x, int y, int z) {
    unsigned int i, k;
    if (x < 0) {
        i = 0;
    } else if (x < CUNK_CHUNK_SIZE) {
        i = 1;
    } else {
        i = 2;
    }

    if (y < 0 || y > CUNK_CHUNK_MAX_HEIGHT)
        return air_data;

    if (z < 0) {
        k = 0;
    } else if (z < CUNK_CHUNK_SIZE) {
        k = 1;
    } else {
        k = 2;
    }
    //assert(!neighbours || neighbours[1][1][1] == chunk);
    if (i == 1 && k == 1) {
        return chunk_get_block_data(chunk, x, y, z);
    } else {
        if (neighbours.neighbours[i][k])
            return chunk_get_block_data(neighbours.neighbours[i][k], x & 15, y, z & 15);
    }

    return air_data;
}

void chunk_mesh(const Chunk* chunk, ChunkNeighbors& neighbours, std::vector<float>& g, size_t* num_verts) {
    *num_verts = 0;
    for (int section = 0; section < CUNK_CHUNK_SECTIONS_COUNT; section++) {
        for (int x = 0; x < CUNK_CHUNK_SIZE; x++)
            for (int y = 0; y < CUNK_CHUNK_SIZE; y++)
                for (int z = 0; z < CUNK_CHUNK_SIZE; z++) {
                    int world_y = y + section * CUNK_CHUNK_SIZE;
                    if (access_safe(chunk, neighbours, x, world_y, z) != air_data) {
                        if (access_safe(chunk, neighbours, x, world_y + 1, z) == air_data) {
                            paste_plus_y_face(g, x, world_y, z);
                            *num_verts += 6;
                        }
                        if (access_safe(chunk, neighbours, x, world_y - 1, z) == air_data) {
                            paste_minus_y_face(g, x, world_y, z);
                            *num_verts += 6;
                        }

                        if (access_safe(chunk, neighbours, x + 1, world_y, z) == air_data) {
                            paste_plus_x_face(g, x, world_y, z);
                            *num_verts += 6;
                        }
                        if (access_safe(chunk, neighbours, x - 1, world_y, z) == air_data) {
                            paste_minus_x_face(g, x, world_y, z);
                            *num_verts += 6;
                        }

                        if (access_safe(chunk, neighbours, x, world_y, z + 1) == air_data) {
                            paste_plus_z_face(g, x, world_y, z);
                            *num_verts += 6;
                        }
                        if (access_safe(chunk, neighbours, x, world_y, z - 1) == air_data) {
                            paste_minus_z_face(g, x, world_y, z);
                            *num_verts += 6;
                        }
                    }
                }
    }
}

void ChunkMesh::update(imr::Device& d, ChunkNeighbors& n, const Chunk* chunk) {
    buf.reset();

    x = chunk->x;
    y = chunk->y;
    z = chunk->z;

    if (chunk) {
        std::vector<float> g;
        chunk_mesh(chunk, n, g, &num_verts);

        fprintf(stderr, "%zu vertices, totalling %zu KiB of data\n", num_verts, num_verts * sizeof(float) * 5 / 1024);
        fflush(stderr);

        size_t buffer_size = g.size() * sizeof(float);
        void* buffer = g.data();

        buf = std::make_unique<imr::Buffer>(d, buffer_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        buf->uploadDataSync(0, buffer_size, buffer);
    }
}