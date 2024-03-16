#include "frustum.h"

#include "camera.h"
#include "window.h"

struct Frustum createCameraFrustum() {
    struct Frustum frustum;

   /*const float halfVSide = camera.far * tanf(camera.fov * .5f);
    const float halfHSide = halfVSide * window.aspectRatio;
    const vec3 frontMultFar;
    glm_vec3_mul((vec3){camera.far, camera.far, camera.far}, camera.front, frontMultFar);

    vec3 nA, nM;
    glm_vec3_add(camera.position, (vec3){camera.near, camera.near, camera.near}, nA);
    glm_vec3_mul(nA, camera.front, nM);
    glm_vec3_copy(nM, frustum.near.normal);
    glm_vec3_copy(camera.front, frustum.near.distance);

    vec3 fA;
    glm_vec3_add(camera.position, frontMultFar, fA);
    glm_vec3_copy(fA, frustum.far.normal);
    glm_vec3_copy((vec3){-camera.front[0], -camera.front[1], -camera.front[2]}, frustum.far.distance);

    vec3 rS, rM, rC;
    glm_vec3_sub(frontMultFar, camera.right, rS);
    glm_vec3_mul(rS, (vec3){halfHSide, halfHSide, halfHSide}, rM);
    glm_vec3_cross(rM, camera.up, rC);
    frustum.right = { camera.position, rC };

    vec3 lA, lM, lC;
    glm_vec3_add(frontMultFar, camera.right, lA);
    glm_vec3_mul(lA, (vec3){halfHSide, halfHSide, halfHSide}, lM);
    glm_vec3_cross(camera.up, lM, lC);
    frustum.left = { camera.position, lC };


    vec3 tS, tM, tC;
    glm_vec3_sub(frontMultFar, camera.up, tS);
    glm_vec3_mul(tS, (vec3){halfVSide, halfVSide, halfVSide}, tM);
    glm_vec3_cross(camera.right, tM, tC);
    frustum.top = { camera.position, tC };


    vec3 bA, bM, bC;
    glm_vec3_add(frontMultFar, camera.up, bA);
    glm_vec3_mul(bA, (vec3){halfVSide, halfVSide, halfVSide}, bM);
    glm_vec3_cross(camera.right, bM, bC);
    frustum.bottom = { camera.position, bC };*/

    return frustum;
}