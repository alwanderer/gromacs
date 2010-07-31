/*
 * 
 *                This source code is part of
 * 
 *                 G   R   O   M   A   C   S
 * 
 *          GROningen MAchine for Chemical Simulations
 * 
 *                        VERSION 3.3.2
 * Written by David van der Spoel, Erik Lindahl, Berk Hess, and others.
 * Copyright (c) 1991-2000, University of Groningen, The Netherlands.
 * Copyright (c) 2001-2007, The GROMACS development team,
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
 * Groningen Machine for Chemical Simulation
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <math.h>
#include <stdlib.h>

#include "sysstuff.h"
#include "string.h"
#include "typedefs.h"
#include "smalloc.h"
#include "macros.h"
#include "vec.h"
#include "xvgr.h"
#include "pbc.h"
#include "copyrite.h"
#include "futil.h"
#include "statutil.h"
#include "index.h"
#include "nsc.h"
#include "pdbio.h"
#include "confio.h"
#include "rmpbc.h"
#include "names.h"
#include "atomprop.h"
#include "physics.h"
#include "tpxio.h"
#include "gmx_ana.h"


typedef struct {
  atom_id  aa,ab;
  real     d2a,d2b;
} t_conect;

void add_rec(t_conect c[],atom_id i,atom_id j,real d2)
{
  if (c[i].aa == NO_ATID) {
    c[i].aa  = j;
    c[i].d2a = d2;
  }
  else if (c[i].ab == NO_ATID) {
    c[i].ab  = j;
    c[i].d2b = d2;
  }
  else if (d2 < c[i].d2a) {
    c[i].aa  = j;
    c[i].d2a = d2;
  }
  else if (d2 < c[i].d2b) {
    c[i].ab  = j;
    c[i].d2b = d2;
  }
  /* Swap them if necessary: a must be larger than b */
  if (c[i].d2a < c[i].d2b) {
    j        = c[i].ab;
    c[i].ab  = c[i].aa;
    c[i].aa  = j;
    d2       = c[i].d2b;
    c[i].d2b = c[i].d2a;
    c[i].d2a = d2;
  }
}

void do_conect(const char *fn,int n,rvec x[])
{
  FILE     *fp;
  int      i,j;
  t_conect *c;
  rvec     dx;
  real     d2;
  
  fprintf(stderr,"Building CONECT records\n");
  snew(c,n);
  for(i=0; (i<n); i++) 
    c[i].aa = c[i].ab = NO_ATID;
  
  for(i=0; (i<n); i++) {
    for(j=i+1; (j<n); j++) {
      rvec_sub(x[i],x[j],dx);
      d2 = iprod(dx,dx);
      add_rec(c,i,j,d2);
      add_rec(c,j,i,d2);
    }
  }
  fp = ffopen(fn,"a");
  for(i=0; (i<n); i++) {
    if ((c[i].aa == NO_ATID) || (c[i].ab == NO_ATID))
      fprintf(stderr,"Warning dot %d has no conections\n",i+1);
    fprintf(fp,"CONECT%5d%5d%5d\n",i+1,c[i].aa+1,c[i].ab+1);
  }
  ffclose(fp);
  sfree(c);
}

