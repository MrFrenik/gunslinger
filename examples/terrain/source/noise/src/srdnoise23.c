/* srdnoise23, Simplex noise with rotating gradients
 * and a true analytic derivative in 2D and 3D.
 *
 * This is version 2 of srdnoise23 written in early 2008.
 * A stupid bug was corrected. Do not use earlier versions.
 *
 * Author: Stefan Gustavson, 2003-2008
 *
 * Contact: stefan.gustavson@gmail.com
 *
 * This code was GPL licensed until February 2011.
 * As the original author of this code, I hereby
 * release it into the public domain.
 * Please feel free to use it for whatever you want.
 * Credit is appreciated where appropriate, and I also
 * appreciate being told where this code finds any use,
 * but you may do as you like.
 */

/*
 * This is an implementation of Perlin "simplex noise" over two dimensions
 * (x,y) and three dimensions (x,y,z). One extra parameter 't' rotates the
 * underlying gradients of the grid, which gives a swirling, flow-like
 * motion. The derivative is returned, to make it possible to do pseudo-
 * advection and implement "flow noise", as presented by Ken Perlin and
 * Fabrice Neyret at Siggraph 2001.
 *
 * When not animated and presented in one octave only, this noise
 * looks exactly the same as the plain version of simplex noise.
 * It's nothing magical by itself, although the extra animation
 * parameter 't' is useful. Fun stuff starts to happen when you
 * do fractal sums of several octaves, with different rotation speeds
 * and an advection of smaller scales by larger scales (or even the
 * other way around it you feel adventurous).
 *
 * The gradient rotations that can be performed by this noise function
 * and the true analytic derivatives are required to do flow noise.
 * You can't do it properly with regular Perlin noise.
 * The 3D version is my own creation. It's a hack, because unlike the 2D
 * version the gradients rotate around different axes, and therefore
 * they don't remain uncorrelated through the rotation, but it looks OK.
 *
 */

#include <math.h>

#include "srdnoise23.h" /* We strictly don't need this, but play nice. */

#define FASTFLOOR(x) ( ((int)(x)<=(x)) ? ((int)x) : (((int)x)-1) )

/* Static data ---------------------- */

/*
 * Permutation table. This is just a random jumble of all numbers 0-255,
 * repeated twice to avoid wrapping the index at 255 for each lookup.
 */
unsigned char perm[512] = {151,160,137,91,90,15,
  131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
  190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
  88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
  77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
  102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
  135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
  5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
  223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
  129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
  251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
  49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
  138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
  151,160,137,91,90,15,
  131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
  190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
  88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
  77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
  102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
  135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
  5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
  223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
  129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
  251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
  49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
  138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180 
};

/*
 * Gradient tables. These could be programmed the Ken Perlin way with
 * some clever bit-twiddling, but this is more clear, and not really slower.
 */
static float grad2[8][2] = {
  { -1.0f, -1.0f }, { 1.0f, 0.0f } , { -1.0f, 0.0f } , { 1.0f, 1.0f } ,
  { -1.0f, 1.0f } , { 0.0f, -1.0f } , { 0.0f, 1.0f } , { 1.0f, -1.0f }
};

/*
 * For 3D, we define two orthogonal vectors in the desired rotation plane.
 * These vectors are based on the midpoints of the 12 edges of a cube,
 * they all rotate in their own plane and are never coincident or collinear.
 * A larger array of random vectors would also do the job, but these 12
 * (including 4 repeats to make the array length a power of two) work better.
 * They are not random, they are carefully chosen to represent a small
 * isotropic set of directions for any rotation angle.
 */

/* a = sqrt(2)/sqrt(3) = 0.816496580 */
#define a 0.81649658f

static float grad3u[16][3] = {
  { 1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 1.0f }, // 12 cube edges
  { -1.0f, 0.0f, 1.0f }, { 0.0f, -1.0f, 1.0f },
  { 1.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, -1.0f },
  { -1.0f, 0.0f, -1.0f }, { 0.0f, -1.0f, -1.0f },
  { a, a, a }, { -a, a, -a },
  { -a, -a, a }, { a, -a, -a },
  { -a, a, a }, { a, -a, a },
  { a, -a, -a }, { -a, a, -a }
};

