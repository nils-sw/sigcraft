extern "C"
{
#include "enklume/block_data.h"
}

#include "chunk_mesh.h"
#include "nasl/nasl.h"

#include <vector>
#include <functional> // For std::function
#include <cstring>    // For memcpy

//================================================================================================//
// SECTION 1: VERTEX AND QUAD GENERATION FUNCTION
//================================================================================================//

/**
 * @brief Generates a vertex and appends its data to the provided buffer.
 *
 * @param v The vertex structure to populate.
 * @param g The buffer to which the vertex data will be appended.
 * @param color The color of the vertex as a vec3 (values between 0 and 1).
 * @param vx, vy, vz The position of the vertex in chunk-local coordinates.
 * @param t, s The texture coordinates (0-255).
 * @param nx, ny, nz The normal vector components (-1, 0, 1).
 */
static inline void generate_vertex(
    ChunkMesh::Vertex &v, std::vector<uint8_t> &g, const nasl::vec3 &color,
    int vx, int vy, int vz, float t, float s, int nx, int ny, int nz)
{
    v.vx = vx;
    v.vy = vy;
    v.vz = vz;
    v.tt = t;
    v.ss = s;
    v.nnx = nx * 127 + 128;
    v.nny = ny * 127 + 128;
    v.nnz = nz * 127 + 128;
    v.br = color.x * 255;
    v.bg = color.y * 255;
    v.bb = color.z * 255;

    uint8_t tmp[sizeof(v)];
    memcpy(tmp, &v, sizeof(v));
    for (auto b : tmp)
        g.push_back(b);
}

//================================================================================================//
// SECTION 2: CORE MESHING FUNCTIONS
//================================================================================================//

/**
 * @brief Safely accesses block data from the current chunk or its direct neighbors.
 *
 * @param chunk The primary chunk data.
 * @param neighbours A structure containing pointers to the 8 neighboring chunks.
 * @param x, y, z The world coordinates of the block to access.
 * @return The BlockData at the given coordinates, or BlockAir if out of bounds or in a null chunk.
 */
static BlockData access_safe(const ChunkData *chunk, ChunkNeighbors &neighbours, int x, int y, int z)
{

    if (y < 0 || y >= CUNK_CHUNK_MAX_HEIGHT)
    {
        return BlockAir;
    }

    // Determine which neighbor chunk to access based on x and z coordinates
    int chunk_dx = (x < 0) ? 0 : (x >= CUNK_CHUNK_SIZE ? 2 : 1);
    int chunk_dz = (z < 0) ? 0 : (z >= CUNK_CHUNK_SIZE ? 2 : 1);

    const ChunkData *target_chunk = neighbours.neighbours[chunk_dx][chunk_dz];
    if (target_chunk)
    {
        // Normalize coordinates to be within the 0-15 range of the target chunk
        return chunk_get_block_data(target_chunk, x & (CUNK_CHUNK_SIZE - 1), y, z & (CUNK_CHUNK_SIZE - 1));
    }

    return BlockAir;
}

/**
 * @brief Checks if a block is solid. Can be expanded to handle other transparent types.
 */
static bool is_solid(BlockData block)
{
    return block != BlockAir;
}

/**
 * @brief A generalized function that performs greedy meshing on a 2D slice of the chunk.
 *
 * It iterates through the chunk along a primary axis (d0) and processes the other
 * two axes (d1, d2) as a 2D plane to generate merged quads.
 *
 * @param d0, d1, d2 The axes mapping for this slice (e.g., 0,1,2 for X,Y,Z).
 */