void connelly_plot(const char *fn,int ndots,real dots[],rvec x[],t_atoms *atoms,
		   t_symtab *symtab,int ePBC,matrix box,bool bSave)
{
  static const char *atomnm="DOT";
  static const char *resnm ="DOT";
  static const char *title ="Connely Dot Surface Generated by g_sas";

  int  i,i0,r0,ii0,k;
  rvec *xnew;
  t_atoms aaa;

  if (bSave) {  
    i0 = atoms->nr;
    r0 = atoms->nres;
    srenew(atoms->atom,atoms->nr+ndots);
    srenew(atoms->atomname,atoms->nr+ndots);
    srenew(atoms->resinfo,r0+1);
    atoms->atom[i0].resind = r0;
    t_atoms_set_resinfo(atoms,i0,symtab,resnm,r0+1,' ',0,' ');
    srenew(atoms->pdbinfo,atoms->nr+ndots);
    snew(xnew,atoms->nr+ndots);
    for(i=0; (i<atoms->nr); i++)
      copy_rvec(x[i],xnew[i]);
    for(i=k=0; (i<ndots); i++) {
      ii0 = i0+i;
      atoms->atomname[ii0] = put_symtab(symtab,atomnm);
      atoms->pdbinfo[ii0].type = epdbATOM;
      atoms->pdbinfo[ii0].atomnr= ii0;
      atoms->atom[ii0].resind = r0;
      xnew[ii0][XX] = dots[k++];
      xnew[ii0][YY] = dots[k++];
      xnew[ii0][ZZ] = dots[k++];
      atoms->pdbinfo[ii0].bfac  = 0.0;
      atoms->pdbinfo[ii0].occup = 0.0;
    }
    atoms->nr   = i0+ndots;
    atoms->nres = r0+1;
    write_sto_conf(fn,title,atoms,xnew,NULL,ePBC,box);
    atoms->nres = r0;
    atoms->nr   = i0;
  }
  else {
    init_t_atoms(&aaa,ndots,TRUE);
    aaa.atom[0].resind = 0;
    t_atoms_set_resinfo(&aaa,0,symtab,resnm,1,' ',0,' ');
    snew(xnew,ndots);
    for(i=k=0; (i<ndots); i++) {
      ii0 = i;
      aaa.atomname[ii0] = put_symtab(symtab,atomnm);
      aaa.pdbinfo[ii0].type = epdbATOM;
      aaa.pdbinfo[ii0].atomnr= ii0;
      aaa.atom[ii0].resind = 0;
      xnew[ii0][XX] = dots[k++];
      xnew[ii0][YY] = dots[k++];
      xnew[ii0][ZZ] = dots[k++];
      aaa.pdbinfo[ii0].bfac  = 0.0;
      aaa.pdbinfo[ii0].occup = 0.0;
    }
    aaa.nr = ndots;
    write_sto_conf(fn,title,&aaa,xnew,NULL,ePBC,box);
    do_conect(fn,ndots,xnew);
    free_t_atoms(&aaa,FALSE);
  }
  sfree(xnew);
}

real calc_radius(char *atom)
{
  real r;
  
  switch (atom[0]) {
  case 'C':
    r = 0.16;
    break;
  case 'O':
    r = 0.13;
    break;
  case 'N':
    r = 0.14;
    break;
  case 'S':
    r = 0.2;
    break;
  case 'H':
    r = 0.1;
    break;
  default:
    r = 1e-3;
  }
  return r;
}

