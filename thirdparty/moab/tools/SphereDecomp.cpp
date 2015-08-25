#include "SphereDecomp.hpp"
#include "moab/MeshTopoUtil.hpp"
#include "moab/Range.hpp"
#include "moab/CN.hpp"
#include <math.h>
#include <assert.h>
#include <iostream>

#define RR if (MB_SUCCESS != result) return result

const char *SUBDIV_VERTICES_TAG_NAME = "subdiv_vertices";

using namespace moab;

SphereDecomp::SphereDecomp(Interface *impl) 
{
  mbImpl = impl;
}

ErrorCode SphereDecomp::build_sphere_mesh(const char *sphere_radii_tag_name,
                                            EntityHandle *hex_set) 
{
  ErrorCode result = mbImpl->tag_get_handle(sphere_radii_tag_name, 1, MB_TYPE_DOUBLE, sphereRadiiTag); RR;

    // need to make sure all interior edges and faces are created
  Range all_verts;
  result = mbImpl->get_entities_by_type(0, MBVERTEX, all_verts); RR;
  MeshTopoUtil mtu(mbImpl);
  result = mtu.construct_aentities(all_verts);
  
    // create tag to hold vertices
  result = mbImpl->tag_get_handle(SUBDIV_VERTICES_TAG_NAME, 9, MB_TYPE_HANDLE, 
                                  subdivVerticesTag, MB_TAG_DENSE|MB_TAG_EXCL); RR;

    // compute nodal positions for each dimension element
  result = compute_nodes(1); RR;
  result = compute_nodes(2); RR;
  result = compute_nodes(3); RR;
  
    // build hex elements
  std::vector<EntityHandle> sphere_hexes, interstic_hexes;
  result = build_hexes(sphere_hexes, interstic_hexes); 

  result = mbImpl->tag_delete(subdivVerticesTag); RR;

  if (NULL != hex_set) {
    if (0 == *hex_set) {
      EntityHandle this_set;
        // make a new set
      result = mbImpl->create_meshset(MESHSET_SET, this_set); RR;
      *hex_set = this_set;
    }
    
      // save all the hexes to this set
    result = mbImpl->add_entities(*hex_set, &sphere_hexes[0], 
                                  sphere_hexes.size()); RR;
    result = mbImpl->add_entities(*hex_set, &interstic_hexes[0], 
                                  interstic_hexes.size()); RR;
  }
      
  return result;
}

ErrorCode SphereDecomp::compute_nodes(const int dim) 
{
    // get facets of that dimension
  Range these_ents;
  const EntityType the_types[4] = {MBVERTEX, MBEDGE, MBTRI, MBTET};
  
  ErrorCode result = mbImpl->get_entities_by_dimension(0, dim, these_ents); RR;
  assert(mbImpl->type_from_handle(*these_ents.begin()) == the_types[dim] &&
         mbImpl->type_from_handle(*these_ents.rbegin()) == the_types[dim]);
  
  EntityHandle subdiv_vertices[9];
  MeshTopoUtil mtu(mbImpl);
  double avg_pos[3], vert_pos[12], new_vert_pos[12], new_new_vert_pos[3];
  double radii[4], unitv[3];
  int num_verts = CN::VerticesPerEntity(the_types[dim]);
  
  for (Range::iterator rit = these_ents.begin(); rit != these_ents.end(); rit++) {
    
      // get vertices
    const EntityHandle *connect;
    int num_connect;
    result = mbImpl->get_connectivity(*rit, connect, num_connect); RR;

      // compute center
    result = mtu.get_average_position(connect, num_connect, avg_pos); RR;

      // create center vertex
    result = mbImpl->create_vertex(avg_pos, subdiv_vertices[num_verts]); RR;
    
      // get coords of other vertices
    result = mbImpl->get_coords(connect, num_connect, vert_pos); RR;
    
      // get radii associated with each vertex
    result = mbImpl->tag_get_data(sphereRadiiTag, connect, num_connect, radii); RR;
    
      // compute subdiv vertex position for each vertex
    for (int i = 0; i < num_verts; i++) {
      for (int j = 0; j < 3; j++) unitv[j] = avg_pos[j] - vert_pos[3*i+j];
      double vlength = sqrt(unitv[0]*unitv[0] + unitv[1]*unitv[1] + unitv[2]*unitv[2]);
      if (vlength < radii[i]) {
        std::cout << "Radius too large at vertex " << i << std::endl;
        result = MB_FAILURE;
        continue;
      }
      
      
      for (int j = 0; j < 3; j++) unitv[j] /= vlength;
          
      for (int j = 0; j < 3; j++)
        new_vert_pos[3*i+j] = vert_pos[3*i+j] + radii[i] * unitv[j];

      // create vertex at this position
      ErrorCode tmp_result = mbImpl->create_vertex(&new_vert_pos[3*i], subdiv_vertices[i]);
      if (MB_SUCCESS != tmp_result) result = tmp_result;
    }
    
    if (MB_SUCCESS != result) return result;
    
      // compute subdiv vertex positions for vertices inside spheres; just mid-pt between
      // previous subdiv vertex and corner vertex
    for (int i = 0; i < num_verts; i++) {
      for (int j = 0; j < 3; j++) 
        new_new_vert_pos[j] = .5 * (vert_pos[3*i+j] + new_vert_pos[3*i+j]);

      result = mbImpl->create_vertex(new_new_vert_pos, subdiv_vertices[num_verts+1+i]);
    }

      // set the tag
    result = mbImpl->tag_set_data(subdivVerticesTag, &(*rit), 1, subdiv_vertices); RR;
  }
  
  return result;
}

