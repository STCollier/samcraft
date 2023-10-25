#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "world.h"
#include "../game/player.h"
#include "../game/util.h"

struct World world;

#define IDX(x, y) ((x) + (y) * GEN_LENGTH) // Use parentheses!

static void addChunk(int id, ivec2 worldPos) {
    struct Chunk *chunk = malloc(sizeof(struct Chunk));
    
    HASH_FIND_INT(world.chunks, &id, chunk);

    if (chunk == NULL) {
        chunk = malloc(sizeof(struct Chunk));

        chunk->id = id;

        HASH_ADD_INT(world.chunks, id, chunk);
    }

    initChunk(chunk, (ivec2){worldPos[0] - RENDER_DISTANCE, worldPos[1] - RENDER_DISTANCE}); // In terms of chunks, not blocks!
    genChunk(chunk);
}

static void deleteChunk(struct Chunk *chunk) {
    HASH_DEL(world.chunks, chunk);
    free(chunk);
}

static struct Chunk *getChunk(int id) {
    struct Chunk *chunk;

    HASH_FIND_INT(world.chunks, &id, chunk);  // chunk is output pointer

    if (chunk == NULL) {
        ERROR_IMSG("Could not locate chunk at id", id);
        //exit(EXIT_FAILURE);
    }

    return chunk;
}

static void debugChunk(struct Chunk* chunk) {
    printf("CHUNK: id(%d), worldPos(%d,%d) ", chunk->id, chunk->worldPos[0], chunk->worldPos[1]);
}

static void printChunkPos(struct Chunk* chunk) {
    printf("(%d, %d) ", chunk->worldPos[0], chunk->worldPos[1]);
}

void printChunkTable() {
    for (int y = player.chunkPos[1]; y < GEN_LENGTH + player.chunkPos[1]; y++) {
        for (int x = player.chunkPos[0]; x < GEN_LENGTH + player.chunkPos[0]; x++) {
            printChunkPos(getChunk(IDX(x, y)));
        }
        puts("");
    }

    puts("");
}


static void meshWorldChunk(int x, int y) {
    struct Chunk chunkNeighbors[4];

    chunkNeighbors[RIGHT] = *getChunk(IDX(x + 1, y));
    chunkNeighbors[LEFT]  = *getChunk(IDX(x - 1 , y));
    chunkNeighbors[FRONT] = *getChunk(IDX(x, y + 1));
    chunkNeighbors[BACK]  = *getChunk(IDX(x, y - 1));

    //for (int i = 0; i < 4; i++) chunkNeighbors[i].isNull = true;

    meshChunk(getChunk(IDX(x, y)), chunkNeighbors);
    bindChunk(getChunk(IDX(x, y)));
}

void initWorld() {
    world.chunks = NULL; // Initilize to NULL for hashtable

    glm_ivec2_copy((ivec2){0, 0}, world.oldPosition);

    for (int y = 0; y < GEN_LENGTH; y++) {
        for (int x = 0; x < GEN_LENGTH; x++) {
            addChunk(IDX(x, y), (ivec2){x, y});
        }
    }

    // Meshing chunks surrounded by other chunks; see world.h:18
     for (int y = 1; y < GEN_LENGTH - 1; y++) {
        for (int x = 1; x < GEN_LENGTH - 1; x++) {
            meshWorldChunk(x, y);
        }
    }

    LOG("World loaded!");
}

static void moveRight() {
    for (int y = player.chunkPos[1]; y < GEN_LENGTH + player.chunkPos[1]; y++) {
            int x = GEN_LENGTH + player.chunkPos[0];

            deleteChunk(getChunk(IDX(x, y)));
        }

        for (int y = player.chunkPos[1]; y < GEN_LENGTH + player.chunkPos[1]; y++) {
            int x = player.chunkPos[0];

            addChunk(IDX(x, y), (ivec2){x, y});

            if (y > player.chunkPos[1] && y <= RENDER_LENGTH + player.chunkPos[1]) {
                int meshX = x + 1; // Remember, we are always generating 1 extra row of chunks to account for chunk neighbors; we must get the row of chunks previous with (x + 1)

                meshWorldChunk(meshX, y);
            }
        }
}

