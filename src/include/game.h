#ifndef GAME_H
#include"linear_algebra.h"
#include"game_math.h"

#define MAX_OBJS 128

struct AABB {
    Vec2 min;
    Vec2 max;
    void draw(bool intersecting);
    bool intersects(AABB other);
};

struct Obj {
    union {
        u32 exists; // read only. nonzero width/radius means exists
        f32 width;
        f32 radius;
    };
    f32 height; // ignored for circle

    Vec2 pos;
    f32 rot;

    enum {
        Circle,
        Rect,
    } shape;

    bool is_static;

    AABB aabb;

    /* Static objects have none of these: */
    f32 mass;
    f32 inertia;
    Vec2 vel;
    f32 alpha; // angular_vel
    Vec2 force;
    f32 torque;

    static Obj dyn_circle(f32 radius, Vec2 pos, f32 mass)
    {
        Obj obj = {};
        obj.radius = radius;
        obj.pos = pos;
        obj.shape = Circle;
        obj.mass = mass;
        obj.inertia = 0.5F * mass * radius * radius;
        obj.update_aabb();
        return obj;
    }
    static Obj dyn_rect(f32 width, f32 height, Vec2 pos, f32 rot, f32 mass)
    {
        Obj obj = {};
        obj.width = width;
        obj.height = height;
        obj.pos = pos;
        obj.rot = rot;
        obj.shape = Rect;
        obj.mass = mass;
        obj.inertia = (1.0F/12.0F) * mass * (height * height + width * width);
        obj.update_aabb();
        return obj;
    }
    static Obj static_circle(f32 radius, Vec2 pos)
    {
        Obj obj = {};
        obj.radius = radius;
        obj.pos = pos;
        obj.shape = Circle;
        obj.is_static = true;
        obj.update_aabb();
        return obj;
    }
    static Obj static_rect(f32 width, f32 height, Vec2 pos, f32 rot)
    {
        Obj obj = {};
        obj.width = width;
        obj.height = height;
        obj.pos = pos;
        obj.rot = rot;
        obj.shape = Rect;
        obj.is_static = true;
        obj.update_aabb();
        return obj;
    }

    void update_aabb();
};

struct Collision
{
    Obj *objs[2];
    Vec2 points[2];
    Vec2 normal; /* from obj 0 to 1 */
};

struct GameState
{
    Vec2 camera_pos;
    Obj objs[MAX_OBJS];
    bool paused;

    /* Physics */
    Obj *p_coll_pairs[MAX_OBJS * MAX_OBJS][2]; // potential
    Collision collisions[MAX_OBJS * MAX_OBJS];
    Vec2 mouse_force_origin;
    bool mouse_dragging;
};

void physics_update(GameState *game_state, f32 dt);

// Just for destructuring game memory buffer
struct GameMemoryBlock
{
    GameState *game_state;
    GameState game_states[10]; // one per number key
};

#define GAME_H
#endif