ErrorCode SphereDecomp::build_hexes(std::vector<EntityHandle> &sphere_hexes,
                        std::vector<EntityHandle> &interstic_hexes) 
{
    // build hexes inside each tet element separately
  Range tets;
  ErrorCode result = mbImpl->get_entities_by_type(0, MBTET, tets); RR;
  
  for (Range::iterator vit = tets.begin(); vit != tets.end(); vit++) {
    result = subdivide_tet(*vit, sphere_hexes, interstic_hexes); RR;
  }
  
  return MB_SUCCESS;
}

ErrorCode SphereDecomp::subdivide_tet(EntityHandle tet, 
                          std::vector<EntityHandle> &sphere_hexes,
                          std::vector<EntityHandle> &interstic_hexes) 
{
    // 99: (#subdiv_verts/entity=9) * (#edges=6 + #faces=4 + 1=tet)
  EntityHandle subdiv_verts[99];

    // get tet connectivity
  std::vector<EntityHandle> tet_conn;
  ErrorCode result = mbImpl->get_connectivity(&tet, 1, tet_conn); RR;
  
  for (int dim = 1; dim <= 3; dim++) {
      // get entities of this dimension
    std::vector<EntityHandle> ents;
    if (dim != 3) {
      result = mbImpl->get_adjacencies(&tet, 1, dim, false, ents); RR; 
    }
    else ents.push_back(tet);
    
      // for each, get subdiv verts & put into vector
    for (std::vector<EntityHandle>::iterator vit = ents.begin(); vit != ents.end(); vit++) {
      result = retrieve_subdiv_verts(tet, *vit, &tet_conn[0], dim, subdiv_verts); RR;
    }
  }

    // ok, subdiv_verts are in canonical order; now create the hexes, using pre-computed templates

    // Templates are specified in terms of the vertices making up each hex; vertices are specified 
    // by specifying the facet index and type they resolve, and the index of that vertex in that facet's
    // subdivision vertices list.

    // Each facet is subdivided into:
    // - a mid vertex
    // - one vertex for each corner vertex on the facet (located on a line between the mid vertex and
    //   the corresponding corner vertex, a distance equal to the sphere radius away from the corner
    //   vertex)
    // - one vertex midway between each corner vertex and the corresponding "sphere surface" vertex
    // For edges, tris and tets this gives 5, 7 and 9 subdivision vertices, respectively.  Subdivision vertices
    // appear in the list in the order: sphere surface vertices, mid vertex, sphere interior vertices.  In
    // each of those sub lists, vertices are listed in the canonical order of the corresponding corner vertices
    // for that facet.

    // Subdivision vertices for facetes are indexed by listing the facet type they resolve (EDGE, FACE, TET), the index of
    // that facet (integer = 0..5, 0..3, 0 for edges, tris, tet, resp), and subdivision index (AINDEX..EINDEX for
    // edges, AINDEX..GINDEX for tris, AINDEX..IINDEX for tets).

    // Subdivision vertices for all facets of a tet are stored in one subdivision vertex vector, in order of increasing
    // facet dimension and index (index varies fastest).  The ESV, FSV, and TSV macros are used to compute the
    // indices into that vector for various parameters.  The CV macro is used to index into the tet connectivity
    // vector.

    // Subdivision templates for splitting the tet into 28 hexes were derived by hand, and are listed below 
    // (using the indexing scheme described above).

#define EDGE 0
#define FACE 1
#define TET 2
#define AINDEX 0
#define BINDEX 1
#define CINDEX 2
#define DINDEX 3
#define EINDEX 4
#define FINDEX 5
#define GINDEX 6
#define HINDEX 7
#define IINDEX 8
#define V0INDEX 0
#define V1INDEX 1
#define V2INDEX 2
#define V3INDEX 3
#define CV(a) tet_conn[a]
#define ESV(a,b) subdiv_verts[a*9+b]
#define FSV(a,b) subdiv_verts[54+a*9+b]
#define TSV(a,b) subdiv_verts[90+a*9+b]

  EntityHandle this_connect[8], this_hex;

    // first, interstices hexes, three per vertex/spherical surface
// V0:
  int i = 0;
  this_connect[i++]=ESV(0,AINDEX); this_connect[i++]=ESV(0,CINDEX); this_connect[i++]=FSV(3,DINDEX); this_connect[i++]=FSV(3,AINDEX); 
  this_connect[i++]=FSV(0,AINDEX); this_connect[i++]=FSV(0,DINDEX); this_connect[i++]=TSV(0,EINDEX); this_connect[i++]=TSV(0,AINDEX);
  result = mbImpl->create_element(MBHEX, this_connect, 8, this_hex); RR;
  interstic_hexes.push_back(this_hex);

  i = 0;
  this_connect[i++]=FSV(0,AINDEX); this_connect[i++]=FSV(0,DINDEX); this_connect[i++]=TSV(0,EINDEX); this_connect[i++]=TSV(0,AINDEX); 
  this_connect[i++]=ESV(3,AINDEX); this_connect[i++]=ESV(3,CINDEX); this_connect[i++]=FSV(2,DINDEX); this_connect[i++]=FSV(2,AINDEX);
  result = mbImpl->create_element(MBHEX, this_connect, 8, this_hex); RR;
  interstic_hexes.push_back(this_hex);

  i = 0;
  this_connect[i++]=FSV(3,AINDEX); this_connect[i++]=FSV(3,DINDEX); this_connect[i++]=ESV(2,CINDEX); this_connect[i++]=ESV(2,BINDEX); 
  this_connect[i++]=TSV(0,AINDEX); this_connect[i++]=TSV(0,EINDEX); this_connect[i++]=FSV(2,DINDEX); this_connect[i++]=FSV(2,AINDEX);
  result = mbImpl->create_element(MBHEX, this_connect, 8, this_hex); RR;
  interstic_hexes.push_back(this_hex);


// V1:
  i = 0;
  this_connect[i++]=ESV(0,CINDEX); this_connect[i++]=ESV(0,BINDEX); this_connect[i++]=FSV(3,CINDEX); this_connect[i++]=FSV(3,DINDEX); 
  this_connect[i++]=FSV(0,DINDEX); this_connect[i++]=FSV(0,BINDEX); this_connect[i++]=TSV(0,BINDEX); this_connect[i++]=TSV(0,EINDEX);
  result = mbImpl->create_element(MBHEX, this_connect, 8, this_hex); RR;
  interstic_hexes.push_back(this_hex);

  i = 0;
  this_connect[i++]=FSV(0,DINDEX); this_connect[i++]=FSV(0,BINDEX); this_connect[i++]=TSV(0,BINDEX); this_connect[i++]=TSV(0,EINDEX); 
  this_connect[i++]=ESV(4,CINDEX); this_connect[i++]=ESV(4,AINDEX); this_connect[i++]=FSV(1,AINDEX); this_connect[i++]=FSV(1,DINDEX);
  result = mbImpl->create_element(MBHEX, this_connect, 8, this_hex); RR;
  interstic_hexes.push_back(this_hex);

  i = 0;
  this_connect[i++]=FSV(1,DINDEX); this_connect[i++]=FSV(1,AINDEX); this_connect[i++]=TSV(0,BINDEX); this_connect[i++]=TSV(0,EINDEX); 
  this_connect[i++]=ESV(1,CINDEX); this_connect[i++]=ESV(1,AINDEX); this_connect[i++]=FSV(3,CINDEX); this_connect[i++]=FSV(3,DINDEX);
  result = mbImpl->create_element(MBHEX, this_connect, 8, this_hex); RR;
  interstic_hexes.push_back(this_hex);


// V2:
  i = 0;
  this_connect[i++]=FSV(3,DINDEX); this_connect[i++]=ESV(1,CINDEX); this_connect[i++]=ESV(1,BINDEX); this_connect[i++]=FSV(3,BINDEX); 
  this_connect[i++]=TSV(0,EINDEX); this_connect[i++]=FSV(1,DINDEX); this_connect[i++]=FSV(1,BINDEX); this_connect[i++]=TSV(0,CINDEX);
  result = mbImpl->create_element(MBHEX, this_connect, 8, this_hex); RR;
  interstic_hexes.push_back(this_hex);

  i = 0;
  this_connect[i++]=TSV(0,EINDEX); this_connect[i++]=FSV(1,DINDEX); this_connect[i++]=FSV(1,BINDEX); this_connect[i++]=TSV(0,CINDEX); 
  this_connect[i++]=FSV(2,DINDEX); this_connect[i++]=ESV(5,CINDEX); this_connect[i++]=ESV(5,AINDEX); this_connect[i++]=FSV(2,CINDEX);
  result = mbImpl->create_element(MBHEX, this_connect, 8, this_hex); RR;
  interstic_hexes.push_back(this_hex);

  i = 0;
  this_connect[i++]=TSV(0,CINDEX); this_connect[i++]=FSV(2,CINDEX); this_connect[i++]=ESV(2,AINDEX); this_connect[i++]=FSV(3,BINDEX); 
  this_connect[i++]=TSV(0,EINDEX); this_connect[i++]=FSV(2,DINDEX); this_connect[i++]=ESV(2,CINDEX); this_connect[i++]=FSV(3,DINDEX);
  result = mbImpl->create_element(MBHEX, this_connect, 8, this_hex); RR;
  interstic_hexes.push_back(this_hex);


// V3:
  i = 0;
  this_connect[i++]=TSV(0,EINDEX); this_connect[i++]=FSV(1,DINDEX); this_connect[i++]=ESV(5,CINDEX); this_connect[i++]=FSV(2,DINDEX); 
  this_connect[i++]=TSV(0,DINDEX); this_connect[i++]=FSV(1,CINDEX); this_connect[i++]=ESV(5,BINDEX); this_connect[i++]=FSV(2,BINDEX);
  result = mbImpl->create_element(MBHEX, this_connect, 8, this_hex); RR;
  interstic_hexes.push_back(this_hex);

  i = 0;
  this_connect[i++]=FSV(0,DINDEX); this_connect[i++]=ESV(4,CINDEX); this_connect[i++]=FSV(1,DINDEX); this_connect[i++]=TSV(0,EINDEX); 
  this_connect[i++]=FSV(0,CINDEX); this_connect[i++]=ESV(4,BINDEX); this_connect[i++]=FSV(1,CINDEX); this_connect[i++]=TSV(0,DINDEX);
  result = mbImpl->create_element(MBHEX, this_connect, 8, this_hex); RR;
  interstic_hexes.push_back(this_hex);

  i = 0;
  this_connect[i++]=ESV(3,CINDEX); this_connect[i++]=FSV(0,DINDEX); this_connect[i++]=TSV(0,EINDEX); this_connect[i++]=FSV(2,DINDEX); 
  this_connect[i++]=ESV(3,BINDEX); this_connect[i++]=FSV(0,CINDEX); this_connect[i++]=TSV(0,DINDEX); this_connect[i++]=FSV(2,BINDEX);
  result = mbImpl->create_element(MBHEX, this_connect, 8, this_hex); RR;
  interstic_hexes.push_back(this_hex);

    // now, the sphere interiors, four hexes per vertex sphere

// V0:
  i = 0;
  this_connect[i++]=CV(V0INDEX); this_connect[i++]=ESV(0,DINDEX); this_connect[i++]=FSV(3,EINDEX); this_connect[i++]=ESV(2,EINDEX); 
  this_connect[i++]=ESV(3,DINDEX); this_connect[i++]=FSV(0,EINDEX); this_connect[i++]=TSV(0,FINDEX); this_connect[i++]=FSV(2,EINDEX);
  result = mbImpl->create_element(MBHEX, this_connect, 8, this_hex); RR;
  sphere_hexes.push_back(this_hex);

  i = 0;
  this_connect[i++]=ESV(0,DINDEX); this_connect[i++]=ESV(0,AINDEX); this_connect[i++]=FSV(3,AINDEX); this_connect[i++]=FSV(3,EINDEX); 
  this_connect[i++]=FSV(0,EINDEX); this_connect[i++]=FSV(0,AINDEX); this_connect[i++]=TSV(0,AINDEX); this_connect[i++]=TSV(0,FINDEX);
  result = mbImpl->create_element(MBHEX, this_connect, 8, this_hex); RR;
  sphere_hexes.push_back(this_hex);

  i = 0;
  this_connect[i++]=FSV(3,EINDEX); this_connect[i++]=FSV(3,AINDEX); this_connect[i++]=ESV(2,BINDEX); this_connect[i++]=ESV(2,EINDEX); 
  this_connect[i++]=TSV(0,FINDEX); this_connect[i++]=TSV(0,AINDEX); this_connect[i++]=FSV(2,AINDEX); this_connect[i++]=FSV(2,EINDEX);
  result = mbImpl->create_element(MBHEX, this_connect, 8, this_hex); RR;
  sphere_hexes.push_back(this_hex);

  i = 0;
  this_connect[i++]=TSV(0,FINDEX); this_connect[i++]=TSV(0,AINDEX); this_connect[i++]=FSV(2,AINDEX); this_connect[i++]=FSV(2,EINDEX); 
  this_connect[i++]=FSV(0,EINDEX); this_connect[i++]=FSV(0,AINDEX); this_connect[i++]=ESV(3,AINDEX); this_connect[i++]=ESV(3,DINDEX);
  result = mbImpl->create_element(MBHEX, this_connect, 8, this_hex); RR;
  sphere_hexes.push_back(this_hex);


// V1:
  i = 0;
  this_connect[i++]=CV(V1INDEX); this_connect[i++]=ESV(1,DINDEX); this_connect[i++]=FSV(3,GINDEX); this_connect[i++]=ESV(0,EINDEX); 
  this_connect[i++]=ESV(4,DINDEX); this_connect[i++]=FSV(1,EINDEX); this_connect[i++]=TSV(0,GINDEX); this_connect[i++]=FSV(0,FINDEX);
  result = mbImpl->create_element(MBHEX, this_connect, 8, this_hex); RR;
  sphere_hexes.push_back(this_hex);

  i = 0;
  this_connect[i++]=FSV(3,GINDEX); this_connect[i++]=ESV(1,DINDEX); this_connect[i++]=ESV(1,AINDEX); this_connect[i++]=FSV(3,CINDEX); 
  this_connect[i++]=TSV(0,GINDEX); this_connect[i++]=FSV(1,EINDEX); this_connect[i++]=FSV(1,AINDEX); this_connect[i++]=TSV(0,BINDEX);
  result = mbImpl->create_element(MBHEX, this_connect, 8, this_hex); RR;
  sphere_hexes.push_back(this_hex);

  i = 0;
  this_connect[i++]=TSV(0,GINDEX); this_connect[i++]=FSV(1,EINDEX); this_connect[i++]=FSV(1,AINDEX); this_connect[i++]=TSV(0,BINDEX); 
  this_connect[i++]=FSV(0,FINDEX); this_connect[i++]=ESV(4,DINDEX); this_connect[i++]=ESV(4,AINDEX); this_connect[i++]=FSV(0,BINDEX);
  result = mbImpl->create_element(MBHEX, this_connect, 8, this_hex); RR;
  sphere_hexes.push_back(this_hex);

  i = 0;
  this_connect[i++]=ESV(0,BINDEX); this_connect[i++]=ESV(0,EINDEX); this_connect[i++]=FSV(3,GINDEX); this_connect[i++]=FSV(3,CINDEX); 
  this_connect[i++]=FSV(0,BINDEX); this_connect[i++]=FSV(0,FINDEX); this_connect[i++]=TSV(0,GINDEX); this_connect[i++]=TSV(0,BINDEX);
  result = mbImpl->create_element(MBHEX, this_connect, 8, this_hex); RR;
  sphere_hexes.push_back(this_hex);


// V2:
  i = 0;
  this_connect[i++]=ESV(1,BINDEX); this_connect[i++]=ESV(1,EINDEX); this_connect[i++]=FSV(3,FINDEX); this_connect[i++]=FSV(3,BINDEX); 
  this_connect[i++]=FSV(1,BINDEX); this_connect[i++]=FSV(1,FINDEX); this_connect[i++]=TSV(0,HINDEX); this_connect[i++]=TSV(0,CINDEX);
  result = mbImpl->create_element(MBHEX, this_connect, 8, this_hex); RR;
  sphere_hexes.push_back(this_hex);

  i = 0;
  this_connect[i++]=FSV(3,FINDEX); this_connect[i++]=ESV(1,EINDEX); this_connect[i++]=CV(V2INDEX); this_connect[i++]=ESV(2,DINDEX); 
  this_connect[i++]=TSV(0,HINDEX); this_connect[i++]=FSV(1,FINDEX); this_connect[i++]=ESV(5,DINDEX); this_connect[i++]=FSV(2,GINDEX);
  result = mbImpl->create_element(MBHEX, this_connect, 8, this_hex); RR;
  sphere_hexes.push_back(this_hex);

  i = 0;
  this_connect[i++]=TSV(0,HINDEX); this_connect[i++]=FSV(1,FINDEX); this_connect[i++]=ESV(5,DINDEX); this_connect[i++]=FSV(2,GINDEX); 
  this_connect[i++]=TSV(0,CINDEX); this_connect[i++]=FSV(1,BINDEX); this_connect[i++]=ESV(5,AINDEX); this_connect[i++]=FSV(2,CINDEX);
  result = mbImpl->create_element(MBHEX, this_connect, 8, this_hex); RR;
  sphere_hexes.push_back(this_hex);

  i = 0;
  this_connect[i++]=FSV(3,BINDEX); this_connect[i++]=FSV(3,FINDEX); this_connect[i++]=ESV(2,DINDEX); this_connect[i++]=ESV(2,AINDEX); 
  this_connect[i++]=TSV(0,CINDEX); this_connect[i++]=TSV(0,HINDEX); this_connect[i++]=FSV(2,GINDEX); this_connect[i++]=FSV(2,CINDEX);
  result = mbImpl->create_element(MBHEX, this_connect, 8, this_hex); RR;
  sphere_hexes.push_back(this_hex);


// V3:
  i = 0;
  this_connect[i++]=FSV(0,CINDEX); this_connect[i++]=ESV(4,BINDEX); this_connect[i++]=FSV(1,CINDEX); this_connect[i++]=TSV(0,DINDEX); 
  this_connect[i++]=FSV(0,GINDEX); this_connect[i++]=ESV(4,EINDEX); this_connect[i++]=FSV(1,GINDEX); this_connect[i++]=TSV(0,IINDEX);
  result = mbImpl->create_element(MBHEX, this_connect, 8, this_hex); RR;
  sphere_hexes.push_back(this_hex);

  i = 0;
  this_connect[i++]=ESV(3,BINDEX); this_connect[i++]=FSV(0,CINDEX); this_connect[i++]=TSV(0,DINDEX); this_connect[i++]=FSV(2,BINDEX); 
  this_connect[i++]=ESV(3,EINDEX); this_connect[i++]=FSV(0,GINDEX); this_connect[i++]=TSV(0,IINDEX); this_connect[i++]=FSV(2,FINDEX);
  result = mbImpl->create_element(MBHEX, this_connect, 8, this_hex); RR;
  sphere_hexes.push_back(this_hex);

  i = 0;
  this_connect[i++]=TSV(0,DINDEX); this_connect[i++]=FSV(1,CINDEX); this_connect[i++]=ESV(5,BINDEX); this_connect[i++]=FSV(2,BINDEX); 
  this_connect[i++]=TSV(0,IINDEX); this_connect[i++]=FSV(1,GINDEX); this_connect[i++]=ESV(5,EINDEX); this_connect[i++]=FSV(2,FINDEX);
  result = mbImpl->create_element(MBHEX, this_connect, 8, this_hex); RR;
  sphere_hexes.push_back(this_hex);

  i = 0;
  this_connect[i++]=FSV(0,GINDEX); this_connect[i++]=ESV(4,EINDEX); this_connect[i++]=FSV(1,GINDEX); this_connect[i++]=TSV(0,IINDEX); 
  this_connect[i++]=ESV(3,EINDEX); this_connect[i++]=CV(V3INDEX); this_connect[i++]=ESV(5,EINDEX); this_connect[i++]=FSV(2,FINDEX);
  result = mbImpl->create_element(MBHEX, this_connect, 8, this_hex); RR;
  sphere_hexes.push_back(this_hex);

  return result;
}

