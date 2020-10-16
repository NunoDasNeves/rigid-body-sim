/*
 * This file contains the entry point for the SDL platform layer, and implementations of platform layer functions
 */

/****** this should be in a header file but the error checking plugin is bad *****/

#ifndef SDL_MAIN_H

#ifdef _WIN32
#include<windows.h>
#include<SDL.h>
#include<limits.h>

#define LARGE_ALLOC(SZ) VirtualAlloc(NULL, (SZ), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE)
#define LARGE_ALLOC_FIXED(SZ, ADDR) VirtualAlloc((LPVOID)(ADDR), (SZ), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE)
#define LARGE_FREE(PTR,SZ) DEBUG_ASSERT(VirtualFree((PTR), 0, MEM_RELEASE))

static const int MAX_PATH_LENGTH = MAX_PATH;

#else   // _WIN32

#ifdef __linux__
#include<sys/mman.h>
#include<SDL2/SDL.h>
#include<limits.h>

#define LARGE_ALLOC(X) mmap(NULL, (X), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0)
// TODO fixed alloc on linux
#define LARGE_ALLOC_FIXED(SZ, ADDR) LARGE_ALLOC(SZ)
#define LARGE_FREE(X,Y) munmap((X), (Y))

static const int MAX_PATH_LENGTH = PATH_MAX;

#endif // __linux__
#endif // else _WIN32

#include"game_platform_interface.h"

#define EXP_WEIGHTED_AVG(avg, N, new_sample) (((float)(avg) - (float)(avg)/(float)(N)) + (float)(new_sample)/(float)(N))

// distance between two indices 'prev' and 'curr' index in a ring buffer
#define DIST_IN_RING_BUFFER(prev, curr, size) ( \
    ((prev) > (curr) ? (curr) + (size) : (curr)) \
        - (prev) \
)

#define SDL_MAIN_H
#endif

/****** ^ above here should be in a header file but the error checking plugin is bad *****/

static bool running = true;

// Paths
static char* executable_path;                       // allocated by SDL

// Rendering
static SDL_Window* window = NULL;
static SDL_GLContext gl_context = NULL;
static SDL_Texture* texture = NULL;
static const int BYTES_PER_PIXEL = 4;
// TODO dynamic or adjustable
static int target_framerate = 60;
// TODO recompute based on framerate
static float target_frame_ms = 1000.0F/(float)target_framerate;

// Stuff passed to game
static GameMemory game_memory{};
static GameInputBuffer game_input_buffer{};
static GameRenderInfo game_render_info;

/*
void set_game_resolution(int resolution_multiple_index)
{
    game_render_info.resolution_multiple_index = resolution_multiple_index;
    SDL_SetWindowSize(window,
        GAME_ASPECT_RATIO_WIDTH * game_render_info.get_resolution_multiple() * GAME_RESOLUTION_BASE_FACTOR,
        GAME_ASPECT_RATIO_HEIGHT * game_render_info.get_resolution_multiple() * GAME_RESOLUTION_BASE_FACTOR);
}
*/

void *DEBUG_platform_read_entire_file(const char *filename, s64 *returned_size)
{
    SDL_RWops* file = SDL_RWFromFile(filename, "rb");
    if (!file)
    {
        FATAL_PRINTF("SDL Error: %s\n", SDL_GetError());
    }

    int64_t size = SDL_RWsize(file);
    if (size < 0)
    {
        FATAL_PRINTF("SDL Error: %s\n", SDL_GetError());
    }

    void* buffer = malloc(size);
    if (!buffer)
    {
        FATAL_PRINTF("malloc of read buffer failed\n");
    }

    int64_t len = SDL_RWread(file, buffer, size, 1);
    if (len <= 0)
    {
        FATAL_PRINTF("%s\n", SDL_GetError());
    }
    if (len != 1)
    {
        FATAL_PRINTF("Read less than expected: read %I64d, expected 1\n", len);
    }

    if (SDL_RWclose(file))
    {
        FATAL_PRINTF("SDL Error: %s\n", SDL_GetError());
    }

    *returned_size = size;

    return buffer;
}