static float grad3v[16][3] = {
  { -a, a, a }, { -a, -a, a },
  { a, -a, a }, { a, a, a },
  { -a, -a, -a }, { a, -a, -a },
  { a, a, -a }, { -a, a, -a },
  { 1.0f, -1.0f, 0.0f }, { 1.0f, 1.0f, 0.0f },
  { -1.0f, 1.0f, 0.0f }, { -1.0f, -1.0f, 0.0f },
  { 1.0f, 0.0f, 1.0f }, { -1.0f, 0.0f, 1.0f }, // 4 repeats to make 16
  { 0.0f, 1.0f, -1.0f }, { 0.0f, -1.0f, -1.0f }
};

#undef a

/* --------------------------------------------------------------------- */

/*
 * Helper functions to compute rotated gradients and
 * gradients-dot-residualvectors in 2D and 3D.
 */

void gradrot2( int hash, float sin_t, float cos_t, float *gx, float *gy ) {
    int h = hash & 7;
    float gx0 = grad2[h][0];
    float gy0 = grad2[h][1];
    *gx = cos_t * gx0 - sin_t * gy0;
    *gy = sin_t * gx0 + cos_t * gy0;
    return;
}

void gradrot3( int hash, float sin_t, float cos_t, float *gx, float *gy, float *gz ) {
    int h = hash & 15;
    float gux = grad3u[h][0];
    float guy = grad3u[h][1];
    float guz = grad3u[h][2];
    float gvx = grad3v[h][0];
    float gvy = grad3v[h][1];
    float gvz = grad3v[h][2];
    *gx = cos_t * gux + sin_t * gvx;
    *gy = cos_t * guy + sin_t * gvy;
    *gz = cos_t * guz + sin_t * gvz;
    return;
}

float graddotp2( float gx, float gy, float x, float y ) {
  return gx * x + gy * y;
}

float graddotp3( float gx, float gy, float gz, float x, float y, float z ) {
  return gx * x + gy * y + gz * z;
}

/* Skewing factors for 2D simplex grid:
 * F2 = 0.5*(sqrt(3.0)-1.0)
 * G2 = (3.0-Math.sqrt(3.0))/6.0
 */
#define F2 0.366025403
#define G2 0.211324865

/** 2D simplex noise with rotating gradients.
 * If the last two arguments are not null, the analytic derivative
 * (the 2D gradient of the total noise field) is also calculated.
 */
