
/*================================================================
    * Copyright: 2020 John Jackson
    * GSPhysics: Simple 3D/2D physics engine
    * File: gs_physics.h
    All Rights Reserved
=================================================================*/

#ifndef GS_PHYSICS_H
#define GS_PHYSICS_H

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

/** @defgroup gs_physics_util Physics Util
 *  Gunslinger Physics Util
 *  @{
 */

/*==== Collision Shapes ====*/

// 3D shapes
typedef struct gs_line_t     {gs_vec3 a, b;                                                  } gs_line_t;
typedef struct gs_aabb_t     {gs_vec3 min, max;                                              } gs_aabb_t;
typedef struct gs_sphere_t   {gs_vec3 c; float r;                                            } gs_sphere_t;
typedef struct gs_plane_t    {union{gs_vec3 p; struct{float a, b, c;};}; float d;            } gs_plane_t;
typedef struct gs_capsule_t  {gs_vec3 base; float r, height;                                 } gs_capsule_t;
typedef struct gs_ray_t      {gs_vec3 p, d;                                                  } gs_ray_t;
typedef struct gs_poly_t     {gs_vec3* verts; int32_t cnt;                                   } gs_poly_t;
typedef union  gs_frustum_t  {struct {gs_vec4 l, r, t, b, n, f;}; gs_vec4 pl[6]; float v[24];} gs_frustum_t;
typedef struct gs_cylinder_t {float r; gs_vec3 base; float height;                           } gs_cylinder_t; 
typedef struct gs_cone_t     {float r; gs_vec3 base; float height;                           } gs_cone_t;

// 2D shapes
// typedef struct gs_rect_t     {gs_vec2 min; gs_vec2 max;                                      } gs_rect_t;
// typedef struct gs_circle_t   {float r; gs_vec2 c;                                            } gs_circle_t;
// typedef struct gs_triangle_t {gs_vec2 a,b,c;                                                 } gs_triangle_t;
// typedef struct gs_pill_t     {gs_vec2 base; float r, height;                                 } gs_pill_t;

/*
    typedef struct _gs_collision_obj_handle_t 
    {
        void* obj;
        gs_support_func_t support;
        gs_vqs* xform;
    } _gs_collision_obj_handle_t;

    // Wrap this, then expose collision callback to user through gs means

    // Internal support function for all ccd callbacks (will pass in user function through this cb), 
    // this way I can keep everything consistent, only expose gs related APIs, wrap any 3rd party libs, 
    // and possibly change internals in the future to custom implementations without breaking anything.
    void _gs_ccd_support_func(const void* _obj, const ccd_vec3_t* _dir, ccd_vec3_t* _out) 
    {
        const _gs_collision_obj_handle_t* obj = (const _gs_collision_obj_handle_t)_obj;
        if (obj->support) 
        {
            // Call user support function
            gs_vec3 dir = _gs_ccdv32gsv3(_dir);
            gs_vec3 out = gs_default_val();
            obj->support(obj->obj, obj->xform, &dir, &out);

            // Copy over out result for ccd
            _gs_gsv32ccdv3(&out, _out);
        }
    }
*/

// Need 2d collision shapes and responses with GJK+EPA

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
#define gs_cone(...)     gs_ctor(gs_cone_t, __VA_ARGS__)
#define gs_ray(...)      gs_ctor(gs_ray_t, __VA_ARGS__)
#define gs_poly(...)     gs_ctor(gs_poly_t, __VA_ARGS__)
#define gs_frustum(...). gs_ctor(gs_frustum_t, __VA_ARGS__)
#define gs_cylinder(...) gs_ctor(gs_cylinder_t, __VA_ARGS__)
#define gs_hit(...)      gs_ctor(gs_hit_t, __VA_ARGS__)

// Contact info
typedef struct gs_contact_info_t {
    int32_t hit;
    gs_vec3 normal;
    float depth;
    gs_vec3 point;
} gs_contact_info_t;

/* Line/Segment */
GS_API_DECL gs_line_t gs_line_closest_line(const gs_line_t* l, gs_vec3 p);
GS_API_DECL gs_vec3   gs_line_closest_point(const gs_line_t* l, gs_vec3 p);
GS_API_DECL gs_vec3   gs_line_direction(const gs_line_t* l);

/* Ray */

/* Plane */
GS_API_DECL gs_plane_t gs_plane_from_pt_normal(gs_vec3 pt, gs_vec3 n);
GS_API_DECL gs_plane_t gs_plane_from_pts(gs_vec3 a, gs_vec3 b, gs_vec3 c);
GS_API_DECL gs_vec3    gs_plane_normal(const gs_plane_t* p); 
GS_API_DECL gs_vec3    gs_plane_closest_point(const gs_plane_t* p, gs_vec3 pt); 
GS_API_DECL float      gs_plane_signed_distance(const gs_plane_t* p, gs_vec3 pt);
GS_API_DECL float      gs_plane_unsigned_distance(const gs_plane_t* p, gs_vec3 pt);
GS_API_DECL gs_plane_t gs_plane_normalized(const gs_plane_t* p);

/* Sphere */
GS_API_DECL int32_t gs_sphere_vs_sphere(const gs_sphere_t* a, const gs_vqs* xform_a, const gs_sphere_t* b, const gs_vqs* xform_b, gs_contact_info_t* res);
GS_API_DECL int32_t gs_sphere_vs_aabb(const gs_sphere_t* a, const gs_vqs* xform_a, const gs_aabb_t* b, const gs_vqs* xform_b, gs_contact_info_t* res);
GS_API_DECL int32_t gs_sphere_vs_poly(const gs_sphere_t* a, const gs_vqs* xform_a, const gs_poly_t* b, const gs_vqs* xform_b, gs_contact_info_t* res);
GS_API_DECL int32_t gs_sphere_vs_cylinder(const gs_sphere_t* a, const gs_vqs* xform_a, const gs_cylinder_t* b, const gs_vqs* xform_b, gs_contact_info_t* res);
GS_API_DECL int32_t gs_sphere_vs_cone(const gs_sphere_t* a, const gs_vqs* xform_a, const gs_cone_t* b, const gs_vqs* xform_b, gs_contact_info_t* res);
GS_API_DECL int32_t gs_sphere_vs_capsule(const gs_sphere_t* a, const gs_vqs* xform_a, const gs_capsule_t* b, const gs_vqs* xform_b, gs_contact_info_t* res);

/* Box */
GS_API_DECL int32_t gs_aabb_vs_aabb(const gs_aabb_t* a, const gs_vqs* xform_a, const gs_aabb_t* b, const gs_vqs* xform_b, gs_contact_info_t* res);
GS_API_DECL int32_t gs_aabb_vs_sphere(const gs_aabb_t* a, const gs_vqs* xform_a, const gs_sphere_t* b, const gs_vqs* xform_b, gs_contact_info_t* res);
GS_API_DECL int32_t gs_aabb_vs_poly(const gs_aabb_t* a, const gs_vqs* xform_a, const gs_poly_t* b, const gs_vqs* xform_b, gs_contact_info_t* res);
GS_API_DECL int32_t gs_aabb_vs_cylinder(const gs_aabb_t* a, const gs_vqs* xform_a, const gs_cylinder_t* b, const gs_vqs* xform_b, gs_contact_info_t* res);
GS_API_DECL int32_t gs_aabb_vs_cone(const gs_aabb_t* a, const gs_vqs* xform_a, const gs_cone_t* b, const gs_vqs* xform_b, gs_contact_info_t* res);
GS_API_DECL int32_t gs_aabb_vs_capsule(const gs_aabb_t* a, const gs_vqs* xform_a, const gs_capsule_t* b, const gs_vqs* xform_b, gs_contact_info_t* res);

