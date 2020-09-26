/* 
 * This file contains the entry points for game code, called by the platform layer
 */
#include"game_platform_interface.h"
#include"game.h"
#include"rendering.h"

Color background_color = Color{0.5F, 0.5F, 1.0F, 1.0F};
Color grid_color = Color{0.2F, 0.2F, 0.2F, 1.0F};

#define GRID_SPACING 10

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
    for (int i = 0; i < 1024; ++i)
    {
        rendering_draw_line(
            Vec2(-512.0F + i * GRID_SPACING, -512.0F),
            Vec2(0.0F, 1024.0F),
            1,
            grid_color);
        rendering_draw_line(
            Vec2(-512.0F, -512.0F + i * GRID_SPACING),
            Vec2(1024.0F, 0.0F),
            1,
            grid_color);
    }
    
    rendering_draw_rect(
            Vec2((f32)GAME_WIDTH_PX/2.0F, (f32)GAME_HEIGHT_PX/2.0F),
            Vec2(10, 10),
            NULL,
            Color{1,0,0,1.0F});
    
    rendering_draw_rect(
            Vec2((f32)0.0F, 0.0F),
            Vec2(10, 10),
            NULL,
            Color{1,1,1,1.0F});
}

void game_init_memory(GameMemory* game_memory, GameRenderInfo* render_info)
{
    DEBUG_ASSERT(game_memory->memory_size >= sizeof(GameMemoryBlock));

    rendering_init(game_memory, render_info, GAME_WIDTH_PX, GAME_HEIGHT_PX);

    GameMemoryBlock* block = (GameMemoryBlock*)(game_memory->memory);
    GameState* game_state = &block->game_state;

    game_state->dyn_objs[0] = DynObj{1, 10, 0, Vec2(0,0), Vec2(0,0), Vec2(0,0), Vec2(0,0), DynObj::Circle};

}