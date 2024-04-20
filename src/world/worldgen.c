#include "worldgen.h"
#include "chunk.h"
#include <math.h>
#include <stdlib.h>

struct osn_context *ctx;

void worldgenInit(int64_t seed) {
	open_simplex_noise(seed, &ctx);
    srand(seed);
}

int randInRange(int min, int max) {
   return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}
double octaveNoise(ivec3 pos, ivec3 offset, double spread, double size, int numOctaves, double persistence) {
    double total = 0;
    double frequency = 1;
    double amplitude = 1;
    double maxValue = 0;

    for (int i = 0; i < numOctaves; i++) {
        total += open_simplex_noise3(ctx, (double)((pos[0] + offset[0]) / spread * frequency), (double)((pos[1] - offset[1]) / spread * frequency), (double)((pos[2] - offset[2]) / spread * frequency)) * amplitude;

        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= 2;
    }

    // Normalize the result to the range [0, 1]
    return (total / maxValue) * size;
}

int noiseHeight(ivec3 pos, ivec3 offset) {
    double spread = 100;
    double size = 200;
    int numOctaves = 3;
    double persistence = 0.5;

    double value = octaveNoise(pos, offset, spread, size, numOctaves, persistence);
    value += 80;

    return (int) value;
}