/* Capsule */
GS_API_DECL int32_t gs_capsule_vs_aabb(const gs_capsule_t* capsule, const gs_vqs* xform_a, const gs_aabb_t* b, const gs_vqs* xform_b, gs_contact_info_t* res);
GS_API_DECL int32_t gs_capsule_vs_sphere(const gs_capsule_t* capsule, const gs_vqs* xform_a, const gs_sphere_t* b, const gs_vqs* xform_b, gs_contact_info_t* res);
GS_API_DECL int32_t gs_capsule_vs_poly(const gs_capsule_t* capsule, const gs_vqs* xform_a, const gs_poly_t* b, const gs_vqs* xform_b, gs_contact_info_t* res);
GS_API_DECL int32_t gs_capsule_vs_cylinder(const gs_capsule_t* capsule, const gs_vqs* xform_a, const gs_cylinder_t* b, const gs_vqs* xform_b, gs_contact_info_t* res);
GS_API_DECL int32_t gs_capsule_vs_cone(const gs_capsule_t* capsule, const gs_vqs* xform_a, const gs_cone_t* b, const gs_vqs* xform_b, gs_contact_info_t* res);
GS_API_DECL int32_t gs_capsule_vs_capsule(const gs_capsule_t* capsule, const gs_vqs* xform_a, const gs_capsule_t* b, const gs_vqs* xform_b, gs_contact_info_t* res);

/* Poly */
GS_API_DECL int32_t gs_poly_vs_poly(const gs_poly_t* a, const gs_vqs* xform_a, const gs_poly_t* b, const gs_vqs* xform_b, gs_contact_info_t* res);
GS_API_DECL int32_t gs_poly_vs_sphere(const gs_poly_t* a, const gs_vqs* xform_a, const gs_sphere_t* b, const gs_vqs* xform_b, gs_contact_info_t* res);
GS_API_DECL int32_t gs_poly_vs_aabb(const gs_poly_t* a, const gs_vqs* xform_a, const gs_aabb_t* b, const gs_vqs* xform_b, gs_contact_info_t* res);
GS_API_DECL int32_t gs_poly_vs_cylinder(const gs_poly_t* a, const gs_vqs* xform_a, const gs_cylinder_t* b, const gs_vqs* xform_b, gs_contact_info_t* res);
GS_API_DECL int32_t gs_poly_vs_cone(const gs_poly_t* a, const gs_vqs* xform_a, const gs_cone_t* b, const gs_vqs* xform_b, gs_contact_info_t* res);
GS_API_DECL int32_t gs_poly_vs_capsule(const gs_poly_t* a, const gs_vqs* xform_a, const gs_capsule_t* b, const gs_vqs* xform_b, gs_contact_info_t* res);

/* Frustum */

/* Cylinder */
GS_API_DECL int32_t gs_cylinder_vs_cylinder(const gs_cylinder_t* a, const gs_vqs* xform_a, const gs_cylinder_t* b, const gs_vqs* xform_b, gs_contact_info_t* res);
GS_API_DECL int32_t gs_cylinder_vs_sphere(const gs_cylinder_t* a, const gs_vqs* xform_a, const gs_sphere_t* b, const gs_vqs* xform_b, gs_contact_info_t* res);
GS_API_DECL int32_t gs_cylinder_vs_aabb(const gs_cylinder_t* a, const gs_vqs* xform_a, const gs_aabb_t* b, const gs_vqs* xform_b, gs_contact_info_t* res);
GS_API_DECL int32_t gs_cylinder_vs_poly(const gs_cylinder_t* a, const gs_vqs* xform_a, const gs_poly_t* b, const gs_vqs* xform_b, gs_contact_info_t* res);
GS_API_DECL int32_t gs_cylinder_vs_cone(const gs_cylinder_t* a, const gs_vqs* xform_a, const gs_cone_t* b, const gs_vqs* xform_b, gs_contact_info_t* res);
GS_API_DECL int32_t gs_cylinder_vs_capsule(const gs_cylinder_t* a, const gs_vqs* xform_a, const gs_capsule_t* b, const gs_vqs* xform_b, gs_contact_info_t* res);

/* Cone */
GS_API_DECL int32_t gs_cone_vs_cone(const gs_cone_t* a, const gs_vqs* xform_a, const gs_cone_t* b, const gs_vqs* xform_b, gs_contact_info_t* res);
GS_API_DECL int32_t gs_cone_vs_sphere(const gs_cone_t* a, const gs_vqs* xform_a, const gs_sphere_t* b, const gs_vqs* xform_b, gs_contact_info_t* res);
GS_API_DECL int32_t gs_cone_vs_aabb(const gs_cone_t* a, const gs_vqs* xform_a, const gs_aabb_t* b, const gs_vqs* xform_b, gs_contact_info_t* res);
GS_API_DECL int32_t gs_cone_vs_poly(const gs_cone_t* a, const gs_vqs* xform_a, const gs_poly_t* b, const gs_vqs* xform_b, gs_contact_info_t* res);
GS_API_DECL int32_t gs_cone_vs_cylinder(const gs_cone_t* a, const gs_vqs* xform_a, const gs_cylinder_t* b, const gs_vqs* xform_b, gs_contact_info_t* res);
GS_API_DECL int32_t gs_cone_vs_capsule(const gs_cone_t* a, const gs_vqs* xform_a, const gs_capsule_t* b, const gs_vqs* xform_b, gs_contact_info_t* res);

// 2D Shapes (eventually)

/* Hit */

/*==== Support Functions ====*/

// Support function typedef for GJK collision detection
typedef void (* gs_support_func_t)(const void* collider, const gs_vqs* xform, const gs_vec3* dir, gs_vec3* out);

GS_API_DECL void gs_support_poly(const void* p, const gs_vqs* xform, const gs_vec3* dir, gs_vec3* out);
GS_API_DECL void gs_support_sphere(const void* s, const gs_vqs* xform, const gs_vec3* dir, gs_vec3* out);
GS_API_DECL void gs_support_aabb(const void* a, const gs_vqs* xform, const gs_vec3* dir, gs_vec3* out);
GS_API_DECL void gs_support_cylinder(const void* c, const gs_vqs* xform, const gs_vec3* dir, gs_vec3* out);
GS_API_DECL void gs_support_cone(const void* c, const gs_vqs* xform, const gs_vec3* dir, gs_vec3* out);
GS_API_DECL void gs_support_capsule(const void* c, const gs_vqs* xform, const gs_vec3* dir, gs_vec3* out);

/*==== GJK ====*/

#define GS_GJK_FLT_MAX FLT_MAX     // 3.40282347E+38F
#define GS_GJK_EPSILON FLT_EPSILON // 1.19209290E-07F
#define GS_GJK_MAX_ITERATIONS 64

#define GS_EPA_TOLERANCE 0.0001
#define GS_EPA_MAX_NUM_FACES 64
#define GS_EPA_MAX_NUM_LOOSE_EDGES 32
#define GS_EPA_MAX_NUM_ITERATIONS 64

typedef enum gs_gjk_dimension {
    GS_GJK_DIMENSION_2D,
    GS_GJK_DIMENSION_3D
} gs_gjk_dimension;

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
    uint32_t ct;
} gs_gjk_simplex_t;

typedef struct gs_gjk_polytope_face_t {
    gs_gjk_support_point_t points[3];
    gs_vec3 normal;
} gs_gjk_polytope_face_t;

typedef struct gs_gjk_epa_edge_t {
    gs_vec3 normal;
    uint32_t index;
    float distance;
    gs_gjk_support_point_t a, b;
} gs_gjk_epa_edge_t;

