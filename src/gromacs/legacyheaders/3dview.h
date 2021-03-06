/*
 *
 *                This source code is part of
 *
 *                 G   R   O   M   A   C   S
 *
 *          GROningen MAchine for Chemical Simulations
 *
 *                        VERSION 3.2.0
 * Written by David van der Spoel, Erik Lindahl, Berk Hess, and others.
 * Copyright (c) 1991-2000, University of Groningen, The Netherlands.
 * Copyright (c) 2001-2004, The GROMACS development team,
 * check out http://www.gromacs.org for more information.

 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * If you want to redistribute modifications, please consider that
 * scientific software is very special. Version control is crucial -
 * bugs must be traceable. We will be happy to consider code for
 * inclusion in the official distribution, but derived work must not
 * be called official GROMACS. Details are found in the README & COPYING
 * files - if they are missing, get the official version at www.gromacs.org.
 *
 * To help us fund GROMACS development, we humbly ask that you cite
 * the papers on the package - you can find them in the top README file.
 *
 * For more info, check our website at http://www.gromacs.org
 *
 * And Hey:
 * Gromacs Runs On Most of All Computer Systems
 */

#ifndef _3dview_h
#define _3dview_h

#include "typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WW 3

typedef real vec4[4];

typedef real mat4[4][4];

typedef int  iv2[2];

typedef struct {
    matrix box;
    int    ecenter;     /* enum for centering, see pbc.h        */
    vec4   eye, origin; /* The eye and origin position		*/
    mat4   proj;        /* Projection matrix            */
    mat4   Rot;         /* Total rotation matrix                */
    real   sc_x, sc_y;  /* Scaling for aspect ratio		*/
    mat4   RotP[DIM];   /* state for 3d rotations  */
    mat4   RotM[DIM];
} t_3dview;

extern void print_m4(FILE *fp, const char *s, mat4 A);

extern void print_v4(FILE *fp, char *s, int dim, real *a);

extern void m4_op(mat4 m, rvec x, vec4 v);

extern void unity_m4(mat4 m);

extern void mult_matrix(mat4 A, mat4 B, mat4 C);

extern void rotate(int axis, real angle, mat4 A);

extern void translate(real tx, real ty, real tz, mat4 A);

extern void m4_op(mat4 m, rvec x, vec4 v);

extern void calculate_view(t_3dview *view);

extern t_3dview *init_view(matrix box);
/* Generate the view matrix from the eye pos and the origin,
 * applying also the scaling for the aspect ration.
 * There is no accompanying done_view routine: the struct can simply
 * be sfree'd.
 */

/* The following options are present on the 3d struct:
 * zoom (scaling)
 * rotate around the center of the box
 * reset the view
 */

extern gmx_bool zoom_3d(t_3dview *view, real fac);
/* Zoom in or out with factor fac, returns TRUE when zoom successful,
 * FALSE otherwise.
 */

extern void init_rotate_3d(t_3dview *view);
/* Initiates the state of 3d rotation matrices in the structure */

extern void rotate_3d(t_3dview *view, int axis, gmx_bool bPositive);
/* Rotate the eye around the center of the box, around axis */

extern void translate_view(t_3dview *view, int axis, gmx_bool bPositive);
/* Translate the origin at which one is looking */

extern void reset_view(t_3dview *view);
/* Reset the viewing to the initial view */

#ifdef __cplusplus
}
#endif

#endif
