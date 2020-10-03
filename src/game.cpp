/* 
 * This file contains the entry points for game code, called by the platform layer
 */
#include"game_platform_interface.h"
#include"game.h"
#include"rendering.h"

Color background_color = Color{0.4F, 0.4F, 0.4F, 1.0F};
Color grid_color = Color{0.2F, 0.2F, 0.2F, 1.0F};

#define GRID_SPACING 0.1

void game_update_and_render(GameMemory* game_memory, GameInputBuffer* input_buffer, GameRenderInfo* render_info)
{
    GameMemoryBlock* block = (GameMemoryBlock*)(game_memory->memory);
    GameState* game_state = &block->game_state;

    f32 dt = input_buffer->dt;

    game_state->camera_pos = Vec2();

    GameInput *last_input = input_buffer->last_input();
    
    if (last_input->mouse_left_down)
    {
        Vec2 mouse_pos = rendering_window_pos_to_viewport_pos(last_input->mouse_x, last_input->mouse_y);
    }
    if (last_input->space)// && !input_buffer->prev_frame_input(1)->space)
    {
    }

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
        Color obj_color = Color{0.5,0.8,0.5,1.0};
        bool obj_wireframe = true;
        if (obj->is_static) {
            obj_color = Color{0.6,0.6,0.6,1.0};
            obj_wireframe = false;
        }
        switch(obj->shape)
        {
            case Obj::Circle:
                rendering_draw_circle(
                    obj->pos,
                    obj->width,
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
}

void game_init_memory(GameMemory* game_memory, GameRenderInfo* render_info)
{
    DEBUG_ASSERT(game_memory->memory_size >= sizeof(GameMemoryBlock));

    rendering_init(game_memory, render_info, GAME_WIDTH_PX, GAME_HEIGHT_PX);

    GameMemoryBlock* block = (GameMemoryBlock*)(game_memory->memory);
    GameState* game_state = &block->game_state;

    game_state->objs[0] = Obj{1, 0.2, 0.2, Vec2(0.5,0), Obj::Circle, false, Vec2(0,0), Vec2(0,0), Vec2(0,0)};
    game_state->objs[1] = Obj{1, 0.2, 0.1, Vec2(-0.5,0), Obj::Rect, false, Vec2(0,0), Vec2(0,0), Vec2(0,0)};
    game_state->objs[2] = Obj{1, 1.5, 0.2, Vec2(0.0,-0.6), Obj::Rect, true};
}