static void process_slice(
    int d0, int d1, int d2,
    const ChunkData *chunk, ChunkNeighbors &neighbours,
    std::vector<uint8_t> &g, size_t *num_verts)
{
    ChunkMesh::Vertex v;

    const int dims[] = {CUNK_CHUNK_SIZE, CUNK_CHUNK_MAX_HEIGHT, CUNK_CHUNK_SIZE};
    const int d0_max = dims[d0], d1_max = dims[d1], d2_max = dims[d2];

    for (int d0_coord = 0; d0_coord < d0_max; ++d0_coord)
    {
        std::vector<BlockData> mask(d1_max * d2_max, BlockAir);
        std::vector<bool> is_plus_mask(d1_max * d2_max, false);

        // 1. Fill the 2D mask with visible faces on this slice.
        for (int d1_coord = 0; d1_coord < d1_max; ++d1_coord)
        {
            for (int d2_coord = 0; d2_coord < d2_max; ++d2_coord)
            {
                int pos_curr[3] = {0, 0, 0}, pos_prev[3] = {0, 0, 0};
                pos_curr[d0] = d0_coord;
                pos_curr[d1] = d1_coord;
                pos_curr[d2] = d2_coord;
                pos_prev[d0] = d0_coord - 1;
                pos_prev[d1] = d1_coord;
                pos_prev[d2] = d2_coord;

                BlockData block_curr = access_safe(chunk, neighbours, pos_curr[0], pos_curr[1], pos_curr[2]);
                BlockData block_prev = access_safe(chunk, neighbours, pos_prev[0], pos_prev[1], pos_prev[2]);

                if (is_solid(block_curr) == is_solid(block_prev))
                    continue;

                size_t mask_idx = d1_coord * d2_max + d2_coord;
                if (is_solid(block_curr))
                {
                    mask[mask_idx] = block_curr; // Minus face of current block
                }
                else
                {
                    mask[mask_idx] = block_prev; // Plus face of previous block
                    is_plus_mask[mask_idx] = true;
                }
            }
        }

        // 2. Greedily generate quads from the mask.
        for (int j = 0; j < d1_max; ++j)
        {
            for (int i = 0; i < d2_max;)
            {
                if (mask[j * d2_max + i] != BlockAir)
                {
                    BlockData current_block = mask[j * d2_max + i];
                    bool is_plus = is_plus_mask[j * d2_max + i];

                    int w = 1; // Find width (w) along d2 axis
                    while (i + w < d2_max && mask[j * d2_max + (i + w)] == current_block && is_plus_mask[j * d2_max + (i + w)] == is_plus)
                        w++;

                    int h = 1; // Find height (h) along d1 axis
                    bool done = false;
                    while (j + h < d1_max && !done)
                    {
                        for (int k = 0; k < w; ++k)
                        {
                            if (mask[(j + h) * d2_max + (i + k)] != current_block || is_plus_mask[(j + h) * d2_max + (i + k)] != is_plus)
                            {
                                done = true;
                                break;
                            }
                        }
                        if (!done)
                            h++;
                    }

                    int vertex_base_coords[3];
                    vertex_base_coords[d0] = d0_coord;
                    vertex_base_coords[d1] = j;
                    vertex_base_coords[d2] = i;

                    if (is_plus)
                        vertex_base_coords[d0]--;

                    nasl::vec3 color = {block_colors[current_block].r, block_colors[current_block].g, block_colors[current_block].b};

                    int x = vertex_base_coords[0];
                    int y = vertex_base_coords[1];
                    int z = vertex_base_coords[2];

                    // 3. Generate vertices with CORRECT AND CONSISTENT winding order.
                    switch (d0)
                    {
                    case 0:          // X-faces. d1=Y (h), d2=Z (w)
                        if (is_plus) // +X face, winding is CCW when viewed from positive X
                        {
                            generate_vertex(v, g, color, x + 1, y, z, 0, 0, 1, 0, 0);
                            generate_vertex(v, g, color, x + 1, y, z + w, w, 0, 1, 0, 0);
                            generate_vertex(v, g, color, x + 1, y + h, z + w, w, h, 1, 0, 0);
                            generate_vertex(v, g, color, x + 1, y, z, 0, 0, 1, 0, 0);
                            generate_vertex(v, g, color, x + 1, y + h, z + w, w, h, 1, 0, 0);
                            generate_vertex(v, g, color, x + 1, y + h, z, 0, h, 1, 0, 0);
                        }
                        else // -X face, winding is CCW when viewed from negative X
                        {
                            generate_vertex(v, g, color, x, y, z, 0, 0, -1, 0, 0);
                            generate_vertex(v, g, color, x, y + h, z, 0, h, -1, 0, 0);
                            generate_vertex(v, g, color, x, y + h, z + w, w, h, -1, 0, 0);
                            generate_vertex(v, g, color, x, y, z, 0, 0, -1, 0, 0);
                            generate_vertex(v, g, color, x, y + h, z + w, w, h, -1, 0, 0);
                            generate_vertex(v, g, color, x, y, z + w, w, 0, -1, 0, 0);
                        }
                        break;
                    case 1:          // Y-faces. d1=X (h), d2=Z (w)
                        if (is_plus) // +Y face, winding is CCW when viewed from positive Y
                        {
                            generate_vertex(v, g, color, x, y + 1, z, 0, 0, 0, 1, 0);
                            generate_vertex(v, g, color, x + h, y + 1, z, h, 0, 0, 1, 0);
                            generate_vertex(v, g, color, x + h, y + 1, z + w, h, w, 0, 1, 0);
                            generate_vertex(v, g, color, x, y + 1, z, 0, 0, 0, 1, 0);
                            generate_vertex(v, g, color, x + h, y + 1, z + w, h, w, 0, 1, 0);
                            generate_vertex(v, g, color, x, y + 1, z + w, 0, w, 0, 1, 0);
                        }
                        else // -Y face, winding is CCW when viewed from negative Y
                        {
                            generate_vertex(v, g, color, x, y, z, 0, 0, 0, -1, 0);
                            generate_vertex(v, g, color, x, y, z + w, w, 0, 0, -1, 0);
                            generate_vertex(v, g, color, x + h, y, z + w, w, h, 0, -1, 0);
                            generate_vertex(v, g, color, x, y, z, 0, 0, 0, -1, 0);
                            generate_vertex(v, g, color, x + h, y, z + w, w, h, 0, -1, 0);
                            generate_vertex(v, g, color, x + h, y, z, 0, h, 0, -1, 0);
                        }
                        break;
                    case 2:          // Z-faces. d1=Y (h), d2=X (w)
                        if (is_plus) // +Z face, winding is CCW when viewed from positive Z
                        {
                            generate_vertex(v, g, color, x, y, z + 1, 0, 0, 0, 0, 1);
                            generate_vertex(v, g, color, x, y + h, z + 1, 0, h, 0, 0, 1);
                            generate_vertex(v, g, color, x + w, y + h, z + 1, w, h, 0, 0, 1);
                            generate_vertex(v, g, color, x, y, z + 1, 0, 0, 0, 0, 1);
                            generate_vertex(v, g, color, x + w, y + h, z + 1, w, h, 0, 0, 1);
                            generate_vertex(v, g, color, x + w, y, z + 1, w, 0, 0, 0, 1);
                        }
                        else // -Z face, winding is CCW when viewed from negative Z
                        {
                            generate_vertex(v, g, color, x, y, z, 0, 0, 0, 0, -1);
                            generate_vertex(v, g, color, x + w, y, z, w, 0, 0, 0, -1);
                            generate_vertex(v, g, color, x + w, y + h, z, w, h, 0, 0, -1);
                            generate_vertex(v, g, color, x, y, z, 0, 0, 0, 0, -1);
                            generate_vertex(v, g, color, x + w, y + h, z, w, h, 0, 0, -1);
                            generate_vertex(v, g, color, x, y + h, z, 0, h, 0, 0, -1);
                        }
                        break;
                    }
                    *num_verts += 6;

                    // 4. Clear processed area of the mask.
                    for (int l = 0; l < h; ++l)
                        for (int k = 0; k < w; ++k)
                            mask[(j + l) * d2_max + (i + k)] = BlockAir;
                    i += w;
                }
                else
                {
                    i++;
                }
            }
        }
    }
}

