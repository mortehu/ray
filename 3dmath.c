#include <math.h>

#define POW2(x) ((x) * (x))

float
dot(float x[3], float y[3]) {
    return x[0] * y[0] + x[1] * y[1] + x[2] * y[2];
}

void
normalize(float x[3]) {
    float len;
    int i;

    len = sqrt(dot(x, x));

    for(i = 0; i < 3; ++i)
        x[i] /= len;
}

float
sphere_intersect(float s[3], float d[3], float c[3], float r) {
    int i;
    float D;
    float v[3];

    for(i = 0; i < 3; ++i)
        v[i] = s[i] - c[i];

    D = POW2(dot(v, d)) - 4 * (dot(v, v) - POW2(r));

    if(D < 0)
        return -1;

    return 1;
}
