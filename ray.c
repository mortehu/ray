#include "ray.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>

#include "3dmath.h"

#define BUFFER_SIZE (WIDTH * HEIGHT * 4)

#define LENGTH(array) (sizeof(array) / sizeof(array[0]))
#define MAX(x, y) (x > y ? x : y)
#define MIN(x, y) (x < y ? x : y)

#define TAU 6.28318531

#if __GNUC__ >= 3
#  define unlikely(cond) __builtin_expect ((cond), 0)
#  define likely(cond)   __builtin_expect ((cond), 1)
#else
#  define unlikely(cond) (cond)
#  define likely(cond) (cond)
#endif

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
    pthread_mutex_t mutex;

    unsigned char* buffer;
    long next_line;
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
trace(const float s[3], const float d[3], float pixel[3], int n, unsigned int mask) {
    float nearest = HUGE_VAL;
    int nearest_object = -1;
    float nearest_y[3];
    float nearest_r[3];

    for(int j = 0; j < LENGTH(objects); ++j) {
        float r[3], t, y[3];

        if ((1 << j) & mask) continue;

        t = sphere_intersect(y, r, s, d, objects[j].position, objects[j].radius);

        if(likely(t <= 0))
          continue;

        if (t < nearest) {
            nearest = t;
            nearest_object = j;
            memcpy(nearest_y, y, sizeof(nearest_y));
            memcpy(nearest_r, r, sizeof(nearest_y));
        }
    }

    if (nearest_object == -1) return;

    for(int m = 0; m < LENGTH(lights); ++m) {
        float l[3];
        for(int i = 0; i < 3; ++i)
            l[i] = lights[m].position[i] - nearest_y[i];

        float lr_dot = dot(l, nearest_r);
        if (lr_dot > 0) {
          float scale = lr_dot / sqrtf(dot(l, l)) / (1 << n);
          for(int k = 0; k < 3; ++k)
              pixel[k] += lights[m].diffuse[k] * objects[nearest_object].diffuse[k] * scale;
        }
    }

    trace(nearest_y, nearest_r, pixel, n + 1, (1 << nearest_object));
}

static void
trace_line(int l, unsigned char *buf) {
    static const float s[3] = {0, 0, 0};

    for(int i = 0; i < WIDTH; ++i, buf += 4) {
        float pixel[3] = { 0, 0, 0 };

        trace(s, trace_vectors[l][i], pixel, 1, 0);

        buf[0] = MIN(pixel[0], 1.0f) * 255;
        buf[1] = MIN(pixel[1], 1.0f) * 255;
        buf[2] = MIN(pixel[2], 1.0f) * 255;
    }
}

static void *
thread(void *arg) {
    ThreadArg* thread_arg = arg;

    for (;;) {
        pthread_mutex_lock(&thread_arg->mutex);
        if (thread_arg->next_line == HEIGHT) break;
        long line = thread_arg->next_line++;
        pthread_mutex_unlock(&thread_arg->mutex);

        trace_line(line, thread_arg->buffer + line * 4 * WIDTH);
    }

    pthread_mutex_unlock(&thread_arg->mutex);

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
        ThreadArg arg;
        memset(&arg, 0, sizeof(arg));
        pthread_mutex_init(&arg.mutex, NULL);
        arg.buffer = buf;

        int num_threads = sysconf(_SC_NPROCESSORS_CONF) - 1;
        pthread_t* threads = NULL;
        if (num_threads > 0) {
          threads = calloc(sizeof(*threads), num_threads);

          for (int i = 0; i < num_threads; ++i)
            pthread_create(&threads[i], NULL, thread, &arg);
        }

        thread(&arg);

        for(int i = 0; i < num_threads; ++i)
            pthread_join(threads[i], NULL);
    } else {
        for(int i = 0; i < HEIGHT; ++i)
            trace_line(i, buf + i * 4 * WIDTH);
    }
}
