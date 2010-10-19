#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "3dmath.h"

#define WIDTH 1080
#define HEIGHT 1080
#define BUFFER_SIZE (WIDTH * HEIGHT * 4)

#define LENGTH(array) (sizeof(array) / sizeof(array[0]))
#define MAX(x, y) (x > y ? x : y)
#define MIN(x, y) (x < y ? x : y)

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
static Object objects[2] = {{.position={0, 0, -2}, .radius=1, .diffuse={0, 0, .3}},
                            {.position={0, 0, -3}, .radius=1, .diffuse={0, 0, .3}}};
//static Light lights[1] = {{.position={1, 0, 0}, .diffuse={0, 0, .3}}};

static void
trace(float s[3], float d[3], float pixel[3]) {
    int i;
    float intensity, l[3], r[3], t, y[3];

    if(d[1] < 0) {
        if((int)(d[0] / d[1] * 20) % 2)
            pixel[0] += .1;

        if((int)(d[1] / d[0] * 20) % 2)
            pixel[1] += .1;
    }

    l[0] = -1;
    l[1] = 1;
    l[2] = 1;

    normalize(l);

    t = sphere_intersect(y, r, s, d, objects[0].position, objects[0].radius);

    //if(t > 0)
        //pixel[2] = dot(l, r);
}

static void
display(void) {
    static float s[3] = {0, 0, 0};
    int i, j;
    float x, y;
    float d[3];
    float pixel[3];

    memset(buffer, '\0', sizeof(buffer));

    for(i = 0; i < BUFFER_SIZE; i += 4) {
        x = (i / 4) % WIDTH - WIDTH / 2;
        y = (i / 4) / WIDTH - HEIGHT / 2;

        d[0] = x / (WIDTH / 2);
        d[1] = y / (HEIGHT / 2);
        d[2] = -1;

        normalize(d);

        memset(pixel, '\0', sizeof(pixel));
        trace(s, d, pixel);

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
    if (key == 27)
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
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);

    glutMainLoop();

    return 0;
}
