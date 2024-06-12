#include "../core/camera.h"

#include "frustum.h"

struct Frustum updateCameraFrustum() {
    struct Frustum frustum;

    mat4 viewProjection, invViewProjection;

    glm_mat4_mul(camera.projection, camera.view, viewProjection);
    glm_mat4_inv(viewProjection, invViewProjection);

    glm_frustum_planes(viewProjection, frustum.planes);
    glm_frustum_corners(invViewProjection, frustum.corners);

    return frustum;
}

bool boxInFrustum(struct Frustum frustum, struct Chunk chunk) {
    vec3 min = {chunk.position[0] * CHUNK_SIZE, chunk.position[1] * CHUNK_SIZE, chunk.position[2] * CHUNK_SIZE};
    vec3 max = {chunk.position[0] * CHUNK_SIZE + CHUNK_SIZE, chunk.position[1] * CHUNK_SIZE + CHUNK_SIZE, chunk.position[2] * CHUNK_SIZE + CHUNK_SIZE};

    for (int i = 0; i < 6; i++) {
        int out = 0;

        out += ((dot4(frustum.planes[i], (vec4){min[0], min[1], min[2], 1.0f}) < 0.0 ) ? 1 : 0);
        out += ((dot4(frustum.planes[i], (vec4){max[0], min[1], min[2], 1.0f}) < 0.0 ) ? 1 : 0);
        out += ((dot4(frustum.planes[i], (vec4){min[0], max[1], min[2], 1.0f}) < 0.0 ) ? 1 : 0);
        out += ((dot4(frustum.planes[i], (vec4){max[0], max[1], min[2], 1.0f}) < 0.0 ) ? 1 : 0);
        out += ((dot4(frustum.planes[i], (vec4){min[0], min[1], max[2], 1.0f}) < 0.0 ) ? 1 : 0);
        out += ((dot4(frustum.planes[i], (vec4){max[0], min[1], max[2], 1.0f}) < 0.0 ) ? 1 : 0);
        out += ((dot4(frustum.planes[i], (vec4){min[0], max[1], max[2], 1.0f}) < 0.0 ) ? 1 : 0);
        out += ((dot4(frustum.planes[i], (vec4){max[0], max[1], max[2], 1.0f}) < 0.0 ) ? 1 : 0);

        if (out == 8) return false;
    }

    // Advanced culling https://iquilezles.org/articles/frustumcorrect/
    /*int out;
    out = 0; for(int i = 0; i < 8; i++) out += ((frustum.corners[i][0] > max[0]) ? 1 : 0); if(out == 8) return false;
    out = 0; for(int i = 0; i < 8; i++) out += ((frustum.corners[i][0] < min[0]) ? 1 : 0); if(out == 8) return false;
    out = 0; for(int i = 0; i < 8; i++) out += ((frustum.corners[i][1] > max[1]) ? 1 : 0); if(out == 8) return false;
    out = 0; for(int i = 0; i < 8; i++) out += ((frustum.corners[i][1] < min[1]) ? 1 : 0); if(out == 8) return false;
    out = 0; for(int i = 0; i < 8; i++) out += ((frustum.corners[i][2] > max[2]) ? 1 : 0); if(out == 8) return false;
    out = 0; for(int i = 0; i < 8; i++) out += ((frustum.corners[i][2] < min[2]) ? 1 : 0); if(out == 8) return false;*/

    return true;
}