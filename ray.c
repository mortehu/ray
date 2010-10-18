#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define WIDTH 1080
#define HEIGHT 1080
#define BUFFER_SIZE (WIDTH * HEIGHT * 4)

#define LENGTH(array)   (sizeof(array) / sizeof(array[0]))
#define MAX(x, y) (x > y ? x : y)
#define MIN(x, y) (x < y ? x : y)
#define POW2(x) ((x) * (x))
#define POW3(x) ((x) * (x) * (x))
#define POW4(x) ((x) * (x) * (x) * (x))
#define POW5(x) ((x) * (x) * (x) * (x) * (x))

typedef struct {
    float position[3];
    float radius;
    float diffuse[3];
} Object;

typedef struct {
    float position[3];
    float diffuse[3];
} Light;

static Object objects[2] = {{.position={1.5, 0, -20}, .radius=.1, .diffuse={0, 0, .3}},
                            {.position={-1.5, 0, -30}, .radius=.1, .diffuse={0, 0, .3}}};
static Light lights[1] = {{.position={0, 2, -4}, .diffuse={0, .7, .7}}};
static unsigned char buffer[BUFFER_SIZE];

static float
dot(float x[3], float y[3]) {
    return x[0] * y[0] + x[1] * y[1] + x[2] * y[2];
}

static void
normalize(float x[3]) {
    float len;
    int i;

    len = sqrt(dot(x, x));

    for(i = 0; i < 3; ++i)
        x[i] /= len;
}

static void
trace(float s[3], float d[3], float pixel[3], int N) {
    int i, j;
    float D;
    float l[3];
    float n[3];
    float t;
    float v[3];
    float vd, vv;
    float y[3];
    Object *object = objects;
    Light *light = lights;

    for(j = 0; j < LENGTH(objects); ++j) {
        object = &objects[j];

        for(i = 0; i < 3; ++i)
            v[i] = s[i] - object->position[i];

        vd = dot(v, d);
        vv = dot(v, v);

        D = POW2(2 * vd) - 4 * (vv - POW2(object->radius));

        if(D < 0) {
            pixel[0] += .3;
            pixel[1] += .15;
            pixel[2] += .075;
        } else {

            t = MIN(-vd + sqrt(D), -vd - sqrt(D));

            for(i = 0; i < 3; ++i) {
                y[i] = s[i] + t * d[i];
                n[i] = y[i] - object->position[i];
                l[i] = light->position[i] - object->position[i];
            }

            normalize(n);
            normalize(l);

            for(i = 0; i < 3; ++i) {
                n[i] = (object->diffuse[i] + light->diffuse[i]) * dot(n, l);
                //n[i] = object->diffuse[i];
                pixel[i] += n[i];
            }

            if(N)
                trace(y, n, pixel, N - 1);
        }
    }
}

static void
display(void) {
    int i, j;
    float x, y;
    float s[3] = {0, 0, 1};
    float d[3];
    float pixel[3];

    memset(buffer, '\0', sizeof(buffer));

    for(i = 0; i < BUFFER_SIZE; i += 4) {
        x = (i / 4) % WIDTH - WIDTH / 2;
        y = (i / 4) / WIDTH - HEIGHT / 2;

        d[0] = x / (WIDTH / 2);
        d[1] = y / (HEIGHT / 2);
        d[2] = -1;

        memset(pixel, '\0', sizeof(pixel));
        trace(s, d, pixel, 0);

        //printf("%f %f %f\n", pixel[0], pixel[1], pixel[2]);

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
