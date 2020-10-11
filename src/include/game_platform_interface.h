#ifndef GAME_PLATFORM_INTERFACE_H
/*
 * This file contains interfaces used by the platform to call into game code,
 * and interfaces for the game to call back to platform code
 * This file is used by the platform executable and game shared object library.
 */

#include"util.h"

typedef int ImageId;

//static const int GAME_ASPECT_RATIO_WIDTH = 16;
//static const int GAME_ASPECT_RATIO_HEIGHT = 9;
//static const int GAME_RESOLUTION_BASE_FACTOR = 20;

const u32 GAME_WIDTH_PX = 760;
const u32 GAME_HEIGHT_PX = 760;

struct GameRenderInfo
{
    int window_width;
    int window_height;
    bool resized;   // true if window has been resized since last frame

    int resolution_multiple_index = 0;  // index into GAME_RESOLUTION_MULTIPLES

    GameRenderInfo() : window_width(GAME_WIDTH_PX), window_height(GAME_HEIGHT_PX), resized(false) {}
};

struct GameInput
{
    float dt;               // time in seconds since last input (in case we poll more than once a frame)

    // the pixel the mouse pointer is in
    s32 mouse_x;
    s32 mouse_y;

    // button states
    bool mouse_left_down;
    bool mouse_right_down;
    bool mouse_middle_down;

    // wheel
    s32 mouse_wheel_scrolled;

    // keys
    bool up;
    bool down;
    bool left;
    bool right;

    bool p;

    bool _1;
    bool _2;
    bool _3;

    bool space;
    bool esc;
};

struct GameInputBuffer
{
    static const int INPUT_BUFFER_SIZE = 2;

    f32 dt;               // time in seconds since last frame

    // circular game input buffer;
    // input_buffer[last] is input we saw at the start of this frame,
    // input_buffer[(last-1) % INPUT_BUFFER_SIZE] etc are previous inputs
    uint32_t last;
    GameInput buffer[INPUT_BUFFER_SIZE];

    inline GameInput* last_input() { return &(buffer[last]); }
    inline GameInput* prev_frame_input(uint32_t offset)
    {
        DEBUG_ASSERT(offset < INPUT_BUFFER_SIZE);
        return &(buffer[(last + INPUT_BUFFER_SIZE - offset) % INPUT_BUFFER_SIZE]);
    }
};

void *DEBUG_platform_read_entire_file(const char *filename, s64 *returned_size);
char *DEBUG_platform_read_entire_file_as_string(const char *filename, s64 *returned_size);
void DEBUG_platform_free_file_memory(void *memory);

struct GameMemory
{
    void* (*platform_gl_get_proc_address)(const char*);

    unsigned memory_size;
    void* memory;
};

void game_init_memory(GameMemory* game_memory, GameRenderInfo* render_info);
void game_update_and_render(GameMemory* game_memory, GameInputBuffer* input_buffer, GameRenderInfo* render_info);

#define GAME_PLATFORM_INTERFACE_H
#endif