float srdnoise2( float x, float y, float angle, float *dnoise_dx, float *dnoise_dy )
  {
    float n0, n1, n2; /* Noise contributions from the three simplex corners */
    float gx0, gy0, gx1, gy1, gx2, gy2; /* Gradients at simplex corners */
    float sin_t, cos_t; /* Sine and cosine for the gradient rotation angle */
    sin_t = sin( angle );
    cos_t = cos( angle );

    /* Skew the input space to determine which simplex cell we're in */
    float s = ( x + y ) * F2; /* Hairy factor for 2D */
    float xs = x + s;
    float ys = y + s;
    int i = FASTFLOOR( xs );
    int j = FASTFLOOR( ys );

    float t = ( float ) ( i + j ) * G2;
    float X0 = i - t; /* Unskew the cell origin back to (x,y) space */
    float Y0 = j - t;
    float x0 = x - X0; /* The x,y distances from the cell origin */
    float y0 = y - Y0;

    /* For the 2D case, the simplex shape is an equilateral triangle.
     * Determine which simplex we are in. */
    int i1, j1; /* Offsets for second (middle) corner of simplex in (i,j) coords */
    if( x0 > y0 ) { i1 = 1; j1 = 0; } /* lower triangle, XY order: (0,0)->(1,0)->(1,1) */
    else { i1 = 0; j1 = 1; }      /* upper triangle, YX order: (0,0)->(0,1)->(1,1) */

    /* A step of (1,0) in (i,j) means a step of (1-c,-c) in (x,y), and
     * a step of (0,1) in (i,j) means a step of (-c,1-c) in (x,y), where
     * c = (3-sqrt(3))/6   */
    float x1 = x0 - i1 + G2; /* Offsets for middle corner in (x,y) unskewed coords */
    float y1 = y0 - j1 + G2;
    float x2 = x0 - 1.0f + 2.0f * G2; /* Offsets for last corner in (x,y) unskewed coords */
    float y2 = y0 - 1.0f + 2.0f * G2;

    /* Wrap the integer indices at 256, to avoid indexing perm[] out of bounds */
    int ii = i % 256;
    int jj = j % 256;

    /* Calculate the contribution from the three corners */
    float t0 = 0.5f - x0 * x0 - y0 * y0;
    float t20, t40;
    if( t0 < 0.0f ) t40 = t20 = t0 = n0 = gx0 = gy0 = 0.0f; /* No influence */
    else {
      gradrot2( perm[ii + perm[jj]], sin_t, cos_t, &gx0, &gy0 );
      t20 = t0 * t0;
      t40 = t20 * t20;
      n0 = t40 * graddotp2( gx0, gy0, x0, y0 );
    }

    float t1 = 0.5f - x1 * x1 - y1 * y1;
    float t21, t41;
    if( t1 < 0.0f ) t21 = t41 = t1 = n1 = gx1 = gy1 = 0.0f; /* No influence */
    else {
      gradrot2( perm[ii + i1 + perm[jj + j1]], sin_t, cos_t, &gx1, &gy1 );
      t21 = t1 * t1;
      t41 = t21 * t21;
      n1 = t41 * graddotp2( gx1, gy1, x1, y1 );
    }

    float t2 = 0.5f - x2 * x2 - y2 * y2;
    float t22, t42;
    if( t2 < 0.0f ) t42 = t22 = t2 = n2 = gx2 = gy2 = 0.0f; /* No influence */
    else {
      gradrot2( perm[ii + 1 + perm[jj + 1]], sin_t, cos_t, &gx2, &gy2 );
      t22 = t2 * t2;
      t42 = t22 * t22;
      n2 = t42 * graddotp2( gx2, gy2, x2, y2 );
    }

    /* Add contributions from each corner to get the final noise value.
     * The result is scaled to return values in the interval [-1,1]. */
    float noise = 40.0f * ( n0 + n1 + n2 );

    /* Compute derivative, if requested by supplying non-null pointers
     * for the last two arguments */
    if( ( dnoise_dx != 0 ) && ( dnoise_dy != 0 ) )
      {
	/*  A straight, unoptimised calculation would be like:
     *    *dnoise_dx = -8.0f * t20 * t0 * x0 * graddotp2(gx0, gy0, x0, y0) + t40 * gx0;
     *    *dnoise_dy = -8.0f * t20 * t0 * y0 * graddotp2(gx0, gy0, x0, y0) + t40 * gy0;
     *    *dnoise_dx += -8.0f * t21 * t1 * x1 * graddotp2(gx1, gy1, x1, y1) + t41 * gx1;
     *    *dnoise_dy += -8.0f * t21 * t1 * y1 * graddotp2(gx1, gy1, x1, y1) + t41 * gy1;
     *    *dnoise_dx += -8.0f * t22 * t2 * x2 * graddotp2(gx2, gy2, x2, y2) + t42 * gx2;
     *    *dnoise_dy += -8.0f * t22 * t2 * y2 * graddotp2(gx2, gy2, x2, y2) + t42 * gy2;
	 */
        float temp0 = t20 * t0 * graddotp2( gx0, gy0, x0, y0 );
        *dnoise_dx = temp0 * x0;
        *dnoise_dy = temp0 * y0;
        float temp1 = t21 * t1 * graddotp2( gx1, gy1, x1, y1 );
        *dnoise_dx += temp1 * x1;
        *dnoise_dy += temp1 * y1;
        float temp2 = t22 * t2 * graddotp2( gx2, gy2, x2, y2 );
        *dnoise_dx += temp2 * x2;
        *dnoise_dy += temp2 * y2;
        *dnoise_dx *= -8.0f;
        *dnoise_dy *= -8.0f;
        /* This corrects a bug in the original implementation */
        *dnoise_dx += t40 * gx0 + t41 * gx1 + t42 * gx2;
        *dnoise_dy += t40 * gy0 + t41 * gy1 + t42 * gy2;
        *dnoise_dx *= 40.0f; /* Scale derivative to match the noise scaling */
        *dnoise_dy *= 40.0f;
      }
    return noise;
  }

/* Skewing factors for 3D simplex grid:
 * F3 = 1/3
 * G3 = 1/6 */
#define F3 0.333333333
#define G3 0.166666667

