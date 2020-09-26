#ifndef RENDERING_H
#include"game_platform_interface.h"
#include"linear_algebra.h"

struct Color
{
    float r;
    float g;
    float b;
    float a;
};

void rendering_init(GameMemory* game_memory, GameRenderInfo* render_info, float width, float height);

/* Convert a pixel coord in window space to viewport space */
Vec2 rendering_window_pos_to_viewport_pos(int x, int y);

/* Texture handle must be created before use */
typedef void* RenderTexture;
RenderTexture rendering_create_texture(void* image_data, uint32_t width, uint32_t height);
void rendering_replace_texture(RenderTexture tex, void* image_data);

void rendering_clear_screen(GameRenderInfo* render_info, Color color);

void rendering_set_camera(Vec2 pos);

/* Pos is centre of rect. Texture is optional, and will be stretched across the rect */
void rendering_draw_rect(Vec2 pos, Vec2 size, RenderTexture tex, Color color);

/* Draw one frame of a sprite sheet */
/* Pos is centre of rect. Size is in pixels, of one sprite on the texture, used with row and col to crop image */
void rendering_draw_sprite(Vec2 pos, Vec2 size, RenderTexture tex, uint32_t row, uint32_t col, Color color, bool hflip);

void rendering_draw_line(Vec2 origin, Vec2 point, float width, Color color);

//void rendering_draw_point(Vec2 pos, float size, Color color);

/* Pos is bottom left corner of text */
//void rendering_draw_text(Vec2 pos, float size, char* text, Color color);

#define RENDERING_H
#endif