/* 
 * This file contains the entry points for game code, called by the platform layer
 */
#include"game_platform_interface.h"
#include"game.h"
#include"rendering.h"

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

    rendering_clear_screen(render_info, Color{0.0F, 0.0F, 0.0F, 1.0F});
    rendering_set_camera(game_state->camera_pos);
    rendering_draw_rect(
            Vec2((f32)GAME_WIDTH_PX/2.0F, (f32)GAME_HEIGHT_PX/2.0F),
            Vec2(GAME_WIDTH_PX, GAME_HEIGHT_PX),
            NULL,
            Color{0,0,0,0.0F});

    /*
    rendering_draw_rect(
            Vec2(game_state->water_source.x + 2.5F, game_state->water_source.y + 2.5F),
            Vec2(5.0F,5.0F),
            NULL,
            Color{1.0F,0,0.0F,1.0F});
    */

    /*
    rendering_draw_line(
        Vec2(4.0F, 4.0F),
        Vec2(1.0F, 1.0F),
        1,
        Color{0,0,1.0F,1.0F});
    */
}

void game_init_memory(GameMemory* game_memory, GameRenderInfo* render_info)
{
    DEBUG_ASSERT(game_memory->memory_size >= sizeof(GameMemoryBlock));

    rendering_init(game_memory, render_info, GAME_WIDTH_PX, GAME_HEIGHT_PX);

    GameMemoryBlock* block = (GameMemoryBlock*)(game_memory->memory);
    GameState* game_state = &block->game_state;

    game_state->dyn_objs.push_back(Circle(2.0F));
}