void sas_plot(int nfile,t_filenm fnm[],real solsize,int ndots,
	      real qcut,bool bSave,real minarea,bool bPBC,
	      real dgs_default,bool bFindex, const output_env_t oenv)
{
  FILE         *fp,*fp2,*fp3=NULL,*vp;
  const char   *flegend[] = { "Hydrophobic", "Hydrophilic", 
			      "Total", "D Gsolv" };
  const char   *vlegend[] = { "Volume (nm\\S3\\N)", "Density (g/l)" };
  const char   *vfile;
  real         t;
  gmx_atomprop_t aps=NULL;
  gmx_rmpbc_t  gpbc=NULL;
  t_trxstatus  *status;
  int          ndefault;
  int          i,j,ii,nfr,natoms,flag,nsurfacedots,res;
  rvec         *xtop,*x;
  matrix       topbox,box;
  t_topology   top;
  char         title[STRLEN];
  int          ePBC;
  bool         bTop;
  t_atoms      *atoms;
  bool         *bOut,*bPhobic;
  bool         bConnelly;
  bool         bResAt,bITP,bDGsol;
  real         *radius,*dgs_factor=NULL,*area=NULL,*surfacedots=NULL;
  real         at_area,*atom_area=NULL,*atom_area2=NULL;
  real         *res_a=NULL,*res_area=NULL,*res_area2=NULL;
  real         totarea,totvolume,totmass=0,density,harea,tarea,fluc2;
  atom_id      **index,*findex;
  int          *nx,nphobic,npcheck,retval;
  char         **grpname,*fgrpname;
  real         dgsolv;

  bITP   = opt2bSet("-i",nfile,fnm);
  bResAt = opt2bSet("-or",nfile,fnm) || opt2bSet("-oa",nfile,fnm) || bITP;

  bTop = read_tps_conf(ftp2fn(efTPS,nfile,fnm),title,&top,&ePBC,
		       &xtop,NULL,topbox,FALSE);
  atoms = &(top.atoms);
  
  if (!bTop) {
    fprintf(stderr,"No tpr file, will not compute Delta G of solvation\n");
    bDGsol = FALSE;
  } else {
    bDGsol = strcmp(*(atoms->atomtype[0]),"?") != 0;
    if (!bDGsol) {
      fprintf(stderr,"Warning: your tpr file is too old, will not compute "
	      "Delta G of solvation\n");
    } else {
      printf("In case you use free energy of solvation predictions:\n");
      please_cite(stdout,"Eisenberg86a");
    }
  }

  aps = gmx_atomprop_init();
  
  if ((natoms=read_first_x(oenv,&status,ftp2fn(efTRX,nfile,fnm),
			   &t,&x,box))==0)
    gmx_fatal(FARGS,"Could not read coordinates from statusfile\n");

  if ((ePBC != epbcXYZ) || (TRICLINIC(box))) {
    fprintf(stderr,"\n\nWARNING: non-rectangular boxes may give erroneous results or crashes.\n"
	    "Analysis based on vacuum simulations (with the possibility of evaporation)\n" 
	    "will certainly crash the analysis.\n\n");
  }
  snew(nx,2);
  snew(index,2);
  snew(grpname,2);
  fprintf(stderr,"Select a group for calculation of surface and a group for output:\n");
  get_index(atoms,ftp2fn_null(efNDX,nfile,fnm),2,nx,index,grpname);

  if (bFindex) {
    fprintf(stderr,"Select a group of hydrophobic atoms:\n");
    get_index(atoms,ftp2fn_null(efNDX,nfile,fnm),1,&nphobic,&findex,&fgrpname);
  }
  snew(bOut,natoms);
  for(i=0; i<nx[1]; i++)
    bOut[index[1][i]] = TRUE;

  /* Now compute atomic readii including solvent probe size */
  snew(radius,natoms);
  snew(bPhobic,nx[0]);
  if (bResAt) {
    snew(atom_area,nx[0]);
    snew(atom_area2,nx[0]);
    snew(res_a,atoms->nres);
    snew(res_area,atoms->nres);
    snew(res_area2,atoms->nres);
  }
  if (bDGsol)
    snew(dgs_factor,nx[0]);

  /* Get a Van der Waals radius for each atom */
  ndefault = 0;
  for(i=0; (i<natoms); i++) {
    if (!gmx_atomprop_query(aps,epropVDW,
			    *(atoms->resinfo[atoms->atom[i].resind].name),
			    *(atoms->atomname[i]),&radius[i]))
      ndefault++;
    /* radius[i] = calc_radius(*(top->atoms.atomname[i])); */
    radius[i] += solsize;
  }
  if (ndefault > 0)
    fprintf(stderr,"WARNING: could not find a Van der Waals radius for %d atoms\n",ndefault);
  /* Determine which atom is counted as hydrophobic */
  if (bFindex) {
    npcheck = 0;
    for(i=0; (i<nx[0]); i++) {
      ii = index[0][i];
      for(j=0; (j<nphobic); j++) {
	if (findex[j] == ii) {
	  bPhobic[i] = TRUE;
	  if (bOut[ii])
	    npcheck++;
	}
      }
    }
    if (npcheck != nphobic)
      gmx_fatal(FARGS,"Consistency check failed: not all %d atoms in the hydrophobic index\n"
		  "found in the normal index selection (%d atoms)",nphobic,npcheck);
  }
  else
    nphobic = 0;
    
  for(i=0; (i<nx[0]); i++) {
    ii = index[0][i];
    if (!bFindex) {
      bPhobic[i] = fabs(atoms->atom[ii].q) <= qcut;
      if (bPhobic[i] && bOut[ii])
	nphobic++;
    }
    if (bDGsol)
      if (!gmx_atomprop_query(aps,epropDGsol,
			      *(atoms->resinfo[atoms->atom[ii].resind].name),
			      *(atoms->atomtype[ii]),&(dgs_factor[i])))
	dgs_factor[i] = dgs_default;
    if (debug)
      fprintf(debug,"Atom %5d %5s-%5s: q= %6.3f, r= %6.3f, dgsol= %6.3f, hydrophobic= %s\n",
	      ii+1,*(atoms->resinfo[atoms->atom[ii].resind].name),
	      *(atoms->atomname[ii]),
	      atoms->atom[ii].q,radius[ii]-solsize,dgs_factor[i],
	      BOOL(bPhobic[i]));
  }
  fprintf(stderr,"%d out of %d atoms were classified as hydrophobic\n",
	  nphobic,nx[1]);
  
  fp=xvgropen(opt2fn("-o",nfile,fnm),"Solvent Accessible Surface","Time (ps)",
	      "Area (nm\\S2\\N)",oenv);
  xvgr_legend(fp,asize(flegend) - (bDGsol ? 0 : 1),flegend,oenv);
  vfile = opt2fn_null("-tv",nfile,fnm);
  if (vfile) {
    if (!bTop) {
      gmx_fatal(FARGS,"Need a tpr file for option -tv");
    }
    vp=xvgropen(vfile,"Volume and Density","Time (ps)","",oenv);
    xvgr_legend(vp,asize(vlegend),vlegend,oenv);
    totmass  = 0;
    ndefault = 0;
    for(i=0; (i<nx[0]); i++) {
      real mm;
      ii = index[0][i];
      /*
      if (!query_atomprop(atomprop,epropMass,
			  *(top->atoms.resname[top->atoms.atom[ii].resnr]),
			  *(top->atoms.atomname[ii]),&mm))
	ndefault++;
      totmass += mm;
      */
      totmass += atoms->atom[ii].m;
    }
    if (ndefault)
      fprintf(stderr,"WARNING: Using %d default masses for density calculation, which most likely are inaccurate\n",ndefault);
  }
  else
    vp = NULL;
    
  gmx_atomprop_destroy(aps);

  if (bPBC)
    gpbc = gmx_rmpbc_init(&top.idef,ePBC,natoms,box);
  
  nfr=0;
  do {
    if (bPBC)
      gmx_rmpbc(gpbc,box,x,x);
    
    bConnelly = (nfr==0 && opt2bSet("-q",nfile,fnm));
    if (bConnelly) {
      if (!bTop)
	gmx_fatal(FARGS,"Need a tpr file for Connelly plot");
      flag = FLAG_ATOM_AREA | FLAG_DOTS;
    } else {
      flag = FLAG_ATOM_AREA;
    }
    if (vp) {
      flag = flag | FLAG_VOLUME;
    }
      
    if (debug)
      write_sto_conf("check.pdb","pbc check",atoms,x,NULL,ePBC,box);

    retval = nsc_dclm_pbc(x,radius,nx[0],ndots,flag,&totarea,
			  &area,&totvolume,&surfacedots,&nsurfacedots,
			  index[0],ePBC,bPBC ? box : NULL);
    if (retval)
      gmx_fatal(FARGS,"Something wrong in nsc_dclm_pbc");
    
    if (bConnelly)
      connelly_plot(ftp2fn(efPDB,nfile,fnm),
		    nsurfacedots,surfacedots,x,atoms,
		    &(top.symtab),ePBC,box,bSave);
    harea  = 0; 
    tarea  = 0;
    dgsolv = 0;
    if (bResAt)
      for(i=0; i<atoms->nres; i++)
	res_a[i] = 0;
    for(i=0; (i<nx[0]); i++) {
      ii = index[0][i];
      if (bOut[ii]) {
	at_area = area[i];
	if (bResAt) {
	  atom_area[i] += at_area;
	  atom_area2[i] += sqr(at_area);
	  res_a[atoms->atom[ii].resind] += at_area;
	}
	tarea += at_area;
	if (bDGsol)
	  dgsolv += at_area*dgs_factor[i];
	if (bPhobic[i])
	  harea += at_area;
      }
    }
    if (bResAt)
      for(i=0; i<atoms->nres; i++) {
	res_area[i] += res_a[i];
	res_area2[i] += sqr(res_a[i]);
      }
    fprintf(fp,"%10g  %10g  %10g  %10g",t,harea,tarea-harea,tarea);
    if (bDGsol)
      fprintf(fp,"  %10g\n",dgsolv);
    else
      fprintf(fp,"\n");
    
    /* Print volume */
    if (vp) {
      density = totmass*AMU/(totvolume*NANO*NANO*NANO);
      fprintf(vp,"%12.5e  %12.5e  %12.5e\n",t,totvolume,density);
    }
    if (area) {
      sfree(area);
      area = NULL;
    }
    if (surfacedots) {
      sfree(surfacedots);
      surfacedots = NULL;
    }
    nfr++;
  } while (read_next_x(oenv,status,&t,natoms,x,box));

  if (bPBC)  
    gmx_rmpbc_done(gpbc);

  fprintf(stderr,"\n");
  close_trj(status);
  ffclose(fp);
  if (vp)
    ffclose(vp);
    
  /* if necessary, print areas per atom to file too: */
  if (bResAt) {
    for(i=0; i<atoms->nres; i++) {
      res_area[i] /= nfr;
      res_area2[i] /= nfr;
    }
    for(i=0; i<nx[0]; i++) {
      atom_area[i] /= nfr;
      atom_area2[i] /= nfr;
    }
    fprintf(stderr,"Printing out areas per atom\n");
    fp  = xvgropen(opt2fn("-or",nfile,fnm),"Area per residue","Residue",
		   "Area (nm\\S2\\N)",oenv);
    fp2 = xvgropen(opt2fn("-oa",nfile,fnm),"Area per atom","Atom #",
		   "Area (nm\\S2\\N)",oenv);
    if (bITP) {
      fp3 = ftp2FILE(efITP,nfile,fnm,"w");
      fprintf(fp3,"[ position_restraints ]\n"
	      "#define FCX 1000\n"
	      "#define FCY 1000\n"
	      "#define FCZ 1000\n"
	      "; Atom  Type  fx   fy   fz\n");
    }
    for(i=0; i<nx[0]; i++) {
      ii = index[0][i];
      res = atoms->atom[ii].resind;
      if (i==nx[0]-1 || res!=atoms->atom[index[0][i+1]].resind) {
	fluc2 = res_area2[res]-sqr(res_area[res]);
	if (fluc2 < 0)
	  fluc2 = 0;
	fprintf(fp,"%10d  %10g %10g\n",
		atoms->resinfo[res].nr,res_area[res],sqrt(fluc2));
      }
      fluc2 = atom_area2[i]-sqr(atom_area[i]);
      if (fluc2 < 0)
	fluc2 = 0;
      fprintf(fp2,"%d %g %g\n",index[0][i]+1,atom_area[i],sqrt(fluc2));
      if (bITP && (atom_area[i] > minarea))
	fprintf(fp3,"%5d   1     FCX  FCX  FCZ\n",ii+1);
    }
    if (bITP)
      ffclose(fp3);
    ffclose(fp);
  }

    /* Be a good citizen, keep our memory free! */
    sfree(x);
    sfree(nx);
    for(i=0;i<2;i++)
    {
        sfree(index[i]);
        sfree(grpname[i]);
    }
    sfree(bOut);
    sfree(radius);
    sfree(bPhobic);
    
    if(bResAt)
    {
        sfree(atom_area);
        sfree(atom_area2);
        sfree(res_a);
        sfree(res_area);
        sfree(res_area2);
    }
    if(bDGsol)
    {
        sfree(dgs_factor);
    }
}

