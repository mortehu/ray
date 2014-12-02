#include "ray.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

#include "3dmath.h"

#define BUFFER_SIZE (WIDTH * HEIGHT * 4)

#define LENGTH(array) (sizeof(array) / sizeof(array[0]))
#define MAX(x, y) (x > y ? x : y)
#define MIN(x, y) (x < y ? x : y)

#define TAU 6.28318531

typedef struct {
    float position[3];
    float radius;
    float diffuse[3];
} Object;

typedef struct {
    float position[3];
    float diffuse[3];
} Light;

typedef struct {
    unsigned char* buffer;
    long line;
} ThreadArg;

static float trace_vectors[HEIGHT][WIDTH][3];
static int trace_vectors_initialized;

static Object objects[] = {
    {.position={-1.414, -1, -3}, .radius=1, .diffuse={.8, 0, .8}},
    {.position={0, 1.414, -3}, .radius=1, .diffuse={0, .8, .8}},
    {.position={0, 0, -3}, .radius=.25, .diffuse={.8, .8, .8}},
    {.position={1.414, -1, -3}, .radius=1, .diffuse={.8, .8, 0}}
};
static Light lights[] = {
    {.position={-3, 3, -4}, .diffuse={0, .6, .6}},
    {.position={0, 30, -4}, .diffuse={1, 1, 1}}
};

static void
trace(float s[3], const float d[3], float pixel[3], int n) {
    int i, j, k, m;
    float l[3], r[3], t, y[3];

    for(j = 0; j < LENGTH(objects); ++j) {
        t = sphere_intersect(y, r, s, d, objects[j].position, objects[j].radius);

        if(t > 0) {
            for(m = 0; m < LENGTH(lights); ++m) {
                for(i = 0; i < 3; ++i)
                    l[i] = lights[m].position[i] - y[i];

                float lr_dot = dot(l, r);
                if (lr_dot > 0) {
                  float scale = lr_dot / sqrtf(dot(l, l)) / (1 << n);
                  for(k = 0; k < 3; ++k)
                      pixel[k] += lights[m].diffuse[k] * objects[j].diffuse[k] * scale;
                }

                trace(y, r, pixel, n + 1);
            }
        }
    }
}

static void
trace_line(int l, unsigned char *buf) {
    static float s[3] = {0, 0, 0};
    float y = l - HEIGHT / 2;

    for(int i = 0; i < WIDTH; ++i) {
        float pixel[3];
        memset(pixel, '\0', sizeof(pixel));

        const float* d = trace_vectors[l][i];

        trace(s, d, pixel, 1);

        for(int j = 0; j < 3; ++j)
            buf[i * 4 + j] = MIN(255 * pixel[j], 255);
    }
}

static void *
thread(void *arg) {
    ThreadArg* thread_arg = arg;

    trace_line(thread_arg->line, thread_arg->buffer + thread_arg->line * 4 * WIDTH);

    return NULL;
}

static void
initialize_trace_vectors(void) {
    for(int y = 0; y < HEIGHT; ++y) {
        for(int x = 0; x < WIDTH; ++x) {
          float* d = trace_vectors[y][x];
          d[0] = ((float)x / WIDTH - 0.5f) * 2.0f;
          d[1] = ((float)y / HEIGHT - 0.5f) * 2.0f * ((float)HEIGHT / WIDTH);
          d[2] = -1;
          normalize(d);
        }
    }
    trace_vectors_initialized = 1;
}

void
trace_scene(float time, unsigned char *buf, int threaded) {
    if (!trace_vectors_initialized)
      initialize_trace_vectors();

    objects[0].position[0] = 1.5 * cos(time);
    objects[0].position[1] = 1.5 * sin(time);
    objects[1].position[0] = 1.5 * cos(time + 1/3. * TAU);
    objects[1].position[1] = 1.5 * sin(time + 1/3. * TAU);
    objects[3].position[0] = 1.5 * cos(time + 2/3. * TAU);
    objects[3].position[1] = 1.5 * sin(time + 2/3. * TAU);
    objects[2].position[2] = -3 + 2 * sin(time * 2);

    if(threaded) {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

        ThreadArg thread_args[HEIGHT];
        pthread_t threads[HEIGHT];
        for(long i = 0; i < HEIGHT; ++i) {
            thread_args[i].line = i;
            thread_args[i].buffer = buf;

            int ret = pthread_create(&threads[i], &attr, thread, &thread_args[i]);

            if(ret) {
                fprintf(stderr, "pthread_create(): %d\n", ret);
                exit(EXIT_FAILURE);
            }
        }

        void *status;
        for(long i = 0; i < HEIGHT; ++i)
            pthread_join(threads[i], &status);
    } else {
        for(int i = 0; i < HEIGHT; ++i)
            trace_line(i, buf + i * 4 * WIDTH);
    }
}