static void moveLeft() {
      for (int y = player.chunkPos[1]; y < GEN_LENGTH + player.chunkPos[1]; y++) {
            int x = player.chunkPos[0] - 1;

            deleteChunk(getChunk(IDX(x, y)));
        }

       for (int y = player.chunkPos[1]; y < GEN_LENGTH + player.chunkPos[1]; y++) {
            int x = GEN_LENGTH + player.chunkPos[0] - 1;

            addChunk(IDX(x, y), (ivec2){x, y});

            if (y > player.chunkPos[1] && y <= RENDER_LENGTH + player.chunkPos[1]) {
                int meshX = x - 1; // Remember, we are always generating 1 extra row of chunks to account for chunk neighbors; we must get the row of chunks previous with (x - 1)

                meshWorldChunk(meshX, y);
            }
        }
}

static void moveForward() {
 for (int x = player.chunkPos[0]; x < GEN_LENGTH + player.chunkPos[0]; x++) {
            int y = player.chunkPos[1] - 1;

            deleteChunk(getChunk(IDX(x, y)));
        }

       for (int x = player.chunkPos[0]; x < GEN_LENGTH + player.chunkPos[0]; x++) {
            int y = GEN_LENGTH + player.chunkPos[1] - 1;

            addChunk(IDX(x, y), (ivec2){x, y});

            if (x > player.chunkPos[0] && x <= RENDER_LENGTH + player.chunkPos[0]) {
                int meshY = y - 1; // Remember, we are always generating 1 extra row of chunks to account for chunk neighbors; we must get the row of chunks previous with (y - 1)

                meshWorldChunk(x, meshY);
            }
        }
}

static void moveBackward() {
  for (int x = player.chunkPos[0]; x < GEN_LENGTH + player.chunkPos[0]; x++) {
            int y = GEN_LENGTH + player.chunkPos[1];

            deleteChunk(getChunk(IDX(x, y)));
        }

        for (int x = player.chunkPos[0]; x < GEN_LENGTH + player.chunkPos[0]; x++) {
            int y = player.chunkPos[1];

            addChunk(IDX(x, y), (ivec2){x, y});

            if (x > player.chunkPos[0] && x <= RENDER_LENGTH + player.chunkPos[0]) {
                int meshY = y + 1; // Remember, we are always generating 1 extra row of chunks to account for chunk neighbors; we must get the row of chunks previous with (x + 1)

                meshWorldChunk(x, meshY);
            }
        }
}

// This function is run only once, when the player moves between chunks
void moveWorld(ivec2 newPosition) {
    ivec2 result;
    glm_ivec2_sub(newPosition, world.oldPosition, result);
    glm_ivec2_copy(newPosition, world.oldPosition);

    if (result[0] == -1 && result[1] == 0) { // right
        moveRight();

        printf("Moved right to (%d, %d)\n", player.chunkPos[0], player.chunkPos[1]);
    } else if (result[0] == 1 && result[1] == 0) { // left
        moveLeft();

        printf("Moved left to (%d, %d)\n", player.chunkPos[0], player.chunkPos[1]);
    }

    if (result[0] == 0 && result[1] == 1) { // front
        moveForward();

        printf("Moved forward to (%d, %d)\n", player.chunkPos[0], player.chunkPos[1]);
    } else if (result[0] == 0 && result[1] == -1) { // back
        moveBackward();

        printf("Moved backward to (%d, %d)\n", player.chunkPos[0], player.chunkPos[1]); 
    }



    //printChunkTable();
}


// Called in update function
void renderWorld(struct Shader shader) {
    for (int y = 1 + player.chunkPos[1]; y < GEN_LENGTH - 1 + player.chunkPos[1]; y++) {
        for (int x = 1 + player.chunkPos[0]; x < GEN_LENGTH - 1 + player.chunkPos[0]; x++) {
            renderChunk(getChunk(IDX(x, y)), shader);
        }
    }
}

/*void destroyBlock(ivec2 chunkPos, ivec3 blockPos) {
    world.chunks[chunkIndex(chunkPos[0], chunkPos[1])].blocks[blockIndex(blockPos[0], blockPos[1], blockPos[2])].id = 1;
}*/