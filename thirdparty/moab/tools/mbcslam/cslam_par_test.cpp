/*
 * cslam_par_test.cpp
 *  test to trigger intersection on a sphere in parallel
 *  it will start from an eulerian mesh (mesh + velocity from Mark Taylor)
 *   file: VELO00.h5m; mesh + velo at 850 milibar, in an unstructured mesh refined from
 *   a cube sphere grid
 *  the mesh is read in parallel (euler mesh);
 *
 *   lagrangian mesh is obtained using
 *   pos (t-dt) = pos(t) -Velo(t)*dt
 *   then, project on the sphere with a given radius
 *
 *  Created on: Apr 22, 2013
 */

#include <iostream>
#include <sstream>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "moab/Core.hpp"
#include "moab/Interface.hpp"
#include "Intx2MeshOnSphere.hpp"
#include <math.h>
#include "TestUtil.hpp"
#include "moab/ParallelComm.hpp"
#include "moab/ProgOptions.hpp"
#include "MBParallelConventions.h"
#include "moab/ReadUtilIface.hpp"
#include "MBTagConventions.hpp"

#include "CslamUtils.hpp"

// for M_PI
#include <math.h>

#ifdef MESHDIR
std::string TestDir( STRINGIFY(MESHDIR) );
#else
std::string TestDir(".");
#endif

using namespace moab;
// some input data
double EPS1=0.2; // this is for box error
std::string input_mesh_file("VELO00_16p.h5m"); // input file, partitioned correctly
double Radius = 1.0; // change to radius
double deltaT = 1.e-6;
void test_intx_in_parallel_elem_based();

int main(int argc, char **argv)
{
  MPI_Init(&argc, &argv);
  EPS1 = 0.000002;
  int result = 0;

  if (argc>1)
  {
    int index=1;
    while (index<argc)
    {
      if (!strcmp( argv[index], "-eps")) // this is for box error
      {
        EPS1=atof(argv[++index]);
      }
      if (!strcmp( argv[index], "-input"))
      {
        input_mesh_file=argv[++index];
      }
      if (!strcmp( argv[index], "-radius"))
      {
        Radius=atof(argv[++index]);
      }
      if (!strcmp( argv[index], "-deltaT"))
      {
        deltaT=atof(argv[++index]);
      }
      index++;
    }
  }
  std::cout << " run: -input " << input_mesh_file << "  -eps " << EPS1 <<
      " -radius " << Radius << " -deltaT " << deltaT << "\n";

  result += RUN_TEST(test_intx_in_parallel_elem_based);

  MPI_Finalize();
  return result;
}
// will save the LOC tag on the euler nodes
ErrorCode  compute_lagrange_mesh_on_sphere(Interface * mb, EntityHandle euler_set)
{
  ErrorCode rval = MB_SUCCESS;

  /*
   * get all quads first, then vertices, then move them on the surface of the sphere
   *  radius is 1, usually
   *  pos (t-dt) = pos(t) -Velo(t)*dt; this will be lagrange mesh, on each processor
   */
  Range quads;
  rval = mb->get_entities_by_type(euler_set, MBQUAD, quads);
  CHECK_ERR(rval);

  Range connecVerts;
  rval = mb->get_connectivity(quads, connecVerts);

  // the LOC tag, should be provided by the user?
  Tag tagh = 0;
  std::string tag_name("DP");
  rval = mb->tag_get_handle(tag_name.c_str(), 3, MB_TYPE_DOUBLE, tagh, MB_TAG_DENSE | MB_TAG_CREAT);
  CHECK_ERR(rval);
  void *data; // pointer to the DP in memory, for each vertex
  int count;

  rval = mb->tag_iterate(tagh, connecVerts.begin(), connecVerts.end(), count, data);
  CHECK_ERR(rval);
  // here we are checking contiguity
  assert(count == (int) connecVerts.size());
  double * ptr_DP=(double*)data;
  // get the coordinates of the old mesh, and move it around using velocity tag

  Tag tagv = 0;
  std::string velo_tag_name("VELO");
  rval = mb->tag_get_handle(velo_tag_name.c_str(), 3, MB_TYPE_DOUBLE, tagv, MB_TAG_DENSE);
  CHECK_ERR(rval);

  /*void *datavelo; // pointer to the VELO in memory, for each vertex

  rval = mb->tag_iterate(tagv, connecVerts.begin(), connecVerts.end(), count, datavelo);
  CHECK_ERR(rval);*/
  // here we are checking contiguity
  assert(count == (int) connecVerts.size());
// now put the vertices in the right place....
  //int vix=0; // vertex index in new array

  for (Range::iterator vit=connecVerts.begin();vit!=connecVerts.end(); vit++ )
  {
    EntityHandle oldV=*vit;
    CartVect posi;
    rval = mb->get_coords(&oldV, 1, &(posi[0]) );
    CHECK_ERR(rval);
    CartVect velo;
    rval = mb->tag_get_data(tagv, &oldV, 1, (void *) &(velo[0]) );
    CHECK_ERR(rval);
    // do some mumbo jumbo, as in python script
    CartVect newPos = posi - deltaT*velo;
    double len1= newPos.length();
    newPos = Radius*newPos/len1;

    ptr_DP[0]=newPos[0];
    ptr_DP[1]=newPos[1];
    ptr_DP[2]=newPos[2];
    ptr_DP+=3; // increment to the next node
  }

  return rval;
}

