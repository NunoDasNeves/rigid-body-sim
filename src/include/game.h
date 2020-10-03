#ifndef GAME_H
#include"linear_algebra.h"
#include"game_math.h"

#define MAX_OBJS 128

struct Obj {
    f32 mass; // if 0, no obj

    f32 width;
    f32 height; // ignored for circle

    Vec2 pos;

    enum {
        Circle,
        Rect,
    } shape;

    bool is_static;

    Vec2 vel;
    Vec2 accel;
    Vec2 alpha; // angular momentum
};

struct GameState
{
    Vec2 camera_pos;
    Obj objs[MAX_OBJS];
};

void render_game(GameState* game_state);

// Just for destructuring game memory buffer
struct GameMemoryBlock
{
    GameState game_state;
};

#define GAME_H
#endif