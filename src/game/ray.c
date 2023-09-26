#include "ray.h"

static inline float min(float x, float y) {
    return x < y ? x : y;
}

static inline float max(float x, float y) {
    return x > y ? x : y;
}

struct Ray ray(vec3 origin, vec3 dir, float length) {
    struct Ray self;

    glm_vec3_copy(origin, self.origin);
    glm_vec3_copy(dir, self.dir);
    glm_vec3_divs(self.dir, 1.0f, self.invDir);

    self.length = length;

    return self;
}

void updateRay(struct Ray *ray, vec3 origin, vec3 dir) {
    glm_vec3_copy(origin, ray->origin);
    glm_vec3_copy(dir, ray->dir);
    glm_vec3_divs(ray->dir, 1.0f, ray->invDir);
}

bool rayIntersection(const struct Ray ray, const struct Box box) {
    float tmin = 0.0, tmax = ray.length;

    for (int d = 0; d < 3; ++d) {
        float t1 = (box.min[d] - ray.origin[d]) * ray.invDir[d];
        float t2 = (box.max[d] - ray.origin[d]) * ray.invDir[d];

        tmin = max(tmin, min(t1, t2));
        tmax = min(tmax, max(t1, t2));
    }

    return tmin < tmax;
}