/* 
 * This file contains the entry points for game code, called by the platform layer
 */
#include"game_platform_interface.h"
#include"game.h"
#include"rendering.h"

Color background_color = Color{0.4F, 0.4F, 0.4F, 1.0F};
Color grid_color = Color{0.2F, 0.2F, 0.2F, 1.0F};
Color mouse_force_on_color = Color{1.0F,0.0F,0.0F,1.0F};
Color mouse_force_off_color = Color{0.1F,0.1F,0.1F,1.0F};
Color aabb_color = Color{1.0F,0.0F,0.0F,1.0F};

#define GRID_SPACING 0.1F

bool AABB::intersects(AABB other)
{
    if (this->min.x > other.max.x || this->max.x < other.min.x)
    {
        return false;
    }
    if (this->min.y > other.max.y || this->max.y < other.min.y)
    {
        return false;
    }
    return true;
}

void AABB::draw(bool intersecting)
{
    Color color = aabb_color;
    if (!intersecting)
    {
        color.r = 0.7F;
    }

    Vec2 extent = this->max - this->min;
    rendering_draw_line(
                    this->min,
                    Vec2(extent.x, 0),
                    1,
                    color);
    rendering_draw_line(
                    this->min,
                    Vec2(0, extent.y),
                    1,
                    color);
    rendering_draw_line(
                    this->max,
                    Vec2(-extent.x, 0),
                    1,
                    color);
    rendering_draw_line(
                    this->max,
                    Vec2(0, -extent.y),
                    1,
                    color);
}

void Obj::update_aabb()
{
    switch(this->shape)
    {
        case Obj::Circle:
            this->aabb.min = this->pos - Vec2(this->radius, this->radius);
            this->aabb.max = this->pos + Vec2(this->radius, this->radius);
            break;
        case Obj::Rect:
        {
            Vec2 verts[] = {
                Vec2(-this->width, -this->height) / 2.0F,
                Vec2(-this->width, this->height) / 2.0F,
                Vec2(this->width, this->height) / 2.0F,
                Vec2(this->width, -this->height) / 2.0F,
            };

            this->aabb.min = this->pos;
            this->aabb.max = this->pos;
            for (int i = 0; i < 4; ++i)
            {
                Mat4 model = Mat4::identity().frame_translate(Vec3(this->pos, 0.0)).frame_rotate_z(this->rot);
                Vec4 point4 = model * Vec4(verts[i], 0, 1);
                Vec2 point = Vec2(point4.x, point4.y);
                this->aabb.min.x = MIN(point.x, this->aabb.min.x);
                this->aabb.min.y = MIN(point.y, this->aabb.min.y);
                this->aabb.max.x = MAX(point.x, this->aabb.max.x);
                this->aabb.max.y = MAX(point.y, this->aabb.max.y);
            }
            break;
        }
    }
}

