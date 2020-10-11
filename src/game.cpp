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
Color aabb_color = Color{1.0F,0.0F,0.0F,1.0F};

#define GRID_SPACING 0.1F

bool AABB::intersects(AABB other)
{
    if (this->min.x > other.max.x || this->max.x < other.min.x)
    {
        return false;
    }
    if (this->min.y > other.max.y || this->max.y < other.min.y)
    {
        return false;
    }
    return true;
}

void AABB::draw(bool intersecting)
{
    Color color = aabb_color;
    if (!intersecting)
    {
        color.r = 0.7F;
    }

    Vec2 extent = this->max - this->min;
    rendering_draw_line(
                    this->min,
                    Vec2(extent.x, 0),
                    1,
                    color);
    rendering_draw_line(
                    this->min,
                    Vec2(0, extent.y),
                    1,
                    color);
    rendering_draw_line(
                    this->max,
                    Vec2(-extent.x, 0),
                    1,
                    color);
    rendering_draw_line(
                    this->max,
                    Vec2(0, -extent.y),
                    1,
                    color);
}

void get_rect_verts(Obj *rect, Vec2 *ret)
{
    f32 width_2 = rect->width / 2.0F;
    f32 height_2 = rect->height / 2.0F;
    Vec2 local_verts[] = {
                Vec2(width_2, height_2),
                Vec2(-width_2, height_2),
                Vec2(-width_2, -height_2),
                Vec2(width_2, -height_2),
            };

    Mat4 model = Mat4::identity().frame_translate(Vec3(rect->pos, 0.0)).frame_rotate_z(rect->rot);
    for (int i = 0; i < 4; ++i)
    {
        Vec4 world_vert = model * Vec4(local_verts[i], 0, 1);
        ret[i] = Vec2(world_vert.x, world_vert.y);
    }
}

void Obj::update_aabb()
{
    switch(this->shape)
    {
        case Obj::Circle:
            this->aabb.min = this->pos - Vec2(this->radius, this->radius);
            this->aabb.max = this->pos + Vec2(this->radius, this->radius);
            break;
        case Obj::Rect:
        {
            Vec2 rect_verts[4];
            get_rect_verts(this, rect_verts);

            this->aabb.min = this->pos;
            this->aabb.max = this->pos;
            for (int i = 0; i < 4; ++i)
            {
                Vec2 *point = &rect_verts[i];
                this->aabb.min.x = MIN(point->x, this->aabb.min.x);
                this->aabb.min.y = MIN(point->y, this->aabb.min.y);
                this->aabb.max.x = MAX(point->x, this->aabb.max.x);
                this->aabb.max.y = MAX(point->y, this->aabb.max.y);
            }
            break;
        }
    }
}

bool polys_colliding_sat(Obj *objs[2], Vec2 *verts[2], u32 num_verts[2], Collision *collision)
{
    /* _e = poly whose edges we're checking, v = poly whose verts we're checking */
    Obj *obj_e = objs[0];
    Obj *obj_v = objs[1];
    Vec2 *verts_e = verts[0];
    Vec2 *verts_v = verts[1];
    u32 num_verts_e = num_verts[0];
    u32 num_verts_v = num_verts[1];
    f32 closest_proj = -99.0F; /* TODO how to initialize this better */
    Collision tmp_collision;
    for (u32 i = 0; i < 2; ++i)
    {
        /* Swap; we need to check both sets of edges */
        if (i) {
            verts_e = verts[1];
            verts_v = verts[0];
            num_verts_e = num_verts[1];
            num_verts_v = num_verts[0];
            obj_e = objs[1];
            obj_v = objs[0];
        }
        for (u32 j = 0; j < num_verts_e; ++j)
        {
            Vec2 edge = verts_e[(j+1) % num_verts_e] - verts_e[j];
            /* normal to the edge */
            Vec2 n = Vec2(edge.y, -edge.x).normalized(); /* edge.rotate(-M_PI/2.0F); */
            u32 num_verts_in_front = 0;
            for (u32 k = 0; k < num_verts_v; ++k)
            {
                /* first point on this edge to vertex 'k' on other poly */
                Vec2 v = verts_v[k] - verts_e[j];
                f32 proj_dist = n.dot(v);
                if (proj_dist > 0.0F)
                {
                    num_verts_in_front++;
                }
                else if (proj_dist > closest_proj)
                {
                    /* additional check to see if this point is really 'behind' the edge */
                    Vec2 v2 = verts_v[k] - verts_e[(j+1) % num_verts_e];
                    if (edge.dot(v) > 0.0F && (edge * -1.0F).dot(v2) > 0.0F)
                    {
                        closest_proj = proj_dist;
                        tmp_collision.objs[0] = obj_e;
                        tmp_collision.objs[1] = obj_v;
                        tmp_collision.points[0] = verts_v[k] + n * proj_dist * -1.0F;
                        tmp_collision.points[1] = verts_v[k];
                        tmp_collision.normal = n;
                    }
                }
            }
            if (num_verts_in_front == num_verts_v)
            {
                return false;
            }
        }
    }
    memcpy(collision, &tmp_collision, sizeof(tmp_collision));

    return true;
}

