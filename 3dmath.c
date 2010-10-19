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
sphere_intersect(float y[3], float r[3], float s[3], float d[3], float c[3], float R) {
    int i;
    float D, n[3], t, v[3];

    for(i = 0; i < 3; ++i)
        v[i] = s[i] - c[i];

    D = POW2(dot(v, d)) - (dot(v, v) - POW2(R));

    if(D < 0)
        return -1;

    t = -dot(v, d) - D;

    for(i = 0; i < 3; ++i) {
        y[i] = s[i] + t * d[i];
        n[i] = y[i] - c[i];
    }

    normalize(n);

    for(i = 0; i < 3; ++i)
        r[i] = d[i] - 2 * dot(n, d) * n[i];

    return t;
}
