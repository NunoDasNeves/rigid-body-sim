#ifndef GAME_H
#include"linear_algebra.h"
#include"game_math.h"

#define MAX_OBJS 128

struct Obj {
    f32 width; // if 0, no obj. diameter for circle
    f32 height; // ignored for circle

    Vec2 pos;
    f32 rot;

    enum {
        Circle,
        Rect,
    } shape;

    bool is_static;

    /* Static objects have none of these: */
    f32 mass;
    f32 inertia;
    Vec2 vel;
    Vec2 alpha; // angular_vel
    Vec2 force;
    f32 torque;

    static Obj dyn_circle(f32 radius, Vec2 pos, f32 mass)
    {
        return Obj {
            radius * 2.0F, 0, pos, 0, Circle, false, mass,
            0.5F * mass * radius * radius // inertia
            };
    }
    static Obj dyn_rect(f32 width, f32 height, Vec2 pos, f32 rot, f32 mass)
    {
        return Obj {
            width, height, pos, rot, Rect, false, mass,
            (1.0F/12.0F) * mass * (height * height + width * width) // inertia
            };
    }
    static Obj static_circle(f32 radius, Vec2 pos)
    {
        return Obj { radius * 2.0F, 0, pos, 0, Circle, true };
    }
    static Obj static_rect(f32 width, f32 height, Vec2 pos, f32 rot)
    {
        return Obj { width, height, pos, rot, Rect, true };
    }
};

struct GameState
{
    Vec2 camera_pos;
    Obj objs[MAX_OBJS];

    /* Physics */
    //Obj p_coll_pairs[MAX_OBJS * MAX_OBJS][2]; // potential
    Vec2 mouse_force_origin;
    bool mouse_dragging;
};

void physics_update(GameState *game_state, f32 dt);

// Just for destructuring game memory buffer
struct GameMemoryBlock
{
    GameState game_state;
};

#define GAME_H
#endif