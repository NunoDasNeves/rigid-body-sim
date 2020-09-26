#ifndef GAME_H
#include"linear_algebra.h"
#include"game_math.h"

#define MAX_DYN_OBJS 128

struct DynObj {
    f32 mass; // if 0, no obj

    f32 width;
    f32 height;

    Vec2 pos;
    Vec2 vel;
    Vec2 accel;
    Vec2 alpha; // angular momentum
    enum {
        Circle,
        Rect,
    } shape;
};

struct StaticRect {
    Vec2 pos;
    f32 width;
    f32 height;
};

struct GameState
{
    Vec2 camera_pos;
    DynObj dyn_objs[MAX_DYN_OBJS];
};

void render_game(GameState* game_state);

// Just for destructuring game memory buffer
struct GameMemoryBlock
{
    GameState game_state;
};

#define GAME_H
#endif