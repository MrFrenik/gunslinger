
/*================================================================
    * Copyright: 2020 John Jackson
    * GSPhysics: Simple 3D/2D physics engine
    * File: gs_physics.h
    All Rights Reserved
=================================================================*/

#ifndef __GS_PHYSICS_H__
#define __GS_PHYSICS_H__

/*
    USAGE: (IMPORTANT)

    =================================================================================================================

    Before including, define the gunslinger physics implementation like this:

        #define GS_PHYSICS_IMPL

    in EXACTLY ONE C or C++ file that includes this header, BEFORE the
    include, like this:

        #define GS_PHYSICS_IMPL
        #include "gs_physics.h"

    All other files should just #include "gs_physics.h" without the #define.

    MUST include "gs.h" and declare GS_IMPL BEFORE this file, since this file relies on that:

        #define GS_IMPL
        #include "gs.h"

        #define GS_PHYSICS_IMPL
        #include "gs_physics.h"

    Special Thanks for reference: 
        - https://github.com/r-lyeh (for collision algorithms)
        - https://gist.github.com/vurtun/95f088e4889da2474ad1ce82d7911fee (for GJK impl)
        - Randy Gaul (for physics system inspiration)
        - https://github.com/Nightmask3/Physics-Framework
        - https://github.com/IainWinter/IwEngine/tree/master/IwEngine/src/physics
 + 
    ================================================================================================================
*/

/*==== Interface ====*/

/*==== Util ====*/

typedef struct gs_phys_xform_t {
    // Rotation/Scale matrix
    gs_mat3 rs;
    // Inverse Rotation/Scale matrix
    gs_mat3 inv_rs;
    // Rotation
    gs_quat rot;
    // Scale
    gs_vec3 scale;
    // Position
    gs_vec3 pos;
} gs_phys_xform_t;

GS_API_DECL gs_phys_xform_t gs_phys_xform_from_vqs(gs_vqs* vqs);

/*==== Collision Shapes ====*/

typedef struct gs_line_t     {gs_vec3 a, b;                                                  } gs_line_t;
typedef struct gs_aabb_t     {gs_vec3 min, max;                                              } gs_aabb_t;
typedef struct gs_sphere_t   {gs_vec3 c; float r;                                            } gs_sphere_t;
typedef struct gs_plane_t    {gs_vec3 p, n;                                                  } gs_plane_t;
typedef struct gs_capsule_t  {gs_vec3 a, b; float r;                                         } gs_capsule_t;
typedef struct gs_ray_t      {gs_vec3 p, d;                                                  } gs_ray_t;
typedef struct gs_triangle_t {gs_vec3 p0,p1,p2;                                              } gs_triangle_t;
typedef struct gs_poly_t     {gs_vec3* verts; int32_t cnt;                                   } gs_poly_t;
typedef union  gs_frustum_t  {struct {gs_vec4 l, r, t, b, n, f;}; gs_vec4 pl[6]; float v[24];} gs_frustum_t;

typedef struct gs_hit_t {
    int32_t hit;
    union {
        // General case
        float depth;
        // Rays: Penetration (t0) and Extraction (t1) pts. along ray line
        struct {float t0, t1;};
        // GJK only
        struct {int32_t hits, iterations; gs_vec3 p0, p1; float distance2;};
    };
    union {gs_vec3 p; gs_vec3 contact_point;};
    union {gs_vec3 n; gs_vec3 normal;};
} gs_hit_t;

// Constructors with args
#define gs_line(...)     gs_ctor(gs_line_t, __VA_ARGS__)
#define gs_sphere(...)   gs_ctor(gs_sphere_t, __VA_ARGS__)
#define gs_aabb(...)     gs_ctor(gs_aabb_t, __VA_ARGS__)
#define gs_plane(...)    gs_ctor(gs_plane_t, __VA_ARGS__)
#define gs_capsule(...)  gs_ctor(gs_capsule_t, __VA_ARGS__)
#define gs_ray(...)      gs_ctor(gs_ray_t, __VA_ARGS__)
#define gs_triangle(...) gs_ctor(gs_triangle_t, __VA_ARGS__)
#define gs_poly(...)     gs_ctor(gs_poly_t, __VA_ARGS__)
#define gs_frustum(...). gs_ctor(gs_frustum_t, __VA_ARGS__)
#define gs_hit(...)      gs_ctor(gs_hit_t, __VA_ARGS__)

// Forward Decl. 
struct gs_gjk_result_t;
struct gs_gjk_epa_result_t;
struct gs_gjk_contact_info_t;

/* Line/Segment */