bool get_collision(Obj **pair, Collision *collision)
{
    Obj *obj_pair[2] = {pair[0], pair[1]};
    /* Order by shape, i.e. swap if circle is first in the pair */
    if (obj_pair[1]->shape == Obj::Rect)
    {
        Obj *tmp = obj_pair[0];
        obj_pair[0] = obj_pair[1];
        obj_pair[1] = tmp;
    }

    /* Now we have 3 cases: circle/circle, rect/circle, rect/rect */
    Vec2 obj2obj = obj_pair[1]->pos - obj_pair[0]->pos;

    if (obj_pair[0]->shape == Obj::Rect)
    {

        Vec2 verts[2][4];
        get_rect_verts(obj_pair[0], verts[0]);

        /* Rect/Rect */
        if (obj_pair[1]->shape == Obj::Rect)
        {
            get_rect_verts(obj_pair[1], verts[1]);
            u32 _num_verts[2] = {4,4};
            Vec2 *_verts[2] = {verts[0], verts[1]};
            return polys_colliding_sat(obj_pair, _verts, _num_verts, collision);
        }
        /* Rect/Circle */
        /* Strategy (generalizes to any convex polygon):
            * 1. Check no vertices are inside circle       (simple)
            * 2. Check no edges intersect circle           (find closest point to circle on line, check if it's in circle)
            * 3. (TODO) Check circle center not inside rectangle  (point in polygon)
            * Alternative (only rectangles):
            * 1. Rotate rect and circle so rect is axis aligned for each v, v.rotate(-rect->rot)
            * 2. Check no vertices are inside circle
            * 3. If circle intersects on an axis with rect, check the center is further than the radius
            * SAT strategy - should probably do this:
            * 1. Check axes on edges of polygon
            * 2. Check axes from each vertex of polygon to center of circle
            */
        Obj *rect = obj_pair[0];
        Obj *circle = obj_pair[1];
        Vec2 * rect_verts = verts[0];
        /* (TODO) Check circle center not inside rectangle here */
        for (int j = 0; j < 4; ++j)
        {
            Vec2 v2circle = circle->pos - rect_verts[j];
            /* Vert in circle */
            /* TODO
                * In this case the circle is also colliding with an edge, we should use that instead
                * UNLESS the circle completely covers the rectangle...then we do something random
                * or use more continuous or sweep-y methods
                */
            if (v2circle.length() < circle->radius)
            {
                Vec2 coll_normal = (circle->pos - rect_verts[j]).normalized();
                collision->objs[0] = rect;
                collision->objs[1] = circle;
                collision->points[0] = rect_verts[j];
                collision->points[1] = circle->pos - coll_normal * circle->radius;
                collision->normal = coll_normal;
                return true;
            }
            Vec2 edge = rect_verts[(j+1) % 4] - rect_verts[j];
            /* Point on line closest to circle */
            Vec2 p = edge.normalized() * (edge.dot(v2circle) / edge.length());
            /* Check if point lies between the verts, by checking its direction and length */
            f32 edotp = edge.dot(p);
            if (edotp < 0.0F || edotp > edge.dot(edge))
            {
                continue;
            }
            /* Check if point in circle */
            Vec2 p2circle = v2circle - p;
            if (p2circle.length() < circle->radius)
            {
                Vec2 p_point = rect_verts[j] + p;
                Vec2 coll_normal = (circle->pos - p_point).normalized();
                collision->objs[0] = rect;
                collision->objs[1] = circle;
                collision->points[0] = p_point;
                collision->points[1] = circle->pos - coll_normal * circle->radius;
                collision->normal = coll_normal;
                return true;
            }
        }
        return false;
    }
    /* Circle/Circle */
    if (obj2obj.length() < (obj_pair[0]->radius + obj_pair[1]->radius))
    {
        Vec2 coll_normal = (obj_pair[1]->pos - obj_pair[0]->pos).normalized();
        collision->objs[0] = obj_pair[0];
        collision->objs[1] = obj_pair[1];
        collision->points[0] = obj_pair[0]->pos + coll_normal * obj_pair[0]->radius;
        collision->points[1] = obj_pair[1]->pos - coll_normal * obj_pair[1]->radius;
        collision->normal = coll_normal;
        return true;
    }
    return false;
}

