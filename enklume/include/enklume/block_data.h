#ifndef CUNK_CHUNK
#define CUNK_CHUNK

#include <stdint.h>
#include <stddef.h>

#include "enklume.h"

#define CUNK_CHUNK_SIZE 16
#define CUNK_CHUNK_MAX_HEIGHT 384
#define CUNK_CHUNK_SECTIONS_COUNT CUNK_CHUNK_MAX_HEIGHT / CUNK_CHUNK_SIZE

typedef uint32_t BlockData;

static BlockData air_data = 0;

typedef struct {
    BlockData block_data[CUNK_CHUNK_SIZE][CUNK_CHUNK_SIZE][CUNK_CHUNK_SIZE];
} ChunkSection;

typedef struct {
    ChunkSection* sections[CUNK_CHUNK_SECTIONS_COUNT];
} ChunkData;

BlockData chunk_get_block_data(const ChunkData*, unsigned x, unsigned y, unsigned z);
void chunk_set_block_data(ChunkData*, unsigned x, unsigned y, unsigned z, BlockData);

void load_from_mcchunk(ChunkData* dst_chunk, McChunk* chunk);

#endif