/* Sphere */
GS_API_DECL int32_t gs_sphere_vs_sphere(const gs_sphere_t* a, gs_vqs* xform_a, const gs_sphere_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_sphere_vs_aabb(const gs_sphere_t* a, gs_vqs* xform_a, const gs_aabb_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_sphere_vs_poly(const gs_sphere_t* a, gs_vqs* xform_a, const gs_poly_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);

/* Box */
GS_API_DECL int32_t gs_aabb_vs_aabb(const gs_aabb_t* a, gs_vqs* xform_a, const gs_aabb_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_aabb_vs_sphere(const gs_aabb_t* a, gs_vqs* xform_a, const gs_sphere_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_aabb_vs_poly(const gs_aabb_t* a, gs_vqs* xform_a, const gs_poly_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);

/* Plane */

/* Capsule */

/* Ray */

/* Triangle */

/* Poly */
GS_API_DECL int32_t gs_poly_vs_poly(const gs_poly_t* a, gs_vqs* xform_a, const gs_poly_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_poly_vs_sphere(const gs_poly_t* a, gs_vqs* xform_a, const gs_sphere_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_poly_vs_aabb(const gs_poly_t* a, gs_vqs* xform_a, const gs_aabb_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);

/* Frustum */

/* Hit */

/*==== Support Functions ====*/

// Support function typedef for GJK collision detection
typedef gs_vec3 (* gs_gjk_support_func_t)(const void* collider, gs_vqs* xform, gs_vec3 search_dir);

GS_API_DECL gs_vec3 gs_gjk_support_poly(const gs_poly_t* p, gs_phys_xform_t* xform, gs_vec3 search_dir);
GS_API_DECL gs_vec3 gs_gjk_support_sphere(const gs_sphere_t* s, gs_phys_xform_t* xform, gs_vec3 search_dir);
GS_API_DECL gs_vec3 gs_gjk_support_aabb(const gs_aabb_t* a, gs_phys_xform_t* xform, gs_vec3 search_dir);

/*==== GJK ====*/

#define GS_GJK_FLT_MAX FLT_MAX     // 3.40282347E+38F
#define GS_GJK_EPSILON FLT_EPSILON // 1.19209290E-07F
#define GS_GJK_MAX_ITERATIONS 64

#define GS_EPA_TOLERANCE 0.00001
#define GS_EPA_MAX_NUM_FACES 64
#define GS_EPA_MAX_NUM_LOOSE_EDGES 32
#define GS_EPA_MAX_NUM_ITERATIONS 64

typedef struct gs_gjk_support_point_t {
    gs_vec3 support_a;
    gs_vec3 support_b;
    gs_vec3 minkowski_hull_vert;
} gs_gjk_support_point_t;

typedef struct gs_gjk_simplex_t {
    union {
        gs_gjk_support_point_t points[4];
        struct {
            gs_gjk_support_point_t a;
            gs_gjk_support_point_t b;
            gs_gjk_support_point_t c;
            gs_gjk_support_point_t d;
        };
    };
} gs_gjk_simplex_t;

typedef struct gs_gjk_polytope_face_t {
    gs_gjk_support_point_t points[3];
    gs_vec3 normal;
} gs_gjk_polytope_face_t;

typedef struct gs_gjk_contact_info_t {
    int32_t hit;
    gs_vec3 normal;
    float depth;
    gs_vec3 points[2];
} gs_gjk_contact_info_t;

typedef struct gs_gjk_collider_info_t {
    const void* collider;
    gs_gjk_support_func_t func;
    gs_phys_xform_t* xform;
} gs_gjk_collider_info_t;

GS_API_DECL int32_t gs_gjk(const gs_gjk_collider_info_t* ci0, const gs_gjk_collider_info_t* ci1, gs_gjk_contact_info_t* res);
GS_API_DECL gs_gjk_support_point_t gs_gjk_generate_support_point(const gs_gjk_collider_info_t* ci0, const gs_gjk_collider_info_t* ci1, gs_vec3 dir);
GS_API_DECL void gs_gjk_update_simplex_triangle(gs_gjk_simplex_t* simplex, int32_t* simp_dim, gs_vec3* search_dir);
GS_API_DECL bool gs_gjk_update_simplex_tetrahedron(gs_gjk_simplex_t* simplex, int32_t* simp_dim, gs_vec3* search_dir);
GS_API_DECL gs_gjk_contact_info_t gs_gjk_epa(const gs_gjk_simplex_t* simplex, const gs_gjk_collider_info_t* ci0, const gs_gjk_collider_info_t* ci1);
GS_API_DECL void gs_gjk_bary(gs_vec3 p, gs_vec3 a, gs_vec3 b, gs_vec3 c, float* u, float* v, float* w);
GS_API_DECL gs_gjk_collider_info_t gs_gjk_collider_info(void* c, gs_gjk_support_func_t f, gs_phys_xform_t* t);

/*==== Physics System ====*/

#define EMPTY_VAR(...) int32_t __empty

/* Rigid Body */

// Just want a single collision shape...not sure exactly how to handle this. 
// Probably keep the data for the collision shape within the scene itself (or physics system)? Then give handles to users for the data.

typedef enum gs_rigid_body_type {
    GS_RIGID_BODY_STATIC, 
    GS_RIGID_BODY_DYNAMIC, 
    GS_RIGID_BODY_KINEMATIC
} gs_rigid_body_type;

typedef enum gs_rigid_body_state_flags {
    GS_RIGID_BODY_STATE_AWAKE       = 0x001,
    GS_RIGID_BODY_STATE_ACTIVE      = 0x002,
    GS_RIGID_BODY_STATE_ALLOW_SLEEP = 0x004,
    GS_RIGID_BODY_STATE_ISLAND      = 0x010,
    GS_RIGID_BODY_STATE_STATIC      = 0x020,
    GS_RIGID_BODY_STATE_DYNAMIC     = 0x040,
    GS_RIGID_BODY_STATE_KINEMATIC   = 0x080,
    GS_RIGID_BODY_STATE_LOCK_AXIS_X = 0x100,
    GS_RIGID_BODY_STATE_LOCK_AXIS_Y = 0x200,
    GS_RIGID_BODY_STATE_LOCK_AXIS_Z = 0x400
} gs_rigid_body_state_flags;

typedef struct gs_rigid_body_t {
    EMPTY_VAR();

    float mass;
    float inverve_mass;
    gs_vec3 linear_velocity;
    gs_vec3 angular_velocity;
    gs_vec3 force;
    gs_vec3 torque;
    gs_quat rotation;
    gs_vec3 local_center;
    gs_vec3 world_center;
    float sleep_time;
    float gravity_scale;
    uint32_t flags;

    uint32_t island_index;
    void * user_data;

} gs_rigid_body_t;

typedef struct gs_rigid_body_desc_t {
    EMPTY_VAR();
} gs_rigid_body_desc_t;

/* Collision Shape */

typedef struct gs_collision_shape_t {
    EMPTY_VAR();
} gs_collision_shape_t;

typedef struct gs_contact_constraint_t {
    EMPTY_VAR();
} gs_contact_constraint_t;

typedef struct gs_raycast_data_t {
    EMPTY_VAR();
} gs_raycast_data_t;

// The listener is used to gather information about two shapes colliding. Physics objects created in these callbacks
// Are not reported until following frame. Callbacks can be called quite frequently, so make them efficient. 
typedef struct gs_contact_listener_t {
    void (* begin_contact)(const gs_contact_constraint_t*);
    void (* end_contact)(const gs_contact_constraint_t*);
} gs_contact_listener_t;

typedef struct gs_query_callback_t {
    bool (* report_shape)(gs_collision_shape_t* shape);
} gs_query_callback_t;

// Contact Manager
typedef struct gs_physics_contact_manager_t {
    EMPTY_VAR();
} gs_physics_contact_manager_t;

/* Physics Scene */

typedef struct gs_physics_scene_t {
    gs_physics_contact_manager_t contact_manager;
    gs_paged_allocator_t paged_allocator;
    gs_stack_allocator_t stack_allocator;
    gs_heap_allocator_t  heap_allocator;
    gs_vec3 gravity;
    float delta_time;
    uint32_t iterations;    
} gs_physics_scene_t;

GS_API_DECL gs_physics_scene_t gs_physics_scene_new();
GS_API_DECL void               gs_physics_scene_step(gs_physics_scene_t* scene);
GS_API_DECL uint32_t           gs_physics_scene_create_body(gs_physics_scene_t* scene, gs_rigid_body_desc_t* desc);
GS_API_DECL void               gs_physics_scene_destroy_body(gs_physics_scene_t* scene, uint32_t id);
GS_API_DECL void               gs_physics_scene_destroy_all_bodies(gs_physics_scene_t* scene);
GS_API_DECL gs_raycast_data_t  gs_physics_scene_raycast(gs_physics_scene_t* scene, gs_query_callback_t* cb);

/*
    Scene
    Contact Manager
    Collision Solver
    Dynamics Engine
    Rigid Body
    Collision Shape
*/

/*==== Implementation ====*/

#ifdef GS_PHYSICS_IMPL

/*==== Util ====*/

GS_API_DECL gs_phys_xform_t gs_phys_xform_from_vqs(gs_vqs* vqs)
{
    gs_phys_xform_t xform = gs_default_val();
    xform.rs = gs_mat3_rsq(vqs->rotation.v, vqs->scale);
    xform.inv_rs = gs_mat3_inverse(xform.rs);
    xform.scale = vqs->scale;
    xform.rot = vqs->rotation;
    xform.pos = vqs->position;
    return xform;
}

GS_API_DECL gs_gjk_collider_info_t gs_gjk_collider_info(void* c, gs_gjk_support_func_t f, gs_phys_xform_t* t)
{
    gs_gjk_collider_info_t info = gs_default_val();
    info.collider = c;
    info.func = f;
    info.xform = t;
    return info;
}

/*==== Collision Shapes ====*/

/* Line/Segment */

/* Sphere */

GS_API_DECL gs_vec3 gs_gjk_support_sphere(const gs_sphere_t* s, gs_phys_xform_t* xform, gs_vec3 dir)
{
    // Bring direction into object space (inv matrix)
    dir = xform ? gs_mat3_mul_vec3(xform->inv_rs, dir) : dir;

    // Calculate support point
    gs_vec3 res = gs_vec3_add(gs_vec3_scale(gs_vec3_norm(dir), s->r), s->c);

    // Convert to world space
    res = xform ? gs_vec3_add(gs_mat3_mul_vec3(xform->rs, res), xform->pos) : res;

    return res;
}

GS_API_DECL int32_t gs_sphere_vs_sphere(const gs_sphere_t* a, gs_vqs* xform_a, const gs_sphere_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    // Specialized, non-gjk function
    gs_vec3 wa = xform_a ? gs_vec3_add(xform_a->position, a->c) : a->c;
    gs_vec3 wb = xform_b ? gs_vec3_add(xform_b->position, b->c) : b->c;

    // Only uniform scale allowed for spheres
    float as = xform_a ? gs_max(gs_max(xform_a->scale.x, xform_a->scale.y), xform_a->scale.z) : 1.f;
    float bs = xform_a ? gs_max(gs_max(xform_b->scale.x, xform_b->scale.y), xform_b->scale.z) : 1.f;

    gs_vec3 d = gs_vec3_sub(wb, wa);
    float r = as * a->r + bs * b->r;
    float d2 = gs_vec3_dot(d,d);
    if (d2 > r*r) return false;

    float l = sqrtf(d2);
    float linv = 1.0f / ((l != 0) ? l: 1.0f);

    if (res)
    {
        res->normal = gs_vec3_scale(d, linv);
        res->depth = r - l;
        d = gs_vec3_scale(res->normal, b->r * bs);
        res->points[0] = gs_vec3_add(wa, d);
        res->points[1] = gs_vec3_sub(wb, d);
    }

    return true;
}

GS_API_DECL int32_t gs_sphere_vs_aabb(const gs_sphere_t* a, gs_vqs* xform_a, const gs_aabb_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    gs_phys_xform_t x0 = xform_a ? gs_phys_xform_from_vqs(xform_a) : gs_ctor(gs_phys_xform_t);
    gs_phys_xform_t x1 = xform_b ? gs_phys_xform_from_vqs(xform_b) : gs_ctor(gs_phys_xform_t);
    gs_gjk_collider_info_t c0 = gs_gjk_collider_info(a, gs_gjk_support_sphere, xform_a ? &x0 : NULL);
    gs_gjk_collider_info_t c1 = gs_gjk_collider_info(b, gs_gjk_support_aabb, xform_b ? &x1 : NULL);
    return gs_gjk(&c0, &c1, res);
}

GS_API_DECL int32_t gs_sphere_vs_poly(const gs_sphere_t* a, gs_vqs* xform_a, const gs_poly_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return gs_poly_vs_sphere(b, xform_b, a, xform_a, res);
}

/* AABB */

GS_API_DECL gs_vec3 gs_gjk_support_aabb(const gs_aabb_t* a, gs_phys_xform_t* xform, gs_vec3 dir)
{
    // Find support in object space (with inverse matrix)
    dir = xform ? gs_mat3_mul_vec3(xform->inv_rs, dir) : dir;

    // Calculate support point now in object space
    gs_vec3 res = gs_default_val();
    res.x = (dir.x > 0.f) ? a->max.x : a->min.x;
    res.y = (dir.y > 0.f) ? a->max.y : a->min.y;
    res.z = (dir.z > 0.f) ? a->max.z : a->min.z;

    // Convert to world space
    res = xform ? gs_vec3_add(gs_mat3_mul_vec3(xform->rs, res), xform->pos) : res;

    return res;
}

GS_API_DECL int32_t gs_aabb_vs_aabb(const gs_aabb_t* a, gs_vqs* xform_a, const gs_aabb_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    // NOTE(john): If neither have transforms, then can default to less expensive AABB vs. AABB check

    gs_phys_xform_t x0 = xform_a ? gs_phys_xform_from_vqs(xform_a) : gs_ctor(gs_phys_xform_t);
    gs_phys_xform_t x1 = xform_b ? gs_phys_xform_from_vqs(xform_b) : gs_ctor(gs_phys_xform_t);
    gs_gjk_collider_info_t c0 = gs_gjk_collider_info(a, gs_gjk_support_aabb, xform_a ? &x0 : NULL);
    gs_gjk_collider_info_t c1 = gs_gjk_collider_info(b, gs_gjk_support_aabb, xform_b ? &x1 : NULL);
    return gs_gjk(&c0, &c1, res);
}

GS_API_DECL int32_t gs_aabb_vs_sphere(const gs_aabb_t* a, gs_vqs* xform_a, const gs_sphere_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return gs_sphere_vs_aabb(b, xform_b, a, xform_a, res);
}

GS_API_DECL int32_t gs_aabb_vs_poly(const gs_aabb_t* a, gs_vqs* xform_a, const gs_poly_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return gs_poly_vs_aabb(b, xform_b, a, xform_a, res);
}

/* Plane */

/* Capsule */

/* Ray */

/* Triangle */

/* Poly */

GS_API_DECL gs_vec3 gs_gjk_support_poly(const gs_poly_t* p, gs_phys_xform_t* xform, gs_vec3 dir)
{
    // Find support in object space (with inverse matrix)
    dir = xform ? gs_mat3_mul_vec3(xform->inv_rs, dir) : dir;

    // Initial furthest point
    gs_vec3 res = p->verts[0];
    float max_dot = gs_vec3_dot(res, dir);

    int32_t max_i = 0;
    for (int32_t i = 1; i < p->cnt; ++i) {
        float d = gs_vec3_dot(dir, p->verts[i]);
        if (d > max_dot) {
            max_dot = d;
            res = p->verts[i];
            max_i = i;
        }
    }

    // Conver to world space
    res = xform ? gs_vec3_add(gs_mat3_mul_vec3(xform->rs, res), xform->pos) : res;

    return res;
}

GS_API_DECL int32_t gs_poly_vs_poly(const gs_poly_t* a, gs_vqs* xform_a, const gs_poly_t* b, gs_vqs* xform_b, gs_gjk_contact_info_t* res)
{
    gs_phys_xform_t x0 = xform_a ? gs_phys_xform_from_vqs(xform_a) : gs_ctor(gs_phys_xform_t);
    gs_phys_xform_t x1 = xform_b ? gs_phys_xform_from_vqs(xform_b) : gs_ctor(gs_phys_xform_t);
    gs_gjk_collider_info_t c0 = gs_gjk_collider_info(a, gs_gjk_support_poly, xform_a ? &x0 : NULL);
    gs_gjk_collider_info_t c1 = gs_gjk_collider_info(b, gs_gjk_support_poly, xform_b ? &x1 : NULL);
    return gs_gjk(&c0, &c1, res);
}

GS_API_DECL int32_t gs_poly_vs_sphere(const gs_poly_t* a, gs_vqs* xform_a, const gs_sphere_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    gs_phys_xform_t x0 = xform_a ? gs_phys_xform_from_vqs(xform_a) : gs_ctor(gs_phys_xform_t);
    gs_phys_xform_t x1 = xform_b ? gs_phys_xform_from_vqs(xform_b) : gs_ctor(gs_phys_xform_t);
    gs_gjk_collider_info_t c0 = gs_gjk_collider_info(a, gs_gjk_support_poly, xform_a ? &x0 : NULL);
    gs_gjk_collider_info_t c1 = gs_gjk_collider_info(b, gs_gjk_support_sphere, xform_b ? &x1 : NULL);
    return gs_gjk(&c0, &c1, res);
}

GS_API_DECL int32_t gs_poly_vs_aabb(const gs_poly_t* a, gs_vqs* xform_a, const gs_aabb_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    gs_phys_xform_t x0 = xform_a ? gs_phys_xform_from_vqs(xform_a) : gs_ctor(gs_phys_xform_t);
    gs_phys_xform_t x1 = xform_b ? gs_phys_xform_from_vqs(xform_b) : gs_ctor(gs_phys_xform_t);
    gs_gjk_collider_info_t c0 = gs_gjk_collider_info(a, gs_gjk_support_poly, xform_a ? &x0 : NULL);
    gs_gjk_collider_info_t c1 = gs_gjk_collider_info(b, gs_gjk_support_aabb, xform_b ? &x1 : NULL);
    return gs_gjk(&c0, &c1, res);
}

/* Frustum */

/* Hit */

/*==== GKJ ====*/

GS_API_DECL int32_t gs_gjk(const gs_gjk_collider_info_t* ci0, const gs_gjk_collider_info_t* ci1, gs_gjk_contact_info_t* res)
{
    // Simplex: just a set of points (a is always most recently added)
    gs_gjk_simplex_t simplex = {0};
    gs_gjk_support_point_t* a = &simplex.a, *b = &simplex.b, *c = &simplex.c, *d = &simplex.d;

    // Cache pointers for collider 0
    void* c0 = ci0->collider;
    gs_gjk_support_func_t f0 = ci0->func;
    gs_phys_xform_t* xform_0 = ci0->xform;

    // Calculate inverse matrix if available
    if (ci0->xform) ci0->xform->inv_rs = gs_mat3_inverse(ci0->xform->rs);

    // Cache pointers for collider 1
    void* c1 = ci1->collider;
    gs_gjk_support_func_t f1 = ci1->func; 
    gs_phys_xform_t* xform_1 = ci1->xform; 

    // Calculate inverse matrix if available
    if (ci1->xform) ci1->xform->inv_rs = gs_mat3_inverse(ci1->xform->rs);

    // Initial search direction between colliders (xaxis for start, can be any random direction)
    gs_vec3 search_dir = GS_XAXIS; 

    // Get initial point for simplex
    *c = gs_gjk_generate_support_point(ci0, ci1, search_dir); 
    search_dir = gs_vec3_neg(c->minkowski_hull_vert); //search in direction of origin

    // Get second point for a line segment simplex
    *b = gs_gjk_generate_support_point(ci0, ci1, search_dir); 

    // We didn't reach the origin, won't enclose it
    if (!gs_vec3_same_dir(b->minkowski_hull_vert, search_dir)) {
        return false;
    }

    // Direction from c to b
    gs_vec3 cmb = gs_vec3_sub(c->minkowski_hull_vert, b->minkowski_hull_vert);
    // Direction back to origin from b
    gs_vec3 nb = gs_vec3_neg(b->minkowski_hull_vert);

    // Search perpendicular to line segment towards origin
    search_dir = gs_vec3_cross(gs_vec3_cross(cmb, nb), cmb);

    // Origin is on this line segment
    if (gs_vec3_eq(search_dir, gs_v3s(0.f))) {

        // Search with normal to cmb/x axis (can be any random axis)
        search_dir = gs_vec3_cross(cmb, GS_XAXIS);

        // Normal with -zaxis
        if (gs_vec3_eq(search_dir, gs_v3s(0.f))) {
            search_dir = gs_vec3_cross(cmb, gs_v3(0.f,0.f,-1.f));
        }
    }

    // Check line, initially
    int32_t simp_dim = 2;
    bool just_turned_three = false;
    
    for (int32_t iterations = 0; iterations < GS_GJK_MAX_ITERATIONS; ++iterations)
    {
        // Generate new support point along updated search direction
        *a = gs_gjk_generate_support_point(ci0, ci1, search_dir); 

        // We didn't reach the origin, won't enclose it
        if (!gs_vec3_same_dir(a->minkowski_hull_vert, search_dir)) { 
            return false; 
        } 
    
        // Increase simplex dimensions
        simp_dim++;

        // Check for case where we just hit three dimensions (hack for now)
        if (just_turned_three) {
            just_turned_three = false;
            simp_dim = 4;
        }

        // Triangle
        if (simp_dim == 3) 
        {
            if (!just_turned_three) just_turned_three = true;
            gs_gjk_update_simplex_triangle(&simplex, &simp_dim, &search_dir);
        }
        // Tetrahedron
        else if (gs_gjk_update_simplex_tetrahedron(&simplex, &simp_dim, &search_dir)) 
        {
            // Capture collision data using EPA if requested by user
            if (res) 
            {
                *res = gs_gjk_epa(&simplex, ci0, ci1);
                return res->hit;
            } 
            else 
            {
                return true;
            }
        }
    }

    return false;
}

GS_API_DECL gs_gjk_support_point_t gs_gjk_generate_support_point(const gs_gjk_collider_info_t* ci0, const gs_gjk_collider_info_t* ci1, gs_vec3 dir)
{
   gs_gjk_support_point_t sp = {0}; 
   sp.support_a = ci0->func(ci0->collider, ci0->xform, dir);
   sp.support_b = ci1->func(ci1->collider, ci1->xform, gs_vec3_neg(dir));
   sp.minkowski_hull_vert = gs_vec3_sub(sp.support_a, sp.support_b);
   return sp;
}

GS_API_DECL void gs_gjk_update_simplex_triangle(gs_gjk_simplex_t* simplex, int32_t* simp_dim, gs_vec3* search_dir)
{
    /* Required winding order:
    //  b
    //  | \
    //  |   \
    //  |    a
    //  |   /
    //  | /
    //  c
    */

    // B -> A
    gs_vec3 bma = gs_vec3_sub(simplex->b.minkowski_hull_vert, simplex->a.minkowski_hull_vert);
    // C -> A
    gs_vec3 cma = gs_vec3_sub(simplex->c.minkowski_hull_vert, simplex->a.minkowski_hull_vert);
    // Normal of ABC
    gs_vec3 n = gs_vec3_cross(bma, cma);
    // Directino to origin
    gs_vec3 AO = gs_vec3_neg(simplex->a.minkowski_hull_vert);

    // Set for line initially
    *simp_dim = 2;

    // Edge AB
    if (gs_vec3_same_dir(gs_vec3_cross(bma, n), AO)) 
    {
        simplex->c = simplex->a;
        *search_dir = gs_vec3_cross(gs_vec3_cross(bma, AO), bma);
        return;
    }
    // Edge AC
    if (gs_vec3_same_dir(gs_vec3_cross(n, cma), AO))
    {
        simplex->b = simplex->a;
        *search_dir = gs_vec3_cross(gs_vec3_cross(cma, AO), cma);
        return;
    }

    // Valid triangle
    simp_dim = 3;

    // Above Triangle
    if (gs_vec3_same_dir(n, AO))
    {
        simplex->d = simplex->c;
        simplex->c = simplex->b;
        simplex->b = simplex->a;
        *search_dir = n;
        return;
    }

    // Below triangle
    simplex->d = simplex->b;
    simplex->b = simplex->a;
    *search_dir = gs_vec3_neg(n);
    return;

}

GS_API_DECL bool gs_gjk_update_simplex_tetrahedron(gs_gjk_simplex_t* simplex, int32_t* simp_dim, gs_vec3* search_dir)
{
    // Direction: B -> A
    gs_vec3 bma = gs_vec3_sub(simplex->b.minkowski_hull_vert, simplex->a.minkowski_hull_vert);
    // Direction: C -> A
    gs_vec3 cma = gs_vec3_sub(simplex->c.minkowski_hull_vert, simplex->a.minkowski_hull_vert);
    // Direction D -> A
    gs_vec3 dma = gs_vec3_sub(simplex->d.minkowski_hull_vert, simplex->a.minkowski_hull_vert);
    // Normal: ABC
    gs_vec3 ABC = gs_vec3_cross(bma, cma);
    // Normal ACD
    gs_vec3 ACD = gs_vec3_cross(cma, dma);
    // Normal: ADB
    gs_vec3 ADB = gs_vec3_cross(dma, bma);
    // Direction to origin
    gs_vec3 AO = gs_vec3_neg(simplex->a.minkowski_hull_vert);
    // Set current dimension to 3
    *simp_dim = 3;

    //Plane-test origin with 3 faces
    /*
    // Note: Kind of primitive approach used here; If origin is in front of a face, just use it as the new simplex.
    // We just go through the faces sequentially and exit at the first one which satisfies dot product. Not sure this 
    // is optimal or if edges should be considered as possible simplices? Thinking this through in my head I feel like 
    // this method is good enough. Makes no difference for AABBS, should test with more complex colliders.
    */
    // In front of ABC
    if (gs_vec3_same_dir(ABC, AO)) 
    {
        simplex->d = simplex->c;
        simplex->c = simplex->b;
        simplex->b = simplex->a;
        *search_dir = ABC;
        return false;
    }
    // In front of ACD
    if (gs_vec3_same_dir(ACD, AO)) 
    {
        simplex->b = simplex->a;
        *search_dir = ACD;
        return false;
    }
    // In front of ADB
    if (gs_vec3_same_dir(ADB, AO)) 
    {
        simplex->c = simplex->d;
        simplex->d = simplex->b;
        simplex->b = simplex->a;
        *search_dir = ADB;
        return false;
    }

    // Else inside tetrahedron, therefore origin is enclosed
    return true;
}

// Find barycentric coordinates of triangle with respect to p
GS_API_DECL void gs_gjk_bary(gs_vec3 p, gs_vec3 a, gs_vec3 b, gs_vec3 c, float* u, float* v, float* w)
{
    gs_vec3 v0 = gs_vec3_sub(b, a), v1 = gs_vec3_sub(c, a), v2 = gs_vec3_sub(p, a);
    float d00 = gs_vec3_dot(v0, v0);
    float d01 = gs_vec3_dot(v0, v1);
    float d11 = gs_vec3_dot(v1, v1);
    float d20 = gs_vec3_dot(v2, v0);
    float d21 = gs_vec3_dot(v2, v1);
    float denom = d00 * d11 - d01 * d01;
    *v = (d11 * d20 - d01 * d21) / denom;
    *w = (d00 * d21 - d01 * d20) / denom;
    *u = 1.0f - *v - *w;
}

//Expanding Polytope Algorithm
GS_API_DECL gs_gjk_contact_info_t gs_gjk_epa(const gs_gjk_simplex_t* simplex, const gs_gjk_collider_info_t* ci0, const gs_gjk_collider_info_t* ci1)
{
    gs_gjk_contact_info_t res = {0};

    // Cache pointers for collider 0
    void* c0 = ci0->collider;
    gs_gjk_support_func_t f0 = ci0->func;
    gs_phys_xform_t* xform_0 = ci0->xform;

    // Cache pointers for collider 1
    void* c1 = ci1->collider;
    gs_gjk_support_func_t f1 = ci1->func;
    gs_phys_xform_t* xform_1 = ci1->xform;

    // Array of polytope faces, each with 3 support points and a normal
    gs_gjk_polytope_face_t faces[GS_EPA_MAX_NUM_FACES] = {0};
    gs_vec3 bma = gs_vec3_sub(simplex->b.minkowski_hull_vert, simplex->a.minkowski_hull_vert);
    gs_vec3 cma = gs_vec3_sub(simplex->c.minkowski_hull_vert, simplex->a.minkowski_hull_vert);
    gs_vec3 dma = gs_vec3_sub(simplex->d.minkowski_hull_vert, simplex->a.minkowski_hull_vert);
    gs_vec3 cmb = gs_vec3_sub(simplex->c.minkowski_hull_vert, simplex->b.minkowski_hull_vert);
    gs_vec3 dmb = gs_vec3_sub(simplex->d.minkowski_hull_vert, simplex->b.minkowski_hull_vert);
    
    faces[0] = gs_ctor(gs_gjk_polytope_face_t, {simplex->a, simplex->b, simplex->c}, gs_vec3_norm(gs_vec3_cross(bma, cma)));    // ABC
    faces[1] = gs_ctor(gs_gjk_polytope_face_t, {simplex->a, simplex->c, simplex->d}, gs_vec3_norm(gs_vec3_cross(cma, dma)));    // ACD
    faces[2] = gs_ctor(gs_gjk_polytope_face_t, {simplex->a, simplex->d, simplex->b}, gs_vec3_norm(gs_vec3_cross(dma, bma)));    // ADB
    faces[3] = gs_ctor(gs_gjk_polytope_face_t, {simplex->b, simplex->d, simplex->c}, gs_vec3_norm(gs_vec3_cross(dmb, cmb)));    // BDC

    int32_t num_faces = 4;
    int32_t closest_face;

    for (int32_t iterations = 0; iterations < GS_EPA_MAX_NUM_ITERATIONS; ++iterations) 
    {
        //Find face that's closest to origin
        float min_dist = gs_vec3_dot(faces[0].points[0].minkowski_hull_vert, faces[0].normal);
        closest_face = 0;
        for (int32_t i = 1; i < num_faces; ++i) 
        {
            float dist = gs_vec3_dot(faces[i].points[0].minkowski_hull_vert, faces[i].normal);
            if (dist < min_dist) 
            {
                min_dist = dist;
                closest_face = i;
            }
        }

        // Search normal to face that's closest to origin
        gs_vec3 search_dir = faces[closest_face].normal; 
        gs_gjk_support_point_t p = gs_gjk_generate_support_point(ci0, ci1, search_dir);
        float depth = gs_vec3_dot(p.minkowski_hull_vert, search_dir);

        // Within tolerance, so extract contact information from hull
        if (depth - min_dist < GS_EPA_TOLERANCE) 
        {
            // Cache local pointers
            gs_gjk_polytope_face_t* f = &faces[closest_face];
            gs_vec3* n = &f->normal;
            gs_gjk_support_point_t* sp0 = &f->points[0];
            gs_gjk_support_point_t* sp1 = &f->points[1];
            gs_gjk_support_point_t* sp2 = &f->points[2];
            gs_vec3* p0 = &sp0->minkowski_hull_vert;
            gs_vec3* p1 = &sp1->minkowski_hull_vert;
            gs_vec3* p2 = &sp2->minkowski_hull_vert;

            // Normal and depth information
            res.hit = true;
            res.normal = gs_vec3_norm(*n);
            res.depth = depth;

            // Get barycentric coordinates of resulting triangle face
            float u = 0.f, v = 0.f, w = 0.f;
            gs_gjk_bary(*n, *p0, *p1, *p2, &u, &v, &w);

            gs_vec3* support_0, *support_1, *support_2;

            // A Contact points
            support_0 = &sp0->support_a;
            support_1 = &sp1->support_a;
            support_2 = &sp2->support_a;

            // Contact point on collider a
            res.points[0] = gs_vec3_add(gs_vec3_add(gs_vec3_scale(*support_0, u), gs_vec3_scale(*support_1, v)), gs_vec3_scale(*support_2, w));

            // B Contact points
            support_0 = &sp0->support_b;
            support_1 = &sp1->support_b;
            support_2 = &sp2->support_b;

            // Contact point on collider b
            res.points[1] = gs_vec3_add(gs_vec3_add(gs_vec3_scale(*support_0, u), gs_vec3_scale(*support_1, v)), gs_vec3_scale(*support_2, w));

            return res;
        }

        // Fix Edges
        gs_gjk_support_point_t loose_edges[GS_EPA_MAX_NUM_LOOSE_EDGES][2]; 
        int32_t num_loose_edges = 0;

        // Find all triangles that are facing p
        for (int32_t i = 0; i < num_faces; ++i)
        {  
            // Grab direction from first point on face at p to i
            gs_vec3 dir = gs_vec3_sub(p.minkowski_hull_vert, faces[i].points[0].minkowski_hull_vert);

            // Triangle i faces p, remove it
            if (gs_vec3_same_dir(faces[i].normal, dir))
            {
                // Add removed triangle's edges to loose edge list.
                // If it's already there, remove it (both triangles it belonged to are gone)
                for (int32_t j = 0; j < 3; ++j)
                {
                    gs_gjk_support_point_t current_edge[2] = {faces[i].points[j], faces[i].points[(j + 1) % 3]};
                    bool found_edge = false;
                    for (int32_t k = 0; k < num_loose_edges; ++k) //Check if current edge is already in list
                    {
                        //Edge is already in the list, remove it
                        if (gs_vec3_eq(loose_edges[k][1].minkowski_hull_vert, current_edge[0].minkowski_hull_vert) 
                                && gs_vec3_eq(loose_edges[k][0].minkowski_hull_vert, current_edge[1].minkowski_hull_vert)) 
                        {
                            // Overwrite current edge with last edge in list
                            loose_edges[k][0] = loose_edges[num_loose_edges - 1][0];
                            loose_edges[k][1] = loose_edges[num_loose_edges - 1][1];
                            num_loose_edges--;

                             // Exit loop because edge can only be shared once
                            found_edge = true;
                            k = num_loose_edges;
                        }
                    }
                    
                    // Add current edge to list (is unique)
                    if (!found_edge) 
                    { 
                        if (num_loose_edges >= GS_EPA_MAX_NUM_LOOSE_EDGES) break;
                        loose_edges[num_loose_edges][0] = current_edge[0];
                        loose_edges[num_loose_edges][1] = current_edge[1];
                        num_loose_edges++;
                    }
                }

                // Remove triangle i from list
                faces[i].points[0] = faces[num_faces - 1].points[0];
                faces[i].points[1] = faces[num_faces - 1].points[1];
                faces[i].points[2] = faces[num_faces - 1].points[2];
                faces[i].normal = faces[num_faces - 1].normal;
                num_faces--;
                i--;
            }
        }
        
        // Reconstruct polytope with p now added
        for (int32_t i = 0; i < num_loose_edges; ++i)
        {
            if (num_faces >= GS_EPA_MAX_NUM_FACES) break;
            faces[num_faces].points[0] = loose_edges[i][0];
            faces[num_faces].points[1] = loose_edges[i][1];
            faces[num_faces].points[2] = p;
            gs_vec3 zmo = gs_vec3_sub(loose_edges[i][0].minkowski_hull_vert, loose_edges[i][1].minkowski_hull_vert);
            gs_vec3 zmp = gs_vec3_sub(loose_edges[i][0].minkowski_hull_vert, p.minkowski_hull_vert);
            faces[num_faces].normal = gs_vec3_norm(gs_vec3_cross(zmo, zmp)); 

            //Check for wrong normal to maintain CCW winding
            // In case dot result is only slightly < 0 (because origin is on face)
            float bias = 0.000001;
            if (gs_vec3_dot(faces[num_faces].points[0].minkowski_hull_vert, faces[num_faces].normal) + bias < 0.f){
                gs_gjk_support_point_t temp = faces[num_faces].points[0];
                faces[num_faces].points[0] = faces[num_faces].points[1];
                faces[num_faces].points[1] = temp;
                faces[num_faces].normal = gs_vec3_scale(faces[num_faces].normal, -1.f);
            }
            num_faces++;
        }
    }

    // Return most recent closest point
    float dot = gs_vec3_dot(faces[closest_face].points[0].minkowski_hull_vert, faces[closest_face].normal);
    gs_vec3 norm = gs_vec3_scale(faces[closest_face].normal, dot);
    res.hit = false;
    res.normal = gs_vec3_norm(norm);
    res.depth = gs_vec3_len(norm);
    return res;
}

#endif // GS_PHYSICS_IMPL
#endif // __GS_PHYSICS_H__


