// GS_API_DECL int32_t gs_gjk(const gs_gjk_collider_info_t* ci0, const gs_gjk_collider_info_t* ci1, gs_gjk_dimension dimension, gs_contact_info_t* res);
// GS_API_DECL gs_contact_info_t gs_gjk_epa(const gs_gjk_simplex_t* simplex, const gs_gjk_collider_info_t* ci0, const gs_gjk_collider_info_t* ci1);
// GS_API_DECL gs_contact_info_t gs_gjk_epa_2d(const gs_gjk_simplex_t* simplex, const gs_gjk_collider_info_t* ci0, const gs_gjk_collider_info_t* ci1);
// GS_API_DECL gs_gjk_collider_info_t gs_gjk_collider_info(void* c, gs_support_func_t f, gs_phys_xform_t* t);
//
GS_API_PRIVATE int32_t _gs_ccd_gjk_internal(const void* c0, const gs_vqs* xform_a, gs_support_func_t f0, const void* c1, const gs_vqs* xform_b, gs_support_func_t f1, gs_contact_info_t* res);

/*==== CCD ====*/

#ifndef GS_PHYSICS_NO_CCD

#include "../external/ccd/src/ccd/ccd_vec3.h"

// Internal collision object conversion handle
typedef struct _gs_collision_obj_handle_t 
{
    const void* obj;
    gs_support_func_t support;
    const gs_vqs* xform;
} _gs_collision_obj_handle_t;

// Internal support function for all ccd callbacks (will pass in user function through this cb), 
// this way I can keep everything consistent, only expose gs related APIs, wrap any 3rd party libs, 
// and possibly change internals in the future to custom implementations without breaking anything.
GS_API_DECL void _gs_ccd_support_func(const void* _obj, const ccd_vec3_t* _dir, ccd_vec3_t* _out);
GS_API_DECL int32_t _gs_ccd_gjk(const _gs_collision_obj_handle_t* c0, const _gs_collision_obj_handle_t* c1, gs_contact_info_t* res);

#endif // GS_PHYSICS_NO_CCD

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

/** @} */ // end of gs_physics_util

/*==== Implementation ====*/

#ifdef GS_PHYSICS_IMPL

// Includes
#ifndef GS_PHYSICS_NO_CCD
    #include "../external/ccd/libccd.c"
#endif

/*==== Support Functions =====*/

// Poly
GS_API_DECL void gs_support_poly(const void* _o, const gs_vqs* xform, const gs_vec3* dir, gs_vec3* out)
{
    const gs_poly_t* p = (gs_poly_t*)_o;

    // Bring direction vector into rotation space
    gs_quat qinv = gs_quat_inverse(xform->rotation);
    gs_vec3 d = gs_quat_rotate(qinv, *dir);

    // Iterate over all points, find dot farthest in direction of d
    double max_dot, dot = 0.0;
    max_dot = (double)-FLT_MAX;
    for (uint32_t i = 0; i < p->cnt; i++) 
    {
        dot = (double)gs_vec3_dot(d, p->verts[i]);
        if (dot > max_dot) {
            *out = p->verts[i];
            max_dot = dot;
        }
    }

    // Transform support point by rotation and translation of object
    *out = gs_quat_rotate(xform->rotation, *out);
    *out = gs_vec3_mul(xform->scale, *out);
    *out = gs_vec3_add(xform->position, *out);
}

// Sphere
GS_API_DECL void gs_support_sphere(const void* _o, const gs_vqs* xform, const gs_vec3* dir, gs_vec3* out)
{
    // Support function is made according to Gino van den Bergen's paper
    //  A Fast and Robust CCD Implementation for Collision Detection of
    //  Convex Objects
    const gs_sphere_t* s = (gs_sphere_t*)_o;
    float scl = gs_max(xform->scale.x, gs_max(xform->scale.z, xform->scale.y));
    *out = gs_vec3_add(gs_vec3_scale(gs_vec3_norm(*dir), scl * s->r), gs_vec3_add(xform->position, s->c));
}

// AABB
GS_API_DECL void gs_support_aabb(const void* _o, const gs_vqs* xform, const gs_vec3* dir, gs_vec3* out)
{
    const gs_aabb_t* a = (gs_aabb_t*)_o;

    // Bring direction vector into rotation space
    gs_quat qinv = gs_quat_inverse(xform->rotation);
    gs_vec3 d = gs_quat_rotate(qinv, *dir);

    // Compute half coordinates and sign for aabb (scale by transform)
    const float hx = (a->max.x - a->min.x) * 0.5f * xform->scale.x;
    const float hy = (a->max.y - a->min.y) * 0.5f * xform->scale.y;
    const float hz = (a->max.z - a->min.z) * 0.5f * xform->scale.z;
    gs_vec3 s = gs_vec3_sign(d);

    // Compure support for aabb
    *out = gs_v3(s.x * hx, s.y * hy, s.z * hz);

    // Transform support point by rotation and translation of object
    *out = gs_quat_rotate(xform->rotation, *out);
    *out = gs_vec3_add(xform->position, *out);
}

// Cylinder
GS_API_DECL void gs_support_cylinder(const void* _o, const gs_vqs* xform, const gs_vec3* dir, gs_vec3* out)
{
    // Support function is made according to Gino van den Bergen's paper
    //  A Fast and Robust CCD Implementation for Collision Detection of
    //  Convex Objects

    const gs_cylinder_t* c = (const gs_cylinder_t*)_o;

    // Bring direction vector into rotation space
    gs_quat qinv = gs_quat_inverse(xform->rotation);
    gs_vec3 d = gs_quat_rotate(qinv, *dir);

    // Compute support point (cylinder is standing on y axis, half height at origin)
    double zdist = sqrt(d.x * d.x + d.z * d.z);
    double hh = (double)c->height * 0.5 * xform->scale.y;
    if (zdist == 0.0) 
    {
        *out = gs_v3(0.0, gs_vec3_signY(d) * hh, 0.0);
    }
    else 
    {
        double r = (double)c->r / zdist;
        *out = gs_v3(r * d.x * xform->scale.x, gs_vec3_signY(d) * hh, r * d.z * xform->scale.z);
    }

    // Transform support point into world space
    *out = gs_quat_rotate(xform->rotation, *out);
    *out = gs_vec3_add(gs_vec3_add(xform->position, c->base), *out);
}

// Capsule
GS_API_DECL void gs_support_capsule(const void* _o, const gs_vqs* xform, const gs_vec3* dir, gs_vec3* out)
{
    const gs_capsule_t* c = (const gs_capsule_t*)_o;

    // Bring direction vector into rotation space
    gs_quat qinv = gs_quat_inverse(xform->rotation);
    gs_vec3 d = gs_quat_rotate(qinv, *dir);

    // Compute support point (cone is standing on y axis, half height at origin)
    const float s = gs_max(xform->scale.x, xform->scale.z);
    *out = gs_vec3_scale(gs_vec3_norm(d), c->r * s);
    double hh = (double)c->height * 0.5 * xform->scale.y;

    if (gs_vec3_dot(d, GS_YAXIS) > 0.0) {
        out->y += hh;
    } else {
        out->y -= hh;
    }

    // Transform support point into world space
    *out = gs_quat_rotate(xform->rotation, *out);
    *out = gs_vec3_add(gs_vec3_add(xform->position, c->base), *out);
}

// Cone
GS_API_DECL void gs_support_cone(const void* _o, const gs_vqs* xform, const gs_vec3* dir, gs_vec3* out)
{
    const gs_cone_t* c = (const gs_cone_t*)_o;

    // Bring direction vector into rotation space
    gs_quat qinv = gs_quat_inverse(xform->rotation);
    gs_vec3 d = gs_quat_rotate(qinv, *dir);

    // Compute support point (cone is standing on y axis, half height at origin)
    double sin_angle = c->r / sqrt((double)c->r * (double)c->r + (double)c->height * (double)c->height);
    double hh = (double)c->height * 0.5 * xform->scale.y;
    double len = sqrt(gs_vec3_len2(d));

    if (d.y > len * sin_angle)
    {
        *out = gs_v3(0.0f, (float)hh, 0.0f);
    }
    else
    {
        double s = sqrt(d.x * d.x + d.z * d.z);
        if (s > (double)GS_GJK_EPSILON)
        {
            double _d = (double)c->r / s;
            *out = gs_v3(d.x * _d * xform->scale.x, (float)-hh, d.z * _d * xform->scale.z);
        }
        else
        {
            *out = gs_v3(0.0, (float)-hh, 0.0);
        }
    }

    // Transform support point into world space
    *out = gs_quat_rotate(xform->rotation, *out);
    *out = gs_vec3_add(gs_vec3_add(xform->position, c->base), *out);
}