/**
 * @brief Main greedy meshing function.
 * Wraps the generalized 'process_slice' function, calling it for each of the three axes.
 */
void chunk_mesh(const ChunkData *chunk, ChunkNeighbors &neighbours, std::vector<uint8_t> &g, size_t *num_verts)
{
    *num_verts = 0;

    process_slice(0, 1, 2, chunk, neighbours, g, num_verts); // Y-Z planes -> X faces
    process_slice(1, 0, 2, chunk, neighbours, g, num_verts); // X-Z planes -> Y faces
    process_slice(2, 1, 0, chunk, neighbours, g, num_verts); // Y-X planes -> Z faces
}

// Undefine macro to prevent it from leaking to other compilation units.
#undef generate_vertex

//================================================================================================//
// SECTION 3: CHUNK MESH CLASS CONSTRUCTOR
//================================================================================================//

/**
 * @brief Constructs a ChunkMesh, generating its geometry and uploading it to the GPU.
 */
ChunkMesh::ChunkMesh(imr::Device &d, ChunkNeighbors &n)
{
    std::vector<uint8_t> g;
    // The central chunk is always at index [1][1] in the neighbors struct.
    chunk_mesh(n.neighbours[1][1], n, g, &num_verts);

    size_t buffer_size = g.size(); // g is a vector of bytes, so size() is the size in bytes.
    void *buffer = g.data();

    if (buffer_size > 0)
    {
        buf = std::make_unique<imr::Buffer>(d, buffer_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        buf->uploadDataSync(0, buffer_size, buffer);
    }
}