void integrate_vel_alpha(Obj *obj, f32 dt)
{
    obj->vel = obj->vel + ((obj->force / obj->mass) * dt);
    obj->alpha = obj->alpha + ((obj->torque / obj->inertia) * dt);
}

void integrate_pos_rot(Obj *obj, f32 dt)
{
    obj->pos = obj->pos + (obj->vel * dt);
    obj->rot = obj->rot + obj->alpha * dt;
}

void game_update_and_render(GameMemory* game_memory, GameInputBuffer* input_buffer, GameRenderInfo* render_info)
{
    GameMemoryBlock* block = (GameMemoryBlock*)(game_memory->memory);
    GameState* game_state = &block->game_state;

    f32 dt = input_buffer->dt;

    game_state->camera_pos = Vec2();

    GameInput *last_input = input_buffer->last_input();
    
    bool mouse_force_on = false;
    Vec2 mouse_pos = rendering_window_pos_to_viewport_pos(last_input->mouse_x, last_input->mouse_y);
    bool mouse_released = false;
    if (last_input->mouse_left_down)
    {
        if (!game_state->mouse_dragging)
        {
            game_state->mouse_dragging = true;
            game_state->mouse_force_origin = mouse_pos;
        }
    }
    else
    {
        if (game_state->mouse_dragging)
        {
            game_state->mouse_dragging = false;
            mouse_released = true;
        }
    }
    if (last_input->space && !input_buffer->prev_frame_input(1)->space)
    {
        game_state->mouse_dragging = false;
        mouse_released = true;
        mouse_pos = Vec2(-0.5736842155456543F, 0.5763158798217773F);
        game_state->mouse_force_origin = Vec2(0.9710527658462524F, -0.9736841917037964F);
    }

    /* frame advance */
    if (last_input->p && !input_buffer->prev_frame_input(1)->p)
    {
        game_state->paused = !game_state->paused;
    }
    if (game_state->paused)
    {
        if (last_input->right && !input_buffer->prev_frame_input(1)->right)
        {
        }
        else
        {
            return;
        }
    }

    /* physics - apply forces*/
    Obj *objs = game_state->objs;

    /* slow down */
    dt *= 0.8F;

    for (int i = 0; i < MAX_OBJS; ++i)
    {
        Obj *obj = &objs[i];
        if (!obj->exists || obj->is_static)
            continue;

        /* Compute forces */
        obj->torque = 0.0F;
        obj->force = Vec2{0.0F, 0.0F};
        /* Gravity */
        //obj->force = obj->force + Vec2(0, -9.81F);
        /* Mouse force */
        Vec2 mouse_to_obj = obj->pos - mouse_pos;
        /* TODO this check is hacky, redo */
        f32 radius_mouse_check = obj->shape == Obj::Rect ? MAX(obj->width, obj->height) / 2.0F : obj->radius;
        if (mouse_to_obj.length() < radius_mouse_check)
        {
            mouse_force_on = true;
            if (mouse_released)
            {
                /* scale length on constant factor */
                f32 m_force_scale = 100.0F;
                Vec2 m_force = mouse_pos - game_state->mouse_force_origin;
                DEBUG_PRINTF("mouse_pos = Vec2(%.16fF, %.16fF);\n", mouse_pos.x, mouse_pos.y);
                DEBUG_PRINTF("game_state->mouse_force_origin = Vec2(%.16fF, %.16fF);\n", game_state->mouse_force_origin.x, game_state->mouse_force_origin.y);
                m_force = m_force * m_force_scale;
                Vec2 obj_to_mouse = mouse_to_obj * -1.0F;
                obj->torque = obj->torque + (obj_to_mouse.x * m_force.y - obj_to_mouse.y * m_force.x);
                obj->force = obj->force + m_force;
            }
        }

        integrate_vel_alpha(obj, dt);
        integrate_pos_rot(obj, dt);
    }

    /* physics - collision detection */
    /* broad phase - compute AABBs */
    for (int i = 0; i < MAX_OBJS; ++i)
    {
        Obj *obj = &objs[i];
        if (!obj->exists || obj->is_static)
            continue;
        obj->update_aabb();
    }
    /* broad phase - produce pairs of potentially colliding objects */
    /* brute forceee */
    u32 p_coll_num = 0;
    for (int i = 0; i < MAX_OBJS; ++i)
    {
        Obj *objA = &objs[i];
        if (!objA->exists)
            continue;
        for (int j = i + 1; j < MAX_OBJS; ++j)
        {
            Obj *objB = &objs[j];
            if (!objB->exists)
                continue;
            if (objA->is_static && objB->is_static)
                continue;
            if (objA->aabb.intersects(objB->aabb))
            {
                game_state->p_coll_pairs[p_coll_num][0] = objA;
                game_state->p_coll_pairs[p_coll_num][1] = objB;
                p_coll_num++;
            }
        }
    }
    /* narrow phase - produce pairs of colliding objects */
    u32 coll_num = 0;
    Collision *collision = &game_state->collisions[0];
    for (u32 i = 0; i < p_coll_num; ++i)
    {
        Obj **obj_pair = game_state->p_coll_pairs[i];

        if (!get_collision(obj_pair, collision))
            continue;

        f32 abs_dt = dt;
        f32 curr_dt = dt/2.0F;
        f32 threshold = dt/16.0F;
        while (curr_dt > threshold) {
            do
            {
                abs_dt -= curr_dt;
                if (!obj_pair[0]->is_static)
                    integrate_pos_rot(obj_pair[0], -curr_dt);
                if (!obj_pair[1]->is_static)
                    integrate_pos_rot(obj_pair[1], -curr_dt);

                if (curr_dt > threshold)
                    curr_dt /= 2.0F;
            } while ((abs_dt - curr_dt) >= 0.0F && get_collision(obj_pair, collision));

            // TODO compute dist between collision points; this is probably not a good way
            if (curr_dt < threshold)
            {
                break;
            }

            do
            {
                abs_dt += curr_dt;
                if (!obj_pair[0]->is_static)
                    integrate_pos_rot(obj_pair[0], curr_dt);
                if (!obj_pair[1]->is_static)
                    integrate_pos_rot(obj_pair[1], curr_dt);

                if (curr_dt > threshold)
                    curr_dt /= 2.0F;
            } while ((abs_dt + curr_dt) <= dt && !get_collision(obj_pair, collision));
        }
        coll_num++;
        collision = &game_state->collisions[coll_num];
    }

    for (u32 i = 0; i < coll_num; ++i)
    {
        collision = &game_state->collisions[i];
        Obj **coll_objs = collision->objs;
        Vec2 *points = collision->points;
        Vec2 rs[2] = {
            points[0] - coll_objs[0]->pos,
            points[1] - coll_objs[1]->pos
        };
        f32 mass_recip[2] = {
            coll_objs[0]->is_static ? 0.0F : 1.0F / coll_objs[0]->mass,
            coll_objs[1]->is_static ? 0.0F : 1.0F / coll_objs[1]->mass
        };
        Vec2 tangential_vels[2] = {
            Vec3(0, 0, coll_objs[0]->alpha).cross(Vec3(rs[0], 0)).xy(),
            Vec3(0, 0, coll_objs[1]->alpha).cross(Vec3(rs[1], 0)).xy()
        };
        /* Velocities at points of collision */
        Vec2 point_vels[2] = {
            coll_objs[0]->vel + tangential_vels[0],
            coll_objs[1]->vel + tangential_vels[1]
        };
        /* Compute impulse */
        f32 coeff_restitution = 1.0F;
        Vec2 vel_diff = point_vels[0] - point_vels[1];
        f32 J_numerator = -(vel_diff.dot(collision->normal)) * (coeff_restitution + 1.0F);
        Vec3 rs_cross_n_div_I[2] = {
            coll_objs[0]->is_static ? Vec3() : Vec3(rs[0], 0).cross(Vec3(collision->normal, 0)) / coll_objs[0]->inertia,
            coll_objs[1]->is_static ? Vec3() : Vec3(rs[1], 0).cross(Vec3(collision->normal, 0)) / coll_objs[1]->inertia
        };
        Vec2 cross_rs[2] = {
            rs_cross_n_div_I[0].cross(Vec3(rs[0], 0)).xy(),
            rs_cross_n_div_I[1].cross(Vec3(rs[1], 0)).xy()
        };
        f32 J_denominator = mass_recip[0] + mass_recip[1] + collision->normal.dot(cross_rs[0] + cross_rs[1]);
        f32 J_mag = J_numerator / J_denominator;
        Vec2 J = collision->normal * J_mag;
        /* Update vels and alphas */
        if (!coll_objs[0]->is_static)
        {
            coll_objs[0]->vel = coll_objs[0]->vel + J / coll_objs[0]->mass;
            coll_objs[0]->alpha = coll_objs[0]->alpha + (rs[0].x * J.y - rs[0].y * J.x) / coll_objs[0]->inertia;
        }
        if (!coll_objs[1]->is_static)
        {
            coll_objs[1]->vel = coll_objs[1]->vel - J / coll_objs[1]->mass;
            coll_objs[1]->alpha = coll_objs[1]->alpha - (rs[1].x * J.y - rs[1].y * J.x) / coll_objs[1]->inertia;
        }
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

    /* objects */
    for (int i = 0; i < MAX_OBJS; ++i)
    {
        Obj *obj = &game_state->objs[i];
        if (!obj->exists)
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
                    obj->rot,
                    obj->radius,
                    obj_color,
                    obj_wireframe);
                break;
            case Obj::Rect:
                rendering_draw_rect(
                    obj->pos,
                    obj->rot,
                    Vec2(obj->width, obj->height),
                    NULL,
                    obj_color,
                    obj_wireframe);
                break;
        }
    }

    /* draw physics stuff */
    /* actual collisions */
    for (u32 i = 0; i < coll_num; ++i)
    {
        collision = &game_state->collisions[i];
        Color coll_normal_color = Color{0.0F,0.0F,1.0F,1.0F};
        for (u32 j = 0; j < 2; ++j)
        {
            Vec2 normal = collision->normal * (j ? -1.0F : 1.0F) * 0.1F;
            rendering_draw_line(
                    collision->points[j] + normal,
                    normal * -1.0F,
                    2,
                    coll_normal_color);
            rendering_draw_circle(
                    collision->points[j],
                    0,
                    0.01F,
                    coll_normal_color,
                    false);
        }
    }

    /* aabbs */
    /*for (int i = 0; i < MAX_OBJS; ++i)
    {
        Obj *obj = &game_state->objs[i];
        if (!obj->exists)
            continue;
        obj->aabb.draw(false);
    }*/

    /* p coll pairs (AABBs colliding) */
    /*for (u32 i = 0; i < p_coll_num; ++i)
    {
        game_state->p_coll_pairs[i][0]->aabb.draw(true);
        game_state->p_coll_pairs[i][1]->aabb.draw(true);
    }*/

    /* mouse force */
    if (game_state->mouse_dragging)
    {
        rendering_draw_line(
                    game_state->mouse_force_origin,
                    mouse_pos - game_state->mouse_force_origin,
                    2,
                    mouse_force_on ? mouse_force_on_color : mouse_force_off_color);
        rendering_draw_circle(
                    mouse_pos,
                    0,
                    0.01F,
                    mouse_force_on ? mouse_force_on_color : mouse_force_off_color,
                    false);
    }
}

