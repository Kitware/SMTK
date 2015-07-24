/*
 * IntxRllCssphere_test.cpp
 */


#include "IntxRllCssphere.hpp"
#include "../../test/TestUtil.hpp"
using namespace moab;

int main(int argc, char* argv[])
{
  // check command line arg// Euler grid is red, arrival, Lagrangian is blue, departure
  // will will keep the
  const char *filename_mesh1 = STRINGIFY(MESHDIR) "/mbcslam/outRLLMesh.g";
  const char *filename_mesh2 = STRINGIFY(MESHDIR) "/mbcslam/outCSMesh.g";
  double R = 1.; // input
  double epsrel=1.e-8;
  const char *newFile = "intx.vtk";
  if (argc == 6)
  {
    filename_mesh1 = argv[1];
    filename_mesh2 = argv[2];
    R = atof(argv[3]);
    epsrel = atof(argv[4]);
    newFile = argv[5];
  }
  else
  {
    printf("Usage: %s <mesh_filename1> <mesh_filename2> <radius> <epsrel> <newFile>\n",
        argv[0]);
    if (argc != 1)
      return 1;
    printf("No files specified.  Defaulting to: %s  %s  %f %g %s\n",
        filename_mesh1, filename_mesh2, R, epsrel, newFile);
  }

  // read meshes in 2 file sets
  ErrorCode rval = MB_SUCCESS;
  Core moab;
  Interface * mb = &moab; // global
  EntityHandle sf1, sf2;
  rval = mb->create_meshset(MESHSET_SET, sf1);
  if (MB_SUCCESS != rval)
    return 1;
  rval = mb->create_meshset(MESHSET_SET, sf2);
  if (MB_SUCCESS != rval)
    return 1;
  rval = mb->load_file(filename_mesh1, &sf1);
  if (MB_SUCCESS != rval)
    return 1;
  rval = mb->load_file(filename_mesh2, &sf2);
  if (MB_SUCCESS != rval)
    return 1;

  EntityHandle outputSet;
  rval = mb->create_meshset(MESHSET_SET, outputSet);
  if (MB_SUCCESS != rval)
    return 1;

  // CslamUtils
  rval = fix_degenerate_quads(mb, sf1);
  if (MB_SUCCESS != rval)
    return 1;

  rval =positive_orientation(mb, sf1, R);
  if (MB_SUCCESS != rval)
    return 1;

  rval =positive_orientation(mb, sf2, R);
  if (MB_SUCCESS != rval)
    return 1;

  /*// set the edge tags on all elements sf1
  rval = set_edge_type_flag(mb, sf1); // form all edges, and set on them type 1 if constant latitude
  // add them to the set after this, just so we have them
  if (MB_SUCCESS != rval)
    return 1;*/

  IntxRllCssphere  worker(mb);

  worker.SetErrorTolerance(R*epsrel);
  //worker.SetEntityType(moab::MBQUAD);
  worker.SetRadius(R);
  //worker.enable_debug();

  rval = worker.intersect_meshes(sf1, sf2, outputSet);
  //compute total area with 2 methods

  if (MB_SUCCESS != rval)
    std::cout << " failed to intersect meshes\n";
  rval = mb->write_mesh(newFile, &outputSet, 1);
  if (MB_SUCCESS != rval)
    return 1;
  return 0;

}
