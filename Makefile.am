AM_CFLAGS = -Wall -Wextra -pedantic -std=c99 -flto

bin_PROGRAMS = ray ray-headless


ray_SOURCES = main_glut.c ray.c 3dmath.c 3dmath.h
ray_LDFLAGS = -lGL -lGLU -lglut -lm -pthread

ray_headless_SOURCES = main_headless.c ray.c 3dmath.c 3dmath.h
ray_headless_LDFLAGS = -lm -pthread
