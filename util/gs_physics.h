
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

/** @defgroup gs_physics_util Physics Util
 *  Gunslinger Physics Util
 *  @{
 */

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

// 3D shapes
typedef struct gs_line_t     {gs_vec3 a, b;                                                  } gs_line_t;
typedef struct gs_aabb_t     {gs_vec3 min, max;                                              } gs_aabb_t;
typedef struct gs_sphere_t   {gs_vec3 c; float r;                                            } gs_sphere_t;
typedef struct gs_plane_t    {gs_vec3 p, n;                                                  } gs_plane_t;
typedef struct gs_capsule_t  {gs_vec3 base; float r, height;                                 } gs_capsule_t;
typedef struct gs_ray_t      {gs_vec3 p, d;                                                  } gs_ray_t;
typedef struct gs_poly_t     {gs_vec3* verts; int32_t cnt;                                   } gs_poly_t;
typedef union  gs_frustum_t  {struct {gs_vec4 l, r, t, b, n, f;}; gs_vec4 pl[6]; float v[24];} gs_frustum_t;
typedef struct gs_cylinder_t {float r; gs_vec3 base; float height;                           } gs_cylinder_t; 
typedef struct gs_cone_t     {float r; gs_vec3 base; float height;                           } gs_cone_t;

// 2D shapes
typedef struct gs_quad_t     {gs_vec2 min; gs_vec2 max;                                      } gs_quad_t;
typedef struct gs_circle_t   {float r; gs_vec2 c;                                            } gs_circle_t;
typedef struct gs_triangle_t {gs_vec2 a,b,c;                                                 } gs_triangle_t;
typedef struct gs_pill_t     {gs_vec2 base; float r, height;                                 } gs_pill_t;

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
#define gs_ray(...)      gs_ctor(gs_ray_t, __VA_ARGS__)
#define gs_poly(...)     gs_ctor(gs_poly_t, __VA_ARGS__)
#define gs_frustum(...). gs_ctor(gs_frustum_t, __VA_ARGS__)
#define gs_cylinder(...) gs_ctor(gs_cylinder_t, __VA_ARGS__)
#define gs_cone(...)     gs_ctor(gs_cone_t, __VA_ARGS__)
#define gs_hit(...)      gs_ctor(gs_hit_t, __VA_ARGS__)
#define gs_pill(...)     gs_ctor(gs_pill_t, __VA_ARGS__)
#define gs_quad(...)     gs_ctor(gs_quad_t, __VA_ARGS__)
#define gs_triangle(...) gs_ctor(gs_triangle_t, __VA_ARGS__)
#define gs_circle(...)   gs_ctor(gs_circle_t, __VA_ARGS__)

// Forward Decl. 
struct gs_gjk_result_t;
struct gs_gjk_epa_result_t;
struct gs_gjk_contact_info_t;

/* Line/Segment */
GS_API_DECL float gs_line_closest_line(float* t1, float* t2, gs_vec3* c1, gs_vec3* c2, gs_line_t l, gs_line_t m);

