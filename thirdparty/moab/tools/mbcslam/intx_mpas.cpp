/*
 * mpas file test
 *
 *  Created on: Feb 12, 2013
 */

// copy from case1 test
#ifdef HAVE_ZOLTAN

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
#include "moab/ProgOptions.hpp"
#include "MBTagConventions.hpp"
#include "TestUtil.hpp"
#include "moab/ParallelComm.hpp"

#include "CslamUtils.hpp"
#include <time.h>

#ifdef MESHDIR
std::string TestDir( STRINGIFY(MESHDIR) );
#else
#error Specify MESHDIR to run unit tests
#endif

// for M_PI
#include <math.h>

using namespace moab;
// some input data
double gtol = 1.e-9; // this is for geometry tolerance

double radius = 1.;// in m:  6371220.

double t = 0.1, delta_t = 0.05; // check the script
bool Verbose = false;
double rot= M_PI/10;

ErrorCode manufacture_lagrange_mesh_on_sphere(Interface * mb,
    EntityHandle euler_set)
{
  ErrorCode rval = MB_SUCCESS;

  /*
   * get all plys first, then vertices, then move them on the surface of the sphere
   *  radius is 1., most of the time
   *
   */
  Range polygons;
  rval = mb->get_entities_by_dimension(euler_set, 2, polygons);
  if (MB_SUCCESS != rval)
    return rval;

  Range connecVerts;
  rval = mb->get_connectivity(polygons, connecVerts);
  if (MB_SUCCESS != rval)
    return rval;


  Tag tagh = 0;
  std::string tag_name("DP");
  rval = mb->tag_get_handle(tag_name.c_str(), 3, MB_TYPE_DOUBLE, tagh, MB_TAG_DENSE | MB_TAG_CREAT);
  CHECK_ERR(rval);
  void *data; // pointer to the LOC in memory, for each vertex
  int count;

  rval = mb->tag_iterate(tagh, connecVerts.begin(), connecVerts.end(), count, data);
  CHECK_ERR(rval);
  // here we are checking contiguity
  assert(count == (int) connecVerts.size());
  double * ptr_DP=(double*)data;
  // get the coordinates of the old mesh, and move it around the sphere in the same way as in the
  // python script

  // now put the vertices in the right place....
  //int vix=0; // vertex index in new array
  double T=5;// check the script


  for (Range::iterator vit=connecVerts.begin();vit!=connecVerts.end(); vit++ )
  {
    EntityHandle oldV=*vit;
    CartVect posi;
    rval = mb->get_coords(&oldV, 1, &(posi[0]) );
    CHECK_ERR(rval);
    // do some mumbo jumbo, as in python script
    SphereCoords sphCoord = cart_to_spherical(posi);
    double lat1 = sphCoord.lat-2*M_PI*t/T; // 0.1/5
    double uu = 3*radius/ T * pow(sin(lat1), 2)*sin(2*sphCoord.lon)*cos(M_PI*t/T);
    uu+=2*radius*M_PI*cos(sphCoord.lon)/T;
    double vv = 3*radius/T*(sin(2*lat1))*cos(sphCoord.lon)*cos(M_PI*t/T);
    double vx = -uu*sin(sphCoord.lon)-vv*sin(sphCoord.lat)*cos(sphCoord.lon);
    double vy = -uu*cos(sphCoord.lon)-vv*sin(sphCoord.lat)*sin(sphCoord.lon);
    double vz = vv*cos(sphCoord.lat);
    posi = posi + delta_t * CartVect(vx, vy, vz);
    double x2= posi[0]*cos(rot)-posi[1]*sin(rot);
    double y2= posi[0]*sin(rot) + posi[1]*cos(rot);
    CartVect newPos(x2, y2, posi[2]);
    double len1= newPos.length();
    newPos = radius*newPos/len1;

    ptr_DP[0]=newPos[0];
    ptr_DP[1]=newPos[1];
    ptr_DP[2]=newPos[2];
    ptr_DP+=3; // increment to the next node
  }
  return rval;
}

