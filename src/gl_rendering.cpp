#include"glad/glad.h"
#include"rendering.h"

#define SHADERS_PATH "shaders/"
#define VERT_EXT ".vert"
#define FRAG_EXT ".frag"

struct Shader
{
    static const int MAX_NAME = 64;
    static const int MAX_PATH = MAX_NAME + sizeof(SHADERS_PATH) + sizeof(VERT_EXT); // MAX NAME includes the null character
    bool initialized;
    char name[MAX_NAME];   // determines which shader to load: shaders/<name>.vert and shaders/<name>.frag
    GLuint id;
};

// Use Texture* internally, and RenderTexture externally
struct Texture
{
    bool initialized = false;
    GLuint id;
    uint32_t width;
    uint32_t height;
};

struct Rect
{
    GLuint VAO;
    GLuint vertices_VBO;
    GLuint texture_coords_VBO;
    GLuint EBO;
};

#define NUM_CIRCLE_POINTS 64

struct Circle
{
    GLuint VAO;
    GLuint vertices_VBO;
    GLuint EBO;
};

struct Line
{
    GLuint VAO;
    GLuint vertices_VBO;
    GLuint EBO;
};

static const uint32_t RECT_TEX_COORDS_ARRAY_SIZE = 8;
static const uint32_t RECT_TEX_COORDS_SIZE = RECT_TEX_COORDS_ARRAY_SIZE * sizeof(GLfloat);
static const GLfloat RECT_DEFAULT_TEX_COORDS[RECT_TEX_COORDS_ARRAY_SIZE] =
    {1, 1,
    0, 1,
    0, 0,
    1, 0};

// Used to draw all primitive
static Rect global_rect;
static Line global_line;
static Circle global_circle;

// The shader used to draw rects
static Shader sprite_shader;

// idk
static const int MAX_TEXTURES = 256;
static Texture textures[MAX_TEXTURES];
static Texture* empty_texture;


static const float GAME_ASPECT = (float)GAME_WIDTH_PX/(float)GAME_HEIGHT_PX;

static GLint gl_viewport_x = 0;
static GLint gl_viewport_y = 0;
static GLsizei gl_viewport_width = GAME_WIDTH_PX;
static GLsizei gl_viewport_height = GAME_HEIGHT_PX;

// Camera view matrix for world -> camera coords
static Mat4 view;

// Camera projection matrix for pixel -> screen space transformation
static Mat4 projection;

// buffer for errors from opengl
static const int INFO_LOG_SIZE = 512;
static char info_log[INFO_LOG_SIZE];

/*
 * Initialized a shader struct
 */
static void load_shader(GameMemory* game_memory, Shader* shader, char* name)
{
    int success;

    GLuint vert_id, frag_id;
    int64_t vert_len, frag_len;
    char* vert_source;
    char* frag_source;

    char path_buf[Shader::MAX_PATH] = {};

    int64_t name_len = strlen(name);

    DEBUG_ASSERT(!shader->initialized);
    DEBUG_ASSERT(name_len < Shader::MAX_NAME);

    memcpy((void*)shader->name, name, name_len);
    shader->name[name_len] = '\0';

    DEBUG_PRINTF("Loading shader \"%s\"\n", name);

    // read in shader source

    // make vert path
    // TODO make own strlcat function; this is dumb
    strncat(path_buf, SHADERS_PATH, Shader::MAX_PATH - 1);
    strncat(path_buf, name, Shader::MAX_PATH - strlen(path_buf) - 1);
    strncat(path_buf, VERT_EXT, Shader::MAX_PATH - strlen(path_buf) - 1);

    vert_source = DEBUG_platform_read_entire_file_as_string(path_buf, &vert_len);

    // make frag path
    path_buf[strlen(SHADERS_PATH) + name_len] = '\0';
    strncat(path_buf, FRAG_EXT, Shader::MAX_PATH - strlen(path_buf) - 1);
    frag_source = DEBUG_platform_read_entire_file_as_string(path_buf, &frag_len);

    // create shader objects
    vert_id = glCreateShader(GL_VERTEX_SHADER);
    frag_id = glCreateShader(GL_FRAGMENT_SHADER);

    // compile the source, and check if compilation was successful
    glShaderSource(vert_id, 1, &vert_source, NULL);
    glCompileShader(vert_id);
    glGetShaderiv(vert_id, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vert_id, INFO_LOG_SIZE, NULL, info_log);
        FATAL_PRINTF("Vertex shader compilation failed: %s\n", info_log);
    }

    glShaderSource(frag_id, 1, &frag_source, NULL);
    glCompileShader(frag_id);
    glGetShaderiv(frag_id, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(frag_id, INFO_LOG_SIZE, NULL, info_log);
        FATAL_PRINTF("Fragment shader compilation failed: %s\n", info_log);
    }

    // Create and link the shader program which uses these shaders
    shader->id = glCreateProgram();
    glAttachShader(shader->id, vert_id);
    glAttachShader(shader->id, frag_id);
    glLinkProgram(shader->id);

    // Check for errors
    glGetProgramiv(shader->id, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shader->id, INFO_LOG_SIZE, NULL, info_log);
        FATAL_PRINTF("Shader program linking failed: %s\n", info_log);
    }

    // Aaand clean up
    glDeleteShader(vert_id);
    DEBUG_platform_free_file_memory(vert_source);
    glDeleteShader(frag_id);
    DEBUG_platform_free_file_memory(frag_source);

    shader->initialized = true;
}

