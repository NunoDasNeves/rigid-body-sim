#ifndef GAME_MATH_H

// TODO remove this dependency
#include<math.h>
#include"util.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846F
#endif

#define SIGN(X) ((X) > 0.0F ? (1) : ((X) == 0.0F ? (0) : (-1)))
#define MAX(X, Y) ((X) >= (Y) ? (X) : (Y))
#define MIN(X, Y) ((X) <= (Y) ? (X) : (Y))
#define FABS(X) ((X) >= 0.0F ? (X) : -(X))
#define FLOAT_EQ(X, Y, EPS) (FABS((X)-(Y)) <= (EPS))

int clamp(int val, int lo, int hi);
float clamp(float val, float lo, float hi);

struct PerlinNoiseGenerator
{
    int p[512];
    /*
    f32 x_grad[256];
    f32 y_grad[256];
    f32 z_grad[256];
    */

    PerlinNoiseGenerator(u32 seed);
    f32 noise(f32 x, f32 y, f32 z);
    f32 octave_noise(f32 x, f32 y, f32 z, u8 octaves, f32 persistence);
};


#define GAME_MATH_H
#endif