ErrorCode SphereDecomp::retrieve_subdiv_verts(EntityHandle tet, EntityHandle this_ent,
                                  const EntityHandle *tet_conn,
                                  const int dim, EntityHandle *subdiv_verts) 
{
    // get the subdiv verts for this entity
  ErrorCode result;
  
    // if it's a tet, just put them on the end & return
  if (tet == this_ent) {
    result = mbImpl->tag_get_data(subdivVerticesTag, &this_ent, 1, &subdiv_verts[90]);
    return MB_SUCCESS;
  }
  
    // if it's a sub-entity, need to find index, relative orientation, and offset
    // get connectivity of sub-entity
  std::vector<EntityHandle> this_conn;
  result = mbImpl->get_connectivity(&this_ent, 1, this_conn); RR;
  
    // get relative orientation
  std::vector<int> conn_tet_indices(this_conn.size());
  for (size_t i = 0; i < this_conn.size(); ++i)
    conn_tet_indices[i] = std::find(tet_conn, tet_conn+4, this_conn[i]) - tet_conn;
  int sense, side_no, offset;
  int success = CN::SideNumber(MBTET, &conn_tet_indices[0],
                                 this_conn.size(), dim, side_no, sense, offset);
  if (-1 == success) return MB_FAILURE;
  
    // start of this entity's subdiv_verts; edges go first, then preceding sides, then this one;
    // this assumes 6 edges/tet
  EntityHandle *subdiv_start = &subdiv_verts[((dim-1)*6 + side_no) * 9];
  
    // get subdiv_verts and put them into proper place
  result = mbImpl->tag_get_data(subdivVerticesTag, &this_ent, 1, subdiv_start);

    // could probably do this more elegantly, but isn't worth it
#define SWITCH(a,b) {EntityHandle tmp_handle = a; a = b; b = tmp_handle;}
  switch (dim) {
    case 1:
      if (offset != 0 || sense == -1) {
        SWITCH(subdiv_start[0], subdiv_start[1]);
        SWITCH(subdiv_start[3], subdiv_start[4]);
      }
      break;
    case 2:
        // rotate first
      if (0 != offset) {
        std::rotate(subdiv_start, subdiv_start+offset, subdiv_start+3);
        std::rotate(subdiv_start+4, subdiv_start+4+offset, subdiv_start+7);
      }
        // now flip, if necessary
      if (-1 == sense) {
        SWITCH(subdiv_start[1], subdiv_start[2]);
        SWITCH(subdiv_start[5], subdiv_start[6]);
      }
      break;
    default:
      return MB_FAILURE;
  }
  
    // ok, we're done
  return MB_SUCCESS;
}
