#ifndef AABB_H
#define AABB_H
#include <cglm/cglm.h>

typedef struct {
    vec2 min, max;
} box2f;


inline bool aabb_collide(box2f a, box2f b) {
    for (int i = 0; i < 2; ++i) {
        if (a.min[i] > b.max[i] || a.max[i] < b.min[i]) {
            return false;
        }
    }
    return true;
}
#endif
