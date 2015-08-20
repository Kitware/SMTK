/*
 * create_dp.cpp
 *
 *  Created on: Dec 12, 2013
 *  just to add a "DP" tag to an already partitioned mesh, based on some formula
 *  it is launched in serial
 */
#include "moab/Core.hpp"
#include "moab/Interface.hpp"
#include <iostream>
#include <math.h>
#include <TestUtil.hpp>

#include "CslamUtils.hpp"
#include <assert.h>
using namespace moab;

double radius = 1.;// in m:  6371220.
int field_type = 1;

ErrorCode add_field_value(Interface & mb)
{
  ErrorCode rval = MB_SUCCESS;

  Tag tagTracer = 0;
  std::string tag_name("Tracer");
  rval = mb.tag_get_handle(tag_name.c_str(), 1, MB_TYPE_DOUBLE, tagTracer, MB_TAG_DENSE | MB_TAG_CREAT);
  CHECK_ERR(rval);

  // tagElem is the average computed at each element, from nodal values
  Tag tagElem = 0;
  std::string tag_name2("TracerAverage");
  rval = mb.tag_get_handle(tag_name2.c_str(), 1, MB_TYPE_DOUBLE, tagElem, MB_TAG_DENSE | MB_TAG_CREAT);
  CHECK_ERR(rval);

  Tag tagArea = 0;
  std::string tag_name4("Area");
  rval = mb.tag_get_handle(tag_name4.c_str(), 1, MB_TYPE_DOUBLE, tagArea, MB_TAG_DENSE | MB_TAG_CREAT);
  CHECK_ERR(rval);

  /*
   * get all plys first, then vertices, then move them on the surface of the sphere
   *  radius is 1., most of the time
   *
   */
  Range polygons;
  rval = mb.get_entities_by_dimension(0, 2, polygons);
  if (MB_SUCCESS != rval)
    return rval;

  Range connecVerts;
  rval = mb.get_connectivity(polygons, connecVerts);
  if (MB_SUCCESS != rval)
    return rval;



  void *data; // pointer to the LOC in memory, for each vertex
  int count;

  rval = mb.tag_iterate(tagTracer, connecVerts.begin(), connecVerts.end(), count, data);
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
      rval = mb.get_coords(&oldV, 1, &(posi[0]) );
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
      rval = mb.get_coords(&oldV, 1, &(posi[0]) );
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
      rval = mb.get_coords(&oldV, 1, &(posi[0]) );
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
    rval = mb.tag_iterate(tagElem, iter, polygons.end(), count, data);
    CHECK_ERR(rval);
    double * ptr=(double*)data;

    rval = mb.tag_iterate(tagArea, iter, polygons.end(), count, data);
    CHECK_ERR(rval);
    double * ptrArea=(double*)data;
    for (int i=0; i<count; i++, iter++, ptr++, ptrArea++)
    {
      const moab::EntityHandle * conn = NULL;
      int num_nodes = 0;
      rval = mb.get_connectivity(*iter, conn, num_nodes);
      CHECK_ERR(rval);
      if (num_nodes==0)
        return MB_FAILURE;
      std::vector<double> nodeVals(num_nodes);
      double average=0.;
      rval = mb.tag_get_data(tagTracer, conn, num_nodes, &nodeVals[0] );
      CHECK_ERR(rval);
      for (int j=0; j<num_nodes; j++)
        average+=nodeVals[j];
      average/=num_nodes;
      *ptr = average;

      // now get area
      std::vector<double> coords;
      coords.resize(3*num_nodes);
      rval = mb.get_coords(conn, num_nodes, &coords[0]);
      CHECK_ERR(rval);
      *ptrArea =  area_spherical_polygon_lHuiller (&coords[0], num_nodes, radius);

      // we should have used some
      // total mass:
      local_mass += *ptrArea * average;
    }

  }

  // now we can delete the tags? not yet
  return MB_SUCCESS;
}

int main(int argc, char **argv)
{

  if (argc < 3)
  {
    std::cout << " usage: create_dp <input> <output> -t <time>  -dt <delta_t> [-skipdp] -f <field> -h \n";
    return 1;
  }

  bool skip= false;
  double dt=0.1;
  double t=0.1; // corresponding to diffusion first step

  int index = 2;
  char * input_mesh1 = argv[1];
  char * output = argv[2];
  while (index < argc)
  {
    if (!strcmp(argv[index], "-t")) // this is for radius to project
    {
      t = atof(argv[++index]);
    }
    if (!strcmp(argv[index], "-dt")) // delete partition sets
    {
      dt = atof(argv[++index]);
    }

    if (!strcmp(argv[index], "-h"))
    {
      std::cout << " usage: create_dp <input> <output> -t <time>  -dt <delta_t>  [-skipdp] -f <field>  -h \n";
      return 1;
    }
    if (!strcmp(argv[index], "-f")) // delete partition sets
    {
      field_type = atoi(argv[++index]);
    }

    if (!strcmp(argv[index], "-skipdp") )
    {
      skip = true;
    }
    index++;
  }

  Core moab;
  Interface & mb = moab;

  ErrorCode rval;

  rval = mb.load_mesh(input_mesh1);

  std::cout  << " -t " << t <<  " -dt " << dt << " input: " << input_mesh1 <<
      "  output: " << output << "\n";

  // skip if we need for DP (already existing)
  if (skip)
  {
    std::cout<<" do not add DP tag \n";
  }
  else
  {
    Range verts;
    rval = mb.get_entities_by_dimension(0, 0, verts);
    if (MB_SUCCESS != rval)
      return 1;

    double *x_ptr, *y_ptr, *z_ptr;
    int count;
    rval = mb.coords_iterate(verts.begin(), verts.end(), x_ptr, y_ptr, z_ptr, count);
    if (MB_SUCCESS != rval)
        return 1;
    assert(count == (int) verts.size()); // should end up with just one contiguous chunk of vertices

    Tag tagh = 0;
    std::string tag_name("DP");
    rval = mb.tag_get_handle(tag_name.c_str(), 3, MB_TYPE_DOUBLE, tagh, MB_TAG_DENSE | MB_TAG_CREAT);
    CHECK_ERR(rval);
    void *data; // pointer to the LOC in memory, for each vertex
    int count_tag;

    rval = mb.tag_iterate(tagh, verts.begin(), verts.end(), count_tag, data);
    CHECK_ERR(rval);
    // here we are checking contiguity
    assert(count_tag == (int) verts.size());
    double * ptr_DP=(double*)data;

    for (int v = 0; v < count; v++) {
       //EntityHandle v = verts[v];
       CartVect pos( x_ptr[v], y_ptr[v] , z_ptr[v]);
       CartVect newPos;
       departure_point_case1(pos, t, dt, newPos);
       ptr_DP[0]=newPos[0];
       ptr_DP[1]=newPos[1];
       ptr_DP[2]=newPos[2];
       ptr_DP+=3; // increment to the next vertex
    }
  }

  rval = add_field_value(mb);

  mb.write_file(output);

  return 0;
}




