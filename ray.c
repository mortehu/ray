#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "3dmath.h"

#define WIDTH 1920
#define HEIGHT 1200
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

static unsigned char buffer[BUFFER_SIZE];
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
trace(float s[3], float d[3], float pixel[3], int n) {
    int i, j, k, m;
    float l[3], r[3], t, y[3];

    for(j = 0; j < LENGTH(objects); ++j) {
        t = sphere_intersect(y, r, s, d, objects[j].position, objects[j].radius);

        if(t > 0) {
            for(m = 0; m < LENGTH(lights); ++m) {
                for(i = 0; i < 3; ++i)
                    l[i] = lights[m].position[i] - y[i];

                normalize(l);
                for(k = 0; k < 3; ++k)
                    pixel[k] += lights[m].diffuse[k] * objects[j].diffuse[k] * (MAX(dot(l, r), 0)) / (1 << n);

                trace(y, r, pixel, n + 1);
            }
        }
    }
}

static void
display(void) {
    static float s[3] = {0, 0, 0};
    int i, j;
    float x, y;
    float d[3];
    float pixel[3];
    float time = (float)glutGet(GLUT_ELAPSED_TIME) / 1000;

    objects[0].position[0] = 1.5 * cos(time);
    objects[0].position[1] = 1.5 * sin(time);
    objects[1].position[0] = 1.5 * cos(time + 1/3. * TAU);
    objects[1].position[1] = 1.5 * sin(time + 1/3. * TAU);
    objects[3].position[0] = 1.5 * cos(time + 2/3. * TAU);
    objects[3].position[1] = 1.5 * sin(time + 2/3. * TAU);
    objects[2].position[2] = -3 + 2 * sin(time * 3);

    memset(buffer, '\0', sizeof(buffer));

    for(i = 0; i < BUFFER_SIZE; i += 4) {
        x = (i / 4) % WIDTH - WIDTH / 2;
        y = (i / 4) / WIDTH - HEIGHT / 2;

        memset(pixel, '\0', sizeof(pixel));

        d[0] = x / (WIDTH / 2);
        d[1] = y / (HEIGHT / 2) * ((float)HEIGHT / (float)WIDTH);
        d[2] = -1;

        normalize(d);

        trace(s, d, pixel, 1);

        for(j = 0; j < 3; ++j)
            buffer[i + j] = MIN(255 * pixel[j], 255);
    }

    glDrawPixels(WIDTH, HEIGHT, GL_BGRA, GL_UNSIGNED_BYTE, buffer);

    glutSwapBuffers();
}

static void
reshape(int w, int h) {
    glViewport(0, 0, w, h);
}

static void
keyboard(unsigned char key, int x, int y) {
    if(key == 27)
        exit(0);
}

static int
init(int argc, char **argv, int w, int h) {
    glutInit(&argc, argv);

    glutInitWindowPosition(0, 0);
    glutInitWindowSize(w, h);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutCreateWindow(argv[0]);

    glDepthMask(0);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    return 0;
}

int
main(int argc, char **argv) {
    if (init(argc, argv, WIDTH, HEIGHT))
        return 1;

    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);

    glutMainLoop();

    return 0;
}