RenderTexture rendering_create_texture(void* image_data, uint32_t width, uint32_t height)
{
    Texture* ret = NULL;
    for (int i = 0; i < MAX_TEXTURES; ++i)
    {
        if (!textures[i].initialized)
        {
            ret = &(textures[i]);
            break;
        }
    }
    if (!ret) {
        DEBUG_PRINTF("ERROR: Failed to allocate texture\n");
        return ret;
    }

    // Create and load texture
    glGenTextures(1, &ret->id);
    glBindTexture(GL_TEXTURE_2D, ret->id);

    // init textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    glGenerateMipmap(GL_TEXTURE_2D);    // we need to do this, even though we aren't using mipmaps

    ret->width = width;
    ret->height = height;

    ret->initialized = true;

    return ret;
}

void rendering_replace_texture(RenderTexture _tex, void* image_data)
{
    Texture *tex = (Texture *)_tex;

    glBindTexture(GL_TEXTURE_2D, tex->id);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->width, tex->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    glGenerateMipmap(GL_TEXTURE_2D);    // we need to do this, even though we aren't using mipmaps
}

static void init_rect(Rect* rect)
{
    // Create and bind VAO
    glGenVertexArrays(1, &rect->VAO);
    glBindVertexArray(rect->VAO);
    {
        // Vertex position array
        glGenBuffers(1, &rect->vertices_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, rect->vertices_VBO);
        GLfloat vertices[12] = {0};
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);

        // vertex data interpretation - applied to currently bound VBO
        // 1st arg is the vertex attribute location we want to configure (0), we specify this in the shader with layout(location = 0)
        // 2nd arg is the number of components per vertex
        // 3rd arg is the type of each element
        // 4th arg is whether data should be normalized (mapped into range [-1, 1] or [0, 1] for unsigned)
        // 5th arg is the stride
        // 6th arg is a (byte) offset of where to start in the array

        static const int VERT_POS_LOCATION = 0;
        glVertexAttribPointer(VERT_POS_LOCATION, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(VERT_POS_LOCATION); // enable it, giving the vertex attribute location

        // Texture coords
        // TODO make tex coords efficient - only change when needed or something, or store all coordinates up front, and index them in the shader based on curr frame
        glGenBuffers(1, &rect->texture_coords_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, rect->texture_coords_VBO);
        static const int TEX_COORDS_LOCATION = 1;
        glVertexAttribPointer(TEX_COORDS_LOCATION, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(TEX_COORDS_LOCATION);
        //default to be overwritten at runtime if needed
        glBufferData(GL_ARRAY_BUFFER, RECT_TEX_COORDS_SIZE, RECT_DEFAULT_TEX_COORDS, GL_STREAM_DRAW);

        // Index array
        glGenBuffers(1, &rect->EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rect->EBO);
        const GLuint indices[2 * 3] = {0, 1, 2,       // 2 triangles in CCW order; note these correspond to vertices[] above
                                       0, 2, 3};
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    }
    // unbind the VAO
    DEBUG_ASSERT(rect->VAO != 0);
    glBindVertexArray(0);
}

static void init_circle(Circle* circle)
{
    // Create and bind VAO
    glGenVertexArrays(1, &circle->VAO);
    glBindVertexArray(circle->VAO);
    {
        // Vertex position array
        glGenBuffers(1, &circle->vertices_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, circle->vertices_VBO);
        GLfloat vertices[(NUM_CIRCLE_POINTS + 1) * 3] = {0};
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);

        // vertex data interpretation - applied to currently bound VBO
        // 1st arg is the vertex attribute location we want to configure (0), we specify this in the shader with layout(location = 0)
        // 2nd arg is the number of components per vertex
        // 3rd arg is the type of each element
        // 4th arg is whether data should be normalized (mapped into range [-1, 1] or [0, 1] for unsigned)
        // 5th arg is the stride
        // 6th arg is a (byte) offset of where to start in the array

        static const int VERT_POS_LOCATION = 0;
        glVertexAttribPointer(VERT_POS_LOCATION, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(VERT_POS_LOCATION); // enable it, giving the vertex attribute location

        // Index array
        glGenBuffers(1, &circle->EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, circle->EBO);
        // sequence of triangles in ccw order
        GLuint indices[NUM_CIRCLE_POINTS * 3] = {};
        for (int i = 0; i < NUM_CIRCLE_POINTS; ++i)
        {
            int start_i = i * 3;
            // center
            indices[start_i + 0] = NUM_CIRCLE_POINTS;
            // current point
            indices[start_i + 1] = i;
            // next point, wrapping back to first
            indices[start_i + 2] = (i + 1) % NUM_CIRCLE_POINTS;
        }
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    }
    // unbind the VAO
    DEBUG_ASSERT(circle->VAO != 0);
    glBindVertexArray(0);
}

static void init_line(Line* line)
{
    // Create and bind VAO
    glGenVertexArrays(1, &line->VAO);
    glBindVertexArray(line->VAO);
    {
        // Vertex position array
        glGenBuffers(1, &line->vertices_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, line->vertices_VBO);
        GLfloat vertices[6] = {0};
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);
        static const int VERT_POS_LOCATION = 0;
        glVertexAttribPointer(VERT_POS_LOCATION, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(VERT_POS_LOCATION); // enable it, giving the vertex attribute location

        // Index array
        glGenBuffers(1, &line->EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, line->EBO);
        const GLuint indices[2] = {0, 1};
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    }
    // unbind the VAO
    DEBUG_ASSERT(line->VAO != 0);
    glBindVertexArray(0);
}

void rendering_init(GameMemory* game_memory, GameRenderInfo* render_info, float width, float height)
{
    // Load OpenGL extensions with GLAD
    if (!gladLoadGLLoader((GLADloadproc)game_memory->platform_gl_get_proc_address))
    {
        FATAL_PRINTF("Failed to initialize GLAD");
    }

    // specify top as the height the origin is in the bottom left
    projection = Mat4::ortho(
        -1.0F,
        1.0F,
        -1.0F,
        1.0F,
        -1.0F, 1.0F);

    view = Mat4::identity();

    load_shader(game_memory, &sprite_shader, "sprite");
    
    // create 1x1 white texture for default/untextured quads
    unsigned char buf[4] = {255, 255, 255, 255};
    empty_texture = (Texture*)rendering_create_texture(buf, 1, 1);
    if (!empty_texture)
    {
        FATAL_PRINTF("Failed to create empty texture!");
    }

    init_rect(&global_rect);
    init_circle(&global_circle);
    init_line(&global_line);
}

void rendering_clear_screen(GameRenderInfo* render_info, Color color)
{
    if (render_info->resized)
    {
        float w_width = (float)render_info->window_width;
        float w_height = (float)render_info->window_height;
        float aspect = w_width / w_height;
        if (aspect > GAME_ASPECT)
        {
            gl_viewport_height = render_info->window_height;
            gl_viewport_width = render_info->window_height * GAME_ASPECT;
            gl_viewport_x = w_width/2.0F - gl_viewport_width/2.0F;
            gl_viewport_y = 0.0F;
        }
        else
        {
            gl_viewport_width = render_info->window_width;
            gl_viewport_height = render_info->window_width / GAME_ASPECT;
            gl_viewport_x = 0.0F;
            gl_viewport_y = w_height/2.0F - gl_viewport_height/2.0F;;
        }
        glViewport(gl_viewport_x, gl_viewport_y, gl_viewport_width, gl_viewport_height);
        render_info->resized = false;
        DEBUG_PRINTF("Resizing viewport (%d, %d)\n", gl_viewport_width, gl_viewport_height);
    }
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

Vec2 rendering_window_pos_to_viewport_pos(int x, int y)
{
    float rescale_factor = 2.0F / (float)gl_viewport_width; // window pixels to game units
    Vec2 rescale_offset = Vec2{-1.0F, -1.0F};

    return Vec2((float)(x - gl_viewport_x), (float)(y - gl_viewport_y)) * rescale_factor + rescale_offset;
}

void rendering_set_camera(Vec2 pos)
{
    view = Mat4::identity().frame_translate(Vec3(pos * (-1.0F), 0.0F));
}

void draw_rect(Vec2 pos, f32 rot, Vec2 size, Texture* tex, GLfloat* tex_coords, Color color, bool wireframe)
{
    Rect* rect = &global_rect;
    Shader* shader = &sprite_shader;

    GLfloat width_2 = size.x / 2.0F;
    GLfloat height_2 = size.y / 2.0F;
    GLfloat vertices[12] = {
        // top right
        width_2,    height_2,   0.0F,
        // top left
        -width_2,   height_2,   0.0F,
        // bottom left
        -width_2,  -height_2,   0.0F,
        // bottom right
        width_2,   -height_2,   0.0F,
    };

    glBindVertexArray(rect->VAO);
    {
        // Set vertices
        glBindBuffer(GL_ARRAY_BUFFER, rect->vertices_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);

        // Set the texture coords
        glBindBuffer(GL_ARRAY_BUFFER, rect->texture_coords_VBO);
        glBufferData(GL_ARRAY_BUFFER, RECT_TEX_COORDS_SIZE, tex_coords, GL_STREAM_DRAW);

        // Set current shader program
        glUseProgram(shader->id);

        // Set projection matrix uniform
        int loc = glGetUniformLocation(shader->id, "projection");
        glUniformMatrix4fv(loc, 1, GL_FALSE, projection.data);

        // Set view matrix uniform
        loc = glGetUniformLocation(shader->id, "view");
        glUniformMatrix4fv(loc, 1, GL_FALSE, view.data);

        loc = glGetUniformLocation(shader->id, "model");

        Mat4 model = Mat4::identity().frame_translate(Vec3(pos, 0.0)).frame_rotate_z(rot);
        glUniformMatrix4fv(loc, 1, GL_FALSE, model.data);

        // Set texture uniform
        // NOTE for a single texture active texture unit is 0 by default, and uniform is set to that texure unit
        // each sampler needs a different texture unit
        glActiveTexture(GL_TEXTURE0);   // texture unit 0
        glBindTexture(GL_TEXTURE_2D, tex->id);   // bind to texture unit 0
        loc = glGetUniformLocation(shader->id, "sprite_texture");
        glUniform1i(loc, GL_TEXTURE0); // put whatevers in texture unit 0 into the uniform sampler

        // color overlay
        loc = glGetUniformLocation(shader->id, "color_blend");
        glUniform4f(loc, color.r, color.g, color.b, color.a);

        // draw wireframe
        if (wireframe)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        // draw
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
    // unbind VAO
    glBindVertexArray(0);
}

void rendering_draw_rect(Vec2 pos, f32 rot, Vec2 size, RenderTexture tex, Color color, bool wireframe)
{
    Texture* texture = (Texture*)tex;
    if (!texture)
    {
        texture = empty_texture;
    }
    else
    {
        DEBUG_ASSERT(texture->initialized);
    }
    draw_rect(pos, rot, size, texture, (GLfloat*)RECT_DEFAULT_TEX_COORDS, color, wireframe);
}

void rendering_draw_sprite(Vec2 pos, Vec2 size, RenderTexture tex, uint32_t row, uint32_t col, Color color, bool hflip)
{
    Texture* texture = (Texture*)tex;
    DEBUG_ASSERT(texture && texture->initialized);

    // Compute texture coordinates
    // combine vertices and texture coords into one buffer

    float sprite_frac_width = (float)size.x / (float)texture->width;
    float sprite_frac_height = (float)size.y / (float)texture->height;

    // bottom left origin (fractions of total image width and height)
    float bottom_left_x = col * sprite_frac_width;
    // jank because we store animations indexed from the top-left, but access the texture in openGL from the bottom-left
    float bottom_left_y = 1.0F - (sprite_frac_height * (row + 1));

    GLfloat texture_coords[RECT_TEX_COORDS_ARRAY_SIZE] = {
        // top right
        hflip ? bottom_left_x : bottom_left_x + sprite_frac_width,
        bottom_left_y + sprite_frac_height,
        // top left
        hflip ? bottom_left_x + sprite_frac_width : bottom_left_x,
        bottom_left_y + sprite_frac_height,
        // bottom left
        hflip ? bottom_left_x + sprite_frac_width : bottom_left_x,
        bottom_left_y,
        // bottom right
        hflip ? bottom_left_x : bottom_left_x + sprite_frac_width,
        bottom_left_y
    };

    draw_rect(pos, 0, size, texture, texture_coords, color, false);
}

void rendering_draw_line(Vec2 origin, Vec2 point, float width, Color color)
{
    Line* line = &global_line;
    Shader* shader = &sprite_shader;

    GLfloat vertices[6] = {
        0.0F, 0.0F, 0.0F,
        point.x, point.y, 0.0F,
    };

    glBindVertexArray(line->VAO);
    {
        // Set vertices
        glBindBuffer(GL_ARRAY_BUFFER, line->vertices_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);

        // Set current shader program
        glUseProgram(shader->id);

        // Set projection matrix uniform
        int loc = glGetUniformLocation(shader->id, "projection");
        glUniformMatrix4fv(loc, 1, GL_FALSE, projection.data);

        // Set view matrix uniform
        loc = glGetUniformLocation(shader->id, "view");
        glUniformMatrix4fv(loc, 1, GL_FALSE, view.data);

        Mat4 model = Mat4::identity().frame_translate(Vec3(origin, 0.0));
        loc = glGetUniformLocation(shader->id, "model");
        glUniformMatrix4fv(loc, 1, GL_FALSE, model.data);

        // Set texture uniform
        // NOTE for a single texture active texture unit is 0 by default, and uniform is set to that texure unit
        // each sampler needs a different texture unit
        glActiveTexture(GL_TEXTURE0);   // texture unit 0
        glBindTexture(GL_TEXTURE_2D, empty_texture->id);   // bind to texture unit 0
        loc = glGetUniformLocation(shader->id, "sprite_texture");
        glUniform1i(loc, GL_TEXTURE0); // put whatevers in texture unit 0 into the uniform sampler

        // color overlay
        loc = glGetUniformLocation(shader->id, "color_blend");
        glUniform4f(loc, color.r, color.g, color.b, color.a);

        // draw
        glLineWidth(width);
        glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, 0);
    }
    // unbind VAO
    glBindVertexArray(0);
}

void rendering_draw_circle(Vec2 pos, f32 rot, f32 radius, Color color, bool wireframe)
{
    Circle* circle = &global_circle;
    Shader* shader = &sprite_shader;

    // for each point, we need 3 vertices
    GLfloat vertices[(NUM_CIRCLE_POINTS + 1) * 3] = {};
    for (int i = 0; i < NUM_CIRCLE_POINTS; i++)
    {
        int start_i = i * 3;
        f32 rads = 2.0F*M_PI*(f32)i/(f32)NUM_CIRCLE_POINTS;
        vertices[start_i + 0] = cosf(rads)*radius; // x
        vertices[start_i + 1] = sinf(rads)*radius; // y
        vertices[start_i + 2] = 0; // z
    }
    // center is 0
    vertices[(NUM_CIRCLE_POINTS * 3) + 0] = 0;
    vertices[(NUM_CIRCLE_POINTS * 3) + 1] = 0;
    vertices[(NUM_CIRCLE_POINTS * 3) + 2] = 0;

    glBindVertexArray(circle->VAO);
    {
        // Set vertices
        glBindBuffer(GL_ARRAY_BUFFER, circle->vertices_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);

        // Set current shader program
        glUseProgram(shader->id);

        // Set projection matrix uniform
        int loc = glGetUniformLocation(shader->id, "projection");
        glUniformMatrix4fv(loc, 1, GL_FALSE, projection.data);

        // Set view matrix uniform
        loc = glGetUniformLocation(shader->id, "view");
        glUniformMatrix4fv(loc, 1, GL_FALSE, view.data);

        loc = glGetUniformLocation(shader->id, "model");

        Mat4 model = Mat4::identity().frame_translate(Vec3(pos, 0.0)).frame_rotate_z(rot);
        glUniformMatrix4fv(loc, 1, GL_FALSE, model.data);

        // color overlay
        loc = glGetUniformLocation(shader->id, "color_blend");
        glUniform4f(loc, color.r, color.g, color.b, color.a);

        // draw wireframe
        if (wireframe)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        // draw
        glDrawElements(GL_TRIANGLES, NUM_CIRCLE_POINTS * 3, GL_UNSIGNED_INT, 0);
    }
    // unbind VAO
    glBindVertexArray(0);
}