/*==== Collision Shapes ====*/

/* Line/Segment */

GS_API_DECL gs_line_t gs_line_closest_line(const gs_line_t* l, gs_vec3 p)
{
    gs_vec3 cp = gs_line_closest_point(l, p);
    return gs_line(p, cp);
}

GS_API_DECL gs_vec3 gs_line_closest_point(const gs_line_t* l, gs_vec3 p)
{
    gs_vec3 pt = gs_default_val();
    gs_vec3 ab = gs_vec3_sub(l->b, l->a);
    gs_vec3 pa = gs_vec3_sub(p, l->a);
    float t = gs_vec3_dot(pa, ab) / gs_vec3_dot(ab, ab); 
    t = gs_clamp(t, 0.f, 1.f);
    pt = gs_vec3_add(l->a, gs_vec3_scale(ab, t));
    return pt;
}

GS_API_DECL gs_vec3 gs_line_direction(const gs_line_t* l)
{
    return gs_vec3_norm(gs_vec3_sub(l->b, l->a)); 
}

/* Plane */

// Modified from: https://graphics.stanford.edu/~mdfisher/Code/Engine/Plane.cpp.html
GS_API_DECL gs_plane_t gs_plane_from_pt_normal(gs_vec3 pt, gs_vec3 n)
{
    gs_plane_t p = gs_default_val();
    gs_vec3 nn = gs_vec3_norm(n);
    p.a = nn.x; p.b = nn.y; p.c = nn.z;
    p.d = -gs_vec3_dot(pt, nn);
    return p;
}

GS_API_DECL gs_plane_t gs_plane_from_pts(gs_vec3 a, gs_vec3 b, gs_vec3 c)
{
    gs_vec3 n = gs_vec3_norm(gs_vec3_cross(gs_vec3_sub(b, a), gs_vec3_sub(c, a)));
    return gs_plane_from_pt_normal(a, n);
}

GS_API_DECL gs_vec3 gs_plane_normal(const gs_plane_t* p)
{
    return gs_vec3_norm(gs_v3(p->a, p->b, p->c));
}

GS_API_DECL gs_vec3 gs_plane_closest_point(const gs_plane_t* p, gs_vec3 pt)
{
    return gs_vec3_sub(pt, gs_vec3_scale(gs_plane_normal(p), gs_plane_signed_distance(p, pt)));
}

GS_API_DECL float gs_plane_signed_distance(const gs_plane_t* p, gs_vec3 pt)
{
    return (p->a * pt.x + p->b * pt.y + p->c * pt.z + p->d);
}

GS_API_DECL float gs_plane_unsigned_distance(const gs_plane_t* p, gs_vec3 pt)
{
    return fabsf(gs_plane_signed_distance(p, pt));
}

GS_API_DECL gs_plane_t gs_plane_normalized(const gs_plane_t* p)
{
    gs_plane_t pn = gs_default_val();
    float d = sqrtf(p->a * p->a + p->b * p->b + p->c * p->c);
    pn.a = p->a / d; pn.b = p->b / d; pn.c = p->c / d; pn.d = p->d / d;
    return pn;
}

GS_API_DECL int32_t gs_plane_vs_sphere(const gs_plane_t* a, gs_vqs* xform_a, const gs_sphere_t* b, gs_vqs* xform_b, struct gs_contact_info_t* res)
{
    // Cache necesary transforms, matrices
    gs_mat4 mat = xform_a ? gs_vqs_to_mat4(xform_a) : gs_mat4_identity();
    gs_mat4 inv = xform_a ? gs_mat4_inverse(mat) : gs_mat4_identity();
    gs_vqs local = gs_vqs_relative_transform(xform_a, xform_b);

    // Transform sphere center into local cone space
    gs_vec3 sc = xform_a ? gs_mat4_mul_vec3(inv, xform_b ? gs_vec3_add(xform_b->position, b->c) : b->c) : b->c;

    // Determine closest point from sphere center to plane
    gs_vec3 cp = gs_plane_closest_point(a, sc);

    // Determine if sphere is intersecting this point
    float sb = xform_b ? gs_max(local.scale.x, gs_max(local.scale.y, local.scale.z)) : 1.f;
    gs_vec3 dir = gs_vec3_sub(cp, sc);
    gs_vec3 n = gs_vec3_norm(dir);
    float d = gs_vec3_len(dir);
    float r = sb * b->r;

    if (d > r) 
    {
        return false;
    }

    // Construct contact information
    if (res) 
    {
        res->hit = true;
        res->depth = (r - d);
        res->normal = gs_mat4_mul_vec3(mat, n); 
        res->point = gs_mat4_mul_vec3(mat, cp);
    }

    return true;    
}

/* Ray */

/* Frustum */

/* Hit */

#define _GS_COLLIDE_FUNC_IMPL(_TA, _TB, _F0, _F1)\
    GS_API_DECL int32_t gs_##_TA##_vs_##_TB(const gs_##_TA##_t* a, const gs_vqs* xa, const gs_##_TB##_t* b, const gs_vqs* xb, gs_contact_info_t* r)\
    {\
        return _gs_ccd_gjk_internal(a, xa, (_F0), b, xb, (_F1), r);\
    }

/* Sphere */

_GS_COLLIDE_FUNC_IMPL(sphere, sphere, gs_support_sphere, gs_support_sphere);      // Sphere vs. Sphere 
_GS_COLLIDE_FUNC_IMPL(sphere, cylinder, gs_support_sphere, gs_support_cylinder);  // Sphere vs. Cylinder
_GS_COLLIDE_FUNC_IMPL(sphere, cone, gs_support_sphere, gs_support_cone);          // Sphere vs. Cone
_GS_COLLIDE_FUNC_IMPL(sphere, aabb, gs_support_sphere, gs_support_aabb);          // Sphere vs. AABB
_GS_COLLIDE_FUNC_IMPL(sphere, capsule, gs_support_sphere, gs_support_capsule);    // Sphere vs. Capsule
_GS_COLLIDE_FUNC_IMPL(sphere, poly, gs_support_sphere, gs_support_poly);          // Sphere vs. Poly

/* AABB */

_GS_COLLIDE_FUNC_IMPL(aabb, aabb, gs_support_aabb, gs_support_aabb);          // AABB vs. AABB
_GS_COLLIDE_FUNC_IMPL(aabb, cylinder, gs_support_aabb, gs_support_cylinder);  // AABB vs. Cylinder
_GS_COLLIDE_FUNC_IMPL(aabb, cone, gs_support_aabb, gs_support_cone);          // AABB vs. Cone
_GS_COLLIDE_FUNC_IMPL(aabb, sphere, gs_support_aabb, gs_support_sphere);      // AABB vs. Sphere
_GS_COLLIDE_FUNC_IMPL(aabb, capsule, gs_support_aabb, gs_support_capsule);    // AABB vs. Capsule
_GS_COLLIDE_FUNC_IMPL(aabb, poly, gs_support_aabb, gs_support_poly);          // AABB vs. Poly

/* Capsule */