char *DEBUG_platform_read_entire_file_as_string(const char *filename, s64 *returned_size)
{
    void* file_data = DEBUG_platform_read_entire_file(filename, returned_size);
    char* string = (char*)malloc(*returned_size + 1);
    if (!string)
    {
        FATAL_PRINTF("malloc of read string buffer failed\n");
    }

    SDL_memcpy(string, file_data, *returned_size);
    string[*returned_size] = '\0';
    free(file_data);

    return string;
}

void DEBUG_platform_free_file_memory(void *memory)
{
    free(memory);
}

static void handle_event(SDL_Event* e)
{
    bool key_state = false;

    switch(e->type)
    {
        case SDL_QUIT:
            running = false;
            break;

        case SDL_WINDOWEVENT:
        {
            switch(e->window.event)
            {
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                {
                    game_render_info.window_width = e->window.data1;
                    game_render_info.window_height = e->window.data2;
                    game_render_info.resized = true;
                    break;
                }
            }
        } break;
        case SDL_MOUSEBUTTONDOWN:
            key_state = true;
        case SDL_MOUSEBUTTONUP:
        {
            GameInput* input = &(game_input_buffer.buffer[game_input_buffer.last]);
            switch(e->button.button)
            {
                case SDL_BUTTON_LEFT:
                    input->mouse_left_down = key_state;
                    break;
                case SDL_BUTTON_RIGHT:
                    input->mouse_right_down = key_state;
                    break;
            }
            break;
        }
        case SDL_KEYDOWN:
            key_state = true;
        case SDL_KEYUP:
        {
            SDL_Keycode keycode = e->key.keysym.sym;
            GameInput* input = &(game_input_buffer.buffer[game_input_buffer.last]);
            // TODO make these remappable
            switch(keycode)
            {
                case SDLK_LEFT:
                case SDLK_a:
                    input->left = key_state;
                    break;
                case SDLK_UP:
                case SDLK_w:
                    input->up = key_state;
                    break;
                case SDLK_RIGHT:
                case SDLK_d:
                    input->right = key_state;
                    break;
                case SDLK_DOWN:
                case SDLK_s:
                    input->down = key_state;
                    break;
                case SDLK_p:
                    input->p = key_state;
                    break;
                case SDLK_r:
                    input->r = key_state;
                    break;
                case SDLK_1:
                    input->_1 = key_state;
                    break;
                case SDLK_2:
                    input->_2 = key_state;
                    break;
                case SDLK_3:
                    input->_3 = key_state;
                    break;
                case SDLK_SPACE:
                    input->space = key_state;
                    break;
                case SDLK_ESCAPE:
                    input->esc = key_state;
                    break;
            }
            break;
        }
    }
}

static void poll_mouse()
{
    GameInput* game_input = &(game_input_buffer.buffer[game_input_buffer.last]);
    int window_pixel_x, window_pixel_y;
    SDL_GetMouseState(&window_pixel_x, &window_pixel_y);

    game_input->mouse_x = window_pixel_x;
    game_input->mouse_y = game_render_info.window_height - window_pixel_y;   // put 0,0 in lower left
}

