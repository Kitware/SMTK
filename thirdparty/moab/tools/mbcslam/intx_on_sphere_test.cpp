/*
 * intx_on_sphere_test.cpp
 *
 *  Created on: Oct 3, 2012
 *      Author: iulian
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
#include "../test/TestUtil.hpp"
#include <math.h>

using namespace moab;

int main(int argc, char* argv[])
{
  // check command line arg// Euler grid is red, arrival, Lagrangian is blue, departure
  // will will keep the
  const char *filename_mesh1 = STRINGIFY(MESHDIR) "/mbcslam/lagrangeHomme.vtk";
  const char *filename_mesh2 = STRINGIFY(MESHDIR) "/mbcslam/eulerHomme.vtk";
  double R = 6. * sqrt(3.) / 2; // input
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
    printf("No files specified.  Defaulting to: %s  %s  %f %f %s\n",
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
  rval = fix_degenerate_quads(mb, sf2);
  if (MB_SUCCESS != rval)
    return 1;

  rval =positive_orientation(mb, sf1, R);
  if (MB_SUCCESS != rval)
    return 1;

  rval =positive_orientation(mb, sf2, R);
  if (MB_SUCCESS != rval)
    return 1;


  Intx2MeshOnSphere  worker(mb);

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
  double initial_area = area_on_sphere_lHuiller(mb, sf1, R);
  double area_method1 = area_on_sphere_lHuiller(mb, outputSet, R);
  double area_method2 = area_on_sphere(mb, outputSet, R);

  std::cout << "initial area: " << initial_area << "\n";
  std::cout<< " area with l'Huiller: " << area_method1 << " with Girard: " << area_method2<< "\n";
  std::cout << " relative difference areas " << fabs(area_method1-area_method2)/area_method1 << "\n";
  std::cout << " relative error " << fabs(area_method1-initial_area)/area_method1 << "\n";

  return 0;

}


