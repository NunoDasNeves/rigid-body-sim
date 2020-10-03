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
    
    Vec2 mouse_pos = Vec2{999,999};
    bool mouse_force = false;
    if (last_input->mouse_left_down)
    {
        mouse_pos = rendering_window_pos_to_viewport_pos(last_input->mouse_x, last_input->mouse_y);
    }
    if (last_input->space)// && !input_buffer->prev_frame_input(1)->space)
    {
    }

    /* physics */
    //physics_update(game_state, dt);
    Obj *objs = game_state->objs;
    
    /* slow down */
    dt *= 0.8F;

    for (int i = 0; i < MAX_OBJS; ++i)
    {
        Obj *obj = &objs[i];
        if (!obj->mass || obj->is_static)
            continue;
        /* Gravity */
        //obj->vel.y = obj->vel.y + -9.81F * dt;
        /* Mouse force */
        Vec2 m_force = obj->pos - mouse_pos;
        if (m_force.length() < MAX(obj->width, obj->height) / 2.0F)
        {
            /* scale length on constant factor */
            const f32 m_force_scale = 5.0F;
            m_force = m_force.normalized() * (m_force.length() / (MAX(obj->width, obj->height) / 2.0F)) * m_force_scale;
            obj->vel = obj->vel + (m_force * dt);
            mouse_force = true;
        }

        /* Integrate */
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
        if (!obj->mass)
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
                    obj->width / 2.0F,
                    obj_color,
                    obj_wireframe);
                break;
            case Obj::Rect:
                rendering_draw_rect(
                    obj->pos,
                    Vec2(obj->width, obj->height),
                    NULL,
                    obj_color,
                    obj_wireframe);
                break;
        }
    }

    /* mouse force */
    rendering_draw_circle(
                    mouse_pos,
                    0.01F,
                    mouse_force ? mouse_force_on_color : mouse_force_off_color,
                    false);
}

void game_init_memory(GameMemory* game_memory, GameRenderInfo* render_info)
{
    DEBUG_ASSERT(game_memory->memory_size >= sizeof(GameMemoryBlock));

    rendering_init(game_memory, render_info, GAME_WIDTH_PX, GAME_HEIGHT_PX);

    GameMemoryBlock* block = (GameMemoryBlock*)(game_memory->memory);
    GameState* game_state = &block->game_state;

    game_state->objs[0] = Obj{1, 0.4F, 0.0F, Vec2(0.5F,0.0F), Obj::Circle, false, Vec2(0,0), Vec2(0,0), Vec2(0,0)};
    game_state->objs[1] = Obj{1, 0.3F, 0.2F, Vec2(-0.5F,0.0F), Obj::Rect, false, Vec2(0,0), Vec2(0,0), Vec2(0,0)};
    game_state->objs[2] = Obj{1, 1.5F, 0.2F, Vec2(0.0F,-0.6F), Obj::Rect, true};
}