/* Sphere */
GS_API_DECL int32_t gs_sphere_vs_sphere(const gs_sphere_t* a, gs_vqs* xform_a, const gs_sphere_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_sphere_vs_aabb(const gs_sphere_t* a, gs_vqs* xform_a, const gs_aabb_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_sphere_vs_poly(const gs_sphere_t* a, gs_vqs* xform_a, const gs_poly_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_sphere_vs_cylinder(const gs_sphere_t* a, gs_vqs* xform_a, const gs_cylinder_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_sphere_vs_cone(const gs_sphere_t* a, gs_vqs* xform_a, const gs_cone_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_sphere_vs_capsule(const gs_sphere_t* a, gs_vqs* xform_a, const gs_capsule_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_sphere_vs_quad(const gs_sphere_t* a, gs_vqs* xform_a, const gs_quad_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_sphere_vs_triangle(const gs_sphere_t* a, gs_vqs* xform_a, const gs_triangle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_sphere_vs_circle(const gs_sphere_t* a, gs_vqs* xform_a, const gs_circle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_sphere_vs_pill(const gs_sphere_t* a, gs_vqs* xform_a, const gs_pill_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);

/* Box */
GS_API_DECL int32_t gs_aabb_vs_aabb(const gs_aabb_t* a, gs_vqs* xform_a, const gs_aabb_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_aabb_vs_sphere(const gs_aabb_t* a, gs_vqs* xform_a, const gs_sphere_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_aabb_vs_poly(const gs_aabb_t* a, gs_vqs* xform_a, const gs_poly_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_aabb_vs_cylinder(const gs_aabb_t* a, gs_vqs* xform_a, const gs_cylinder_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_aabb_vs_cone(const gs_aabb_t* a, gs_vqs* xform_a, const gs_cone_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_aabb_vs_capsule(const gs_aabb_t* a, gs_vqs* xform_a, const gs_capsule_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_aabb_vs_quad(const gs_aabb_t* a, gs_vqs* xform_a, const gs_quad_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_aabb_vs_triangle(const gs_aabb_t* a, gs_vqs* xform_a, const gs_triangle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_aabb_vs_circle(const gs_aabb_t* a, gs_vqs* xform_a, const gs_circle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_aabb_vs_pill(const gs_aabb_t* a, gs_vqs* xform_a, const gs_pill_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);

/* Plane */

/* Capsule */
GS_API_DECL int32_t gs_capsule_vs_aabb(const gs_capsule_t* capsule, gs_vqs* xform_a, const gs_aabb_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_capsule_vs_sphere(const gs_capsule_t* capsule, gs_vqs* xform_a, const gs_sphere_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_capsule_vs_poly(const gs_capsule_t* capsule, gs_vqs* xform_a, const gs_poly_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_capsule_vs_cylinder(const gs_capsule_t* capsule, gs_vqs* xform_a, const gs_cylinder_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_capsule_vs_cone(const gs_capsule_t* capsule, gs_vqs* xform_a, const gs_cone_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_capsule_vs_capsule(const gs_capsule_t* capsule, gs_vqs* xform_a, const gs_capsule_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_capsule_vs_quad(const gs_capsule_t* capsule, gs_vqs* xform_a, const gs_quad_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_capsule_vs_triangle(const gs_capsule_t* capsule, gs_vqs* xform_a, const gs_triangle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_capsule_vs_circle(const gs_capsule_t* capsule, gs_vqs* xform_a, const gs_circle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_capsule_vs_pill(const gs_capsule_t* capsule, gs_vqs* xform_a, const gs_pill_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);

/* Ray */

/* Poly */
GS_API_DECL int32_t gs_poly_vs_poly(const gs_poly_t* a, gs_vqs* xform_a, const gs_poly_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_poly_vs_sphere(const gs_poly_t* a, gs_vqs* xform_a, const gs_sphere_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_poly_vs_aabb(const gs_poly_t* a, gs_vqs* xform_a, const gs_aabb_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_poly_vs_cylinder(const gs_poly_t* a, gs_vqs* xform_a, const gs_cylinder_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_poly_vs_cone(const gs_poly_t* a, gs_vqs* xform_a, const gs_cone_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_poly_vs_capsule(const gs_poly_t* a, gs_vqs* xform_a, const gs_capsule_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_poly_vs_quad(const gs_poly_t* a, gs_vqs* xform_a, const gs_quad_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_poly_vs_triangle(const gs_poly_t* a, gs_vqs* xform_a, const gs_triangle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_poly_vs_circle(const gs_poly_t* a, gs_vqs* xform_a, const gs_circle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_poly_vs_pill(const gs_poly_t* a, gs_vqs* xform_a, const gs_pill_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);

/* Frustum */

/* Cylinder */
GS_API_DECL int32_t gs_cylinder_vs_cylinder(const gs_cylinder_t* a, gs_vqs* xform_a, const gs_cylinder_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_cylinder_vs_sphere(const gs_cylinder_t* a, gs_vqs* xform_a, const gs_sphere_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_cylinder_vs_aabb(const gs_cylinder_t* a, gs_vqs* xform_a, const gs_aabb_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_cylinder_vs_poly(const gs_cylinder_t* a, gs_vqs* xform_a, const gs_poly_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_cylinder_vs_cone(const gs_cylinder_t* a, gs_vqs* xform_a, const gs_cone_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_cylinder_vs_capsule(const gs_cylinder_t* a, gs_vqs* xform_a, const gs_capsule_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_cylinder_vs_quad(const gs_cylinder_t* a, gs_vqs* xform_a, const gs_quad_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_cylinder_vs_triangle(const gs_cylinder_t* a, gs_vqs* xform_a, const gs_triangle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_cylinder_vs_circle(const gs_cylinder_t* a, gs_vqs* xform_a, const gs_circle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_cylinder_vs_pill(const gs_cylinder_t* a, gs_vqs* xform_a, const gs_pill_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);

/* Cone */
GS_API_DECL int32_t gs_cone_vs_cone(const gs_cone_t* a, gs_vqs* xform_a, const gs_cone_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_cone_vs_sphere(const gs_cone_t* a, gs_vqs* xform_a, const gs_sphere_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_cone_vs_aabb(const gs_cone_t* a, gs_vqs* xform_a, const gs_aabb_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_cone_vs_poly(const gs_cone_t* a, gs_vqs* xform_a, const gs_poly_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_cone_vs_cylinder(const gs_cone_t* a, gs_vqs* xform_a, const gs_cylinder_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_cone_vs_capsule(const gs_cone_t* a, gs_vqs* xform_a, const gs_capsule_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_cone_vs_quad(const gs_cone_t* a, gs_vqs* xform_a, const gs_quad_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_cone_vs_triangle(const gs_cone_t* a, gs_vqs* xform_a, const gs_triangle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_cone_vs_circle(const gs_cone_t* a, gs_vqs* xform_a, const gs_circle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_cone_vs_pill(const gs_cone_t* a, gs_vqs* xform_a, const gs_pill_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);

// 2D Shapes

/* Triangle */
GS_API_DECL int32_t gs_triangle_vs_triangle(const gs_triangle_t* a, gs_vqs* xform_a, const gs_triangle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_triangle_vs_quad(const gs_triangle_t* a, gs_vqs* xform_a, const gs_quad_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_triangle_vs_sphere(const gs_triangle_t* a, gs_vqs* xform_a, const gs_sphere_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_triangle_vs_aabb(const gs_triangle_t* a, gs_vqs* xform_a, const gs_aabb_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_triangle_vs_poly(const gs_triangle_t* a, gs_vqs* xform_a, const gs_poly_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_triangle_vs_cone(const gs_triangle_t* a, gs_vqs* xform_a, const gs_cone_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_triangle_vs_cylinder(const gs_triangle_t* a, gs_vqs* xform_a, const gs_cylinder_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_triangle_vs_capsule(const gs_triangle_t* a, gs_vqs* xform_a, const gs_capsule_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_triangle_vs_triangle_2d(const gs_triangle_t* a, gs_vqs* xform_a, const gs_triangle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_triangle_vs_quad_2d(const gs_triangle_t* a, gs_vqs* xform_a, const gs_quad_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_triangle_vs_circle_2d(const gs_triangle_t* a, gs_vqs* xform_a, const gs_circle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_triangle_vs_pill_2d(const gs_triangle_t* a, gs_vqs* xform_a, const gs_pill_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);

/* Quad */
GS_API_DECL int32_t gs_quad_vs_quad(const gs_quad_t* a, gs_vqs* xform_a, const gs_quad_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_quad_vs_sphere(const gs_quad_t* a, gs_vqs* xform_a, const gs_sphere_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_quad_vs_aabb(const gs_quad_t* a, gs_vqs* xform_a, const gs_aabb_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_quad_vs_poly(const gs_quad_t* a, gs_vqs* xform_a, const gs_poly_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_quad_vs_cone(const gs_quad_t* a, gs_vqs* xform_a, const gs_cone_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_quad_vs_cylinder(const gs_quad_t* a, gs_vqs* xform_a, const gs_cylinder_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_quad_vs_capsule(const gs_quad_t* a, gs_vqs* xform_a, const gs_capsule_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_quad_vs_triangle(const gs_quad_t* a, gs_vqs* xform_a, const gs_triangle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_quad_vs_circle(const gs_quad_t* a, gs_vqs* xform_a, const gs_circle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_quad_vs_pill(const gs_quad_t* a, gs_vqs* xform_a, const gs_pill_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_quad_vs_triangle_2d(const gs_quad_t* a, gs_vqs* xform_a, const gs_triangle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_quad_vs_quad_2d(const gs_quad_t* a, gs_vqs* xform_a, const gs_quad_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_quad_vs_circle_2d(const gs_quad_t* a, gs_vqs* xform_a, const gs_circle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_quad_vs_pill_2d(const gs_quad_t* a, gs_vqs* xform_a, const gs_pill_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);

/* Circle */
GS_API_DECL int32_t gs_circle_vs_sphere(const gs_circle_t* a, gs_vqs* xform_a, const gs_sphere_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_circle_vs_aabb(const gs_circle_t* a, gs_vqs* xform_a, const gs_aabb_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_circle_vs_poly(const gs_circle_t* a, gs_vqs* xform_a, const gs_poly_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_circle_vs_cone(const gs_circle_t* a, gs_vqs* xform_a, const gs_cone_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_circle_vs_cylinder(const gs_circle_t* a, gs_vqs* xform_a, const gs_cylinder_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_circle_vs_capsule(const gs_circle_t* a, gs_vqs* xform_a, const gs_capsule_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_circle_vs_triangle(const gs_circle_t* a, gs_vqs* xform_a, const gs_triangle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_circle_vs_pill(const gs_circle_t* a, gs_vqs* xform_a, const gs_pill_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_circle_vs_circle(const gs_circle_t* a, gs_vqs* xform_a, const gs_circle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_circle_vs_quad(const gs_circle_t* a, gs_vqs* xform_a, const gs_quad_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_circle_vs_quad_2d(const gs_circle_t* a, gs_vqs* xform_a, const gs_quad_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_circle_vs_circle_2d(const gs_circle_t* a, gs_vqs* xform_a, const gs_circle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_circle_vs_triangle_2d(const gs_circle_t* a, gs_vqs* xform_a, const gs_triangle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_circle_vs_pill_2d(const gs_circle_t* a, gs_vqs* xform_a, const gs_pill_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);

/* Pill (2D Capsule) */
GS_API_DECL int32_t gs_pill_vs_sphere(const gs_pill_t* a, gs_vqs* xform_a, const gs_sphere_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_pill_vs_aabb(const gs_pill_t* a, gs_vqs* xform_a, const gs_aabb_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_pill_vs_poly(const gs_pill_t* a, gs_vqs* xform_a, const gs_poly_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_pill_vs_cone(const gs_pill_t* a, gs_vqs* xform_a, const gs_cone_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_pill_vs_cylinder(const gs_pill_t* a, gs_vqs* xform_a, const gs_cylinder_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_pill_vs_capsule(const gs_pill_t* a, gs_vqs* xform_a, const gs_capsule_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_pill_vs_triangle(const gs_pill_t* a, gs_vqs* xform_a, const gs_triangle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_pill_vs_pill(const gs_pill_t* a, gs_vqs* xform_a, const gs_pill_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_pill_vs_circle(const gs_pill_t* a, gs_vqs* xform_a, const gs_circle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_pill_vs_quad(const gs_pill_t* a, gs_vqs* xform_a, const gs_quad_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_pill_vs_quad_2d(const gs_pill_t* a, gs_vqs* xform_a, const gs_quad_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_pill_vs_circle_2d(const gs_pill_t* a, gs_vqs* xform_a, const gs_circle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_pill_vs_triangle_2d(const gs_pill_t* a, gs_vqs* xform_a, const gs_triangle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);
GS_API_DECL int32_t gs_pill_vs_pill_2d(const gs_pill_t* a, gs_vqs* xform_a, const gs_pill_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res);

/* Hit */

/*==== Support Functions ====*/

// Support function typedef for GJK collision detection
typedef gs_vec3 (* gs_gjk_support_func_t)(const void* collider, gs_vqs* xform, gs_vec3 search_dir);

GS_API_DECL gs_vec3 gs_gjk_support_poly(const gs_poly_t* p, gs_phys_xform_t* xform, gs_vec3 search_dir);
GS_API_DECL gs_vec3 gs_gjk_support_sphere(const gs_sphere_t* s, gs_phys_xform_t* xform, gs_vec3 search_dir);
GS_API_DECL gs_vec3 gs_gjk_support_aabb(const gs_aabb_t* a, gs_phys_xform_t* xform, gs_vec3 search_dir);
GS_API_DECL gs_vec3 gs_gjk_support_cylinder(const gs_cylinder_t* c, gs_phys_xform_t* xform, gs_vec3 search_dir);
GS_API_DECL gs_vec3 gs_gjk_support_cone(const gs_cone_t* c, gs_phys_xform_t* xform, gs_vec3 search_dir);
GS_API_DECL gs_vec3 gs_gjk_support_capsule(const gs_capsule_t* c, gs_phys_xform_t* xform, gs_vec3 search_dir);
GS_API_DECL gs_vec3 gs_gjk_support_quad(const gs_quad_t* c, gs_phys_xform_t* xform, gs_vec3 search_dir);
GS_API_DECL gs_vec3 gs_gjk_support_triangle(const gs_triangle_t* c, gs_phys_xform_t* xform, gs_vec3 search_dir);
GS_API_DECL gs_vec3 gs_gjk_support_circle(const gs_circle_t* c, gs_phys_xform_t* xform, gs_vec3 search_dir);
GS_API_DECL gs_vec3 gs_gjk_support_pill(const gs_pill_t* c, gs_phys_xform_t* xform, gs_vec3 search_dir);

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

typedef struct gs_gjk_contact_info_t {
    int32_t hit;
    gs_vec3 normal;
    float depth;
    gs_vec3 points[2];
} gs_gjk_contact_info_t;

typedef struct gs_gjk_epa_edge_t {
    gs_vec3 normal;
    uint32_t index;
    float distance;
    gs_gjk_support_point_t a, b;
} gs_gjk_epa_edge_t;

typedef struct gs_gjk_collider_info_t {
    const void* collider;
    gs_gjk_support_func_t func;
    gs_phys_xform_t* xform;
} gs_gjk_collider_info_t;

GS_API_DECL int32_t gs_gjk(const gs_gjk_collider_info_t* ci0, const gs_gjk_collider_info_t* ci1, gs_gjk_dimension dimension, gs_gjk_contact_info_t* res);
GS_API_DECL gs_gjk_contact_info_t gs_gjk_epa(const gs_gjk_simplex_t* simplex, const gs_gjk_collider_info_t* ci0, const gs_gjk_collider_info_t* ci1);
GS_API_DECL gs_gjk_contact_info_t gs_gjk_epa_2d(const gs_gjk_simplex_t* simplex, const gs_gjk_collider_info_t* ci0, const gs_gjk_collider_info_t* ci1);
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

/** @} */ // end of gs_physics_util

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

// Default 3d version
int32_t _gs_gjk_internal(
    void* c0, gs_vqs* xform_a, gs_gjk_support_func_t f0, 
    void* c1, gs_vqs* xform_b, gs_gjk_support_func_t f1, 
    struct gs_gjk_result_t* res
)
{
    gs_phys_xform_t x0 = xform_a ? gs_phys_xform_from_vqs(xform_a) : gs_ctor(gs_phys_xform_t);
    gs_phys_xform_t x1 = xform_b ? gs_phys_xform_from_vqs(xform_b) : gs_ctor(gs_phys_xform_t);
    gs_gjk_collider_info_t ci0 = gs_gjk_collider_info(c0, f0, xform_a ? &x0 : NULL);
    gs_gjk_collider_info_t ci1 = gs_gjk_collider_info(c1, f1, xform_b ? &x1 : NULL);
    return gs_gjk(&ci0, &ci1, GS_GJK_DIMENSION_3D, res);
}

// 2d version
int32_t _gs_gjk_internal_2d(
    void* c0, gs_vqs* xform_a, gs_gjk_support_func_t f0, 
    void* c1, gs_vqs* xform_b, gs_gjk_support_func_t f1, 
    struct gs_gjk_result_t* res
)
{
    gs_phys_xform_t x0 = xform_a ? gs_phys_xform_from_vqs(xform_a) : gs_ctor(gs_phys_xform_t);
    gs_phys_xform_t x1 = xform_b ? gs_phys_xform_from_vqs(xform_b) : gs_ctor(gs_phys_xform_t);
    gs_gjk_collider_info_t ci0 = gs_gjk_collider_info(c0, f0, xform_a ? &x0 : NULL);
    gs_gjk_collider_info_t ci1 = gs_gjk_collider_info(c1, f1, xform_b ? &x1 : NULL);
    return gs_gjk(&ci0, &ci1, GS_GJK_DIMENSION_2D, res);
}

/*==== Support Functions =====*/

// Poly

GS_API_DECL gs_vec3 gs_gjk_support_poly(const gs_poly_t* p, gs_phys_xform_t* xform, gs_vec3 dir)
{
    // Find support in object space (with inverse matrix)
    dir = xform ? gs_mat3_mul_vec3(xform->inv_rs, dir) : dir;

    // Initial furthest point
    gs_vec3 res = p->verts[0];
    float max_dot = gs_vec3_dot(res, dir);

    for (int32_t i = 1; i < p->cnt; ++i) {
        float d = gs_vec3_dot(dir, p->verts[i]);
        if (d > max_dot) {
            max_dot = d;
            res = p->verts[i];
        }
    }

    // Conver to world space
    res = xform ? gs_vec3_add(gs_mat3_mul_vec3(xform->rs, res), xform->pos) : res;

    return res;
}

// Sphere

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

// AABB

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

// Cone

#define MAX_CONE_PTS 10
GS_API_DECL gs_vec3 gs_gjk_support_cone(const gs_cone_t* c, gs_phys_xform_t* xform, gs_vec3 dir)
{
    // Disc and point for support functions, using base information
    gs_vec3 pts[MAX_CONE_PTS] = gs_default_val();
    const float step = 360.f / (float)MAX_CONE_PTS;
    for (uint32_t i = 0; i < MAX_CONE_PTS - 1; ++i) {
        float a = gs_deg2rad(step * (float)i);
        pts[i] = gs_v3(sinf(a) * c->r, c->base.y, cosf(a) * c->r);
    }
    pts[MAX_CONE_PTS - 1] = gs_vec3_add(c->base, gs_v3(0.f, c->height, 0.f));

    // Find support in object space (with inverse matrix)
    dir = xform ? gs_mat3_mul_vec3(xform->inv_rs, dir) : dir;

    // Initial furthest point
    gs_vec3 res = pts[0];
    float max_dot = gs_vec3_dot(res, dir);

    for (int32_t i = 1; i < MAX_CONE_PTS; ++i) {
        float d = gs_vec3_dot(dir, pts[i]);
        if (d > max_dot) {
            max_dot = d;
            res = pts[i];
        }
    }

    // Convert to world space
    res = xform ? gs_vec3_add(gs_mat3_mul_vec3(xform->rs, res), xform->pos) : res;

    return res;
}

// Cylinder

GS_API_DECL gs_vec3 gs_gjk_support_cylinder(const gs_cylinder_t* c, gs_phys_xform_t* xform, gs_vec3 dir)
{
    dir = xform ? gs_mat3_mul_vec3(xform->inv_rs, dir) : dir;

    gs_vec3 dir_xz = gs_v3(dir.x, 0.f, dir.z);
    gs_vec3 res = gs_vec3_scale(gs_vec3_norm(dir_xz), c->r);
    res.y = (dir.y > 0.f) ? c->base.y + c->height : c->base.y;

    // Conver to world space
    res = xform ? gs_vec3_add(gs_mat3_mul_vec3(xform->rs, res), xform->pos) : res;

    return res;
}

// Capsule

GS_API_DECL gs_vec3 gs_gjk_support_capsule(const gs_capsule_t* c, gs_phys_xform_t* xform, gs_vec3 dir)
{
    // Find search dir in object space
    dir = xform ? gs_mat3_mul_vec3(xform->inv_rs, dir) : dir;

    // Compute result
    gs_vec3 res = gs_vec3_scale(gs_vec3_norm(dir), c->r);

    // Find y
    res.y = (dir.y > 0.f) ? c->base.y + c->height + c->r : c->base.y - c->r;

    // Convert to world space
    res = xform ? gs_vec3_add(gs_mat3_mul_vec3(xform->rs, res), xform->pos) : res;

    return res;
}

// Quad

GS_API_DECL gs_vec3 gs_gjk_support_quad(const gs_quad_t* q, gs_phys_xform_t* xform, gs_vec3 dir)
{
    // Find support in object space (with inverse matrix)
    dir = xform ? gs_mat3_mul_vec3(xform->inv_rs, dir) : dir;
    dir.z = 0.f;

    // Calculate support point now in object space
    gs_vec3 res = gs_default_val();
    res.x = (dir.x > 0.f) ? q->max.x : q->min.x;
    res.y = (dir.y > 0.f) ? q->max.y : q->min.y;
    res.z = 0.f;

    // Convert to world space
    res = xform ? gs_vec3_add(gs_mat3_mul_vec3(xform->rs, res), xform->pos) : res;

    return res;
}

// Triangle
GS_API_DECL gs_vec3 gs_gjk_support_triangle(const gs_triangle_t* t, gs_phys_xform_t* xform, gs_vec3 dir)
{
    // Cache points
    gs_vec3 a = gs_v3(t->a.x, t->a.y, 0.f);
    gs_vec3 b = gs_v3(t->b.x, t->b.y, 0.f);
    gs_vec3 c = gs_v3(t->c.x, t->c.y, 0.f);

    // Find support in object space (with inverse matrix)
    dir = xform ? gs_mat3_mul_vec3(xform->inv_rs, dir) : dir;
    dir.z = 0.f;

    // Find farther point from direction
    gs_vec3 res = gs_default_val();
    res = gs_vec3_dot(dir, a) > gs_vec3_dot(dir, b) ? a : b;
    res = gs_vec3_dot(dir, res) > gs_vec3_dot(dir, c) ? res : c;

    // Convert to world space
    res = xform ? gs_vec3_add(gs_mat3_mul_vec3(xform->rs, res), xform->pos) : res;

    return res;
}

// Circle

GS_API_DECL gs_vec3 gs_gjk_support_circle(const gs_circle_t* c, gs_phys_xform_t* xform, gs_vec3 dir)
{
    // Circle is in XY plane
    // Bring direction into object space (inv matrix)
    dir = xform ? gs_mat3_mul_vec3(xform->inv_rs, dir) : dir;

    // Calculate support point
    dir.z = 0.f;
    gs_vec3 res = gs_vec3_add(gs_vec3_scale(gs_vec3_norm(dir), c->r), gs_v3(c->c.x, c->c.y, 0.f));

    // Convert to world space
    res = xform ? gs_vec3_add(gs_mat3_mul_vec3(xform->rs, res), xform->pos) : res;

    return res;
}

// Pill

GS_API_DECL gs_vec3 gs_gjk_support_pill(const gs_pill_t* c, gs_phys_xform_t* xform, gs_vec3 dir)
{
    // Find search dir in object space
    dir = xform ? gs_mat3_mul_vec3(xform->inv_rs, dir) : dir;

    // Compute result
    gs_vec3 dir_xy = gs_v3(dir.x, dir.y, 0.f);
    gs_vec3 res = gs_vec3_scale(gs_vec3_norm(dir_xy), c->r);

    // Find y
    res.y = (dir_xy.y > 0.f) ? c->base.y + c->height + c->r : c->base.y - c->r;

    // Convert to world space
    res = xform ? gs_vec3_add(gs_mat3_mul_vec3(xform->rs, res), xform->pos) : res;

    return res;
}

/*==== Collision Shapes ====*/

/* Line/Segment */

GS_API_DECL float gs_line_closest_line(float* t1, float* t2, gs_vec3* c1, gs_vec3* c2, gs_line_t l, gs_line_t m) 
{
    gs_vec3 r, d1, d2;
    d1 = gs_vec3_sub(l.b, l.a); /* direction vector segment s1 */
    d2 = gs_vec3_sub(m.b, m.a); /* direction vector segment s2 */
    r = gs_vec3_sub(l.a, m.a);

    float i = gs_vec3_dot(d1, d1);
    float e = gs_vec3_dot(d2, d2);
    float f = gs_vec3_dot(d2, r);

    if (i <= FLT_EPSILON && e <= FLT_EPSILON) {
        /* both segments degenerate into points */
        gs_vec3 d12;
        *t1 = *t2 = 0.0f;
        *c1 = l.a;
        *c2 = m.a;
        d12 = gs_vec3_sub(*c1, *c2);
        return gs_vec3_dot(d12,d12);
    }
    if (i > FLT_EPSILON) {
        float c = gs_vec3_dot(d1,r);
        if (e > FLT_EPSILON) {
            /* non-degenerate case */
            float b = gs_vec3_dot(d1,d2);
            float denom = i*e - b*b;

            /* compute closest point on L1/L2 if not parallel else pick any t2 */
            if (denom != 0.0f)
                *t1 = gs_clamp(0.0f, (b*f - c*e) / denom, 1.0f);
            else *t1 = 0.0f;

            /* cmpute point on L2 closest to S1(s) */
            *t2 = (b*(*t1) + f) / e;
            if (*t2 < 0.0f) {
                *t2 = 0.0f;
                *t1 = gs_clamp(0.0f, -c/i, 1.0f);
            } else if (*t2 > 1.0f) {
                *t2 = 1.0f;
                *t1 = gs_clamp(0.0f, (b-c)/i, 1.0f);
            }
        } else {
            /* second segment degenerates into a point */
            *t1 = gs_clamp(0.0f, -c/i, 1.0f);
            *t2 = 0.0f;
        }
    } else {
        /* first segment degenerates into a point */
        *t2 = gs_clamp(0.0f, f/e, 1.0f);
        *t1 = 0.0f;
    }
    /* calculate closest points */
    gs_vec3 n, d12;
    n = gs_vec3_scale(d1, *t1);
    *c1 = gs_vec3_add(l.a, n);
    n = gs_vec3_scale(d2, *t2);
    *c2 = gs_vec3_add(m.a, n);

    /* calculate squared distance */
    d12 = gs_vec3_sub(*c1, *c2);
    return gs_vec3_dot(d12,d12);
}

/* Sphere */

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
        res->hit = true;
    }

    return true;
}

GS_API_DECL int32_t gs_sphere_vs_aabb(const gs_sphere_t* a, gs_vqs* xform_a, const gs_aabb_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_sphere, b, xform_b, gs_gjk_support_aabb, res);
}

GS_API_DECL int32_t gs_sphere_vs_poly(const gs_sphere_t* a, gs_vqs* xform_a, const gs_poly_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_sphere, b, xform_b, gs_gjk_support_poly, res);
}

GS_API_DECL int32_t gs_sphere_vs_cylinder(const gs_sphere_t* a, gs_vqs* xform_a, const gs_cylinder_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_sphere, b, xform_b, gs_gjk_support_cylinder, res);
}

GS_API_DECL int32_t gs_sphere_vs_cone(const gs_sphere_t* a, gs_vqs* xform_a, const gs_cone_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_sphere, b, xform_b, gs_gjk_support_cone, res);
}

GS_API_DECL int32_t gs_sphere_vs_capsule(const gs_sphere_t* a, gs_vqs* xform_a, const gs_capsule_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_sphere, b, xform_b, gs_gjk_support_capsule, res);
}

GS_API_DECL int32_t gs_sphere_vs_quad(const gs_sphere_t* a, gs_vqs* xform_a, const gs_quad_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_sphere, b, xform_b, gs_gjk_support_quad, res);
}

GS_API_DECL int32_t gs_sphere_vs_triangle(const gs_sphere_t* a, gs_vqs* xform_a, const gs_triangle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_sphere, b, xform_b, gs_gjk_support_triangle, res);
}

GS_API_DECL int32_t gs_sphere_vs_circle(const gs_sphere_t* a, gs_vqs* xform_a, const gs_circle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_sphere, b, xform_b, gs_gjk_support_circle, res);
}

GS_API_DECL int32_t gs_sphere_vs_pill(const gs_sphere_t* a, gs_vqs* xform_a, const gs_pill_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_sphere, b, xform_b, gs_gjk_support_pill, res);
}

/* AABB */

GS_API_DECL int32_t gs_aabb_vs_aabb(const gs_aabb_t* a, gs_vqs* xform_a, const gs_aabb_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    // NOTE(john): If neither have transforms, then can default to less expensive AABB vs. AABB check
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_aabb, b, xform_b, gs_gjk_support_aabb, res);
}

GS_API_DECL int32_t gs_aabb_vs_sphere(const gs_aabb_t* a, gs_vqs* xform_a, const gs_sphere_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_aabb, b, xform_b, gs_gjk_support_sphere, res);
}

GS_API_DECL int32_t gs_aabb_vs_poly(const gs_aabb_t* a, gs_vqs* xform_a, const gs_poly_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_aabb, b, xform_b, gs_gjk_support_poly, res);
}

GS_API_DECL int32_t gs_aabb_vs_cylinder(const gs_aabb_t* a, gs_vqs* xform_a, const gs_cylinder_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_aabb, b, xform_b, gs_gjk_support_cylinder, res);
}

GS_API_DECL int32_t gs_aabb_vs_cone(const gs_aabb_t* a, gs_vqs* xform_a, const gs_cone_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_aabb, b, xform_b, gs_gjk_support_cone, res);
}

GS_API_DECL int32_t gs_aabb_vs_capsule(const gs_aabb_t* a, gs_vqs* xform_a, const gs_capsule_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_aabb, b, xform_b, gs_gjk_support_capsule, res);
}

GS_API_DECL int32_t gs_aabb_vs_quad(const gs_aabb_t* a, gs_vqs* xform_a, const gs_quad_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_aabb, b, xform_b, gs_gjk_support_quad, res);
}

GS_API_DECL int32_t gs_aabb_vs_triangle(const gs_aabb_t* a, gs_vqs* xform_a, const gs_triangle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_aabb, b, xform_b, gs_gjk_support_triangle, res);
}

GS_API_DECL int32_t gs_aabb_vs_circle(const gs_aabb_t* a, gs_vqs* xform_a, const gs_circle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_aabb, b, xform_b, gs_gjk_support_circle, res);
}

GS_API_DECL int32_t gs_aabb_vs_pill(const gs_aabb_t* a, gs_vqs* xform_a, const gs_pill_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_aabb, b, xform_b, gs_gjk_support_pill, res);
}

/* Plane */

/* Capsule */

GS_API_DECL int32_t gs_capsule_vs_aabb(const gs_capsule_t* a, gs_vqs* xform_a, const gs_aabb_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_capsule, b, xform_b, gs_gjk_support_aabb, res);
}

GS_API_DECL int32_t gs_capsule_vs_sphere(const gs_capsule_t* a, gs_vqs* xform_a, const gs_sphere_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_capsule, b, xform_b, gs_gjk_support_sphere, res);
}

GS_API_DECL int32_t gs_capsule_vs_poly(const gs_capsule_t* a, gs_vqs* xform_a, const gs_poly_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_capsule, b, xform_b, gs_gjk_support_poly, res);
}

GS_API_DECL int32_t gs_capsule_vs_cylinder(const gs_capsule_t* a, gs_vqs* xform_a, const gs_cylinder_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_capsule, b, xform_b, gs_gjk_support_cylinder, res);
}

GS_API_DECL int32_t gs_capsule_vs_cone(const gs_capsule_t* a, gs_vqs* xform_a, const gs_cone_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_capsule, b, xform_b, gs_gjk_support_cone, res);
}

GS_API_DECL int32_t gs_capsule_vs_capsule(const gs_capsule_t* a, gs_vqs* xform_a, const gs_capsule_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_capsule, b, xform_b, gs_gjk_support_capsule, res);
}

GS_API_DECL int32_t gs_capsule_vs_quad(const gs_capsule_t* a, gs_vqs* xform_a, const gs_quad_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_capsule, b, xform_b, gs_gjk_support_quad, res);
}

GS_API_DECL int32_t gs_capsule_vs_triangle(const gs_capsule_t* a, gs_vqs* xform_a, const gs_triangle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_capsule, b, xform_b, gs_gjk_support_triangle, res);
}

GS_API_DECL int32_t gs_capsule_vs_circle(const gs_capsule_t* a, gs_vqs* xform_a, const gs_circle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_capsule, b, xform_b, gs_gjk_support_circle, res);
}

GS_API_DECL int32_t gs_capsule_vs_pill(const gs_capsule_t* a, gs_vqs* xform_a, const gs_pill_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_capsule, b, xform_b, gs_gjk_support_pill, res);
}

/* Ray */


/* Poly */

GS_API_DECL int32_t gs_poly_vs_poly(const gs_poly_t* a, gs_vqs* xform_a, const gs_poly_t* b, gs_vqs* xform_b, gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_poly, b, xform_b, gs_gjk_support_poly, res);
}

GS_API_DECL int32_t gs_poly_vs_sphere(const gs_poly_t* a, gs_vqs* xform_a, const gs_sphere_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_poly, b, xform_b, gs_gjk_support_sphere, res);
}

GS_API_DECL int32_t gs_poly_vs_aabb(const gs_poly_t* a, gs_vqs* xform_a, const gs_aabb_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_poly, b, xform_b, gs_gjk_support_aabb, res);
}

GS_API_DECL int32_t gs_poly_vs_cylinder(const gs_poly_t* a, gs_vqs* xform_a, const gs_cylinder_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_poly, b, xform_b, gs_gjk_support_cylinder, res);
}

GS_API_DECL int32_t gs_poly_vs_cone(const gs_poly_t* a, gs_vqs* xform_a, const gs_cone_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_poly, b, xform_b, gs_gjk_support_cone, res);
}

GS_API_DECL int32_t gs_poly_vs_capsule(const gs_poly_t* a, gs_vqs* xform_a, const gs_capsule_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_poly, b, xform_b, gs_gjk_support_capsule, res);
}

GS_API_DECL int32_t gs_poly_vs_quad(const gs_poly_t* a, gs_vqs* xform_a, const gs_quad_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_poly, b, xform_b, gs_gjk_support_quad, res);
}

GS_API_DECL int32_t gs_poly_vs_triangle(const gs_poly_t* a, gs_vqs* xform_a, const gs_triangle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_poly, b, xform_b, gs_gjk_support_triangle, res);
}

GS_API_DECL int32_t gs_poly_vs_circle(const gs_poly_t* a, gs_vqs* xform_a, const gs_circle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_poly, b, xform_b, gs_gjk_support_circle, res);
}

GS_API_DECL int32_t gs_poly_vs_pill(const gs_poly_t* a, gs_vqs* xform_a, const gs_pill_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_poly, b, xform_b, gs_gjk_support_pill, res);
}

/* Frustum */

/* Cylinder */

GS_API_DECL int32_t gs_cylinder_vs_cylinder(const gs_cylinder_t* a, gs_vqs* xform_a, const gs_cylinder_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_cylinder, b, xform_b, gs_gjk_support_cylinder, res);
}

GS_API_DECL int32_t gs_cylinder_vs_sphere(const gs_cylinder_t* a, gs_vqs* xform_a, const gs_sphere_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return gs_sphere_vs_cylinder(b, xform_b, a, xform_a, res);
}

GS_API_DECL int32_t gs_cylinder_vs_aabb(const gs_cylinder_t* a, gs_vqs* xform_a, const gs_aabb_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_cylinder, b, xform_b, gs_gjk_support_aabb, res);
}

GS_API_DECL int32_t gs_cylinder_vs_poly(const gs_cylinder_t* a, gs_vqs* xform_a, const gs_poly_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_cylinder, b, xform_b, gs_gjk_support_poly, res);
}

GS_API_DECL int32_t gs_cylinder_vs_cone(const gs_cylinder_t* a, gs_vqs* xform_a, const gs_cone_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_cylinder, b, xform_b, gs_gjk_support_cone, res);
}

GS_API_DECL int32_t gs_cylinder_vs_capsule(const gs_cylinder_t* a, gs_vqs* xform_a, const gs_capsule_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_cylinder, b, xform_b, gs_gjk_support_capsule, res);
}

GS_API_DECL int32_t gs_cylinder_vs_quad(const gs_cylinder_t* a, gs_vqs* xform_a, const gs_quad_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_cylinder, b, xform_b, gs_gjk_support_quad, res);
}

GS_API_DECL int32_t gs_cylinder_vs_triangle(const gs_cylinder_t* a, gs_vqs* xform_a, const gs_triangle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_cylinder, b, xform_b, gs_gjk_support_triangle, res);
}

GS_API_DECL int32_t gs_cylinder_vs_circle(const gs_cylinder_t* a, gs_vqs* xform_a, const gs_circle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_cylinder, b, xform_b, gs_gjk_support_circle, res);
}

GS_API_DECL int32_t gs_cylinder_vs_pill(const gs_cylinder_t* a, gs_vqs* xform_a, const gs_pill_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_cylinder, b, xform_b, gs_gjk_support_pill, res);
}

/* Cone */

GS_API_DECL int32_t gs_cone_vs_cone(const gs_cone_t* a, gs_vqs* xform_a, const gs_cone_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_cone, b, xform_b, gs_gjk_support_cone, res);
}

GS_API_DECL int32_t gs_cone_vs_sphere(const gs_cone_t* a, gs_vqs* xform_a, const gs_sphere_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_cone, b, xform_b, gs_gjk_support_sphere, res);
}

GS_API_DECL int32_t gs_cone_vs_aabb(const gs_cone_t* a, gs_vqs* xform_a, const gs_aabb_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_cone, b, xform_b, gs_gjk_support_aabb, res);
}

GS_API_DECL int32_t gs_cone_vs_poly(const gs_cone_t* a, gs_vqs* xform_a, const gs_poly_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_cone, b, xform_b, gs_gjk_support_poly, res);
}

GS_API_DECL int32_t gs_cone_vs_cylinder(const gs_cone_t* a, gs_vqs* xform_a, const gs_cylinder_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_cone, b, xform_b, gs_gjk_support_cylinder, res);
}

GS_API_DECL int32_t gs_cone_vs_capsule(const gs_cone_t* a, gs_vqs* xform_a, const gs_capsule_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_cone, b, xform_b, gs_gjk_support_capsule, res);
}

GS_API_DECL int32_t gs_cone_vs_quad(const gs_cone_t* a, gs_vqs* xform_a, const gs_quad_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_cone, b, xform_b, gs_gjk_support_quad, res);
}

GS_API_DECL int32_t gs_cone_vs_triangle(const gs_cone_t* a, gs_vqs* xform_a, const gs_triangle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_cone, b, xform_b, gs_gjk_support_triangle, res);
}

GS_API_DECL int32_t gs_cone_vs_circle(const gs_cone_t* a, gs_vqs* xform_a, const gs_circle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_cone, b, xform_b, gs_gjk_support_circle, res);
}

GS_API_DECL int32_t gs_cone_vs_pill(const gs_cone_t* a, gs_vqs* xform_a, const gs_pill_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_cone, b, xform_b, gs_gjk_support_pill, res);
}

/* Quad */

GS_API_DECL int32_t gs_quad_vs_quad(const gs_quad_t* a, gs_vqs* xform_a, const gs_quad_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_quad, b, xform_b, gs_gjk_support_quad, res);
}

GS_API_DECL int32_t gs_quad_vs_sphere(const gs_quad_t* a, gs_vqs* xform_a, const gs_sphere_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_quad, b, xform_b, gs_gjk_support_sphere, res);
}

GS_API_DECL int32_t gs_quad_vs_aabb(const gs_quad_t* a, gs_vqs* xform_a, const gs_aabb_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_quad, b, xform_b, gs_gjk_support_aabb, res);
}

GS_API_DECL int32_t gs_quad_vs_poly(const gs_quad_t* a, gs_vqs* xform_a, const gs_poly_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_quad, b, xform_b, gs_gjk_support_poly, res);
}

GS_API_DECL int32_t gs_quad_vs_cone(const gs_quad_t* a, gs_vqs* xform_a, const gs_cone_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_quad, b, xform_b, gs_gjk_support_cone, res);
}

GS_API_DECL int32_t gs_quad_vs_cylinder(const gs_quad_t* a, gs_vqs* xform_a, const gs_cylinder_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_quad, b, xform_b, gs_gjk_support_cylinder, res);
}

GS_API_DECL int32_t gs_quad_vs_capsule(const gs_quad_t* a, gs_vqs* xform_a, const gs_capsule_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_quad, b, xform_b, gs_gjk_support_capsule, res);
}

GS_API_DECL int32_t gs_quad_vs_triangle(const gs_quad_t* a, gs_vqs* xform_a, const gs_triangle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_quad, b, xform_b, gs_gjk_support_triangle, res);
}

GS_API_DECL int32_t gs_quad_vs_circle(const gs_quad_t* a, gs_vqs* xform_a, const gs_circle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_quad, b, xform_b, gs_gjk_support_circle, res);
}

GS_API_DECL int32_t gs_quad_vs_pill(const gs_quad_t* a, gs_vqs* xform_a, const gs_pill_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_quad, b, xform_b, gs_gjk_support_pill, res);
}

GS_API_DECL int32_t gs_quad_vs_triangle_2d(const gs_quad_t* a, gs_vqs* xform_a, const gs_triangle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal_2d(a, xform_a, gs_gjk_support_quad, b, xform_b, gs_gjk_support_triangle, res);
}

GS_API_DECL int32_t gs_quad_vs_quad_2d(const gs_quad_t* a, gs_vqs* xform_a, const gs_quad_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal_2d(a, xform_a, gs_gjk_support_quad, b, xform_b, gs_gjk_support_quad, res);
}

GS_API_DECL int32_t gs_quad_vs_circle_2d(const gs_quad_t* a, gs_vqs* xform_a, const gs_circle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal_2d(a, xform_a, gs_gjk_support_quad, b, xform_b, gs_gjk_support_circle, res);
}

GS_API_DECL int32_t gs_quad_vs_pill_2d(const gs_quad_t* a, gs_vqs* xform_a, const gs_pill_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal_2d(a, xform_a, gs_gjk_support_quad, b, xform_b, gs_gjk_support_pill, res);
}

/* Circle */

GS_API_DECL int32_t gs_circle_vs_sphere(const gs_circle_t* a, gs_vqs* xform_a, const gs_sphere_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_circle, b, xform_b, gs_gjk_support_sphere, res);
}

GS_API_DECL int32_t gs_circle_vs_aabb(const gs_circle_t* a, gs_vqs* xform_a, const gs_aabb_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_circle, b, xform_b, gs_gjk_support_aabb, res);
}

GS_API_DECL int32_t gs_circle_vs_poly(const gs_circle_t* a, gs_vqs* xform_a, const gs_poly_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_circle, b, xform_b, gs_gjk_support_poly, res);
}

GS_API_DECL int32_t gs_circle_vs_cone(const gs_circle_t* a, gs_vqs* xform_a, const gs_cone_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_circle, b, xform_b, gs_gjk_support_cone, res);
}

GS_API_DECL int32_t gs_circle_vs_cylinder(const gs_circle_t* a, gs_vqs* xform_a, const gs_cylinder_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_circle, b, xform_b, gs_gjk_support_cylinder, res);
}

GS_API_DECL int32_t gs_circle_vs_capsule(const gs_circle_t* a, gs_vqs* xform_a, const gs_capsule_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_circle, b, xform_b, gs_gjk_support_capsule, res);
}

GS_API_DECL int32_t gs_circle_vs_triangle(const gs_circle_t* a, gs_vqs* xform_a, const gs_triangle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_circle, b, xform_b, gs_gjk_support_triangle, res);
}

GS_API_DECL int32_t gs_circle_vs_pill(const gs_circle_t* a, gs_vqs* xform_a, const gs_pill_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_circle, b, xform_b, gs_gjk_support_pill, res);
}

GS_API_DECL int32_t gs_circle_vs_circle(const gs_circle_t* a, gs_vqs* xform_a, const gs_circle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_circle, b, xform_b, gs_gjk_support_circle, res);
}

GS_API_DECL int32_t gs_circle_vs_quad(const gs_circle_t* a, gs_vqs* xform_a, const gs_quad_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_circle, b, xform_b, gs_gjk_support_quad, res);
}

GS_API_DECL int32_t gs_circle_vs_quad_2d(const gs_circle_t* a, gs_vqs* xform_a, const gs_quad_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal_2d(a, xform_a, gs_gjk_support_circle, b, xform_b, gs_gjk_support_quad, res);
}

GS_API_DECL int32_t gs_circle_vs_circle_2d(const gs_circle_t* a, gs_vqs* xform_a, const gs_circle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    // Specialized, non-gjk function
    gs_vec3 c0 = gs_v3(a->c.x, a->c.y, 0.f);
    gs_vec3 c1 = gs_v3(b->c.x, b->c.y, 0.f);
    gs_vec3 wa = xform_a ? gs_vec3_add(xform_a->position, c0) : c0;
    gs_vec3 wb = xform_b ? gs_vec3_add(xform_b->position, c1) : c1;
    wa.z = 0.f;
    wb.z = 0.f;

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
        res->hit = true;
    }

    return true;
}

GS_API_DECL int32_t gs_circle_vs_triangle_2d(const gs_circle_t* a, gs_vqs* xform_a, const gs_triangle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal_2d(a, xform_a, gs_gjk_support_circle, b, xform_b, gs_gjk_support_triangle, res);
}

GS_API_DECL int32_t gs_circle_vs_pill_2d(const gs_circle_t* a, gs_vqs* xform_a, const gs_pill_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal_2d(a, xform_a, gs_gjk_support_circle, b, xform_b, gs_gjk_support_pill, res);
}

/* Pill (2D Capsule) */
GS_API_DECL int32_t gs_pill_vs_sphere(const gs_pill_t* a, gs_vqs* xform_a, const gs_sphere_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_pill, b, xform_b, gs_gjk_support_sphere, res);
}

GS_API_DECL int32_t gs_pill_vs_aabb(const gs_pill_t* a, gs_vqs* xform_a, const gs_aabb_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_pill, b, xform_b, gs_gjk_support_aabb, res);
}

GS_API_DECL int32_t gs_pill_vs_poly(const gs_pill_t* a, gs_vqs* xform_a, const gs_poly_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_pill, b, xform_b, gs_gjk_support_poly, res);
}

GS_API_DECL int32_t gs_pill_vs_cone(const gs_pill_t* a, gs_vqs* xform_a, const gs_cone_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_pill, b, xform_b, gs_gjk_support_cone, res);
}

GS_API_DECL int32_t gs_pill_vs_cylinder(const gs_pill_t* a, gs_vqs* xform_a, const gs_cylinder_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_pill, b, xform_b, gs_gjk_support_cylinder, res);
}

GS_API_DECL int32_t gs_pill_vs_capsule(const gs_pill_t* a, gs_vqs* xform_a, const gs_capsule_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_pill, b, xform_b, gs_gjk_support_capsule, res);
}

GS_API_DECL int32_t gs_pill_vs_triangle(const gs_pill_t* a, gs_vqs* xform_a, const gs_triangle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_pill, b, xform_b, gs_gjk_support_triangle, res);
}

GS_API_DECL int32_t gs_pill_vs_pill(const gs_pill_t* a, gs_vqs* xform_a, const gs_pill_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_pill, b, xform_b, gs_gjk_support_pill, res);
}

GS_API_DECL int32_t gs_pill_vs_circle(const gs_pill_t* a, gs_vqs* xform_a, const gs_circle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_pill, b, xform_b, gs_gjk_support_circle, res);
}

GS_API_DECL int32_t gs_pill_vs_quad(const gs_pill_t* a, gs_vqs* xform_a, const gs_quad_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_pill, b, xform_b, gs_gjk_support_quad, res);
}

GS_API_DECL int32_t gs_pill_vs_quad_2d(const gs_pill_t* a, gs_vqs* xform_a, const gs_quad_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal_2d(a, xform_a, gs_gjk_support_pill, b, xform_b, gs_gjk_support_quad, res);
}

GS_API_DECL int32_t gs_pill_vs_circle_2d(const gs_pill_t* a, gs_vqs* xform_a, const gs_circle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal_2d(a, xform_a, gs_gjk_support_pill, b, xform_b, gs_gjk_support_circle, res);
}

GS_API_DECL int32_t gs_pill_vs_triangle_2d(const gs_pill_t* a, gs_vqs* xform_a, const gs_triangle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal_2d(a, xform_a, gs_gjk_support_pill, b, xform_b, gs_gjk_support_triangle, res);
}

GS_API_DECL int32_t gs_pill_vs_pill_2d(const gs_pill_t* a, gs_vqs* xform_a, const gs_pill_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal_2d(a, xform_a, gs_gjk_support_pill, b, xform_b, gs_gjk_support_pill, res);
}

/* Triangle */

GS_API_DECL int32_t gs_triangle_vs_triangle(const gs_triangle_t* a, gs_vqs* xform_a, const gs_triangle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_triangle, b, xform_b, gs_gjk_support_triangle, res);
}

GS_API_DECL int32_t gs_triangle_vs_quad(const gs_triangle_t* a, gs_vqs* xform_a, const gs_quad_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_triangle, b, xform_b, gs_gjk_support_quad, res);
}

GS_API_DECL int32_t gs_triangle_vs_sphere(const gs_triangle_t* a, gs_vqs* xform_a, const gs_sphere_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_triangle, b, xform_b, gs_gjk_support_sphere, res);
}

GS_API_DECL int32_t gs_triangle_vs_aabb(const gs_triangle_t* a, gs_vqs* xform_a, const gs_aabb_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_triangle, b, xform_b, gs_gjk_support_aabb, res);
}

GS_API_DECL int32_t gs_triangle_vs_poly(const gs_triangle_t* a, gs_vqs* xform_a, const gs_poly_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_triangle, b, xform_b, gs_gjk_support_poly, res);
}

GS_API_DECL int32_t gs_triangle_vs_cone(const gs_triangle_t* a, gs_vqs* xform_a, const gs_cone_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_triangle, b, xform_b, gs_gjk_support_cone, res);
}

GS_API_DECL int32_t gs_triangle_vs_cylinder(const gs_triangle_t* a, gs_vqs* xform_a, const gs_cylinder_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_triangle, b, xform_b, gs_gjk_support_cylinder, res);
}

GS_API_DECL int32_t gs_triangle_vs_capsule(const gs_triangle_t* a, gs_vqs* xform_a, const gs_capsule_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal(a, xform_a, gs_gjk_support_triangle, b, xform_b, gs_gjk_support_capsule, res);
}

GS_API_DECL int32_t gs_triangle_vs_triangle_2d(const gs_triangle_t* a, gs_vqs* xform_a, const gs_triangle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal_2d(a, xform_a, gs_gjk_support_triangle, b, xform_b, gs_gjk_support_triangle, res);
}

GS_API_DECL int32_t gs_triangle_vs_quad_2d(const gs_triangle_t* a, gs_vqs* xform_a, const gs_quad_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal_2d(a, xform_a, gs_gjk_support_triangle, b, xform_b, gs_gjk_support_quad, res);
}

GS_API_DECL int32_t gs_triangle_vs_circle_2d(const gs_triangle_t* a, gs_vqs* xform_a, const gs_circle_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal_2d(a, xform_a, gs_gjk_support_triangle, b, xform_b, gs_gjk_support_circle, res);
}

GS_API_DECL int32_t gs_triangle_vs_pill_2d(const gs_triangle_t* a, gs_vqs* xform_a, const gs_pill_t* b, gs_vqs* xform_b, struct gs_gjk_contact_info_t* res)
{
    return _gs_gjk_internal_2d(a, xform_a, gs_gjk_support_triangle, b, xform_b, gs_gjk_support_pill, res);
}

/* Hit */

/*==== GKJ ====*/

// Internal functions
bool gs_gjk_check_if_simplex_contains_origin(gs_gjk_simplex_t* simplex, gs_vec3* search_dir, gs_gjk_dimension dimension);
void gs_gjk_simplex_push(gs_gjk_simplex_t* simplex, gs_gjk_support_point_t p);
void gs_gjk_simplex_push_back(gs_gjk_simplex_t* simplex, gs_gjk_support_point_t p);
void gs_gjk_simplex_insert(gs_gjk_simplex_t* simplex, uint32_t idx, gs_gjk_support_point_t p);
void gs_gjk_bary(gs_vec3 p, gs_vec3 a, gs_vec3 b, gs_vec3 c, float* u, float* v, float* w);
gs_gjk_epa_edge_t gs_gjk_epa_find_closest_edge(gs_gjk_simplex_t* simplex);
gs_gjk_support_point_t gs_gjk_generate_support_point(const gs_gjk_collider_info_t* ci0, const gs_gjk_collider_info_t* ci1, gs_vec3 dir);

// Modified from: https://github.com/Nightmask3/Physics-Framework/blob/master/PhysicsFramework/PhysicsManager.cpp
int32_t gs_gjk(const gs_gjk_collider_info_t* ci0, const gs_gjk_collider_info_t* ci1, gs_gjk_dimension dimension, gs_gjk_contact_info_t* res)
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

void gs_gjk_simplex_insert(gs_gjk_simplex_t* simplex, uint32_t idx, gs_gjk_support_point_t p)
{
    if (idx > 4) return;
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
GS_API_DECL gs_gjk_contact_info_t gs_gjk_epa(
    const gs_gjk_simplex_t* simplex, 
    const gs_gjk_collider_info_t* ci0, 
    const gs_gjk_collider_info_t* ci1
)
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

// Expanding Polytope Algorithm 2D
GS_API_DECL gs_gjk_contact_info_t gs_gjk_epa_2d(
    const gs_gjk_simplex_t* simplex, 
    const gs_gjk_collider_info_t* ci0, 
    const gs_gjk_collider_info_t* ci1
)
{
    gs_gjk_contact_info_t res = gs_default_val();
    gs_gjk_support_point_t p = gs_default_val();
    if (simplex->ct == 1)
    {
        const gs_vec3 search_dirs[] = {
            gs_v3(1.0f, 0.0f, 0.f),
            gs_v3(-1.0f, 0.0f, 0.f),
            gs_v3(0.0f, 1.0f, 0.f),
            gs_v3(0.0f, -1.0f, 0.f)
        };

        for (int32_t i = 0; i < 4; ++i)
        {
            p = gs_gjk_generate_support_point(ci0, ci1, search_dirs[i]);

            // Grab direction from first point on face at p to i
            gs_vec3 dir = gs_vec3_sub(p.minkowski_hull_vert, simplex->a.minkowski_hull_vert);

            if (gs_vec3_len2(dir) >= 0.0001f)
            {
                gs_gjk_simplex_push_back(simplex, p);
                // simplex.push_back(next_support_point);

                break;
            }
        }
    }

    if (simplex->ct == 2)
    {
        // AB
        gs_vec3 ab = gs_vec3_sub(simplex->a.minkowski_hull_vert, simplex->b.minkowski_hull_vert);
        gs_vec3 sd3 = gs_vec3_cross(gs_vec3_cross(gs_v3(ab.x, ab.y, 0.f), 
            gs_vec3_neg(gs_v3(simplex->b.minkowski_hull_vert.x, simplex->b.minkowski_hull_vert.y, 0.f))), gs_v3(ab.x, ab.y, 0.f));
        gs_vec3 search_dir = gs_default_val();
        search_dir.x = sd3.x;
        search_dir.y = sd3.y;
        p = gs_gjk_generate_support_point(ci0, ci1, search_dir);
        gs_vec3 dir = gs_vec3_sub(p.minkowski_hull_vert, simplex->a.minkowski_hull_vert);

        if (gs_vec3_len2(dir) < 0.0001f)
        {
            p = gs_gjk_generate_support_point(ci0, ci1, gs_vec3_neg(search_dir));
        }
        gs_gjk_simplex_push_back(simplex, p);
    }

    gs_gjk_epa_edge_t e = gs_default_val();
    uint32_t iterations = 0;
    while (iterations < GS_EPA_MAX_NUM_ITERATIONS)
    {
        e = gs_gjk_epa_find_closest_edge(simplex);
        p = gs_gjk_generate_support_point(ci0, ci1, e.normal);

        double d = (double)gs_vec3_dot(p.minkowski_hull_vert, e.normal);

        // Success, calculate results
        if (d - e.distance < GS_EPA_TOLERANCE)
        {
            // Cache local pointers
            // gs_gjk_polytope_face_t* f = &faces[closest_face];
            // gs_vec3* n = &e.normal;
            // gs_gjk_support_point_t* sp0 = &e.a;
            // gs_gjk_support_point_t* sp1 = &e.b;
            // gs_gjk_support_point_t* sp2 = &f->points[2];
            // gs_vec3* p0 = &sp0->minkowski_hull_vert;
            // gs_vec3* p1 = &sp1->minkowski_hull_vert;
            // gs_vec3* p2 = &sp2->minkowski_hull_vert;

            // Normal and depth information
            res.hit = true;
            res.normal = gs_vec3_norm(e.normal);
            res.depth = e.distance;

            gs_vec3 line_vec = gs_vec3_sub(e.a.minkowski_hull_vert, e.b.minkowski_hull_vert);
            gs_vec3 projO = gs_vec3_scale(gs_vec3_scale(line_vec, 1.f / gs_vec3_len2(line_vec)), gs_vec3_dot(line_vec, gs_vec3_neg(e.b.minkowski_hull_vert)));
            float s, t;
            s = sqrt(gs_vec3_len2(projO) / gs_vec3_len2(line_vec));
            t = 1.f - s;
            int32_t next_idx = (e.index + 1) % simplex->ct;

            res.points[0] = gs_vec3_add(gs_vec3_scale(simplex->points[e.index].support_a, s), gs_vec3_scale(simplex->points[next_idx].support_a, t));
            res.points[1] = gs_vec3_add(gs_vec3_scale(simplex->points[e.index].support_b, s), gs_vec3_scale(simplex->points[next_idx].support_b, t));

            break;
        }
        else
        {
            gs_gjk_simplex_insert(simplex, e.index + 1, p);
        }

        // Increment iterations
        iterations++;
    }

    gs_vec3 line_vec = gs_vec3_sub(e.a.minkowski_hull_vert, e.b.minkowski_hull_vert);
    gs_vec3 projO = gs_vec3_scale(gs_vec3_scale(line_vec, 1.f / gs_vec3_len2(line_vec)), gs_vec3_dot(line_vec, gs_vec3_neg(e.b.minkowski_hull_vert)));
    float s, t;
    s = sqrt(gs_vec3_len2(projO) / gs_vec3_len2(line_vec));
    t = 1.f - s;
    int32_t next_idx = (e.index + 1) % simplex->ct;
    res.hit = true;
    res.points[0] = gs_vec3_add(gs_vec3_scale(simplex->points[e.index].support_a, s), gs_vec3_scale(simplex->points[next_idx].support_a, t));
    res.points[1] = gs_vec3_add(gs_vec3_scale(simplex->points[e.index].support_b, s), gs_vec3_scale(simplex->points[next_idx].support_b, t));

    return res;

}

gs_gjk_epa_edge_t gs_gjk_epa_find_closest_edge(gs_gjk_simplex_t* simplex)
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
        norm_3d = gs_vec3_cross(gs_vec3_cross(gs_v3(edge.x, edge.y, 0), 
            gs_v3(simplex->points[i].minkowski_hull_vert.x, simplex->points[i].minkowski_hull_vert.y, 0.f)), gs_v3(edge.x, edge.y, 0.f));
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

#endif // GS_PHYSICS_IMPL
#endif // __GS_PHYSICS_H__


























