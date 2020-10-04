#ifndef LINEAR_ALGEBRA_H
#include"util.h"
#include"game_math.h"

struct Vec2
{
    float x;
    float y;

    Vec2() : x(0.0F), y(0.0F) {}
    Vec2(float x, float y) : x(x), y(y) {}
    ~Vec2(){}

    Vec2& operator=(const Vec2& v)
    {
        x = v.x;
        y = v.y;
        return *this;
    }

    Vec2 operator+(const Vec2& v)
    {
        return Vec2(x + v.x, y + v.y);
    }
    Vec2 operator-(const Vec2& v)
    {
        return Vec2(x - v.x, y - v.y);
    }
    Vec2 operator*(const float& s)
    {
        return Vec2(x * s, y * s);
    }
    Vec2 operator/(const float& s)
    {
        return Vec2(x / s, y / s);
    }
    inline float length()
    {
        return sqrtf(x * x + y * y);
    }
    Vec2 normalized()
    {
        return Vec2(x, y) / length();
    }
    Vec2 rotate(float radians)
    {
        float cos_r = cosf(radians);
        float sin_r = sinf(radians);
        return Vec2(cos_r * x - sin_r * y,
                    sin_r * x + cos_r * y);
    }
    void debug_print()
    {
        DEBUG_PRINTF("(%f, %f)\n", x, y);
    }
};

struct Vec3
{
    float x;
    float y;
    float z;

    Vec3() : x(0.0F), y(0.0F), z(0.0F) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    Vec3(Vec2 v2, float z) : x(v2.x), y(v2.y), z(z) {}
    ~Vec3(){}

    Vec3 operator+(const Vec3& v)
    {
        return Vec3(x + v.x, y + v.y, z + v.z);
    }
    Vec3 operator-(const Vec3& v)
    {
        return Vec3(x - v.x, y - v.y, z - v.z);
    }
    Vec3 operator*(const float& s)
    {
        return Vec3(x*s, y*s, z*s);
    }
    Vec3 operator/(const float& s)
    {
        return Vec3(x/s, y/s, z/s);
    }
    void debug_print()
    {
        DEBUG_PRINTF("(%f, %f, %f)\n", x, y, z);
    }
};

struct Vec4
{
    float x;
    float y;
    float z;
    float w;

    Vec4() : x(0.0F), y(0.0F), z(0.0F), w(0.0F) {}
    Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    Vec4(Vec2 v2, float z, float w) : x(v2.x), y(v2.y), z(z), w(w) {}
    ~Vec4(){}

    Vec4 operator+(const Vec4& v)
    {
        return Vec4(x + v.x, y + v.y, z + v.z, w + v.w);
    }
    Vec4 operator-(const Vec4& v)
    {
        return Vec4(x - v.x, y - v.y, z - v.z, w - v.w);
    }
    Vec4 operator*(const float& s)
    {
        return Vec4(x*s, y*s, z*s, w*s);
    }
    Vec4 operator/(const float& s)
    {
        return Vec4(x/s, y/s, z/s, w/s);
    }
    void debug_print()
    {
        DEBUG_PRINTF("(%f, %f, %f, %f)\n", x, y, z, w);
    }
};

struct Mat4
{
    static const int SIZE = 4;
    // stored with contiguous vectors layed out next to each other
    // i.e. a.x a.y a.z 0 b.x b.y b.z 0 c.x c.y c.z 0 d.x d.y d.z 1
    float data[SIZE * SIZE];

    Mat4()
    {
        for (int i = 0; i < SIZE * SIZE; ++i)
        {
            this->data[i] = 0.0F;
        }
    }
    Mat4(float data[SIZE*SIZE])
    {
        for (int i = 0; i < SIZE * SIZE; ++i)
        {
            this->data[i] = data[i];
        }
    }

    // replace last column with a vector
    Mat4(float data[SIZE*(SIZE - 1)], Vec4 v)
    {
        for (int i = 0; i < SIZE * (SIZE-1); ++i)
        {
            this->data[i] = data[i];
        }
        this->data[SIZE * (SIZE - 1) + 0] = v.x;
        this->data[SIZE * (SIZE - 1) + 1] = v.y;
        this->data[SIZE * (SIZE - 1) + 2] = v.z;
        this->data[SIZE * (SIZE - 1) + 3] = v.w;
    }
    ~Mat4(){}

    Mat4 operator+(const Mat4& m)
    {
        Mat4 ret(data);
        for (int i = 0; i < SIZE * SIZE; ++i)
        {
            ret.data[i] += m.data[i];
        }
        return ret;
    }

    Mat4 operator-(const Mat4& m)
    {
        Mat4 ret(data);
        for (int i = 0; i < SIZE * SIZE; ++i)
        {
            ret.data[i] -= m.data[i];
        }
        return ret;
    }

