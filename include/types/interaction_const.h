/* -*- mode: c; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; c-file-style: "stroustrup"; -*-
 *
 *
 *                This source code is part of
 *
 *                 G   R   O   M   A   C   S
 *
 *          GROningen MAchine for Chemical Simulations
 *
 * Written by David van der Spoel, Erik Lindahl, Berk Hess, and others.
 * Copyright (c) 1991-2000, University of Groningen, The Netherlands.
 * Copyright (c) 2001-2012, The GROMACS development team,
 * check out http://www.gromacs.org for more information.
 *
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
 * Gallium Rubidium Oxygen Manganese Argon Carbon Silicon
 */

#ifndef _INTERACTION_CONST_
#define _INTERACTION_CONST_

#ifdef __cplusplus
extern "C" {
#endif

enum { tableformatNONE, tableformatF, tableformatFDV0 };

typedef struct {
    /* VdW */
    real rvdw;

    /* Coulomb */
    real rcoulomb;

    /* Cut-off */
    real rlist;

    /* PME/Ewald */
    real ewaldcoeff;
  
    /* Dielectric constant resp. multiplication factor for charges */
    real epsilon_r,epsilon_rf,epsfac;  
  
    /* Constants for reaction fields */
    real k_rf,c_rf;

    /* type of electrostatics (defined in enums.h) */
    int  eeltype;

    /* Force/energy interpolation tables, linear in force, quadratic in V */
    real tabq_scale;
    int  tabq_size;
    int  tabq_format;
    /* Coulomb force table, size of array is tabsize (when used) */
    real *tabq_coul_F;
    /* Coulomb energy table, size of array is tabsize (when used) */
    real *tabq_coul_V;
    /* Coulomb force+energy table, size of array is tabsize*4 (when used) */
    real *tabq_coul_FDV0;
} interaction_const_t;

#ifdef __cplusplus
}
#endif

#endif /* _INTERACTION_CONST_ */