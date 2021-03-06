/*********************** write_propagating_form.c *************************/
/* MIMD version 6 */
/* NOT MAINTAINED.  TEST BEFORE USE. Consider using write_propagating_form_lean.c */
/* 
 *  Write out the propagating form factors to 
 *  a disk file.
 *
 *
 *
 *  NOTE this is scalar code -- it should only be run on 
 *  one node.
 *
 *  Subroutine argument
 *   filename  :: the name of the disk file
 *   corr      :: the propagating three point function
 *   no_zonked :: the number of zonked kappa values
 *   hl_flag   :: flag saying where the heavy-heavy or  heavy-light form factors 
 *                are to be written.
 *   dim       :: the amount of data
 *
 */

/** $Header: /lqcdproj/detar/cvsroot/milc_qcd/propagating_form_factor/write_propagating_form.c,v 1.1 2005/02/23 00:05:53 detar Exp $ ***/

#include "prop_form_includes.h"

#ifdef DEBUGDEF
#include "debug_form.h"
#endif


void write_prop_form(complex *corr, char filename[], 
int hl_flag, int no_zonked, int numer_of_operators, int no_copies, int dim)
{
  FILE *fp ;
  size_t nobj = (size_t) dim ;
  /* Hack to distinguish single and double precision files */
  const int magic_number = 14567332 + 8*(sizeof(Real) - 4)); 
  const int version_number  = 1  ; /** update this flag when the data format changes ***/
  size_t writecount ;
  int check_sum ; 

  int32type p_momstore_local[MAXPMOM][3] ;
  int32type q_momstore_local[MAXMOM][3] ;
  int i , p ; 


  /* Memory for some header information ***/
#define HEADER_DIM_WRITE_CORR 13
  size_t  header_size = (size_t) HEADER_DIM_WRITE_CORR ;
  int32type header_data[HEADER_DIM_WRITE_CORR] ;
#undef HEADER_DIM_WRITE_CORR

  /*** calculate the checksum of the form factors ****/
  check_sum =  bsd_sum((char*)corr, sizeof(complex)*dim)  ;


  /***** load up the header *****/
  header_data[0] = (int32type) magic_number ;
  header_data[1] =  (int32type) version_number ;
  header_data[2] =  (int32type) nt ;
  header_data[3] =  (int32type) check_sum ;
  header_data[4] =  (int32type) no_p_values ;
  header_data[5] =  (int32type) no_q_values ;
  header_data[6] =  (int32type) numer_of_operators ;
  header_data[7] =  (int32type) no_spectator  ;
  header_data[8] =  (int32type) no_zonked  ;
  header_data[9] =  (int32type) dim ;
  header_data[10] =  (int32type) hl_flag ;
  header_data[11] =  (int32type) no_sequential  ;
  header_data[12] =  (int32type) no_copies ;


  /**** open the file ******/
  if( (fp = fopen(filename ,"wb")) == NULL )
  {
    printf("ERROR::write_prop_form::Could not open the file %s\n",filename);
    terminate(1);
  }

  /** write the header information ***/
  if( fwrite(header_data,sizeof(int32type),header_size,fp) != header_size )
  {
    printf("There was an error during the writing of the form factor HEADER to %s\n",
	   filename);
    terminate(1);
  }


  /*** write the external q--momentum to file ***/
  writecount = (size_t) 3*no_q_values ;
  for(i = 0 ; i < 3 ; ++i)
    for(p = 0 ; p < no_q_values ; ++p )
      q_momstore_local[p][i] =  q_momstore[p][i] ; 

  if( fwrite(q_momstore_local  ,sizeof(int32type),writecount,fp) != writecount   )
  {
    printf("There was an error during the writing of the Q momentum table to %s\n",
	   filename);
    terminate(1);
  }


  /*** write the external p--momentum to file ***/
  writecount = (size_t) 3*no_p_values ;
  for(i = 0 ; i < 3 ; ++i)
    for(p = 0 ; p < no_p_values ; ++p )
      p_momstore_local[p][i] =  p_momstore[p][i] ; 


  if( fwrite(p_momstore_local  ,sizeof(int32type),writecount,fp) != writecount   )
  {
    printf("There was an error during the writing of the momentum table to %s\n",filename);
    terminate(1);
  }




  /** write the three point function to disk *****/
  if( fwrite(corr,sizeof(complex),nobj,fp) != nobj   )
  {
    printf("There was an error during the writing of the form factors to %s\n",
	   filename);
    terminate(1);
  }


  /*** close the file ****/
  if( fclose(fp) != 0 )
  {
    printf("There was an error during the closing of %s \n",filename);
    terminate(1);
  }


  printf("I have written propagating form factors to the file %s\n",filename);

}