    Mat4 operator*(const float& s)
    {
        Mat4 ret(data);
        for (int i = 0; i < SIZE * SIZE; ++i)
        {
            ret.data[i] *= s;
        }
        return ret;
    }

    Mat4 operator/(const float& s)
    {
        Mat4 ret(data);
        for (int i = 0; i < SIZE * SIZE; ++i)
        {
            ret.data[i] /= s;
        }
        return ret;
    }

    Mat4 operator*(const Mat4& m)
    {
        // TODO this faster
        Mat4 ret;
        for (int c = 0; c < SIZE; ++c)
        {
            for (int r = 0; r < SIZE; ++r)
            {
                for (int i = 0; i < SIZE; ++i)
                {
                    ret.data[c * SIZE + r] += (data[i * SIZE + r] * m.data[c * SIZE + i]);
                }
            }
        }
        return ret;
    }

    Vec4 operator*(const Vec4& v)
    {
        // TODO this faster
        float v_data[4] = {v.x, v.y, v.z, v.w};
        Vec4 ret;
        for (int i = 0; i < SIZE; ++i)
        {
            ret.x += (data[i * SIZE + 0] * v_data[i]);
            ret.y += (data[i * SIZE + 1] * v_data[i]);
            ret.z += (data[i * SIZE + 2] * v_data[i]);
            ret.w += (data[i * SIZE + 3] * v_data[i]);
        }
        return ret;
    }

    static Mat4 identity()
    {
        float mat_data[SIZE*SIZE] = {1, 0, 0, 0,
                                     0, 1, 0, 0,
                                     0, 0, 1, 0,
                                     0, 0, 0, 1};
        return Mat4(mat_data);
    }

    static Mat4 ortho(float left, float right, float bottom, float top, float near, float far)
    {
        // the arguments describe the size and position of a 3d box in camera coordinates (near and far, i.e. z axis, is inverted from camera coords)
        // we need a matrix that transforms this box into a 2 unit wide cube centred at the origin (i.e. camera coords to CVV coords)
        // so there's two steps

        // 1. translate to the camera coordinates
        // for this we need the vector from the origin to the centre of the box
        // each coordinate to the centre of the box is c1 + (c2-c1) / 2 which can be written (c1 + c2) / 2
        // also we negate it, because we want to move the coordinates toward the origin, not away
        // note that since near and far are inverted, the box is 'flipped' around the z axis, which will be corrected in the scaling step
        Vec4 v(
            -(left + right) / 2.0F,
            -(bottom + top) / 2.0F,
            -(near + far) / 2.0F,
            1.0F);
        // note the matrix data layout matches opengl's (see Mat4)
        float t_mat_data[4*3] = {1, 0, 0, 0,
                                 0, 1, 0, 0,
                                 0, 0, 1, 0};
        Mat4 translate(t_mat_data, v);

        // 2. scale to a 2x2x2 box
        // Since we translated to the origin first, the box will be centered there
        // Essentially we are scaling each axis to a percentage of the box side length, times 2 (because its 2x2x2, not 1x1x1)
        // i.e. to scale axis a to c2-c1, we have a/(c2-c1) * 2, or a * 2/(c2-c1)
        // Invert the Z scaling to fix the flipping of near and far

        float s_mat_data[4*4] = {2.0F/(right - left), 0,                   0,                  0,
                                 0,                   2.0F/(top - bottom), 0,                  0,
                                 0,                   0,                   -2.0F/(far - near), 0,
                                 0,                   0,                   0,                  1};
        Mat4 scale(s_mat_data);

        return scale * translate;
    }

    // Treating the matrix as a 3D coord frame, return the new coord frame translated
    Mat4 frame_translate(Vec3 v)
    {
        Mat4 ret(data);
        ret.data[12 + 0] += v.x;
        ret.data[12 + 1] += v.y;
        ret.data[12 + 2] += v.z;
        return ret;
    }

    Mat4 frame_scale(f32 factor)
    {
        Mat4 ret(data);
        ret.data[0] *= factor;
        ret.data[5] *= factor;
        ret.data[10] *= factor;
        return ret;
    }

    Mat4 frame_rotate_z(f32 theta)
    {
        float r_mat_data[] = {cosf(theta),  sinf(theta), 0, 0,
                              -sinf(theta), cosf(theta), 0, 0,
                              0,            0,           1, 0,
                              0,            0,           0, 1};
        Mat4 rotation_matrix(r_mat_data);
        return *this * rotation_matrix;
    }

    void debug_print()
    {
        // print in normal looking way..not how data is layed out
        for (int r = 0; r < SIZE; ++r)
        {
            DEBUG_PRINTF("( ");
            for (int c = 0; c < SIZE; ++c)
            {
                DEBUG_PRINTF("%8.4f ", data[r + c * SIZE]);
            }
            DEBUG_PRINTF(")\n");
        }
    }
};

inline float dist(Vec2 a, Vec2 b)
{
    return sqrtf((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

#define LINEAR_ALGEBRA_H
#endif