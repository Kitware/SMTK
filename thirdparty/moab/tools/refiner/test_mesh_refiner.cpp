#include "moab/Core.hpp"
#include "EdgeSizeSimpleImplicit.hpp"
#include "SimplexTemplateRefiner.hpp"
#include "MeshRefiner.hpp"
#include "moab/Interface.hpp"
#include "MBParallelConventions.h"

#ifdef MOAB_HAVE_MPI
#include "moab/ParallelComm.hpp"
#include "moab_mpi.h"
#endif // MOAB_HAVE_MPI

#include <iostream>
#include <sstream>
#include <map>

#include "sys/time.h"

using namespace moab;

#define STRINGIFY_(A) #A
#define STRINGIFY(A) STRINGIFY_(A)
#ifdef MESHDIR
std::string TestDir( STRINGIFY(MESHDIR) );
#else
std::string TestDir( "./" );
#endif

int TestMeshRefiner( int argc, char* argv[] )
{
  int nprocs, rank;
#ifdef MOAB_HAVE_MPI
  MPI_Init( &argc, &argv );
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#else // MOAB_HAVE_MPI
  nprocs = 1;
  rank = 0;
#endif // MOAB_HAVE_MPI
  //sleep(20);

  // Create the input mesh and, if -new-mesh is specified, an output mesh
  std::string ifname = argc > 1 ? argv[1] : TestDir + "fourVolsBare.cub";
  bool input_is_output = false, do_output = false;
  std::string output_filename;
  if ( argc > 2) {
    if (!strcmp( argv[2], "-new-mesh" )) input_is_output = true;
    else {
      do_output = true;
      output_filename = std::string(argv[2]);
    }
  }
  
  Interface* imesh = new Core; // ( rank, nprocs );
  Interface* omesh = input_is_output ? imesh : new Core; // ( rank, nprocs );

#ifdef MOAB_HAVE_MPI
  // Use an ParallelComm object to help set up the input mesh
  ParallelComm* ipcomm = new ParallelComm( imesh, MPI_COMM_WORLD );
  //ReadParallel* readpar = new ReadParallel( imesh, ipcomm );
#endif // MOAB_HAVE_MPI

  EntityHandle set_handle;
  std::ostringstream parallel_options;
#ifdef MOAB_HAVE_MPI
  if (nprocs > 1) {
    parallel_options
      << "PARALLEL=READ_DELETE" << ";" // NB: You can use BCAST_DELETE or READ_DELETE here.
      //<< "PARALLEL=BCAST_DELETE" << ";" // NB: You can use BCAST_DELETE or READ_DELETE here.
      << "PARTITION=MATERIAL_SET" << ";"
      << "PARTITION_DISTRIBUTE" << ";"
      << "PARALLEL_RESOLVE_SHARED_ENTS" << ";"
      << "CPUTIME";
  }
#endif
  ErrorCode rval = imesh->create_meshset(MESHSET_SET, set_handle);
  if (MB_SUCCESS != rval) {
    std::cout << "Trouble creating set, exiting." << std::endl;
    return 1;
  }

  rval = imesh->load_file( ifname.c_str(), &set_handle, parallel_options.str().c_str() );
  if (MB_SUCCESS != rval) {
    std::cout << "Trouble reading mesh file " << ifname << ", exiting." << std::endl;
    return 1;
  }
  
  // Print out what we have so far, one process at a time
  for ( int i = 0; i < nprocs; ++ i )
  {
    MPI_Barrier( MPI_COMM_WORLD );
    if ( i == rank )
      {
      std::cout << "\n************** Rank: " << ( rank ) << " of: " << nprocs << "\n";
      imesh->list_entities( 0, 0 );
      std::cout << "**************\n\n";
      }
    MPI_Barrier( MPI_COMM_WORLD );
  }

  // The refiner will need an implicit function to be used as an indicator function for subdivision:
  EdgeSizeSimpleImplicit* eval = new EdgeSizeSimpleImplicit();
  eval->set_ratio( 2. );
  // Refine the mesh
  MeshRefiner* mref = new MeshRefiner( imesh, omesh );
  SimplexTemplateRefiner* eref = new SimplexTemplateRefiner;
  mref->set_entity_refiner( eref );
  //mref->add_vertex_tag( tag_floatular );
  //mref->add_vertex_tag( tag_intular );
  // (We don't add tag_gid to the refiner's tag manager because it is special)
  eref->set_edge_size_evaluator( eval );
  Range ents_to_refine;
  imesh->get_entities_by_type( set_handle, MBTET, ents_to_refine ); // refine just the tets
  //ents_to_refine.insert( set_handle ); // refine everything multiple times (because subsets are not disjoint)
  struct timeval tic, toc;
  gettimeofday( &tic, 0 );
  mref->refine( ents_to_refine );
  gettimeofday( &toc, 0 );
  std::cout << "\nTime: " << ( (toc.tv_sec - tic.tv_sec) * 1000 + (toc.tv_usec - tic.tv_usec) / 1000. ) << " ms\n\n";

  if (do_output) {
    parallel_options.clear();
    if (nprocs > 1)
      parallel_options << "PARALLEL=WRITE_PART";
    omesh->write_file( output_filename.c_str(), NULL, parallel_options.str().c_str() );
  }
  
  // Print out the results, one process at a time
#ifdef MOAB_HAVE_MPI
  for ( int i = 0; i < nprocs; ++ i )
    {
    MPI_Barrier( MPI_COMM_WORLD );
    if ( i == rank )
      {
      std::cout << "\n************** Rank: " << ( rank ) << " of: " << nprocs << "\n";
      omesh->list_entities( 0, 0 );
      std::cout << "**************\n\n";
      }
    MPI_Barrier( MPI_COMM_WORLD );
    }
#else // MOAB_HAVE_MPI
  omesh->list_entities( 0, 1 );
#endif // MOAB_HAVE_MPI

  // Clean up
#ifdef MOAB_HAVE_MPI
  //delete readpar;
  delete ipcomm;
#endif // MOAB_HAVE_MPI
  if ( omesh != imesh )
    delete omesh;
  delete imesh;
  delete mref; // mref will delete eref

#ifdef MOAB_HAVE_MPI
  MPI_Barrier( MPI_COMM_WORLD );
  MPI_Finalize();
#endif // MOAB_HAVE_MPI

  return 0;
}

int main( int argc, char* argv[] )
{
  return TestMeshRefiner( argc, argv );
}
