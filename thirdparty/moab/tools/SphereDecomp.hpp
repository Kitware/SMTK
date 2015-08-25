#ifndef SPHERE_DECOMP_HPP
#define SPHERE_DECOMP_HPP

#include "moab/Interface.hpp"

class SphereDecomp 
{
public:
  SphereDecomp(moab::Interface *impl);

  moab::ErrorCode build_sphere_mesh(const char *sphere_radii_tag_name, 
                                moab::EntityHandle *hex_set = NULL);
  
private:

    //! compute subdivision vertices on entities of specified dimension
  moab::ErrorCode compute_nodes(const int dim);

    //! subdivide tets based on subdiv vertices, returning in lists according
    //! to whether they're inside or outside spheres
  moab::ErrorCode build_hexes(std::vector<moab::EntityHandle> &sphere_hexes,
                          std::vector<moab::EntityHandle> &interstic_hexes);
  
    //! subdivide an individual tet
  moab::ErrorCode subdivide_tet(moab::EntityHandle tet, 
                            std::vector<moab::EntityHandle> &sphere_hexes,
                            std::vector<moab::EntityHandle> &interstic_hexes);
  
    //! retrieve the subdivision vertices for a given entity in a given tet,
    //! placing them in the array oriented wrt the tet
  moab::ErrorCode retrieve_subdiv_verts(moab::EntityHandle tet, moab::EntityHandle this_ent,
                                    const moab::EntityHandle *tet_conn,
                                    const int dim, moab::EntityHandle *subdiv_verts);
  
    //! tag used to hold sphere radii (assigned to vertices)
  moab::Tag sphereRadiiTag;

    //! used to store subdiv vertices for a given d>0 entity
  moab::Tag subdivVerticesTag;
  
    //! MOAB interface ptr
  moab::Interface *mbImpl;
  
};
#endif
