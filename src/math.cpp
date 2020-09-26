// TODO remove
#include<random>

#include"game_math.h"

int clamp(int val, int lo, int hi)
{
    if (val < lo) return lo;
    if (val > hi) return hi;
    return val;
}

float clamp(float val, float lo, float hi)
{
    if (val < lo) return lo;
    if (val > hi) return hi;
    return val;
}

PerlinNoiseGenerator::PerlinNoiseGenerator(u32 seed)
{
    int i, j;

    std::mt19937 generator(seed); 
    /* std::uniform_real_distribution<f32> real_distribution(-1.0, 1.0); */

    for (i = 0; i < 256; ++i)
    {
        /*
        f32 grad[3];
        f32 len;
        for (int j = 0; j < 3; ++j)
            grad[0] = real_distribution(generator);

        len = sqrtf(grad[0]*grad[0] + grad[1]*grad[1] + grad[2]*grad[2]);

        x_grad[i] = grad[0]/len;
        y_grad[i] = grad[1]/len;
        z_grad[i] = grad[2]/len;
        */

        p[i] = i;
    }

    /* permute the sequence */
    for (i = 0; i < 255; ++i)
    {
        std::uniform_int_distribution<int> int_distribution(i, 255); /* inclusive */
        j = int_distribution(generator);
        int tmp = p[i];
        p[i] = p[j];
        p[j] = tmp;
        p[i + 256] = p[i]; // copy array over itself
    }
}

/* take lower 4 bytes of hash to determine which gradient to dot with */
/* */
f32 grad(int hash, f32 x, f32 y, f32 z)
{
    switch(hash & 0xF)
    {
        case 0x0: return  x + y;
        case 0x1: return -x + y;
        case 0x2: return  x - y;
        case 0x3: return -x - y;
        case 0x4: return  x + z;
        case 0x5: return -x + z;
        case 0x6: return  x - z;
        case 0x7: return -x - z;
        case 0x8: return  y + z;
        case 0x9: return -y + z;
        case 0xA: return  y - z;
        case 0xB: return -y - z;
        case 0xC: return  y + x; // same as x + y...
        case 0xD: return -y + z; // ^ so these are more likely...?
        case 0xE: return  y - x; // ^
        case 0xF: return -y - z; // ^
        default: return 0; // never happens
    }
}

f32 fade(f32 t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

int inc(u8 num) {
    num++;
    num &= 255;
    return num;
}

f32 lerp(f32 a, f32 b, f32 t)
{
    return a + (b - a) * t;
}

f32 PerlinNoiseGenerator::noise(f32 x, f32 y, f32 z)
{
    // do we need this? I think mod doesn't work properly with < 0
    if (x < 0) x = -x;
    if (y < 0) y = -y;
    if (z < 0) z = -z;

    // unit cube coords
    u8 x_i = (u32)x & 255;
    u8 y_i = (u32)y & 255;
    u8 z_i = (u32)z & 255;
    // offset into unit cube
    f32 x_f = x-(u32)x;
    f32 y_f = y-(u32)y;
    f32 z_f = z-(u32)z;

    // these remap the offset onto a smoother function, so it's flatter near the edge/corner
    f32 u = fade(x_f);
    f32 v = fade(y_f);
    f32 w = fade(z_f);
    
    /* indices of gradients for corners, 'hashed' into p array, which can overflow because it's double in size */
    int aaa, aba, aab, abb, baa, bba, bab, bbb;
    aaa = p[p[p[    x_i ]+    y_i ]+    z_i ]; // hash by following x, y, z
    aba = p[p[p[    x_i ]+inc(y_i)]+    z_i ]; // inc to get 1 offset somewhere in chain
    aab = p[p[p[    x_i ]+    y_i ]+inc(z_i)]; // note if z_i == y_i, then these two  ^ are the same.
    abb = p[p[p[    x_i ]+inc(y_i)]+inc(z_i)];
    baa = p[p[p[inc(x_i)]+    y_i ]+    z_i ];
    bba = p[p[p[inc(x_i)]+inc(y_i)]+    z_i ];
    bab = p[p[p[inc(x_i)]+    y_i ]+inc(z_i)];
    bbb = p[p[p[inc(x_i)]+inc(y_i)]+inc(z_i)];

    f32 p_x0y0z0 = grad(aaa, x_f, y_f, z_f);    /* coord is point minus grid corner */
    f32 p_x0y1z0 = grad(aba, x_f, y_f - 1, z_f);
    f32 p_x0y0z1 = grad(aab, x_f, y_f, z_f - 1);
    f32 p_x0y1z1 = grad(abb, x_f, y_f - 1, z_f - 1);
    f32 p_x1y0z0 = grad(baa, x_f - 1, y_f, z_f);
    f32 p_x1y1z0 = grad(bba, x_f - 1, y_f - 1, z_f);
    f32 p_x1y0z1 = grad(bab, x_f - 1, y_f, z_f - 1);
    f32 p_x1y1z1 = grad(bbb, x_f - 1, y_f - 1, z_f - 1);

    f32 l_x1, l_x2, l_y1, l_y2;
    l_x1 = lerp(p_x0y0z0, p_x1y0z0, u); // x axis
    l_x2 = lerp(p_x0y1z0, p_x1y1z0, u);
    l_y1 = lerp(l_x1, l_x2, v); // y axis
    l_x1 = lerp(p_x0y0z1, p_x1y0z1, u);
    l_x2 = lerp(p_x0y1z1, p_x1y1z1, u);
    l_y2 = lerp(l_x1, l_x2, v); // y axis
    return (lerp(l_y1, l_y2, w) + 1.0F)/2.0F; // z axis, and map to 0..1
}

f32 PerlinNoiseGenerator::octave_noise(f32 x, f32 y, f32 z, u8 octaves, f32 persistence)
{
    f32 ret = 0.0F;
    f32 freq = 1.0F;
    f32 amp = 1.0F;
    f32 max_val = 0.0F;
    for (int i = 0; i < octaves; ++i)
    {
        ret += noise(x * freq, y * freq, z * freq) * amp;
        max_val += amp;
        amp *= persistence;
        freq *= 2;
    }
    return ret/max_val;
}