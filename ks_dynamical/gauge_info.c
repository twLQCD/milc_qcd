/*********************** gauge_info.c *************************/
/* MIMD version 6 */

/* For ks_dynamical */

/* Application-dependent routine for writing gauge info file */
/* This file is an ASCII companion to the gauge configuration file
   and contains information about the action used to generate it.
   This information is consistently written in the pattern

       keyword  value

   or

       keyword[n] value1 value2 ... valuen

   where n is an integer.

   To maintain a semblance of consistency, the possible keywords are
   listed in io_lat.h.  Add more as the need arises, but be sure
   to notify the rest of the collaboration.

   */

#include "ks_dyn_includes.h"

/*---------------------------------------------------------------------------*/
/* This routine writes the ASCII info file.  It is called from one of
   the lattice output routines in io_lat4.c.*/

void write_appl_gauge_info(FILE *fp, gauge_file *gf)
{

  /* Write generic information */
  write_generic_gauge_info(fp, gf);


  /* The rest are optional */

  write_gauge_info_item(fp,"action.description","%s",
			"\"Gauge plus fermion\"",0,0);
  write_gauge_info_item(fp,"gauge.description","%s",
			"\"One plaquette gauge action\"",0,0);
  write_gauge_info_item(fp,"gauge.beta11","%f",(char *)&beta,0,0);
  write_gauge_info_item(fp,"quark.description","%s","\"KS fermions\"",0,0);
  write_gauge_info_item(fp,"quark.flavors","%d",(char *)&nflavors,0,0);
  write_gauge_info_item(fp,"quark.mass","%f",(char *)&mass,0,0);

}
