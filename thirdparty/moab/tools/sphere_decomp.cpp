/*
 * Sphere decomp tool.  Meshes a group of spheres and the interstices between with
 * hex elements, by triangulating the vertices corresponding to sphere centers
 * and subdividing those tets.  For a description of the subdivision template used,
 * see comments in the subdivide_tet function below.
 */

#include "moab/Core.hpp"
#include "SphereDecomp.hpp"
#include <iostream>

const char *SPHERE_RADII_TAG_NAME = "SPHERE_RADII";

#define RR if (moab::MB_SUCCESS != result) return result

int main(int argc, char *argv[]) 
{
  if (argc < 3) {
    std::cout << "Usage: " << argv[0] << " <input_mesh> <output_mesh>" << std::endl;
    return 0;
  }
  
    // create MOAB
  moab::Interface *mbImpl = new moab::Core();
  
    // read in mesh
  moab::ErrorCode result = mbImpl->load_mesh(argv[1]); 
  if (moab::MB_SUCCESS != result) {
    std::cout << "Problems loading mesh." << std::endl;
    return 1;
  }

  moab::Tag sphere_radii_tag = 0;
  double dum_val = 0.1;
  result = mbImpl->tag_get_handle( SPHERE_RADII_TAG_NAME,
                                   1, moab::MB_TYPE_DOUBLE,
                                   sphere_radii_tag,
                                   moab::MB_TAG_DENSE|moab::MB_TAG_CREAT,
                                   &dum_val); 
  if (moab::MB_SUCCESS != result) {
    std::cout << "Problem allocating SPHERE_RADII tag." << std::endl;
    return 1;
  }

  SphereDecomp sd(mbImpl);

  moab::EntityHandle this_set = 0;
  result = sd.build_sphere_mesh(SPHERE_RADII_TAG_NAME, &this_set); RR;
  
    // write mesh
  result = mbImpl->write_mesh(argv[2], &this_set, 1); RR;
  
  return 0;
}

