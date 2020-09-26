#ifndef GAME_H
#include<vector>
#include"linear_algebra.h"
#include"game_math.h"

struct DynamicObj {
    Vec2 pos;
    Vec2 vel;
    Vec2 accel;
    Vec2 alpha; // angular momentum
    f32 mass;
};

struct Circle : DynamicObj {
    f32 radius;
    Circle(f32 r) : radius(r) {}
};

struct StaticObj {
    Vec2 pos;
};

struct StaticRect : StaticObj {
    f32 width;
    f32 height;
};

struct GameState
{
    Vec2 camera_pos;
    std::vector<DynamicObj> dyn_objs;
};

void render_game(GameState* game_state);

// Just for destructuring game memory buffer
struct GameMemoryBlock
{
    GameState game_state;
};

#define GAME_H
#endif