_GS_COLLIDE_FUNC_IMPL(capsule, capsule, gs_support_capsule, gs_support_capsule);    // Capsule vs. Capsule
_GS_COLLIDE_FUNC_IMPL(capsule, cylinder, gs_support_capsule, gs_support_cylinder);  // Capsule vs. Cylinder
_GS_COLLIDE_FUNC_IMPL(capsule, cone, gs_support_capsule, gs_support_cone);          // Capsule vs. Cone
_GS_COLLIDE_FUNC_IMPL(capsule, sphere, gs_support_capsule, gs_support_sphere);      // Capsule vs. Sphere
_GS_COLLIDE_FUNC_IMPL(capsule, aabb, gs_support_capsule, gs_support_aabb);          // Capsule vs. AABB
_GS_COLLIDE_FUNC_IMPL(capsule, poly, gs_support_capsule, gs_support_poly);          // Capsule vs. Poly

/* Poly */

_GS_COLLIDE_FUNC_IMPL(poly, poly, gs_support_poly, gs_support_poly);          // Poly vs. Poly
_GS_COLLIDE_FUNC_IMPL(poly, cylinder, gs_support_poly, gs_support_cylinder);  // Poly vs. Cylinder
_GS_COLLIDE_FUNC_IMPL(poly, cone, gs_support_poly, gs_support_cone);          // Poly vs. Cone
_GS_COLLIDE_FUNC_IMPL(poly, sphere, gs_support_poly, gs_support_sphere);      // Poly vs. Sphere
_GS_COLLIDE_FUNC_IMPL(poly, aabb, gs_support_poly, gs_support_aabb);          // Poly vs. AABB
_GS_COLLIDE_FUNC_IMPL(poly, capsule, gs_support_poly, gs_support_capsule);    // Poly vs. Capsule

/* Cylinder */

_GS_COLLIDE_FUNC_IMPL(cylinder, cylinder, gs_support_cylinder, gs_support_cylinder);  // Cylinder vs. Cylinder
_GS_COLLIDE_FUNC_IMPL(cylinder, poly, gs_support_poly, gs_support_poly);              // Cylinder vs. Poly
_GS_COLLIDE_FUNC_IMPL(cylinder, cone, gs_support_cylinder, gs_support_cone);          // Cylinder vs. Cone
_GS_COLLIDE_FUNC_IMPL(cylinder, sphere, gs_support_cylinder, gs_support_sphere);      // Cylinder vs. Sphere
_GS_COLLIDE_FUNC_IMPL(cylinder, aabb, gs_support_cylinder, gs_support_aabb);          // Cylinder vs. AABB
_GS_COLLIDE_FUNC_IMPL(cylinder, capsule, gs_support_cylinder, gs_support_capsule);    // Cylinder vs. Capsule

/* Cone */

_GS_COLLIDE_FUNC_IMPL(cone, cone, gs_support_cone, gs_support_cone);          // Cone vs. Cone
_GS_COLLIDE_FUNC_IMPL(cone, poly, gs_support_poly, gs_support_poly);          // Cone vs. Poly
_GS_COLLIDE_FUNC_IMPL(cone, cylinder, gs_support_cone, gs_support_cylinder);  // Cone vs. Cylinder
_GS_COLLIDE_FUNC_IMPL(cone, sphere, gs_support_cone, gs_support_sphere);      // Cone vs. Sphere
_GS_COLLIDE_FUNC_IMPL(cone, aabb, gs_support_cone, gs_support_aabb);          // Cone vs. AABB
_GS_COLLIDE_FUNC_IMPL(cone, capsule, gs_support_cone, gs_support_capsule);    // Cone vs. Capsule

/*==== GKJ ====*/

// Need 2D GJK/EPA impl in external (modify from chipmunk 2d)