void game_update_and_render(GameMemory* game_memory, GameInputBuffer* input_buffer, GameRenderInfo* render_info)
{
    GameMemoryBlock* block = (GameMemoryBlock*)(game_memory->memory);
    GameState* game_state = &block->game_state;

    f32 dt = input_buffer->dt;

    game_state->camera_pos = Vec2();

    GameInput *last_input = input_buffer->last_input();
    
    bool mouse_force_on = false;
    Vec2 mouse_pos = rendering_window_pos_to_viewport_pos(last_input->mouse_x, last_input->mouse_y);
    bool mouse_released = false;
    if (last_input->mouse_left_down)
    {
        if (!game_state->mouse_dragging)
        {
            game_state->mouse_dragging = true;
            game_state->mouse_force_origin = mouse_pos;
        }
    }
    else
    {
        if (game_state->mouse_dragging)
        {
            game_state->mouse_dragging = false;
            mouse_released = true;
        }
    }

    /* physics - apply forces*/
    Obj *objs = game_state->objs;
    
    /* slow down */
    dt *= 0.8F;

    for (int i = 0; i < MAX_OBJS; ++i)
    {
        Obj *obj = &objs[i];
        if (!obj->exists || obj->is_static)
            continue;

        /* Compute forces */
        obj->torque = 0.0F;
        obj->force = Vec2{0.0F, 0.0F};
        /* Gravity */
        //obj->vel.y = obj->vel.y + -9.81F * dt;
        /* Mouse force */
        Vec2 mouse_to_obj = obj->pos - mouse_pos;
        /* TODO this check is hacky, redo */
        f32 radius_mouse_check = obj->shape == Obj::Rect ? MAX(obj->width, obj->height) / 2.0F : obj->radius;
        if (mouse_to_obj.length() < radius_mouse_check)
        {
            mouse_force_on = true;
            if (mouse_released)
            {
                /* scale length on constant factor */
                static const f32 m_force_scale = 30.0F;
                Vec2 m_force = mouse_pos - game_state->mouse_force_origin;
                m_force = m_force * m_force_scale;
                Vec2 obj_to_mouse = mouse_to_obj * -1.0F;
                obj->torque = obj->torque + (obj_to_mouse.x * m_force.y - obj_to_mouse.y * m_force.x);
                obj->force = obj->force + m_force;
            }
        }

        /* Integrate */
        obj->vel = obj->vel + ((obj->force / obj->mass) * dt);
        obj->pos = obj->pos + (obj->vel * dt);
        obj->alpha = obj->alpha + ((obj->torque / obj->inertia) * dt);
        obj->rot = obj->rot + obj->alpha * dt;
    }

    /* physics - collision detection */
    /* broad phase - compute AABBs */
    for (int i = 0; i < MAX_OBJS; ++i)
    {
        Obj *obj = &objs[i];
        if (!obj->exists || obj->is_static)
            continue;
        obj->update_aabb();
    }
    /* broad phase - produce pairs of potentially colliding objects */
    /* brute forceee */
    u32 p_coll_num = 0;
    for (int i = 0; i < MAX_OBJS; ++i)
    {
        Obj *objA = &objs[i];
        if (!objA->exists)
            continue;
        for (int j = i + 1; j < MAX_OBJS; ++j)
        {
            Obj *objB = &objs[j];
            if (!objB->exists)
                continue;
            if (objA->aabb.intersects(objB->aabb))
            {
                game_state->p_coll_pairs[p_coll_num][0] = objA;
                game_state->p_coll_pairs[p_coll_num][1] = objB;
                p_coll_num++;
            }
        }
    }
    /* TODO narrow phase - produce pairs of colliding objects */
    for (u32 i = 0; i < p_coll_num; ++i)
    {

    }

    /* rendering */
    rendering_clear_screen(render_info, background_color);
    rendering_set_camera(game_state->camera_pos);

    /* Grid lines */
    for (int i = 0; i < 20; ++i)
    {
        rendering_draw_line(
            Vec2(-1.0F + i * GRID_SPACING, -1.0F),
            Vec2(0.0F, 2.0F),
            1,
            grid_color);
        rendering_draw_line(
            Vec2(-1.0F, -1.0F + i * GRID_SPACING),
            Vec2(2.0F, 0.0F),
            1,
            grid_color);
    }

    /* objects */
    for (int i = 0; i < MAX_OBJS; ++i)
    {
        Obj *obj = &game_state->objs[i];
        if (!obj->exists)
            continue;
        Color obj_color = Color{0.5F,0.8F,0.5F,1.0F};
        bool obj_wireframe = true;
        if (obj->is_static) {
            obj_color = Color{0.6F,0.6F,0.6F,1.0F};
            obj_wireframe = false;
        }
        switch(obj->shape)
        {
            case Obj::Circle:
                rendering_draw_circle(
                    obj->pos,
                    obj->rot,
                    obj->radius,
                    obj_color,
                    obj_wireframe);
                break;
            case Obj::Rect:
                rendering_draw_rect(
                    obj->pos,
                    obj->rot,
                    Vec2(obj->width, obj->height),
                    NULL,
                    obj_color,
                    obj_wireframe);
                break;
        }
    }

    /* draw physics stuff */
    /* aabbs */
    for (int i = 0; i < MAX_OBJS; ++i)
    {
        Obj *obj = &game_state->objs[i];
        if (!obj->exists)
            continue;
        obj->aabb.draw(false);
    }

    /* p coll pairs (AABBs colliding) */
    for (u32 i = 0; i < p_coll_num; ++i)
    {
        game_state->p_coll_pairs[i][0]->aabb.draw(true);
        game_state->p_coll_pairs[i][1]->aabb.draw(true);
    }

    /* mouse force */
    if (game_state->mouse_dragging)
    {
        rendering_draw_line(
                    game_state->mouse_force_origin,
                    mouse_pos - game_state->mouse_force_origin,
                    2,
                    mouse_force_on ? mouse_force_on_color : mouse_force_off_color);
        rendering_draw_circle(
                    mouse_pos,
                    0,
                    0.01F,
                    mouse_force_on ? mouse_force_on_color : mouse_force_off_color,
                    false);
    }
}

void game_init_memory(GameMemory* game_memory, GameRenderInfo* render_info)
{
    DEBUG_ASSERT(game_memory->memory_size >= sizeof(GameMemoryBlock));

    rendering_init(game_memory, render_info, GAME_WIDTH_PX, GAME_HEIGHT_PX);

    GameMemoryBlock* block = (GameMemoryBlock*)(game_memory->memory);
    GameState* game_state = &block->game_state;

    game_state->objs[0] = Obj::dyn_circle(0.4F, Vec2(0.5F,0.0F), 1);
    game_state->objs[1] = Obj::dyn_rect(0.3F, 0.2F, Vec2(-0.5F,0.0F), M_PI / 4.0F, 1);
    game_state->objs[2] = Obj::static_rect(1.5F, 0.2F, Vec2(0.0F,-0.6F), 0);
}