void test_intx_in_parallel_elem_based()
{
  std::string opts = std::string("PARALLEL=READ_PART;PARTITION=PARALLEL_PARTITION")+
        std::string(";PARALLEL_RESOLVE_SHARED_ENTS");
  Core moab;
  Interface & mb = moab;
  EntityHandle euler_set;
  ErrorCode rval;
  rval = mb.create_meshset(MESHSET_SET, euler_set);
  CHECK_ERR(rval);
  std::string example(TestDir + "/" +  input_mesh_file);

  rval = mb.load_file(example.c_str(), &euler_set, opts.c_str());

  ParallelComm* pcomm = ParallelComm::get_pcomm(&mb, 0);
  CHECK_ERR(rval);

  rval = pcomm->check_all_shared_handles();
  CHECK_ERR(rval);

  // everybody will get a DP tag, including the non owned entities; so exchange tags is not required for LOC (here)
  rval = compute_lagrange_mesh_on_sphere(&mb, euler_set);
  CHECK_ERR(rval);

  int rank = pcomm->proc_config().proc_rank();

  std::stringstream ste;
  ste<<"initial" << rank<<".vtk";
  mb.write_file(ste.str().c_str(), 0, 0, &euler_set, 1);

  Intx2MeshOnSphere worker(&mb);

  worker.SetRadius(Radius);
  worker.set_box_error(EPS1);//
  //worker.SetEntityType(MBQUAD);

  worker.SetErrorTolerance(Radius*1.e-8);
  std::cout << "error tolerance epsilon_1="<< Radius*1.e-8 << "\n";
  //  worker.locate_departure_points(euler_set);

  // we need to make sure the covering set is bigger than the euler mesh
  EntityHandle covering_lagr_set;
  rval = mb.create_meshset(MESHSET_SET, covering_lagr_set);
  CHECK_ERR(rval);

  rval = worker.create_departure_mesh_2nd_alg(euler_set, covering_lagr_set);
  CHECK_ERR(rval);

  std::stringstream ss;
  ss<<"partial" << rank<<".vtk";
  mb.write_file(ss.str().c_str(), 0, 0, &covering_lagr_set, 1);
  EntityHandle outputSet;
  rval = mb.create_meshset(MESHSET_SET, outputSet);
  CHECK_ERR(rval);
  rval = worker.intersect_meshes(covering_lagr_set, euler_set, outputSet);
  CHECK_ERR(rval);

  //std::string opts_write("PARALLEL=WRITE_PART");
  //rval = mb.write_file("manuf.h5m", 0, opts_write.c_str(), &outputSet, 1);
  std::string opts_write("");
  std::stringstream outf;
  outf<<"intersect" << rank<<".h5m";
  rval = mb.write_file(outf.str().c_str(), 0, 0, &outputSet, 1);
  double intx_area = area_on_sphere(&mb, outputSet, Radius);
  double arrival_area = area_on_sphere(&mb, euler_set, Radius) ;
  std::cout<< "On rank : " << rank << " arrival area: " << arrival_area<<
      "  intersection area:" << intx_area << " rel error: " << fabs((intx_area-arrival_area)/arrival_area) << "\n";
  CHECK_ERR(rval);
}