// Internal functions
/*
bool gs_gjk_check_if_simplex_contains_origin(gs_gjk_simplex_t* simplex, gs_vec3* search_dir, gs_gjk_dimension dimension);
void gs_gjk_simplex_push(gs_gjk_simplex_t* simplex, gs_gjk_support_point_t p);
void gs_gjk_simplex_push_back(gs_gjk_simplex_t* simplex, gs_gjk_support_point_t p);
void gs_gjk_simplex_push_front(gs_gjk_simplex_t* simplex, gs_gjk_support_point_t p);
void gs_gjk_simplex_insert(gs_gjk_simplex_t* simplex, uint32_t idx, gs_gjk_support_point_t p);
void gs_gjk_bary(gs_vec3 p, gs_vec3 a, gs_vec3 b, gs_vec3 c, float* u, float* v, float* w);
// gs_gjk_epa_edge_t gs_gjk_epa_find_closest_edge(gs_gjk_simplex_t* simplex);
gs_gjk_support_point_t gs_gjk_generate_support_point(const gs_gjk_collider_info_t* ci0, const gs_gjk_collider_info_t* ci1, gs_vec3 dir);
gs_gjk_epa_edge_t gs_gjk_epa_find_closest_edge(gs_dyn_array(gs_gjk_support_point_t) polytope);

// Modified from: https://github.com/Nightmask3/Physics-Framework/blob/master/PhysicsFramework/PhysicsManager.cpp
int32_t gs_gjk(const gs_gjk_collider_info_t* ci0, const gs_gjk_collider_info_t* ci1, gs_gjk_dimension dimension, gs_contact_info_t* res)
{
    // Simplex simplex;
    gs_gjk_simplex_t simplex = gs_default_val();
    gs_vec3 search_dir = gs_v3s(1.f);
    gs_gjk_support_point_t new_pt = gs_gjk_generate_support_point(ci0, ci1, search_dir);

    // Stability check
    if (gs_vec3_dot(search_dir, new_pt.minkowski_hull_vert) >= gs_vec3_len(new_pt.minkowski_hull_vert) * 0.8f) 
    {
        // the chosen direction is invalid, will produce (0,0,0) for a subsequent direction later
        search_dir = gs_v3(0.f, 1.f, 0.f);
        new_pt = gs_gjk_generate_support_point(ci0, ci1, search_dir);
    }
    gs_gjk_simplex_push(&simplex, new_pt);

    // Invert the search direction for the next point
    search_dir = gs_vec3_neg(search_dir);

    uint32_t iterationCount = 0;

    while (true)
    {
        if (iterationCount++ >= GS_GJK_MAX_ITERATIONS) 
            return false;
        // Stability check
        // Error, for some reason the direction vector is broken
        if (gs_vec3_len(search_dir) <= 0.0001f)
            return false;

        // Add a new point to the simplex
        gs_gjk_support_point_t new_pt = gs_gjk_generate_support_point(ci0, ci1, search_dir);
        gs_gjk_simplex_push(&simplex, new_pt);

        // If projection of newly added point along the search direction has not crossed the origin,
        // the Minkowski Difference could not contain the origin, objects are not colliding
        if (gs_vec3_dot(new_pt.minkowski_hull_vert, search_dir) < 0.0f)
        {
            return false;
        }
        else
        {
            // If the new point IS past the origin, check if the simplex contains the origin, 
            // If it doesn't modify search direction to point towards to origin
            if (gs_gjk_check_if_simplex_contains_origin(&simplex, &search_dir, dimension))
            {
                // Capture collision data using EPA if requested by user
                if (res) 
                {
                    switch (dimension) {
                        case GS_GJK_DIMENSION_3D: *res = gs_gjk_epa(&simplex, ci0, ci1); break;
                        case GS_GJK_DIMENSION_2D: *res = gs_gjk_epa_2d(&simplex, ci0, ci1); break;
                    }
                    return res->hit;
                } 
                else 
                {
                    return true;
                }
            }
        }
    }
}

GS_API_DECL gs_gjk_support_point_t gs_gjk_generate_support_point(const gs_gjk_collider_info_t* ci0, const gs_gjk_collider_info_t* ci1, gs_vec3 dir)
{
   gs_gjk_support_point_t sp = {0}; 
   sp.support_a = ci0->func(ci0->collider, ci0->xform, dir);
   sp.support_b = ci1->func(ci1->collider, ci1->xform, gs_vec3_neg(dir));
   sp.minkowski_hull_vert = gs_vec3_sub(sp.support_a, sp.support_b);
   return sp;
}

// Closest point method taken from Erin Catto's GDC 2010 slides
// Returns the closest point
gs_vec3 gs_closest_point_on_line_from_target_point(gs_line_t line, gs_vec3 point, float* u, float* v)
{
    gs_vec3 line_seg = gs_vec3_sub(line.b, line.a);
    gs_vec3 normalized = gs_vec3_norm(line_seg);
    *v = gs_vec3_dot(gs_vec3_neg(line.a), normalized) / gs_vec3_len(line_seg);
    *u = gs_vec3_dot(line.b, normalized) / gs_vec3_len(line_seg);
    gs_vec3 closest_point;
    if (*u <= 0.0f)
    {
        closest_point = line.b;
    }
    else if (*v <= 0.0f)
    {
        closest_point = line.a;
    }
    else
    {
        closest_point = gs_vec3_add(gs_vec3_scale(line.a, *u), gs_vec3_scale(line.b, *v));
    }

    return closest_point;
}

void gs_gjk_simplex_push(gs_gjk_simplex_t* simplex, gs_gjk_support_point_t p)
{
    simplex->ct = gs_min(simplex->ct + 1, 4);

    for (int32_t i = simplex->ct - 1; i > 0; i--)
        simplex->points[i] = simplex->points[i - 1];

    simplex->points[0] = p;
}

void gs_gjk_simplex_push_back(gs_gjk_simplex_t* simplex, gs_gjk_support_point_t p)
{
    if (simplex->ct >= 4) return;
    simplex->ct = gs_min(simplex->ct + 1, 4);
    simplex->points[simplex->ct - 1] = p;
}

void gs_gjk_simplex_push_front(gs_gjk_simplex_t* simplex, gs_gjk_support_point_t p)
{
    if (simplex->ct == 3) {
        simplex->points[3] = simplex->points[2];
        simplex->points[2] = simplex->points[1];
        simplex->points[1] = simplex->points[0];
        simplex->points[0] = p;
    }
    else if (simplex->ct == 2) {
        simplex->points[2] = simplex->points[1];
        simplex->points[1] = simplex->points[0];
        simplex->points[0] = p;
    }
    simplex->ct = gs_min(simplex->ct + 1, 4);
}

void gs_gjk_simplex_insert(gs_gjk_simplex_t* simplex, uint32_t idx, gs_gjk_support_point_t p)
{
    // Need more points (this is where polytope comes into play, I think...)
    // Splice the simplex by index

    if (idx > 4) return;

    simplex->ct = gs_min(simplex->ct + 1, 4);

    for (int32_t i = simplex->ct - 1; i > idx; i--)
        simplex->points[i] = simplex->points[i - 1];

    simplex->points[idx] = p;
}

void gs_gjk_simplex_clear(gs_gjk_simplex_t* simplex)
{
    simplex->ct = 0;
}

void gs_gjk_simplex_set1(gs_gjk_simplex_t* simplex, gs_gjk_support_point_t a)
{
    simplex->ct = 1;
    simplex->a = a;
}

void gs_gjk_simplex_set2(gs_gjk_simplex_t* simplex, gs_gjk_support_point_t a, gs_gjk_support_point_t b)
{
    simplex->ct = 2;
    simplex->a = a;
    simplex->b = b;
}

void gs_gjk_simplex_set3(gs_gjk_simplex_t* simplex, gs_gjk_support_point_t a, gs_gjk_support_point_t b, gs_gjk_support_point_t c)
{
    simplex->ct = 3;
    simplex->a = a;
    simplex->b = b;
    simplex->c = c;
}

void gs_gjk_simplex_set4(gs_gjk_simplex_t* simplex, gs_gjk_support_point_t a, gs_gjk_support_point_t b, gs_gjk_support_point_t c, gs_gjk_support_point_t d)
{
    simplex->ct = 4;
    simplex->a = a;
    simplex->b = b;
    simplex->c = c;
    simplex->d = d;
}

bool gs_gjk_check_if_simplex_contains_origin(gs_gjk_simplex_t* simplex, gs_vec3* search_dir, gs_gjk_dimension dimension)
{
    // Line case
    if (simplex->ct == 2)
    {
        // Line cannot contain the origin, only search for the direction to it 
        gs_vec3 new_point_to_old_point = gs_vec3_sub(simplex->b.minkowski_hull_vert, simplex->a.minkowski_hull_vert);
        gs_vec3 new_point_to_origin = gs_vec3_neg(simplex->a.minkowski_hull_vert);

        // Method given by Erin Catto in GDC 2010 presentation
        float u = 0.0f, v = 0.0f;
        gs_line_t line = gs_line(simplex->a.minkowski_hull_vert, simplex->b.minkowski_hull_vert);
        gs_vec3 origin = gs_v3s(0.f);
        gs_vec3 closest_point = gs_closest_point_on_line_from_target_point(line, origin, &u, &v);
    
        // Test vertex region of new simplex point first as highest chance to be there
        if (v <= 0.0f)
        {
            gs_gjk_simplex_set1(simplex, simplex->a);
            *search_dir = gs_vec3_neg(closest_point);
            return false;
        }
        else if (u <= 0.0f)
        {
            gs_gjk_simplex_set1(simplex, simplex->b);
            *search_dir = gs_vec3_neg(closest_point);
            return false;
        }
        else
        {
            *search_dir = gs_vec3_neg(closest_point);
            return false;
        }
    }

    // Triangle case
    else if (simplex->ct == 3)
    {
        // Find the newly added features
        gs_vec3 new_point_to_origin = gs_vec3_neg(simplex->a.minkowski_hull_vert);
        gs_vec3 edge1 = gs_vec3_sub(simplex->b.minkowski_hull_vert, simplex->a.minkowski_hull_vert);    // AB
        gs_vec3 edge2 = gs_vec3_sub(simplex->c.minkowski_hull_vert, simplex->a.minkowski_hull_vert);    // AC
        // Find the normals to the triangle and the two edges
        gs_vec3 triangle_normal = gs_vec3_cross(edge1, edge2);  // ABC
        gs_vec3 edge1_normal = gs_vec3_cross(edge1, triangle_normal);
        gs_vec3 edge2_normal = gs_vec3_cross(triangle_normal, edge2);

        // If origin is closer to the area along the second edge normal (if same_dir(AB X ABC, -A))
        if (gs_vec3_dot(edge2_normal, new_point_to_origin) > 0.0f)
        {
            // If closer to the edge then find the normal that points towards the origin
            if (gs_vec3_dot(edge2, new_point_to_origin) > 0.0f)
            {
                // [AC]
                *search_dir = gs_vec3_triple_cross_product(edge2, new_point_to_origin, edge2);
                gs_gjk_simplex_clear(simplex);
                gs_gjk_simplex_set2(simplex, simplex->a, simplex->c);
                return false;
            }
            // If closer to the new simplex point 
            else
            {
                // The "Star case" from the Muratori lecture
                // If new search direction should be along edge normal 
                if (gs_vec3_dot(edge1, new_point_to_origin) > 0.0f)
                {
                    // [AB]
                    *search_dir = gs_vec3_triple_cross_product(edge1, new_point_to_origin, edge1);
                    gs_gjk_simplex_clear(simplex);
                    gs_gjk_simplex_set2(simplex, simplex->a, simplex->b);
                    return false;
                }
                else // If new search direction should be along the new Simplex point
                {
                    // Return new simplex point alone [A]
                    *search_dir = new_point_to_origin;
                    gs_gjk_simplex_clear(simplex);
                    gs_gjk_simplex_set1(simplex, simplex->a);
                    return false;
                }
            }
        }
        // In 2D, this is a "success" case, otherwise keep going for 3D
        else
        {
            // Max simplex dimension is a triangle
            if (dimension == GS_GJK_DIMENSION_2D) 
            {
                return true;
            }

            // The "Star case" from the Muratori lecture
            // If closer to the first edge normal
            if (gs_vec3_dot(edge1_normal, new_point_to_origin) > 0.0f)
            {
                // If new search direction should be along edge normal
                if (gs_vec3_dot(edge1, new_point_to_origin) > 0.0f)
                {
                    // Return it as [A, B]
                    *search_dir = gs_vec3_triple_cross_product(edge1, new_point_to_origin, edge1);
                    gs_gjk_simplex_clear(simplex);
                    gs_gjk_simplex_set2(simplex, simplex->a, simplex->b);
                    return false;
                }
                else // If new search direction should be along the new Simplex point
                {
                    // Return new simplex point alone [A]
                    *search_dir = new_point_to_origin;
                    gs_gjk_simplex_clear(simplex);
                    gs_gjk_simplex_set1(simplex, simplex->a);
                    return false;
                }
            }
            else
            {
                // Check if it is above the triangle
                if (gs_vec3_dot(triangle_normal, new_point_to_origin) > 0.0f)
                {
                    // No need to change the simplex, return as [A, B, C]
                    *search_dir = triangle_normal;
                    return false;
                }
                else // Has to be below the triangle, all 4 other possible regions checked
                {
                    // Return simplex as [A, C, B]
                    *search_dir = gs_vec3_neg(triangle_normal);
                    gs_gjk_simplex_set3(simplex, simplex->a, simplex->c, simplex->b);
                    return false;
                }
            }
        }
    }
    // Tetrahedron for 3D case
    else if (simplex->ct == 4)
    {
        // the simplex is a tetrahedron, must check if it is outside any of the side triangles,
        // if it is then set the simplex equal to that triangle and continue, otherwise we know
        // there is an intersection and exit

        gs_vec3 edge1 = gs_vec3_sub(simplex->b.minkowski_hull_vert, simplex->a.minkowski_hull_vert);
        gs_vec3 edge2 = gs_vec3_sub(simplex->c.minkowski_hull_vert, simplex->a.minkowski_hull_vert);
        gs_vec3 edge3 = gs_vec3_sub(simplex->d.minkowski_hull_vert, simplex->a.minkowski_hull_vert);
        
        gs_vec3 face1_normal = gs_vec3_cross(edge1, edge2);
        gs_vec3 face2_normal = gs_vec3_cross(edge2, edge3);
        gs_vec3 face3_normal = gs_vec3_cross(edge3, edge1);
        
        gs_vec3 new_point_to_origin = gs_vec3_neg(simplex->a.minkowski_hull_vert);

        bool contains = true;
        if (gs_vec3_dot(face1_normal, new_point_to_origin) > 0.0f)
        {
            // Origin is in front of first face, simplex is correct already
            // goto tag;
            contains = false;
        }

        if (!contains && gs_vec3_dot(face2_normal, new_point_to_origin) > 0.0f)
        {
            // Origin is in front of second face, simplex is set to this triangle [A, C, D]
            gs_gjk_simplex_clear(simplex);
            gs_gjk_simplex_set3(simplex, simplex->a, simplex->c, simplex->d);
            contains = false;
        }

        if (!contains && gs_vec3_dot(face3_normal, new_point_to_origin) > 0.0f)
        {
            // Origin is in front of third face, simplex is set to this triangle [A, D, B]
            gs_gjk_simplex_clear(simplex);
            gs_gjk_simplex_set3(simplex, simplex->a, simplex->d, simplex->b);
            contains = false;
        }

        // If reached here it means the simplex MUST contain the origin, intersection confirmed
        if (contains) {
            return true;
        }

        gs_vec3 edge1_normal = gs_vec3_cross(edge1, face1_normal);
        if (gs_vec3_dot(edge1_normal, new_point_to_origin) > 0.0f)
        {
            // Origin is along the normal of edge1, set the simplex to that edge [A, B]
            *search_dir = gs_vec3_triple_cross_product(edge1, new_point_to_origin, edge1);
            gs_gjk_simplex_clear(simplex);
            gs_gjk_simplex_set2(simplex, simplex->a, simplex->b);
            return false;
        }

        gs_vec3 edge2Normal = gs_vec3_cross(face1_normal, edge2);
        if (gs_vec3_dot(edge2Normal, new_point_to_origin) > 0.0f)
        {
            // Origin is along the normal of edge2, set the simplex to that edge [A, C]
            *search_dir = gs_vec3_triple_cross_product(edge2, new_point_to_origin, edge2);
            gs_gjk_simplex_clear(simplex);
            gs_gjk_simplex_set2(simplex, simplex->a, simplex->c);
            return false;
        }

        // If reached here the origin is along the first face normal, set the simplex to this face [A, B, C]
        *search_dir = face1_normal;
        gs_gjk_simplex_clear(simplex);
        gs_gjk_simplex_set3(simplex, simplex->a, simplex->b, simplex->c);
        return false;
    }
    return false;
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
GS_API_DECL gs_contact_info_t gs_gjk_epa(
    const gs_gjk_simplex_t* simplex, 
    const gs_gjk_collider_info_t* ci0, 
    const gs_gjk_collider_info_t* ci1
)
{
    gs_contact_info_t res = {0};

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

// Expanding Polytope Algorithm 2D
GS_API_DECL gs_contact_info_t gs_gjk_epa_2d(
    const gs_gjk_simplex_t* simplex, 
    const gs_gjk_collider_info_t* ci0, 
    const gs_gjk_collider_info_t* ci1
)
{
    gs_dyn_array(gs_gjk_support_point_t) polytope = NULL;
    gs_contact_info_t res = gs_default_val();
    gs_gjk_support_point_t p = gs_default_val();

    // Copy over simplex into polytope array
    for (uint32_t i = 0; i < simplex->ct; ++i) {
        gs_dyn_array_push(polytope, simplex->points[i]);
    }

    // p = gs_gjk_generate_support_point(ci0, ci1, gs_v3s(1.f));
    // gs_gjk_simplex_push_front(simplex, p);
    gs_gjk_epa_edge_t e = gs_default_val();
    uint32_t iterations = 0;
    while (iterations < GS_EPA_MAX_NUM_ITERATIONS)
    {
        // Find closest edge to origin from simplex
        e = gs_gjk_epa_find_closest_edge(polytope);
        p = gs_gjk_generate_support_point(ci0, ci1, e.normal);

        double d = (double)gs_vec3_dot(p.minkowski_hull_vert, e.normal);

        // Success, calculate results
        if (d - e.distance < GS_EPA_TOLERANCE)
        {
            // Normal and depth information
            res.normal = gs_vec3_norm(e.normal);
            res.depth = d;
            break;
        }
        else
        {
            // Need to think about this more. This is fucked.
            // Need an "expanding" simplex, that only allows for unique points to be included.
            // This is just overwriting. I need to actually insert then push back.
            // gs_gjk_simplex_insert(simplex, e.index + 1, p);
            // Insert into polytope array at idx + 1
            for (uint32_t i = 0; i < gs_dyn_array_size(polytope); ++i)
            {
               gs_vec3* p = &polytope[i].minkowski_hull_vert; 
               gs_printf("<%.2f, %.2f>, ", p->x, p->y);
            }
            gs_dyn_array_push(polytope, p);

            for (int32_t i = gs_dyn_array_size(polytope) - 1; i > e.index + 1; i--)
                polytope[i] = polytope[i - 1];

            polytope[e.index + 1] = p;

            gs_println("pts after: ");
            for (uint32_t i = 0; i < gs_dyn_array_size(polytope); ++i)
            {
               gs_vec3* p = &polytope[i].minkowski_hull_vert; 
               gs_printf("<%.2f, %.2f>, ", p->x, p->y);
            }
        }

        // Increment iterations
        iterations++;
    }

    // gs_vec3 line_vec = gs_vec3_sub(e.a.minkowski_hull_vert, e.b.minkowski_hull_vert);
    // gs_vec3 projO = gs_vec3_scale(gs_vec3_scale(line_vec, 1.f / gs_vec3_len2(line_vec)), gs_vec3_dot(line_vec, gs_vec3_neg(e.b.minkowski_hull_vert)));
    // float s, t;
    // s = sqrt(gs_vec3_len2(projO) / gs_vec3_len2(line_vec));
    // t = 1.f - s;
    // int32_t next_idx = (e.index + 1) % simplex->ct;
    res.hit = true;
    // res.points[0] = gs_vec3_add(gs_vec3_scale(simplex->points[e.index].support_a, s), gs_vec3_scale(simplex->points[next_idx].support_a, t));
    // res.points[1] = gs_vec3_add(gs_vec3_scale(simplex->points[e.index].support_b, s), gs_vec3_scale(simplex->points[next_idx].support_b, t));

    return res;

}

gs_gjk_epa_edge_t gs_gjk_epa_find_closest_edge(gs_dyn_array(gs_gjk_support_point_t) polytope)
{
   gs_gjk_epa_edge_t res = gs_default_val();
   float min_dist = FLT_MAX;
   uint32_t min_index = 0;
   gs_vec3 min_normal = gs_default_val();
   for (uint32_t i = 0; i < gs_dyn_array_size(polytope); ++i)
   {
        uint32_t j = (i + 1) % gs_dyn_array_size(polytope);
        gs_gjk_support_point_t* a = &polytope[0];
        gs_gjk_support_point_t* b = &polytope[1];
        gs_vec3 dir = gs_vec3_sub(b->minkowski_hull_vert, a->minkowski_hull_vert);
        gs_vec3 norm = gs_vec3_norm(gs_v3(dir.y, -dir.x, 0.f));
        float dist = gs_vec3_dot(norm, a->minkowski_hull_vert);

        // If distance is negative, then we need to correct for winding order mistakes
        if (dist < 0) {
            dist *= -1;
            norm = gs_vec3_neg(norm);
        }

        // Check for min distance
        if (dist < min_dist) {
            min_dist = dist;
            min_normal = norm;
            min_index = j;
        }
   }
   res.index = min_index;
   res.normal = min_normal;
   res.distance = min_dist;
   return res;
}

gs_gjk_epa_edge_t gs_gjk_epa_find_closest_edge2(gs_gjk_simplex_t* simplex)
{
    gs_gjk_epa_edge_t result = gs_default_val();
    uint32_t next_idx = 0;
    float min_dist = FLT_MAX, curr_dist = 0.f;
    gs_vec3 norm, edge;
    gs_vec3 norm_3d;

    for (int32_t i = 0; i < simplex->ct; ++i)
    {
        next_idx = (i + 1) % simplex->ct;
        edge = gs_vec3_sub(simplex->points[next_idx].minkowski_hull_vert, simplex->points[i].minkowski_hull_vert);
        norm_3d = gs_vec3_triple_cross_product(gs_v3(edge.x, edge.y, 0), 
            gs_v3(simplex->points[i].minkowski_hull_vert.x, simplex->points[i].minkowski_hull_vert.y, 0.f), 
            gs_v3(edge.x, edge.y, 0.f));
        norm.x = norm_3d.x;
        norm.y = norm_3d.y;

        norm = gs_vec3_norm(norm);
        curr_dist = gs_vec3_dot(norm, simplex->points[i].minkowski_hull_vert);
        if (curr_dist < min_dist)
        { 
            min_dist = curr_dist;
            result.a = simplex->points[i];
            result.b = simplex->points[next_idx];
            result.normal = norm;
            result.distance = curr_dist;
            result.index = i;
        }
    }

    return result;
}
*/

