/************************* control.c *******************************/
/* MIMD version 7 */
/* Main procedure for SU3 with dynamical staggered fermions        */
/* general quark action, general gauge action */

/* This file is for lattice generation with the RHMC algorithm */

#define CONTROL
#include "ks_imp_includes.h"	/* definitions files and prototypes */
#include "lattice_qdp.h"

#ifdef MILC_GLOBAL_DEBUG
#include "debug.h"
#endif /* MILC_GLOBAL_DEBUG */

/* For information */
#define NULL_FP -1

EXTERN gauge_header start_lat_hdr;	/* Input gauge field header */

int
main( int argc, char **argv )
{
  int meascount,traj_done,i;
  int prompt;
  int s_iters, avs_iters, avspect_iters, avbcorr_iters;
  double dtime, dclock();
  
  initialize_machine(&argc,&argv);
#ifdef HAVE_QDP
  QDP_initialize(&argc, &argv);
#ifndef QDP_PROFILE
  QDP_profcontrol(0);
#endif
#endif
  /* Remap standard I/O */
  if(remap_stdio_from_args(argc, argv) == 1)terminate(1);
  
  g_sync();
  /* set up */
  prompt = setup();

  /* loop over input sets */
  while( readin(prompt) == 0) {
    
    /* perform warmup trajectories */
#ifdef MILC_GLOBAL_DEBUG
    global_current_time_step = 0;
#endif /* MILC_GLOBAL_DEBUG */

    dtime = -dclock();
    for( traj_done=0; traj_done < warms; traj_done++ ){
      update();
    }
    node0_printf("WARMUPS COMPLETED\n"); fflush(stdout);
    
    /* perform measuring trajectories, reunitarizing and measuring 	*/
    meascount=0;		/* number of measurements 		*/
    avspect_iters = avs_iters = avbcorr_iters = 0;

    for( traj_done=0; traj_done < trajecs; traj_done++ ){ 
#ifdef MILC_GLOBAL_DEBUG
#ifdef HISQ_REUNITARIZATION_DEBUG
  {
  int isite, idir;
  site *s;
  FORALLSITES(isite,s) {
    for( idir=XUP;idir<=TUP;idir++ ) {
      lattice[isite].on_step_Y[idir] = 0;
      lattice[isite].on_step_W[idir] = 0;
      lattice[isite].on_step_V[idir] = 0;
    }
  }
  }
#endif /* HISQ_REUNITARIZATION_DEBUG */
#endif /* MILC_GLOBAL_DEBUG */
      /* do the trajectories */
      s_iters=update();

      /* measure every "propinterval" trajectories */
      if( (traj_done%propinterval)==(propinterval-1) ){
	
	/* call gauge_variable fermion_variable measuring routines */
	/* results are printed in output file */
	rephase(OFF);
	g_measure( );
#ifdef MILC_GLOBAL_DEBUG
        g_measure_plaq( );
#ifdef MEASURE_AND_TUNE_HISQ
        g_measure_tune( );
#endif /* MEASURE_AND_TUNE_HISQ */
#endif /* MILC_GLOBAL_DEBUG */
	rephase(ON);
	/* Do some fermion measurements */
#ifdef SPECTRUM 
	/* Fix TUP Coulomb gauge - gauge links only*/
	rephase( OFF );
	gaugefix(TUP,(Real)1.8,500,(Real)GAUGE_FIX_TOL);
	rephase( ON );

	invalidate_all_ferm_links(&fn_links);
#ifdef DM_DU0
	invalidate_all_ferm_links(&fn_links_dmdu0);
#endif
#endif

	for(i=0;i<n_dyn_masses;i++){
	  // Remake the path table if the fermion coeffs change for this mass
// DT IT CAN"T BE RIGHT TO CALL IT WITH dyn_mass
	  //if(make_path_table(&ks_act_paths, &ks_act_paths_dmdu0,dyn_mass[i]))
	  if(make_path_table(&ks_act_paths, &ks_act_paths_dmdu0,   0.0/*TEMP*/   ))
	    {
	      // If they change, invalidate only fat and long links
node0_printf("INVALIDATE\n");
	      invalidate_all_ferm_links(&fn_links);
#ifdef DM_DU0
	      invalidate_all_ferm_links(&fn_links_dmdu0);
#endif
	    }
	    /* Load fat and long links for fermion measurements if needed */
	    load_ferm_links(&fn_links, &ks_act_paths);
#ifdef DM_DU0
	    load_ferm_links(&fn_links_dmdu0, &ks_act_paths_dmdu0);
#endif
	    
	    f_meas_imp( F_OFFSET(phi1), F_OFFSET(xxx1), dyn_mass[i],
			&fn_links, &fn_links_dmdu0);
	    /* Measure derivatives wrto chemical potential */
#ifdef D_CHEM_POT
	    Deriv_O6( F_OFFSET(phi1), F_OFFSET(xxx1), F_OFFSET(xxx2), 
		      dyn_mass[i], &fn_links, &fn_links_dmdu0);
#endif
	    
#ifdef SPECTRUM 
	    if(strstr(spectrum_request,",spectrum,") != NULL)
	      avspect_iters += spectrum2( dyn_mass[i], F_OFFSET(phi1),
					  F_OFFSET(xxx1), &fn_links);
	    
	    if(strstr(spectrum_request,",spectrum_point,") != NULL)
	      avspect_iters += spectrum_fzw( dyn_mass[i], F_OFFSET(phi1),
					     F_OFFSET(xxx1), &fn_links);
	    
	    if(strstr(spectrum_request,",nl_spectrum,") != NULL)
	      avspect_iters += nl_spectrum( dyn_mass[i], F_OFFSET(phi1), 
					    F_OFFSET(xxx1), 
					    F_OFFSET(tempmat1),
					    F_OFFSET(staple),
					    &fn_links);
	    
	    if(strstr(spectrum_request,",spectrum_mom,") != NULL)
	      avspect_iters += spectrum_mom( dyn_mass[i], dyn_mass[i], 
					     F_OFFSET(phi1), 1e-1,
					     &fn_links);
	    
	    // For now we can't do the off-diagonal spectrum if Dirac operators
            // depend on masses.  We need two propagators
	    // if(strstr(spectrum_request,",spectrum_multimom,") != NULL)
	    //     avspect_iters += spectrum_multimom(dyn_mass[i],
	    //				 spectrum_multimom_low_mass,
	    //				 spectrum_multimom_mass_step,
	    //				 spectrum_multimom_nmasses,
	    //				 5e-3, &fn_links);

	    // For now we can't do the off-diagonal spectrum if Dirac operators
            // depend on masses.  We need two propagators
	    //	    if(strstr(spectrum_request,",spectrum_nd,") != NULL){
	    //	      avspect_iters += spectrum_nd( mass1, mass2, 1e-1,
	    //					    &fn_links);

	    if(strstr(spectrum_request,",spectrum_nlpi2,") != NULL)
	      avspect_iters += spectrum_nlpi2( dyn_mass[i], dyn_mass[i],
					       F_OFFSET(phi1),1e-1,
					       &fn_links );
	
	    if(strstr(spectrum_request,",spectrum_singlets,") != NULL)
	      avspect_iters += spectrum_singlets(dyn_mass[i], 5e-3, 
						 F_OFFSET(phi1), &fn_links );

	    // For now we can't do the off-diagonal spectrum if Dirac operators
            // depend on masses.  We need two propagators
	    // if(strstr(spectrum_request,",fpi,") != NULL)
	    // avspect_iters += fpi_2( fpi_mass, fpi_nmasses, 2e-3,
	    //			    &fn_links );
	
#ifdef HYBRIDS
	  if(strstr(spectrum_request,",spectrum_hybrids,") != NULL)
	    avspect_iters += spectrum_hybrids( dyn_mass[i], F_OFFSET(phi1), 
					       5e-3, &fn_links);
#endif
	  if(strstr(spectrum_request,",hvy_pot,") != NULL){
	    rephase( OFF );
	    hvy_pot( F_OFFSET(link[XUP]) );
	    rephase( ON );
	  }
#endif
	}
	avs_iters += s_iters;
	++meascount;
	fflush(stdout);
      }
    }	/* end loop over trajectories */
    
    node0_printf("RUNNING COMPLETED\n"); fflush(stdout);
    if(meascount>0)  {
      node0_printf("average cg iters for step= %e\n",
		   (double)avs_iters/meascount);
    }
    
    dtime += dclock();
    if(this_node==0){
      printf("Time = %e seconds\n",dtime);
      printf("total_iters = %d\n",total_iters);
    }
    fflush(stdout);
    
    /* save lattice if requested */
    if( saveflag != FORGET ){
      rephase( OFF );
      save_lattice( saveflag, savefile, stringLFN );
      rephase( ON );
    }
  }
#ifdef HAVE_QDP
  QDP_finalize();
#endif  
  normal_exit(0);
  return 0;
}