float srdnoise3( float x, float y, float z, float angle,
                 float *dnoise_dx, float *dnoise_dy, float *dnoise_dz )
  {
    float n0, n1, n2, n3; /* Noise contributions from the four simplex corners */
    float noise;          /* Return value */
    float gx0, gy0, gz0, gx1, gy1, gz1; /* Gradients at simplex corners */
    float gx2, gy2, gz2, gx3, gy3, gz3;
    float sin_t, cos_t; /* Sine and cosine for the gradient rotation angle */
    sin_t = sin( angle );
    cos_t = cos( angle );

    /* Skew the input space to determine which simplex cell we're in */
    float s = (x+y+z)*F3; /* Very nice and simple skew factor for 3D */
    float xs = x+s;
    float ys = y+s;
    float zs = z+s;
    int i = FASTFLOOR(xs);
    int j = FASTFLOOR(ys);
    int k = FASTFLOOR(zs);

    float t = (float)(i+j+k)*G3; 
    float X0 = i-t; /* Unskew the cell origin back to (x,y,z) space */
    float Y0 = j-t;
    float Z0 = k-t;
    float x0 = x-X0; /* The x,y,z distances from the cell origin */
    float y0 = y-Y0;
    float z0 = z-Z0;

    /* For the 3D case, the simplex shape is a slightly irregular tetrahedron.
     * Determine which simplex we are in. */
    int i1, j1, k1; /* Offsets for second corner of simplex in (i,j,k) coords */
    int i2, j2, k2; /* Offsets for third corner of simplex in (i,j,k) coords */

    /* TODO: This code would benefit from a backport from the GLSL version! */
    if(x0>=y0) {
      if(y0>=z0)
        { i1=1; j1=0; k1=0; i2=1; j2=1; k2=0; } /* X Y Z order */
        else if(x0>=z0) { i1=1; j1=0; k1=0; i2=1; j2=0; k2=1; } /* X Z Y order */
        else { i1=0; j1=0; k1=1; i2=1; j2=0; k2=1; } /* Z X Y order */
      }
    else { // x0<y0
      if(y0<z0) { i1=0; j1=0; k1=1; i2=0; j2=1; k2=1; } /* Z Y X order */
      else if(x0<z0) { i1=0; j1=1; k1=0; i2=0; j2=1; k2=1; } /* Y Z X order */
      else { i1=0; j1=1; k1=0; i2=1; j2=1; k2=0; } /* Y X Z order */
    }

    /* A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
     * a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
     * a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where
     * c = 1/6.   */

    float x1 = x0 - i1 + G3; /* Offsets for second corner in (x,y,z) coords */
    float y1 = y0 - j1 + G3;
    float z1 = z0 - k1 + G3;
    float x2 = x0 - i2 + 2.0f * G3; /* Offsets for third corner in (x,y,z) coords */
    float y2 = y0 - j2 + 2.0f * G3;
    float z2 = z0 - k2 + 2.0f * G3;
    float x3 = x0 - 1.0f + 3.0f * G3; /* Offsets for last corner in (x,y,z) coords */
    float y3 = y0 - 1.0f + 3.0f * G3;
    float z3 = z0 - 1.0f + 3.0f * G3;

    /* Wrap the integer indices at 256, to avoid indexing perm[] out of bounds */
    int ii = i % 256;
    int jj = j % 256;
    int kk = k % 256;

    /* Calculate the contribution from the four corners */
    float t0 = 0.5f - x0*x0 - y0*y0 - z0*z0;
    float t20, t40;
    if(t0 < 0.0f) n0 = t0 = t20 = t40 = gx0 = gy0 = gz0 = 0.0f;
    else {
      gradrot3( perm[ii + perm[jj + perm[kk]]], sin_t, cos_t, &gx0, &gy0, &gz0 );
      t20 = t0 * t0;
      t40 = t20 * t20;
      n0 = t40 * graddotp3( gx0, gy0, gz0, x0, y0, z0 );
    }

    float t1 = 0.5f - x1*x1 - y1*y1 - z1*z1;
    float t21, t41;
    if(t1 < 0.0f) n1 = t1 = t21 = t41 = gx1 = gy1 = gz1 = 0.0f;
    else {
      gradrot3( perm[ii + i1 + perm[jj + j1 + perm[kk + k1]]], sin_t, cos_t, &gx1, &gy1, &gz1 );
      t21 = t1 * t1;
      t41 = t21 * t21;
      n1 = t41 * graddotp3( gx1, gy1, gz1, x1, y1, z1 );
    }

    float t2 = 0.5f - x2*x2 - y2*y2 - z2*z2;
    float t22, t42;
    if(t2 < 0.0f) n2 = t2 = t22 = t42 = gx2 = gy2 = gz2 = 0.0f;
    else {
      gradrot3( perm[ii + i2 + perm[jj + j2 + perm[kk + k2]]], sin_t, cos_t, &gx2, &gy2, &gz2 );
      t22 = t2 * t2;
      t42 = t22 * t22;
      n2 = t42 * graddotp3( gx2, gy2, gz2, x2, y2, z2 );
    }

    float t3 = 0.5f - x3*x3 - y3*y3 - z3*z3;
    float t23, t43;
    if(t3 < 0.0f) n3 = t3 = t23 = t43 = gx3 = gy3 = gz3 = 0.0f;
    else {
      gradrot3( perm[ii + 1 + perm[jj + 1 + perm[kk + 1]]], sin_t, cos_t, &gx3, &gy3, &gz3 );
      t23 = t3 * t3;
      t43 = t23 * t23;
      n3 = t43 * graddotp3( gx3, gy3, gz3, x3, y3, z3 );
    }

    /*  Add contributions from each corner to get the final noise value.
     * The result is scaled to return values in the range [-1,1] */
    noise = 72.0f * (n0 + n1 + n2 + n3);

    /* Compute derivative, if requested by supplying non-null pointers
     * for the last three arguments */
    if( ( dnoise_dx != 0 ) && ( dnoise_dy != 0 ) && ( dnoise_dz != 0 ))
      {
	/*  A straight, unoptimised calculation would be like:
     *     *dnoise_dx = -8.0f * t20 * t0 * x0 * graddotp3(gx0, gy0, gz0, x0, y0, z0) + t40 * gx0;
     *    *dnoise_dy = -8.0f * t20 * t0 * y0 * graddotp3(gx0, gy0, gz0, x0, y0, z0) + t40 * gy0;
     *    *dnoise_dz = -8.0f * t20 * t0 * z0 * graddotp3(gx0, gy0, gz0, x0, y0, z0) + t40 * gz0;
     *    *dnoise_dx += -8.0f * t21 * t1 * x1 * graddotp3(gx1, gy1, gz1, x1, y1, z1) + t41 * gx1;
     *    *dnoise_dy += -8.0f * t21 * t1 * y1 * graddotp3(gx1, gy1, gz1, x1, y1, z1) + t41 * gy1;
     *    *dnoise_dz += -8.0f * t21 * t1 * z1 * graddotp3(gx1, gy1, gz1, x1, y1, z1) + t41 * gz1;
     *    *dnoise_dx += -8.0f * t22 * t2 * x2 * graddotp3(gx2, gy2, gz2, x2, y2, z2) + t42 * gx2;
     *    *dnoise_dy += -8.0f * t22 * t2 * y2 * graddotp3(gx2, gy2, gz2, x2, y2, z2) + t42 * gy2;
     *    *dnoise_dz += -8.0f * t22 * t2 * z2 * graddotp3(gx2, gy2, gz2, x2, y2, z2) + t42 * gz2;
     *    *dnoise_dx += -8.0f * t23 * t3 * x3 * graddotp3(gx3, gy3, gz3, x3, y3, z3) + t43 * gx3;
     *    *dnoise_dy += -8.0f * t23 * t3 * y3 * graddotp3(gx3, gy3, gz3, x3, y3, z3) + t43 * gy3;
     *    *dnoise_dz += -8.0f * t23 * t3 * z3 * graddotp3(gx3, gy3, gz3, x3, y3, z3) + t43 * gz3;
     */
        float temp0 = t20 * t0 * graddotp3( gx0, gy0, gz0, x0, y0, z0 );
        *dnoise_dx = temp0 * x0;
        *dnoise_dy = temp0 * y0;
        *dnoise_dz = temp0 * z0;
        float temp1 = t21 * t1 * graddotp3( gx1, gy1, gz1, x1, y1, z1 );
        *dnoise_dx += temp1 * x1;
        *dnoise_dy += temp1 * y1;
        *dnoise_dz += temp1 * z1;
        float temp2 = t22 * t2 * graddotp3( gx2, gy2, gz2, x2, y2, z2 );
        *dnoise_dx += temp2 * x2;
        *dnoise_dy += temp2 * y2;
        *dnoise_dz += temp2 * z2;
        float temp3 = t23 * t3 * graddotp3( gx3, gy3, gz3, x3, y3, z3 );
        *dnoise_dx += temp3 * x3;
        *dnoise_dy += temp3 * y3;
        *dnoise_dz += temp3 * z3;
        *dnoise_dx *= -8.0f;
        *dnoise_dy *= -8.0f;
        *dnoise_dz *= -8.0f;
        /* This corrects a bug in the original implementation */
        *dnoise_dx += t40 * gx0 + t41 * gx1 + t42 * gx2 + t43 * gx3;
        *dnoise_dy += t40 * gy0 + t41 * gy1 + t42 * gy2 + t43 * gy3;
        *dnoise_dz += t40 * gz0 + t41 * gz1 + t42 * gz2 + t43 * gz3;
        *dnoise_dx *= 72.0f; /* Scale derivative to match the noise scaling */
        *dnoise_dy *= 72.0f;
        *dnoise_dz *= 72.0f;
      }
    return noise;
  }
