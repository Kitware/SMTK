/*
 * diffusion.cpp
 *
 *  Created on: Aug 12, 2013
 *
 */

/* trigger a diffusion computation in serial, first;
  we will start from an unstructured mesh on a sphere;
  do case 1: give some tracers a "slotted cylinder" ; compute the initial concentrations
  along a slotted cylinder, and see how it varies in time;
  use formula from Nair & Lauritzen paper:
  A class of deformational flow test cases for linear transport problems
on the sphere; see CSLAM Utils case1
  scalar fields are defined at page 4-5 in the paper

  steps:
   first create an initial tracer field, and save it as field on a sphere
   (initial time step 0)
   then use velocities (non-divergent, first), to transport the tracer on
   the sphere, according to some flow fields
*/
// copy from intx_mpas test

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

const char BRIEF_DESC[] =
    "Simulate a transport problem in a semi-Lagrangian formulation\n";
std::ostringstream LONG_DESC;

// non smooth scalar field
// some input data
double gtol = 1.e-9; // this is for geometry tolerance

double radius = 1.;// in m:  6371220.

int numSteps = 3; // number of times with velocity displayed at points
double T = 1;

int case_number = 1; // 1, 2 (non-divergent) 3 divergent

bool writeFiles = false;
bool parallelWrite = false;
bool velocity = false;
int field_type = 1 ; // 1 quasi smooth, 2 - smooth, 3 non-smooth,
#ifdef MESHDIR
std::string TestDir( STRINGIFY(MESHDIR) );
#else
std::string TestDir(".");
#endif

// for M_PI
#include <math.h>

using namespace moab;
ErrorCode add_field_value(Interface * mb, EntityHandle euler_set, int rank, Tag & tagTracer, Tag & tagElem, Tag & tagArea)
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



  void *data; // pointer to the LOC in memory, for each vertex
  int count;

  rval = mb->tag_iterate(tagTracer, connecVerts.begin(), connecVerts.end(), count, data);
  CHECK_ERR(rval);
  // here we are checking contiguity
  assert(count == (int) connecVerts.size());
  double * ptr_DP=(double*)data;
  // lambda is for longitude, theta for latitude
   // param will be: (la1, te1), (la2, te2), b, c; hmax=1, r=1/2
  // nondivergent flow, page 5, case 1, (la1, te1) = (M_PI, M_PI/3)
  //                                    (la2, te2) = (M_PI, -M_PI/3)
  //                 la1,    te1    la2    te2     b     c  hmax  r
  if (field_type==1) // quasi smooth
  {
    double params[] = { M_PI, M_PI/3, M_PI, -M_PI/3, 0.1, 0.9, 1., 0.5};
    for (Range::iterator vit=connecVerts.begin();vit!=connecVerts.end(); vit++ )
    {
      EntityHandle oldV=*vit;
      CartVect posi;
      rval = mb->get_coords(&oldV, 1, &(posi[0]) );
      CHECK_ERR(rval);

      SphereCoords sphCoord = cart_to_spherical(posi);

      ptr_DP[0]=quasi_smooth_field(sphCoord.lon, sphCoord.lat, params);;

      ptr_DP++; // increment to the next node
    }
  }
  else if (2 == field_type) // smooth
  {
    CartVect p1, p2;
    SphereCoords spr;
    spr.R = 1;
    spr.lat = M_PI/3;
    spr.lon= M_PI;
    p1 = spherical_to_cart(spr);
    spr.lat = -M_PI/3;
    p2 = spherical_to_cart(spr);
    //                  x1,    y1,     z1,    x2,   y2,    z2,   h_max, b0
    double params[] = { p1[0], p1[1], p1[2], p2[0], p2[1], p2[2], 1,    5.};
    for (Range::iterator vit=connecVerts.begin();vit!=connecVerts.end(); vit++ )
    {
      EntityHandle oldV=*vit;
      CartVect posi;
      rval = mb->get_coords(&oldV, 1, &(posi[0]) );
      CHECK_ERR(rval);

      SphereCoords sphCoord = cart_to_spherical(posi);

      ptr_DP[0]=smooth_field(sphCoord.lon, sphCoord.lat, params);;

      ptr_DP++; // increment to the next node
    }
  }
  else if (3 == field_type) // slotted
  {
    //                   la1, te1,   la2, te2,       b,   c,   r
    double params[] = { M_PI, M_PI/3, M_PI, -M_PI/3, 0.1, 0.9, 0.5};// no h_max
    for (Range::iterator vit=connecVerts.begin();vit!=connecVerts.end(); vit++ )
    {
      EntityHandle oldV=*vit;
      CartVect posi;
      rval = mb->get_coords(&oldV, 1, &(posi[0]) );
      CHECK_ERR(rval);

      SphereCoords sphCoord = cart_to_spherical(posi);

      ptr_DP[0]=slotted_cylinder_field(sphCoord.lon, sphCoord.lat, params);;

      ptr_DP++; // increment to the next node
    }
  }

  // add average value for quad/polygon (average corners)
  // do some averages


  Range::iterator iter = polygons.begin();
  double local_mass = 0.; // this is total mass on one proc
  while (iter != polygons.end())
  {
    rval = mb->tag_iterate(tagElem, iter, polygons.end(), count, data);
    CHECK_ERR(rval);
    double * ptr=(double*)data;

    rval = mb->tag_iterate(tagArea, iter, polygons.end(), count, data);
    CHECK_ERR(rval);
    double * ptrArea=(double*)data;
    for (int i=0; i<count; i++, iter++, ptr++, ptrArea++)
    {
      const moab::EntityHandle * conn = NULL;
      int num_nodes = 0;
      rval = mb->get_connectivity(*iter, conn, num_nodes);
      CHECK_ERR(rval);
      if (num_nodes==0)
        return MB_FAILURE;
      std::vector<double> nodeVals(num_nodes);
      double average=0.;
      rval = mb->tag_get_data(tagTracer, conn, num_nodes, &nodeVals[0] );
      CHECK_ERR(rval);
      for (int j=0; j<num_nodes; j++)
        average+=nodeVals[j];
      average/=num_nodes;
      *ptr = average;

      // now get area
      std::vector<double> coords;
      coords.resize(3*num_nodes);
      rval = mb->get_coords(conn, num_nodes, &coords[0]);
      CHECK_ERR(rval);
      *ptrArea =  area_spherical_polygon_lHuiller (&coords[0], num_nodes, radius);

      // we should have used some
      // total mass:
      local_mass += *ptrArea * average;
    }

  }
  double total_mass=0.;
  int mpi_err = MPI_Reduce(&local_mass, &total_mass, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  if (MPI_SUCCESS != mpi_err) return MB_FAILURE;

  if (rank==0)
    std::cout << "initial total mass:" << total_mass << "\n";

  // now we can delete the tags? not yet
  return MB_SUCCESS;
}

