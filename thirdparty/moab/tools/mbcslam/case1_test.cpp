/*
 * case1_test.cpp
 *
 *  Created on: Feb 12, 2013
 */

// copy from par_sph_intx
// will save the LOC tag on the euler nodes? maybe
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
#include "../test/TestUtil.hpp"
#include "CslamUtils.hpp"

// for M_PI
#include <math.h>

using namespace moab;
// some input data
double gtol = 0.0001; // this is for geometry tolerance
double CubeSide = 6.; // the above file starts with cube side 6; radius depends on cube side
double t = 0.1, delta_t = 0.43; // check the script

ErrorCode manufacture_lagrange_mesh_on_sphere(Interface * mb,
    EntityHandle euler_set, EntityHandle & lagr_set)
{
  ErrorCode rval = MB_SUCCESS;

  /*
   * get all quads first, then vertices, then move them on the surface of the sphere
   *  radius is in, it comes from MeshKit/python/examples/manufHomme.py :
   *  length = 6.
   *  each edge of the cube will be divided using this meshcount
   *  meshcount = 11
   *   circumscribed sphere radius
   *   radius = length * math.sqrt(3) /2
   */
  double radius = CubeSide / 2 * sqrt(3.); // our value depends on cube side
  Range quads;
  rval = mb->get_entities_by_type(euler_set, MBQUAD, quads);
  if (MB_SUCCESS != rval)
    return rval;

  Range connecVerts;
  rval = mb->get_connectivity(quads, connecVerts);
  if (MB_SUCCESS != rval)
    return rval;

  // create new set
  rval = mb->create_meshset(MESHSET_SET, lagr_set);
  if (MB_SUCCESS != rval)
    return rval;

  // get the coordinates of the old mesh, and move it around the sphere according to case 1
  // now put the vertices in the right place....
  //int vix=0; // vertex index in new array

  // first create departure points (vertices in the lagrange mesh)
  // then connect them in quads
  std::map<EntityHandle, EntityHandle> newNodes;
  for (Range::iterator vit = connecVerts.begin(); vit != connecVerts.end();
      vit++)
  {
    EntityHandle oldV = *vit;
    CartVect posi;
    rval = mb->get_coords(&oldV, 1, &(posi[0]));
    if (MB_SUCCESS != rval)
      return rval;
    // cslam utils, case 1
    CartVect newPos;
    departure_point_case1(posi, t, delta_t, newPos);
    newPos = radius * newPos;
    EntityHandle new_vert;
    rval = mb->create_vertex(&(newPos[0]), new_vert);
    if (MB_SUCCESS != rval)
      return rval;
    newNodes[oldV] = new_vert;
  }
  for (Range::iterator it = quads.begin(); it != quads.end(); it++)
  {
    EntityHandle q = *it;
    int nnodes;
    const EntityHandle * conn4;
    rval = mb->get_connectivity(q, conn4, nnodes);
    if (MB_SUCCESS != rval)
      return rval;
    EntityHandle new_conn[4];
    for (int i = 0; i < nnodes; i++)
    {
      EntityHandle v1 = conn4[i];
      new_conn[i] = newNodes[v1];
    }
    EntityHandle new_quad;
    rval = mb->create_element(MBQUAD, new_conn, 4, new_quad);
    if (MB_SUCCESS != rval)
      return rval;
    rval = mb->add_entities(lagr_set, &new_quad, 1);
    if (MB_SUCCESS != rval)
      return rval;
  }

  return rval;
}
int main(int argc, char **argv)
{


  const char *filename_mesh1 = STRINGIFY(MESHDIR) "/mbcslam/eulerHomme.vtk";
  if (argc > 1)
  {
    int index = 1;
    while (index < argc)
    {
      if (!strcmp(argv[index], "-gtol")) // this is for geometry tolerance
      {
        gtol = atof(argv[++index]);
      }
      if (!strcmp(argv[index], "-cube"))
      {
        CubeSide = atof(argv[++index]);
      }
      if (!strcmp(argv[index], "-dt"))
      {
        delta_t = atof(argv[++index]);
      }
      if (!strcmp(argv[index], "-input"))
      {
        filename_mesh1 = argv[++index];
      }
      index++;
    }
  }
  std::cout << " case 1: use -gtol " << gtol << " -dt " << delta_t <<  " -cube " << CubeSide <<
      " -input " << filename_mesh1 << "\n";



  Core moab;
  Interface & mb = moab;
  EntityHandle euler_set;
  ErrorCode rval;
  rval = mb.create_meshset(MESHSET_SET, euler_set);
  if (MB_SUCCESS != rval)
    return 1;

  rval = mb.load_file(filename_mesh1, &euler_set);

  if (MB_SUCCESS != rval)
    return 1;

  // everybody will get a DP tag, including the non owned entities; so exchange tags is not required for LOC (here)
  EntityHandle lagrange_set;
  rval = manufacture_lagrange_mesh_on_sphere(&mb, euler_set, lagrange_set);
  if (MB_SUCCESS != rval)
    return 1;
  rval = mb.write_file("lagrIni.h5m", 0, 0, &lagrange_set, 1);
 if (MB_SUCCESS != rval)
   std::cout << "can't write lagr set\n";

  rval = enforce_convexity(&mb, lagrange_set);
  if (MB_SUCCESS != rval)
    return 1;

  rval = mb.write_file("lagr.h5m", 0, 0, &lagrange_set, 1);
  if (MB_SUCCESS != rval)
    std::cout << "can't write lagr set\n";

  Intx2MeshOnSphere worker(&mb);

  double radius = CubeSide / 2 * sqrt(3.); // input

  worker.SetRadius(radius);

  //worker.SetEntityType(MBQUAD);

  worker.SetErrorTolerance(gtol);
  std::cout << "error tolerance epsilon_1=" << gtol << "\n";

  EntityHandle outputSet;
  rval = mb.create_meshset(MESHSET_SET, outputSet);
  if (MB_SUCCESS != rval)
    return 1;
  rval = worker.intersect_meshes(lagrange_set, euler_set, outputSet);
  if (MB_SUCCESS != rval)
    return 1;

  std::string opts_write("");
  std::stringstream outf;
  outf << "intersect1" << ".h5m";
  rval = mb.write_file(outf.str().c_str(), 0, 0, &outputSet, 1);
  if (MB_SUCCESS != rval)
    std::cout << "can't write output\n";
  double intx_area = area_on_sphere_lHuiller(&mb, outputSet, radius);
  double arrival_area = area_on_sphere_lHuiller(&mb, euler_set, radius);
  std::cout << " Arrival area: " << arrival_area
      << "  intersection area:" << intx_area << " rel error: "
      << fabs((intx_area - arrival_area) / arrival_area) << "\n";
  if (MB_SUCCESS != rval)
    return 1;

  return 0;
}

