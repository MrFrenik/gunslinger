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

/**
 * Simplex, rotating, derivative noise over 2 dimensions
 */
float srdnoise2( float x, float y, float t, float *dnoise_dx, float *dnoise_dy );

/**
 * Simplex, rotating, derivative noise over 3 dimensions
 */
float srdnoise3( float x, float y, float z, float t, float *dnoise_dx, float *dnoise_dy, float *dnoise_dz );