int gmx_sas(int argc,char *argv[])
{
  const char *desc[] = {
    "g_sas computes hydrophobic, hydrophilic and total solvent accessible surface area.",
    "As a side effect the Connolly surface can be generated as well in",
    "a pdb file where the nodes are represented as atoms and the vertices",
    "connecting the nearest nodes as CONECT records.",
    "The program will ask for a group for the surface calculation",
    "and a group for the output. The calculation group should always",
    "consists of all the non-solvent atoms in the system.",
    "The output group can be the whole or part of the calculation group.",
    "The area can be plotted",
    "per residue and atom as well (options [TT]-or[tt] and [TT]-oa[tt]).",
    "In combination with the latter option an [TT]itp[tt] file can be",
    "generated (option [TT]-i[tt])",
    "which can be used to restrain surface atoms.[PAR]",
    "By default, periodic boundary conditions are taken into account,",
    "this can be turned off using the [TT]-nopbc[tt] option.[PAR]",
    "With the [TT]-tv[tt] option the total volume and density of the molecule can be",
    "computed.",
    "Please consider whether the normal probe radius is appropriate",
    "in this case or whether you would rather use e.g. 0. It is good",
    "to keep in mind that the results for volume and density are very",
    "approximate, in e.g. ice Ih one can easily fit water molecules in the",
    "pores which would yield too low volume, too high surface area and too",
    "high density."
  };

  output_env_t oenv;
  static real solsize = 0.14;
  static int  ndots   = 24;
  static real qcut    = 0.2;
  static real minarea = 0.5, dgs_default=0;
  static bool bSave   = TRUE,bPBC=TRUE,bFindex=FALSE;
  t_pargs pa[] = {
    { "-probe", FALSE, etREAL, {&solsize},
      "Radius of the solvent probe (nm)" },
    { "-ndots",   FALSE, etINT,  {&ndots},
      "Number of dots per sphere, more dots means more accuracy" },
    { "-qmax",    FALSE, etREAL, {&qcut},
      "The maximum charge (e, absolute value) of a hydrophobic atom" },
    { "-f_index", FALSE, etBOOL, {&bFindex},
      "Determine from a group in the index file what are the hydrophobic atoms rather than from the charge" },
    { "-minarea", FALSE, etREAL, {&minarea},
      "The minimum area (nm^2) to count an atom as a surface atom when writing a position restraint file  (see help)" },
    { "-pbc",     FALSE, etBOOL, {&bPBC},
      "Take periodicity into account" },
    { "-prot",    FALSE, etBOOL, {&bSave},
      "Output the protein to the connelly pdb file too" },
    { "-dgs",     FALSE, etREAL, {&dgs_default},
      "default value for solvation free energy per area (kJ/mol/nm^2)" }
  };
  t_filenm  fnm[] = {
    { efTRX, "-f",   NULL,       ffREAD },
    { efTPS, "-s",   NULL,       ffREAD },
    { efXVG, "-o",   "area",     ffWRITE },
    { efXVG, "-or",  "resarea",  ffOPTWR },
    { efXVG, "-oa",  "atomarea", ffOPTWR },
    { efXVG, "-tv",  "volume",   ffOPTWR },
    { efPDB, "-q",   "connelly", ffOPTWR },
    { efNDX, "-n",   "index",    ffOPTRD },
    { efITP, "-i",   "surfat",   ffOPTWR }
  };
#define NFILE asize(fnm)

  CopyRight(stderr,argv[0]);
  parse_common_args(&argc,argv,PCA_CAN_VIEW | PCA_CAN_TIME | PCA_BE_NICE,
		    NFILE,fnm,asize(pa),pa,asize(desc),desc,0,NULL,&oenv);
  if (solsize < 0) {
    solsize=1e-3;
    fprintf(stderr,"Probe size too small, setting it to %g\n",solsize);
  }
  if (ndots < 20) {
    ndots = 20;
    fprintf(stderr,"Ndots too small, setting it to %d\n",ndots);
  }

  please_cite(stderr,"Eisenhaber95");
    
  sas_plot(NFILE,fnm,solsize,ndots,qcut,bSave,minarea,bPBC,dgs_default,bFindex,
          oenv);
  
  do_view(oenv,opt2fn("-o",NFILE,fnm),"-nxy");
  do_view(oenv,opt2fn_null("-or",NFILE,fnm),"-nxy");
  do_view(oenv,opt2fn_null("-oa",NFILE,fnm),"-nxy");

  thanx(stderr);
  
  return 0;
}