void game_init_memory(GameMemory* game_memory, GameRenderInfo* render_info)
{
    DEBUG_ASSERT(game_memory->memory_size >= sizeof(GameMemoryBlock));

    rendering_init(game_memory, render_info, GAME_WIDTH_PX, GAME_HEIGHT_PX);

    GameMemoryBlock* block = (GameMemoryBlock*)(game_memory->memory);
    GameState* game_state = &block->game_state;

    game_state->objs[0] = Obj::static_rect(2.0F, 0.2F, Vec2( 0.0F, 1.0F), 0);
    game_state->objs[1] = Obj::static_rect(2.0F, 0.2F, Vec2( 0.0F,-1.0F), 0);
    game_state->objs[2] = Obj::static_rect(0.2F, 2.0F, Vec2( 1.0F, 0.0F), 0);
    game_state->objs[3] = Obj::static_rect(0.2F, 2.0F, Vec2(-1.0F, 0.0F), 0);
    game_state->objs[4] = Obj::static_circle(0.2F, Vec2(0.0F,0.0F));
    game_state->objs[5] = Obj::static_rect(0.2F, 0.4F, Vec2(0.0F, 0.4F), 0);
    game_state->objs[6] = Obj::static_rect(1.0F, 0.2F, Vec2(-0.5F, 0.0F), 0);

    game_state->objs[7] = Obj::dyn_circle(0.2F, Vec2(0.5F,0.0F), 1);
    game_state->objs[8] = Obj::dyn_rect(0.3F, 0.2F, Vec2(-0.5F,0.5F), M_PI / 4.0F, 1);
}
