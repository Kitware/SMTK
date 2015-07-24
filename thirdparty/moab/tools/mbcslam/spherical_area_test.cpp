/*
 * spherical_area_test.cpp
 *
 *  Created on: Feb 1, 2013
 */
#include <iostream>
#include "moab/Core.hpp"
#include "moab/Interface.hpp"
#include "CslamUtils.hpp"
#include "../test/TestUtil.hpp"

using namespace moab;

int main(int/* argc*/, char** /* argv[]*/)
{
  // check command line arg
  const char *filename_mesh = STRINGIFY(MESHDIR) "/mbcslam/eulerHomme.vtk";

  // read input mesh in a set
  ErrorCode rval = MB_SUCCESS;
  Core moab;
  Interface* mb = &moab;// global
  EntityHandle sf;
  rval = mb->create_meshset(MESHSET_SET, sf);
  if (MB_SUCCESS != rval)
    return 1;

  rval=mb->load_file(filename_mesh, &sf);
  if (MB_SUCCESS != rval)
    return 1;


  double R = 6.; // should be input
  // compare total area with 4*M_PI * R^2

  double total_area = area_on_sphere(mb, sf, R) ;
  double  area_sphere = R*R*M_PI*4.;
  std::cout<<"total area with Girard:  " << total_area << " area_sphere:" << area_sphere << " rel error:"
      << fabs((total_area-area_sphere)/area_sphere) << "\n";

  double area2 = area_on_sphere_lHuiller(mb, sf, R) ;
  std::cout<<"total area with l'Huiller: " << area2 << " area_sphere:" << area_sphere << " rel error:"
        << fabs((total_area-area_sphere)/area_sphere) << "\n";

  return 0;
}