int main(int argc, char **argv)
{

  MPI_Init(&argc, &argv);

  std::string extra_read_opts;
  std::string fileN= TestDir + "/io/mpasx1.642.t.2.nc";
  const char *filename_mesh1 = fileN.c_str();
  bool flux_form = false;
  if (argc > 1)
  {
    int index = 1;
    while (index < argc)
    {
      if (!strcmp(argv[index], "-gtol")) // this is for geometry tolerance
      {
        gtol = atof(argv[++index]);
      }
      if (!strcmp(argv[index], "-dt"))
      {
        delta_t = atof(argv[++index]);
      }
      if (!strcmp(argv[index], "-input"))
      {
        filename_mesh1 = argv[++index];
      }
      if (!strcmp(argv[index], "-R"))
      {
        radius = atof(argv[++index]);
      }
      if (!strcmp(argv[index], "-O"))
      {
        extra_read_opts = std::string(argv[++index]);
      }
      if (!strcmp(argv[index], "-FF"))
      {
        flux_form= true;
      }
      if (!strcmp(argv[index], "-v"))
      {
        Verbose = true;
      }
      if (!strcmp(argv[index], "-t"))
      {
        t = atof(argv[++index]);
      }
      if (!strcmp(argv[index], "-t"))
      {
        t = atof(argv[++index]);
      }
      if (!strcmp(argv[index], "-rot"))
      {
        rot = atof(argv[++index]);
        rot = M_PI/rot; // so rot 50 means rotate with M_PI/50 radians
      }

      index++;
    }
  }
  // start copy
  std::string opts = std::string("PARALLEL=READ_PART;PARTITION_METHOD=RCBZOLTAN")+
            std::string(";PARALLEL_RESOLVE_SHARED_ENTS;VARIABLE=;NO_EDGES;")+extra_read_opts;
  Core moab;
  Interface & mb = moab;
  EntityHandle euler_set;
  ErrorCode rval;
  rval = mb.create_meshset(MESHSET_SET, euler_set);
  CHECK_ERR(rval);

  clock_t tt = clock();

  rval = mb.load_file(filename_mesh1, &euler_set, opts.c_str());
  CHECK_ERR(rval);

  ParallelComm* pcomm = ParallelComm::get_pcomm(&mb, 0);
  CHECK_ERR(rval);

  /*rval = pcomm->check_all_shared_handles();
  CHECK_ERR(rval);*/
  // end copy
  int rank = pcomm->proc_config().proc_rank();
  int procs = pcomm->proc_config().proc_size();

  if (0==rank)
    std::cout << " case 1: use -gtol " << gtol << " -dt " << delta_t <<
        " -R " << radius << " -input " << filename_mesh1 << " -t " << t << " -rot " << rot <<  "\n";

  if (0==rank)
  {
      std::cout << "load mesh from " << filename_mesh1 << "\n  on " << procs << " processors in "
            << (clock() - tt) / (double) CLOCKS_PER_SEC << " seconds" << std::endl;
      tt = clock();
  }

  rval = manufacture_lagrange_mesh_on_sphere(&mb, euler_set);
  if (MB_SUCCESS != rval)
    return 1;
  // create a set with quads corresponding to each initial edge spanned with the displacement field

  if (flux_form)
  {
    rval = create_span_quads(&mb, euler_set, rank);
    if (MB_SUCCESS != rval)
      return 1;
  }

  EntityHandle covering_lagr_set;
  rval = mb.create_meshset(MESHSET_SET, covering_lagr_set);
  CHECK_ERR(rval);
  Intx2MeshOnSphere worker(&mb);

  //double radius = 1.; // input

  worker.SetRadius(radius);
  if (0==rank)
  {
     std::cout << "manufacture departure mesh " << filename_mesh1 << "\n  on " << procs << " processors in "
              << (clock() - tt) / (double) CLOCKS_PER_SEC << " seconds" << std::endl;
     tt = clock();
  }
  worker.SetErrorTolerance(gtol);
  rval = worker.create_departure_mesh_2nd_alg(euler_set, covering_lagr_set);
  CHECK_ERR(rval);

  if (0==rank)
  {
     std::cout << "communicate covering mesh on " << procs << " processors in "
              << (clock() - tt) / (double) CLOCKS_PER_SEC << " seconds" << std::endl;
     tt = clock();
  }

  if (Verbose)
  {
    std::stringstream lagrIni;
    lagrIni<<"lagr0" << rank<<".h5m";
    rval = mb.write_file(lagrIni.str().c_str(), 0, 0, &covering_lagr_set, 1);
  }

  rval = enforce_convexity(&mb, covering_lagr_set, rank);
  if (MB_SUCCESS != rval)
    return 1;
  if (Verbose)
  {
    std::stringstream ste;
    ste<<"euler0" << rank<<".h5m";
    rval = mb.write_file(ste.str().c_str(), 0, 0, &euler_set, 1);
  }

  if (MB_SUCCESS != rval)
    std::cout << "can't write lagr set\n";

  EntityHandle outputSet;
  rval = mb.create_meshset(MESHSET_SET, outputSet);
  if (MB_SUCCESS != rval)
    return 1;
  rval = worker.intersect_meshes(covering_lagr_set, euler_set, outputSet);
  if (MB_SUCCESS != rval)
    return 1;
  if (0==rank)
  {
     std::cout << "intersect meshes in " << procs << " processors in "
              << (clock() - tt) / (double) CLOCKS_PER_SEC << " seconds" << std::endl;
     tt = clock();
  }
  if (Verbose && rank<=4)
  {
    std::string opts_write("");
    std::stringstream outf;
    outf << "intersect0" << rank << ".h5m";
    rval = mb.write_file(outf.str().c_str(), 0, 0, &outputSet, 1);
    if (MB_SUCCESS != rval)
      std::cout << "can't write output\n";
  }

  if (rank <= 4)
  {
    double intx_area = area_on_sphere_lHuiller(&mb, outputSet, radius);
    double arrival_area = area_on_sphere_lHuiller(&mb, euler_set, radius);
    std::cout << "On proc " << rank << "  arrival area: " << arrival_area
        << "  intersection area:" << intx_area << " rel error: "
        << fabs((intx_area - arrival_area) / arrival_area) << "\n";
  }

  MPI_Finalize();
  if (MB_SUCCESS != rval)
    return 1;

  return 0;
}

#else

int main(int /*argc*/, char** /*argv*/)
{
  return 0;
}

#endif