ErrorCode compute_velocity_case1(Interface * mb, EntityHandle euler_set, Tag & tagh, int rank, int tStep)
{
  ErrorCode rval = MB_SUCCESS;

  Range polygons;
  rval = mb->get_entities_by_dimension(euler_set, 2, polygons);
  if (MB_SUCCESS != rval)
    return rval;

  Range connecVerts;
  rval = mb->get_connectivity(polygons, connecVerts);
  if (MB_SUCCESS != rval)
    return rval;

  void *data; // pointer to the velo in memory, for each vertex
  int count;

  rval = mb->tag_iterate(tagh, connecVerts.begin(), connecVerts.end(), count, data);
  CHECK_ERR(rval);
  // here we are checking contiguity
  assert(count == (int) connecVerts.size());
  double * ptr_velo=(double*)data;
  // lambda is for longitude, theta for latitude

  for (Range::iterator vit=connecVerts.begin();vit!=connecVerts.end(); vit++ )
  {
    EntityHandle oldV=*vit;
    CartVect posi;
    rval = mb->get_coords(&oldV, 1, &(posi[0]) );
    CHECK_ERR(rval);
    CartVect velo ;
    double t = T * tStep/numSteps; //
    velocity_case1(posi, t, velo);

    ptr_velo[0]= velo[0];
    ptr_velo[1]= velo[1];
    ptr_velo[2]= velo[2];

    // increment to the next node
    ptr_velo+=3;// to next velocity
  }

  std::stringstream velos;
  velos<<"Tracer" << rank<<"_"<<tStep<<  ".vtk";
  rval = mb->write_file(velos.str().c_str(), 0, 0, &euler_set, 1, &tagh, 1);
  CHECK_ERR(rval);


  return MB_SUCCESS;
}