int main(int argc, char* args[])
{

    // Init SDL and SDL_image
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO) < 0)
    {
        FATAL_PRINTF("SDL couldn't be initialized - SDL_Error: %s\n", SDL_GetError());
    }

    // Set up paths
    executable_path = SDL_GetBasePath();
    if (!executable_path) {
        executable_path = SDL_strdup("./");
    }
    uint64_t executable_path_len = strlen(executable_path);

    // Create window
    window = SDL_CreateWindow(
        "Game",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        game_render_info.window_width, game_render_info.window_height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if(window == NULL)
    {
        FATAL_PRINTF("Window could not be created - SDL_Error: %s\n", SDL_GetError());
    }

    // Initialize openGL context
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, PLATFORM_GL_MAJOR_VERSION);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, PLATFORM_GL_MINOR_VERSION);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    gl_context = SDL_GL_CreateContext(window);
    if(gl_context == NULL)
    {
        FATAL_PRINTF("OpenGL context could not be created - SDL_Error: %s\n", SDL_GetError());
    }

    //Use Vsync
    if(SDL_GL_SetSwapInterval(1) < 0)
    {
        FATAL_PRINTF("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
    }

    // init game memory
    game_memory.memory_size = GIBIBYTES(1);
#ifdef FIXED_GAME_MEMORY
    game_memory.memory = LARGE_ALLOC_FIXED(game_memory.memory_size, TEBIBYTES(2));
#else
    game_memory.memory = LARGE_ALLOC(game_memory.memory_size);
#endif

    if (!game_memory.memory)
    {
        FATAL_PRINTF("Couldn't allocate game memory\n");
    }
    game_memory.platform_gl_get_proc_address = SDL_GL_GetProcAddress;

    game_init_memory(&game_memory, &game_render_info);

    // init game input
    memset(&game_input_buffer, 0, sizeof(GameInputBuffer));
    // in seconds
    game_input_buffer.dt = target_frame_ms / 1000.0F;

    ////////////////////////////
    // Now do the game loop
    SDL_Event e;

    // timer
    uint64_t frame_start_time = SDL_GetPerformanceCounter();

    while(running)
    {
        // Input
        // advance game input buffer, and clear next entry
        game_input_buffer.last = (game_input_buffer.last + 1) % GameInputBuffer::INPUT_BUFFER_SIZE;
        memset(&game_input_buffer.buffer[game_input_buffer.last], 0, sizeof(GameInput));
        // copy previous keyboard state (otherwise keys only fire on each keyboard event bounded by OS repeat rate)
        game_input_buffer.buffer[game_input_buffer.last] = \
            game_input_buffer.buffer[(game_input_buffer.last + GameInputBuffer::INPUT_BUFFER_SIZE - 1) % GameInputBuffer::INPUT_BUFFER_SIZE];

        // TODO for now these dts are the same as we only poll once a frame
        game_input_buffer.buffer[game_input_buffer.last].dt = game_input_buffer.dt;
        
        while (SDL_PollEvent(&e))
        {
            handle_event(&e);
        }
        poll_mouse();

        // Rendering
        // Call the game code
        game_update_and_render(&game_memory, &game_input_buffer, &game_render_info);

        // Swap buffers (actually make the image appear)
        SDL_GL_SwapWindow(window);

        // Timing
        uint64_t frame_end_time = SDL_GetPerformanceCounter();
        float frame_time_ms = 1000.0F * (float)(frame_end_time - frame_start_time)/(float)SDL_GetPerformanceFrequency();
        float diff_ms = target_frame_ms - frame_time_ms;
        int loops = 0;
        while (diff_ms > 0.1F)
        {
            if (diff_ms > 1.5F)
            {
                //DEBUG_PRINTF("sleeping for: %lf\n", diff_ms);
                SDL_Delay((uint32_t)(diff_ms - 1.0F));
            }
            frame_end_time = SDL_GetPerformanceCounter();
            frame_time_ms = 1000.0F * (float)(frame_end_time - frame_start_time)/(float)SDL_GetPerformanceFrequency();
            diff_ms = target_frame_ms - frame_time_ms;
            loops++;
        }
        //DEBUG_PRINTF("loops: %d\n", loops);
        if (frame_time_ms > (target_frame_ms + 1.0F))
        {
            //DEBUG_PRINTF("frame_time_ms: %lf\n", frame_time_ms);
        }
        frame_start_time = frame_end_time;

    }

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
