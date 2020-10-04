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

#define GRID_SPACING 0.1F

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

    /* physics */
    //physics_update(game_state, dt);
    Obj *objs = game_state->objs;
    
    /* slow down */
    dt *= 0.8F;

    for (int i = 0; i < MAX_OBJS; ++i)
    {
        Obj *obj = &objs[i];
        if (!obj->width || obj->is_static)
            continue;

        /* Compute forces */
        obj->torque = 0.0F;
        obj->force = Vec2{0.0F, 0.0F};
        /* Gravity */
        //obj->vel.y = obj->vel.y + -9.81F * dt;
        /* Mouse force */
        Vec2 mouse_to_obj = obj->pos - mouse_pos;
        if (mouse_to_obj.length() < MAX(obj->width, obj->height) / 2.0F)
        {
            mouse_force_on = true;
            if (mouse_released)
            {
                /* scale length on constant factor */
                static const f32 m_force_scale = 30.0F;
                Vec2 m_force = mouse_pos - game_state->mouse_force_origin;
                m_force = m_force * m_force_scale;
                obj->torque = obj->torque + (mouse_to_obj.x * m_force.y + mouse_to_obj.y * m_force.x);
                obj->force = obj->force + m_force;
            }
        }

        /* Integrate */
        obj->vel = obj->vel + ((obj->force / obj->mass) * dt);
        obj->pos = obj->pos + (obj->vel * dt);
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

    for (int i = 0; i < MAX_OBJS; ++i)
    {
        Obj *obj = &game_state->objs[i];
        if (!obj->width)
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
                    obj->width / 2.0F,
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