ErrorCode compute_tracer_case1(Interface * mb, Intx2MeshOnSphere & worker, EntityHandle euler_set,
    EntityHandle lagr_set, EntityHandle out_set, Tag & tagElem, Tag & tagArea, int rank,
    int tStep, Range & connecVerts)
{
  ErrorCode rval = MB_SUCCESS;

  EntityHandle dum=0;
  Tag corrTag;
  mb->tag_get_handle(CORRTAGNAME, 1, MB_TYPE_HANDLE, corrTag,
                                             MB_TAG_DENSE, &dum);

  double t = tStep * T / numSteps; // numSteps is global; so is T
  double delta_t = T / numSteps; // this is global too, actually
  Range polys;
  rval = mb->get_entities_by_dimension(euler_set, 2, polys);
  CHECK_ERR(rval);

  // change coordinates of lagr mesh vertices
  for (Range::iterator vit = connecVerts.begin(); vit != connecVerts.end();
      vit++)
  {
    EntityHandle oldV = *vit;
    CartVect posi;
    rval = mb->get_coords(&oldV, 1, &(posi[0]));
    CHECK_ERR(rval);
    // cslam utils, case 1
    CartVect newPos;
    departure_point_case1(posi, t, delta_t, newPos);
    newPos = radius * newPos; // do we need this? the radius should be 1
    EntityHandle new_vert;
    rval = mb->tag_get_data(corrTag, &oldV, 1, &new_vert);
    CHECK_ERR(rval);
    // set the new position for the new vertex
    rval = mb->set_coords(&new_vert, 1, &(newPos[0]));
    CHECK_ERR(rval);
  }

  // if in parallel, we have to move some elements to another proc, and receive other cells
  // from other procs
  // lagr and euler are preserved
  if (writeFiles) // so if need to write lagr files too
  {

  }
  EntityHandle covering_set;
  rval = worker.create_departure_mesh_3rd_alg(lagr_set, covering_set);
  if (writeFiles) // so if write
  {
    std::stringstream departureMesh;
    departureMesh << "Departure" << rank << "_" << tStep << ".vtk";
    rval = mb->write_file(departureMesh.str().c_str(), 0, 0, &lagr_set, 1);
    CHECK_ERR(rval);

    std::stringstream newTracer;
    newTracer << "Tracer" << rank << "_" << tStep << ".vtk";
    rval = mb->write_file(newTracer.str().c_str(), 0, 0, &euler_set, 1);
    CHECK_ERR(rval);

    std::stringstream lagr_cover;
    lagr_cover << "Cover" << rank << "_" << tStep << ".vtk";
    rval = mb->write_file(lagr_cover.str().c_str(), 0, 0, &covering_set, 1);
    CHECK_ERR(rval);

  }
  // so we have now the departure at the previous time
  // intersect the 2 meshes (what about some checking of convexity?) for sufficient
  // small dt, it is not an issue;

  // std::cout << "error tolerance epsilon_1=" << gtol << "\n";

  rval = worker.intersect_meshes(covering_set, euler_set, out_set);
  CHECK_ERR(rval);
  if (writeFiles) // so if write
  {
    std::stringstream intx_mesh;
    intx_mesh << "Intx" << rank << "_" << tStep << ".vtk";
    rval = mb->write_file(intx_mesh.str().c_str(), 0, 0, &out_set, 1);
  }

  // serially: lagr is the same order as euler;
  // we need to update now the tracer information on each element, based on
  // initial value and areas of each resulting polygons
  if (parallelWrite && tStep==1)
  {
    std::stringstream resTrace;
    resTrace << "Tracer" << "_" << tStep-1 << ".h5m";
    rval = mb->write_file(resTrace.str().c_str(), 0, "PARALLEL=WRITE_PART", &euler_set, 1, &tagElem, 1);
  }
  rval = worker.update_tracer_data(out_set, tagElem, tagArea);
  CHECK_ERR(rval);

  if (parallelWrite)
  {
    std::stringstream resTrace;
    resTrace << "Tracer" << "_" << tStep << ".h5m";
    rval = mb->write_file(resTrace.str().c_str(), 0, "PARALLEL=WRITE_PART", &euler_set, 1, &tagElem, 1);
  }

  if (writeFiles) // so if write
  {
    std::stringstream newIntx;
    newIntx << "newIntx" << rank << "_" << tStep << ".vtk";
    rval = mb->write_file(newIntx.str().c_str(), 0, 0, &out_set, 1);
    CHECK_ERR(rval);
  }
  // delete now the polygons and the elements of out_set
  // also, all verts that are not in euler set or lagr_set
  Range allVerts;
  rval = mb->get_entities_by_dimension(0, 0, allVerts);
  CHECK_ERR(rval);

  Range allElems;
  rval = mb->get_entities_by_dimension(0, 2, allElems);
  CHECK_ERR(rval);
  // add to polys range the lagr polys
  rval = mb->get_entities_by_dimension(lagr_set, 2, polys); // do not delete lagr set either, with its vertices
  CHECK_ERR(rval);
 // add to the connecVerts range all verts, from all initial polys
  Range vertsToStay;
  rval = mb->get_connectivity(polys, vertsToStay);
  CHECK_ERR(rval);

  Range todeleteVerts = subtract(allVerts, vertsToStay);

  Range todeleteElem = subtract(allElems, polys);

  // empty the out mesh set
  rval = mb->clear_meshset(&out_set, 1);
  CHECK_ERR(rval);

  rval = mb->delete_entities(todeleteElem);
  CHECK_ERR(rval);
  rval = mb->delete_entities(todeleteVerts);
  CHECK_ERR(rval);
  if (rank==0)
    std::cout << " step: " << tStep << "\n";
  return rval;
}
int main(int argc, char **argv)
{

  MPI_Init(&argc, &argv);
  LONG_DESC << "This program simulates a transport problem on a sphere"
        " according to a benchmark from a Nair & Lauritzen paper.\n"
        << "It starts with a partitioned mesh on a sphere, add a tracer, and steps through.\n" <<
        "The flow reverses after half time, and it should return to original configuration, if the integration was exact. ";
  ProgOptions opts(LONG_DESC.str(), BRIEF_DESC);

  // read a homme file, partitioned in 16 so far
  std::string fileN= TestDir + "/mbcslam/fine4.h5m";
  const char *filename_mesh1 = fileN.c_str();

  opts.addOpt<double>("gtolerance,g",
      "geometric absolute tolerance (used for point concidence on the sphere)", &gtol);

  std::string input_file;
  opts.addOpt<std::string>("input_file,i", "input mesh file, partitioned",
      &input_file);
  std::string extra_read_opts;
  opts.addOpt<std::string>("extra_read_options,O", "extra read options ",
        &extra_read_opts);
  //int field_type;
  opts.addOpt<int>("field_type,f",
        "field type--  1: quasi-smooth; 2: smooth; 3: slotted cylinders (non-smooth)", &field_type);

  opts.addOpt<int>("num_steps,n",
          "number of  steps ", &numSteps);

  //bool reorder = false;
  opts.addOpt<void>("write_debug_files,w", "write debugging files during simulation ",
        &writeFiles);

  opts.addOpt<void>("write_velocity_files,v", "Reorder mesh to group entities by partition",
     &velocity);

  opts.addOpt<void>("write_result_in_parallel,p", "write tracer result files",
     &parallelWrite);

  opts.parseCommandLine(argc, argv);

  if (!input_file.empty())
    filename_mesh1=input_file.c_str();

  // read in parallel, in the "euler_set", the initial mesh
  std::string optsRead = std::string("PARALLEL=READ_PART;PARTITION=PARALLEL_PARTITION")+
            std::string(";PARALLEL_RESOLVE_SHARED_ENTS")+extra_read_opts;
  Core moab;
  Interface & mb = moab;
  EntityHandle euler_set;
  ErrorCode rval;
  rval = mb.create_meshset(MESHSET_SET, euler_set);
  CHECK_ERR(rval);

  rval = mb.load_file(filename_mesh1, &euler_set, optsRead.c_str());

  ParallelComm* pcomm = ParallelComm::get_pcomm(&mb, 0);
  CHECK_ERR(rval);

  rval = pcomm->check_all_shared_handles();
  CHECK_ERR(rval);

  int rank = pcomm->proc_config().proc_rank();

  if (0==rank)
  {
    std::cout << " case 1: use -gtol " << gtol <<
        " -R " << radius << " -input " << filename_mesh1 <<  " -f " << field_type <<
        " numSteps: " << numSteps << "\n";
    std::cout<<" write debug results: " << (writeFiles ? "yes" : "no") << "\n";
    std::cout<< " write tracer in parallel: " << ( parallelWrite ? "yes" : "no") << "\n";
    std::cout <<" output velocity: " << (velocity? "yes" : "no") << "\n";
  }

  // tagTracer is the value at nodes
  Tag tagTracer = 0;
  std::string tag_name("Tracer");
  rval = mb.tag_get_handle(tag_name.c_str(), 1, MB_TYPE_DOUBLE, tagTracer, MB_TAG_DENSE | MB_TAG_CREAT);
  CHECK_ERR(rval);

  // tagElem is the average computed at each element, from nodal values
  Tag tagElem = 0;
  std::string tag_name2("TracerAverage");
  rval = mb.tag_get_handle(tag_name2.c_str(), 1, MB_TYPE_DOUBLE, tagElem, MB_TAG_DENSE | MB_TAG_CREAT);
  CHECK_ERR(rval);

  // area of the euler element is fixed, store it; it is used to recompute the averages at each
  // time step
  Tag tagArea = 0;
  std::string tag_name4("Area");
  rval = mb.tag_get_handle(tag_name4.c_str(), 1, MB_TYPE_DOUBLE, tagArea, MB_TAG_DENSE | MB_TAG_CREAT);
  CHECK_ERR(rval);

  // add a field value, quasi smooth first
  rval = add_field_value(&mb, euler_set, rank, tagTracer, tagElem, tagArea);
  CHECK_ERR(rval);

  // iniVals are used for 1-norm error computation
  Range redEls;
  rval = mb.get_entities_by_dimension(euler_set, 2, redEls);
  CHECK_ERR(rval);
  std::vector<double> iniVals(redEls.size());
  rval = mb.tag_get_data(tagElem, redEls, &iniVals[0]);
  CHECK_ERR(rval);

  Tag tagh = 0;
  std::string tag_name3("Case1");
  rval = mb.tag_get_handle(tag_name3.c_str(), 3, MB_TYPE_DOUBLE, tagh, MB_TAG_DENSE | MB_TAG_CREAT);
  CHECK_ERR(rval);
  EntityHandle out_set, lagr_set;
  rval = mb.create_meshset(MESHSET_SET, out_set);
  CHECK_ERR(rval);
  rval = mb.create_meshset(MESHSET_SET, lagr_set);
  CHECK_ERR(rval);
  // copy the initial mesh in the lagrangian set
  // initial vertices will be at the same position as euler;

  rval = deep_copy_set(&mb, euler_set, lagr_set);
  CHECK_ERR(rval);

  Intx2MeshOnSphere worker(&mb);
  worker.SetRadius(radius);

  worker.SetErrorTolerance(gtol);

  Range local_verts;
  rval = worker.build_processor_euler_boxes(euler_set, local_verts);// output also the local_verts
  // these stay fixed for one run
  // other things from intersection might need to change, like input blue set (departure set)
  // so we need also a method to clean memory
  CHECK_ERR(rval);

  for (int i=1; i<numSteps+1; i++)
  {
    // time depends on i; t = i*T/numSteps: ( 0, T/numSteps, 2*T/numSteps, ..., T )
    // this is really just to create some plots; it is not really needed to proceed
    // the compute_tracer_case1 method actually computes the departure point position
    if (velocity)
    {
      rval = compute_velocity_case1(&mb, euler_set, tagh, rank, i);
      CHECK_ERR(rval);
    }

    // this is to actually compute concentrations at time step i, using the
    //  current concentrations
    //
    rval = compute_tracer_case1(&mb, worker, euler_set, lagr_set, out_set,
        tagElem, tagArea, rank, i, local_verts);
    CHECK_ERR(rval);

  }

  //final vals and 1-norm
  Range::iterator iter = redEls.begin();
  double norm1 = 0.;
  int count =0;
  void * data;
  int j=0;// index in iniVals
  while (iter != redEls.end())
  {
    rval = mb.tag_iterate(tagElem, iter, redEls.end(), count, data);
    CHECK_ERR(rval);
    double * ptrTracer=(double*)data;

    rval = mb.tag_iterate(tagArea, iter, redEls.end(), count, data);
    CHECK_ERR(rval);
    double * ptrArea=(double*)data;
    for (int i=0; i<count; i++, iter++, ptrTracer++, ptrArea++, j++)
    {
      //double area = *ptrArea;
      norm1+=fabs(*ptrTracer - iniVals[j])* (*ptrArea);
    }
  }

  double total_norm1=0;
  int mpi_err = MPI_Reduce(&norm1, &total_norm1, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  if (MPI_SUCCESS != mpi_err) return 1;
  if (0==rank)
    std::cout << " numSteps:" << numSteps << " 1-norm:" << total_norm1 << "\n";
  MPI_Finalize();
  return 0;
}
