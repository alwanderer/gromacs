/* -*- mode: c; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; c-file-style: "stroustrup"; -*-
 *
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
 * GROningen Mixture of Alchemy and Childrens' Stories
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>

#include "typedefs.h"
#include "smalloc.h"
#include "gmx_fatal.h"
#include "calcgrid.h"

/* The grid sizes below are based on timing of a 3D cubic grid in fftw
 * compiled with SSE using 4 threads in fft5d.c.
 * A grid size is removed when a larger grid is faster.
 */

/* Small grid size array */
#define g_initNR 15
const int grid_init[g_initNR] = { 6, 8, 10, 12, 14, 16, 20, 24, 25, 28, 32, 36, 40, 42, 44 };

/* For larger grid sizes, a prefactor with any power of 2 can be added.
 * Only sizes divisible by 4 should be used, 90 is allowed, 140 not.
 */
#define g_baseNR 14
const int grid_base[g_baseNR] = { 45, 48, 50, 52, 54, 56, 60, 64, 70, 72, 75, 80, 81, 84 };

real calc_grid(FILE *fp, matrix box, real gr_sp,
               int *nx, int *ny, int *nz)
{
    int  d, n[DIM];
    int  i;
    rvec box_size;
    int  nmin, fac2, try;
    rvec spacing;
    real max_spacing;

    if ((*nx <= 0 || *ny <= 0 || *nz <= 0) && gr_sp <= 0)
    {
        gmx_fatal(FARGS, "invalid fourier grid spacing: %g", gr_sp);
    }

    if (grid_base[g_baseNR-1] % 4 != 0)
    {
        gmx_incons("the last entry in grid_base is not a multiple of 4");
    }

    /* New grid calculation setup:
     *
     * To maintain similar accuracy for triclinic PME grids as for rectangular
     * ones, the max grid spacing should set along the box vectors rather than
     * cartesian X/Y/Z directions. This will lead to slightly larger grids, but
     * it is much better than having to go to pme_order=6.
     *
     * Thus, instead of just extracting the diagonal elements to box_size[d], we
     * now calculate the cartesian length of the vectors.
     *
     * /Erik Lindahl, 20060402.
     */
    for (d = 0; d < DIM; d++)
    {
        box_size[d] = 0;
        for (i = 0; i < DIM; i++)
        {
            box_size[d] += box[d][i]*box[d][i];
        }
        box_size[d] = sqrt(box_size[d]);
    }

    n[XX] = *nx;
    n[YY] = *ny;
    n[ZZ] = *nz;

    if ((*nx <= 0) || (*ny <= 0) || (*nz <= 0))
    {
        if (NULL != fp)
        {
            fprintf(fp, "Calculating fourier grid dimensions for%s%s%s\n",
                    *nx > 0 ? "" : " X", *ny > 0 ? "" : " Y", *nz > 0 ? "" : " Z");
        }
    }

    max_spacing = 0;
    for (d = 0; d < DIM; d++)
    {
        if (n[d] <= 0)
        {
            nmin = (int)(box_size[d]/gr_sp + 0.999);

            i = g_initNR - 1;
            if (grid_init[i] >= nmin)
            {
                /* Take the smallest possible grid in the list */
                while (i > 0 && grid_init[i-1] >= nmin)
                {
                    i--;
                }
                n[d] = grid_init[i];
            }
            else
            {
                /* Determine how many pre-factors of 2 we need */
                fac2 = 1;
                i    = g_baseNR - 1;
                while (fac2*grid_base[i-1] < nmin)
                {
                    fac2 *= 2;
                }
                /* Find the smallest grid that is >= nmin */
                do
                {
                    try = fac2*grid_base[i];
                    /* We demand a factor of 4, avoid 140, allow 90 */
                    if (((try % 4 == 0 && try != 140) || try == 90) &&
                        try >= nmin)
                    {
                        n[d] = try;
                    }
                    i--;
                }
                while (i > 0);
            }
        }

        spacing[d] = box_size[d]/n[d];
        if (spacing[d] > max_spacing)
        {
            max_spacing = spacing[d];
        }
    }
    *nx = n[XX];
    *ny = n[YY];
    *nz = n[ZZ];
    if (NULL != fp)
    {
        fprintf(fp, "Using a fourier grid of %dx%dx%d, spacing %.3f %.3f %.3f\n",
                *nx, *ny, *nz, spacing[XX], spacing[YY], spacing[ZZ]);
    }

    return max_spacing;
}
