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

double octaveNoise(ivec2 pos, ivec2 offset, double spread, double size, int numOctaves, double persistence) {
    double total = 0;
    double frequency = 1;
    double amplitude = 1;
    double maxValue = 0;

    for (int i = 0; i < numOctaves; i++) {
        total += open_simplex_noise2(ctx, (double)((pos[0] + offset[0]) / spread * frequency), (double)((pos[1] - offset[1]) / spread * frequency)) * amplitude;

        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= 2;
    }

    // Normalize the result to the range [0, 1]
    return (total / maxValue) * size;
}

int noiseHeight(ivec2 pos, ivec2 offset) {
    double spread = 75;
    double size = 60;
    int numOctaves = 3;
    double persistence = 0.5;

    double value = octaveNoise(pos, offset, spread, size, numOctaves, persistence);
    value += 40;

    return (int)value;
}