/*===== CCD ======*/

// Useful CCD conversions
void _gs_ccdv32gsv3(const ccd_vec3_t* _in, gs_vec3* _out)
{
    // Safe check against NaNs
    if (gs_is_nan(_in->v[0]) || gs_is_nan(_in->v[1]) || gs_is_nan(_in->v[2])) return;
    *_out = gs_ctor(gs_vec3, (float)_in->v[0], (float)_in->v[1], (float)_in->v[2]);
}

void _gs_gsv32ccdv3(const gs_vec3* _in, ccd_vec3_t* _out)
{
    ccdVec3Set(_out, CCD_REAL(_in->x), CCD_REAL(_in->y), CCD_REAL(_in->z));
}

GS_API_PRIVATE int32_t _gs_ccd_gjk_internal(
    const void* c0, const gs_vqs* xform_a, gs_support_func_t f0,
    const void* c1, const gs_vqs* xform_b, gs_support_func_t f1,
    gs_contact_info_t* res

)
{
    // Convert to appropriate gjk internals, then call ccd
    ccd_t ccd = gs_default_val();
    CCD_INIT(&ccd);

    // set up ccd_t struct
    ccd.support1       = _gs_ccd_support_func;  // support function for first object
    ccd.support2       = _gs_ccd_support_func;  // support function for second object
    ccd.max_iterations = 100;                   // maximal number of iterations
    ccd.epa_tolerance  = 0.0001;                // maximal tolerance for epa to succeed

    // Default transforms
    gs_vqs _xa = gs_vqs_default(), _xb = gs_vqs_default();

    // Collision object 1
    _gs_collision_obj_handle_t h0 = gs_default_val();
    h0.support = f0;
    h0.obj = c0;
    h0.xform = xform_a ? xform_a : &_xa;

    // Collision object 2
    _gs_collision_obj_handle_t h1 = gs_default_val();
    h1.support = f1;
    h1.obj = c1;
    h1.xform = xform_b ? xform_b : &_xb;

    // Call ccd, cache results into res for user
    ccd_real_t depth = CCD_REAL(0.0);
    ccd_vec3_t n = gs_ctor(ccd_vec3_t, 0.f, 0.f, 0.f), p = gs_ctor(ccd_vec3_t, 0.f, 0.f, 0.f);
    int32_t r = ccdGJKPenetration(&h0, &h1, &ccd, &depth, &n, &p);
    bool32 hit = r >= 0 && !gs_is_nan(n.v[0]) && !gs_is_nan(n.v[1]) && !gs_is_nan(n.v[2]);

    if (hit && res)
    {
        res->hit = true;
        res->depth = (float)depth;
        _gs_ccdv32gsv3(&p, &res->point);
        _gs_ccdv32gsv3(&n, &res->normal);
    }

    return r;
}

// typedef void (*ccd_support_fn)(const void *obj, const ccd_vec3_t *dir,
//                                ccd_vec3_t *vec);

// Internal support function for gs -> ccd
GS_API_DECL void _gs_ccd_support_func(const void* _obj, const ccd_vec3_t* _dir, ccd_vec3_t* _out)
{
    const _gs_collision_obj_handle_t* obj = (const _gs_collision_obj_handle_t*)_obj;
    if (obj->support) 
    {
        // Temp copy conversion for direction vector
        gs_vec3 dir = gs_default_val(), out = gs_default_val();
        _gs_ccdv32gsv3((const ccd_vec3_t*)_dir, &dir);

        // Call user provided support function
        // Think I found it...
        obj->support(obj->obj, obj->xform, &dir, &out);

        // Copy over out result for ccd
        _gs_gsv32ccdv3(&out, (ccd_vec3_t*)_out);
    }
}

#endif // GS_PHYSICS_IMPL
#endif // GS_PHYSICS_H


























