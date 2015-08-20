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
//#include "TestUtil.hpp"
#include "moab/ParallelComm.hpp"
#include "moab/ProgOptions.hpp"
#include "MBParallelConventions.h"
#include "moab/ReadUtilIface.hpp"
#include "MBTagConventions.hpp"
#include "TestUtil.hpp"
#include "CslamUtils.hpp"

#ifdef MESHDIR
std::string TestDir( STRINGIFY(MESHDIR) );
#else
std::string TestDir(".");
#endif

//std::string file_name("./uniform_15.g");
//std::string file_name("./eulerHomme.vtk");

using namespace moab;

// computes cell barycenters in Cartesian X,Y,Z coordinates
void get_barycenters(moab::Interface * mb, moab::EntityHandle set,
    moab::Tag &planeTag, moab::Tag &barycenterTag);

// computes gnomonic plane for each cell of the Eulerian mesh 
void get_gnomonic_plane(moab::Interface * mb, moab::EntityHandle set,
    moab::Tag &planeTag);

// computes coefficients (A,B,C) for linear reconstruction in gnomonic coordinates: rho(x,y) = Ax + By + C
void get_linear_reconstruction(moab::Interface * mb, moab::EntityHandle set,
    moab::Tag &rhoTag, moab::Tag &planeTag, moab::Tag &barycenterTag,
    moab::Tag &linearCoefTag);

// evaluates the integral of rho(x,y) over cell, should be equal to cell average rho
void test_linear_reconstruction(moab::Interface * mb, moab::EntityHandle set,
    moab::Tag &rhoTag, moab::Tag &planeTag, moab::Tag &barycenterTag,
    moab::Tag &linearCoefTag);

// set density function
moab::ErrorCode add_field_value(moab::Interface * mb,
    moab::EntityHandle euler_set, int rank, moab::Tag & tagTracer,
    moab::Tag & tagElem, moab::Tag & tagArea, int field_type);

// functions that implement gnomonic projection as in Homme (different from Moab implementation)
int gnomonic_projection_test(const moab::CartVect & pos, double R, int plane,
    double & c1, double & c2);
int reverse_gnomonic_projection_test(const double & c1, const double & c2,
    double R, int plane, moab::CartVect & pos);
void decide_gnomonic_plane_test(const CartVect & pos, int & plane);

double radius = 1.; // in m:  6371220.

int main(int /*argc*/, char **/*argv[]*/) {

  // set up MOAB interface and parallel communication
  moab::Core moab;
  moab::Interface& mb = moab;
  moab::ParallelComm mb_pcomm(&mb, MPI_COMM_WORLD);

  //int rank = mb_pcomm->proc_config().proc_rank();
  int rank = mb_pcomm.proc_config().proc_rank();

  // create meshset
  moab::EntityHandle euler_set;
  moab::ErrorCode rval = mb.create_meshset(MESHSET_SET, euler_set);

  std::stringstream opts;
  //opts << "PARALLEL=READ_PART;PARTITION;PARALLEL_RESOLVE_SHARED_ENTS;GATHER_SET=0;PARTITION_METHOD=TRIVIAL_PARTITION;VARIABLE=";
  //opts << "PARALLEL=READ_PART;PARTITION;PARALLEL_RESOLVE_SHARED_ENTS";
  std::string fileN = TestDir + "/mbcslam/fine4.h5m";

  rval = mb.load_file(fileN.c_str(), &euler_set);
  CHECK_ERR(rval);

  // Create tag for cell density
  moab::Tag rhoTag = 0;
  rval = mb.tag_get_handle("Density", 1, moab::MB_TYPE_DOUBLE, rhoTag,
      moab::MB_TAG_CREAT | moab::MB_TAG_DENSE);
  CHECK_ERR(rval);
  moab::Tag rhoNodeTag = 0;
  rval = mb.tag_get_handle("DensityNode", 1, moab::MB_TYPE_DOUBLE, rhoNodeTag,
      moab::MB_TAG_CREAT | moab::MB_TAG_DENSE);
  CHECK_ERR(rval);
  // Create tag for cell area
  moab::Tag areaTag = 0;
  rval = mb.tag_get_handle("Area", 1, moab::MB_TYPE_DOUBLE, areaTag,
      moab::MB_TAG_CREAT | moab::MB_TAG_DENSE);
  CHECK_ERR(rval);
  // Create tag for cell barycenters in 3D Cartesian space
  moab::Tag barycenterTag = 0;
  rval = mb.tag_get_handle("CellBarycenter", 3, moab::MB_TYPE_DOUBLE,
      barycenterTag, moab::MB_TAG_CREAT | moab::MB_TAG_DENSE);
  CHECK_ERR(rval);
  // Create tag for cell density reconstruction coefficients
  moab::Tag coefRhoTag = 0;
  rval = mb.tag_get_handle("LinearCoefRho", 3, moab::MB_TYPE_DOUBLE, coefRhoTag,
      moab::MB_TAG_CREAT | moab::MB_TAG_DENSE);
  CHECK_ERR(rval);
  // Create tag for index of gnomonic plane for each cell
  moab::Tag planeTag = 0;
  rval = mb.tag_get_handle("gnomonicPlane", 1, moab::MB_TYPE_INTEGER, planeTag,
      moab::MB_TAG_CREAT | moab::MB_TAG_DENSE);
  CHECK_ERR(rval);
  // Set density distributions
  rval = add_field_value(&mb, euler_set, rank, rhoNodeTag, rhoTag, areaTag, 2);
  CHECK_ERR(rval);
  // get cell plane
  get_gnomonic_plane(&mb, euler_set, planeTag);

  // get cell barycenters
  get_barycenters(&mb, euler_set, planeTag, barycenterTag);

  // get linear reconstruction
  get_linear_reconstruction(&mb, euler_set, rhoTag, planeTag, barycenterTag,
      coefRhoTag);

  // test linear reconstruction
  test_linear_reconstruction(&mb, euler_set, rhoTag, planeTag, barycenterTag,
      coefRhoTag);

  return 0;
}

void get_gnomonic_plane(moab::Interface * mb, moab::EntityHandle set,
    moab::Tag &planeTag) {
  // get all entities of dimension 2
  // then get the connectivity, etc
  moab::Range cells;
  ErrorCode rval = mb->get_entities_by_dimension(set, 2, cells);
  if (MB_SUCCESS != rval)
    return;

  for (Range::iterator it = cells.begin(); it != cells.end(); it++) {
    moab::EntityHandle icell = *it;
    // get the nodes, then the coordinates
    const moab::EntityHandle * verts;
    int num_nodes;
    rval = mb->get_connectivity(icell, verts, num_nodes);
    if (MB_SUCCESS != rval)
      return;

    std::vector<double> coords(3 * num_nodes);
    // get coordinates
    rval = mb->get_coords(verts, num_nodes, &coords[0]);
    if (MB_SUCCESS != rval)
      return;

    double centerx = 0;
    double centery = 0;
    double centerz = 0;
    for (int inode = 0; inode < num_nodes; inode++) {
      centerx += coords[inode * 3] / num_nodes;
      centery += coords[inode * 3 + 1] / num_nodes;
      centerz += coords[inode * 3 + 2] / num_nodes;
    }
    double R = std::sqrt(
        centerx * centerx + centery * centery + centerz * centerz);
    centerx = centerx / R;
    centery = centery / R;
    centerz = centerz / R;
    moab::CartVect center(centerx, centery, centerz);
    R = 1.0;

    int plane = 0;
    decide_gnomonic_plane(center, plane);
    //decide_gnomonic_plane_test(center,plane);

    rval = mb->tag_set_data(planeTag, &icell, 1, &plane);
  }
  return;
}

void get_barycenters(moab::Interface * mb, moab::EntityHandle set,
    moab::Tag &planeTag, moab::Tag &barycenterTag) {
  // get all entities of dimension 2
  moab::Range cells;
  ErrorCode rval = mb->get_entities_by_dimension(set, 2, cells);
  if (MB_SUCCESS != rval)
    return;

  // set sphere radius to 1 
  double R = 1.0;

  for (Range::iterator it = cells.begin(); it != cells.end(); it++) {
    moab::EntityHandle icell = *it;

    // get the nodes
    const moab::EntityHandle * verts;
    int num_nodes;
    rval = mb->get_connectivity(icell, verts, num_nodes);
    if (MB_SUCCESS != rval)
      return;

    // get coordinates
    std::vector<double> coords(3 * num_nodes);
    rval = mb->get_coords(verts, num_nodes, &coords[0]);
    if (MB_SUCCESS != rval)
      return;

    // get plane for cell
    int plane = 0;
    rval = mb->tag_get_data(planeTag, &icell, 1, &plane);
    if (MB_SUCCESS != rval)
      return;
    std::vector<double> x(num_nodes);
    std::vector<double> y(num_nodes);
    double area = 0;
    double bary_x = 0;
    double bary_y = 0;
    for (int inode = 0; inode < num_nodes; inode++) {
      // radius should be 1.0, but divide by it just in case for now
      double rad = sqrt(
          coords[inode * 3] * coords[inode * 3]
              + coords[inode * 3 + 1] * coords[inode * 3 + 1]
              + coords[inode * 3 + 2] * coords[inode * 3 + 2]);
      CartVect xyzcoord(coords[inode * 3] / rad, coords[inode * 3 + 1] / rad,
          coords[inode * 3 + 2] / rad);
      gnomonic_projection(xyzcoord, R, plane, x[inode], y[inode]);
      // int dum = gnomonic_projection_test(xyzcoord, R, plane, x[inode],y[inode]);

    }

    for (int inode = 0; inode < num_nodes; inode++) {
      int inode2 = inode + 1;
      if (inode2 >= num_nodes)
        inode2 = 0;
      double xmid = 0.5 * (x[inode] + x[inode2]);
      double ymid = 0.5 * (y[inode] + y[inode2]);
      double r1 = sqrt(1 + x[inode] * x[inode] + y[inode] * y[inode]);
      double rm = sqrt(1 + xmid * xmid + ymid * ymid);
      double r2 = sqrt(1 + x[inode2] * x[inode2] + y[inode2] * y[inode2]);
      double hx = x[inode2] - x[inode];

      area += hx
          * (y[inode] / (r1 * (1 + x[inode] * x[inode]))
              + 4.0 * ymid / (rm * (1 + xmid * xmid))
              + y[inode2] / (r2 * (1 + x[inode2] * x[inode2]))) / 6.0;

      bary_x += hx
          * (x[inode] * y[inode] / (r1 * (1 + x[inode] * x[inode]))
              + 4.0 * xmid * ymid / (rm * (1 + xmid * xmid))
              + x[inode2] * y[inode2] / (r2 * (1 + x[inode2] * x[inode2])))
          / 6.0;

      bary_y += -hx * (1.0 / r1 + 4.0 / rm + 1.0 / r2) / 6.0;
    }

    bary_x = bary_x / area;
    bary_y = bary_y / area;

    moab::CartVect barycent;
    reverse_gnomonic_projection(bary_x, bary_y, R, plane, barycent);
    // reverse_gnomonic_projection_test(bary_x, bary_y, R, plane, barycent);
    std::vector<double> barycenter(3);
    barycenter[0] = barycent[0];
    barycenter[1] = barycent[1];
    barycenter[2] = barycent[2];

    rval = mb->tag_set_data(barycenterTag, &icell, 1, &barycenter[0]);

  }
  return;
}

void get_linear_reconstruction(moab::Interface * mb, moab::EntityHandle set,
    moab::Tag &rhoTag, moab::Tag &planeTag, moab::Tag &barycenterTag,
    moab::Tag &linearCoefTag) {
  // get all entities of dimension 2
  Range cells;
  ErrorCode rval = mb->get_entities_by_dimension(set, 2, cells);
  if (MB_SUCCESS != rval)
    return;

  // set sphere radius to 1 
  double R = 1;

  // Get coefficients of reconstruction (in cubed-sphere coordinates)
  // rho(x,y) = Ax + By + C
  for (Range::iterator it = cells.begin(); it != cells.end(); it++) {
    moab::EntityHandle icell = *it;

    // get the nodes, then the coordinates
    const moab::EntityHandle * verts;
    int num_nodes;
    rval = mb->get_connectivity(icell, verts, num_nodes);
    if (MB_SUCCESS != rval)
      return;

    moab::Range adjacentEdges;
    rval = mb->get_adjacencies(&icell, 1, 1, true, adjacentEdges);
    //int num_edges = adjacentEdges.size();

    // get adjacent cells from edges
    moab::Range adjacentCells;
    rval = mb->get_adjacencies(adjacentEdges, 2, true, adjacentCells,
        Interface::UNION);
    if (MB_SUCCESS != rval)
        return;
    // get plane for cell
    int plane = 0;
    rval = mb->tag_get_data(planeTag, &icell, 1, &plane);

    std::vector<double> dx(adjacentCells.size() - 1);
    std::vector<double> dy(adjacentCells.size() - 1);
    std::vector<double> dr(adjacentCells.size() - 1);
    double bary_x;
    double bary_y;

    // get barycenter of cell where reconstruction occurs
    std::vector<double> barycent(3);
    rval = mb->tag_get_data(barycenterTag, &icell, 1, &barycent[0]);
    CartVect barycenter(barycent[0], barycent[1], barycent[2]);
    double cellbaryx = 0;
    double cellbaryy = 0;
    gnomonic_projection(barycenter, R, plane, cellbaryx, cellbaryy);
    // int rc = gnomonic_projection_test(barycenter, R, plane, cellbaryx, cellbaryy);

    // get density value
    double rhocell = 0;
    rval = mb->tag_get_data(rhoTag, &icell, 1, &rhocell);

    // get barycenters of surrounding cells 
    std::vector<double> cell_barys(3 * adjacentCells.size());
    rval = mb->tag_get_data(barycenterTag, adjacentCells, &cell_barys[0]);

    // get density of surrounding cells 
    std::vector<double> rho_vals(adjacentCells.size());
    rval = mb->tag_get_data(rhoTag, adjacentCells, &rho_vals[0]);

    std::size_t jind = 0;
    for (std::size_t i = 0; i < adjacentCells.size(); i++) {

      if (adjacentCells[i] != icell) {
        CartVect bary_xyz(cell_barys[i * 3], cell_barys[i * 3 + 1],
            cell_barys[i * 3 + 2]);
        gnomonic_projection(bary_xyz, R, plane, bary_x, bary_y);
        // rc = gnomonic_projection_test(bary_xyz, R, plane, bary_x, bary_y);

        dx[jind] = bary_x - cellbaryx;
        dy[jind] = bary_y - cellbaryy;
        dr[jind] = rho_vals[i] - rhocell;

        jind++;
      }
    }

    std::vector<double> linearCoef(3);
    if (adjacentCells.size() == 5) {

      // compute normal equations matrix
      double N11 = dx[1] * dx[1] + dx[2] * dx[2] + dx[3] * dx[3]
          + dx[0] * dx[0];
      double N22 = dy[1] * dy[1] + dy[2] * dy[2] + dy[3] * dy[3]
          + dy[0] * dy[0];
      double N12 = dx[1] * dy[1] + dx[2] * dy[2] + dx[3] * dy[3]
          + dx[0] * dy[0];

      // rhs
      double Rx = dx[1] * dr[1] + dx[2] * dr[2] + dx[3] * dr[3] + dx[0] * dr[0];
      double Ry = dy[1] * dr[1] + dy[2] * dr[2] + dy[3] * dr[3] + dy[0] * dr[0];

      // determinant
      double Det = N11 * N22 - N12 * N12;

      // solution
      linearCoef[0] = (Rx * N22 - Ry * N12) / Det;
      linearCoef[1] = (Ry * N11 - Rx * N12) / Det;
      linearCoef[2] = rhocell - linearCoef[0] * cellbaryx
          - linearCoef[1] * cellbaryy;

    } else {
      // default to first order
      linearCoef[0] = 0.0;
      linearCoef[1] = 0.0;
      linearCoef[2] = rhocell;
      std::cout << "Need 4 adjacent cells for linear reconstruction! \n";
    }

    rval = mb->tag_set_data(linearCoefTag, &icell, 1, &linearCoef[0]);

  }
  return;
}

void test_linear_reconstruction(moab::Interface * mb, moab::EntityHandle set,
    moab::Tag &rhoTag, moab::Tag &planeTag, moab::Tag &barycenterTag,
    moab::Tag &linearCoefTag) {
  // get all entities of dimension 2
  Range cells;
  ErrorCode rval = mb->get_entities_by_dimension(set, 2, cells);
  if (MB_SUCCESS != rval)
    return;

  // set sphere radius to 1 
  double R = 1;

  // For get coefficients for reconstruction (in cubed-sphere coordinates)
  for (Range::iterator it = cells.begin(); it != cells.end(); it++) {
    moab::EntityHandle icell = *it;

    // get the nodes, then the coordinates
    const moab::EntityHandle * verts;
    int num_nodes;
    rval = mb->get_connectivity(icell, verts, num_nodes);
    if (MB_SUCCESS != rval)
      return;

    // get coordinates
    std::vector<double> coords(3 * num_nodes);
    rval = mb->get_coords(verts, num_nodes, &coords[0]);
    if (MB_SUCCESS != rval)
      return;

    // get plane for cell
    int plane = 0;
    rval = mb->tag_get_data(planeTag, &icell, 1, &plane);

    // get vertex coordinates projections
    std::vector<double> x(num_nodes);
    std::vector<double> y(num_nodes);
    for (int inode = 0; inode < num_nodes; inode++) {
      double rad = sqrt(
          coords[inode * 3] * coords[inode * 3]
              + coords[inode * 3 + 1] * coords[inode * 3 + 1]
              + coords[inode * 3 + 2] * coords[inode * 3 + 2]);
      CartVect xyzcoord(coords[inode * 3] / rad, coords[inode * 3 + 1] / rad,
          coords[inode * 3 + 2] / rad);
      gnomonic_projection(xyzcoord, R, plane, x[inode], y[inode]);
      // int dum = gnomonic_projection_test(xyzcoord, R, plane, x[inode],y[inode]);

    }

    double area = 0;
    double int_x = 0;
    double int_y = 0;
    for (int inode = 0; inode < num_nodes; inode++) {
      int inode2 = inode + 1;
      if (inode2 >= num_nodes)
        inode2 = 0;
      double xmid = 0.5 * (x[inode] + x[inode2]);
      double ymid = 0.5 * (y[inode] + y[inode2]);
      double r1 = sqrt(1 + x[inode] * x[inode] + y[inode] * y[inode]);
      double rm = sqrt(1 + xmid * xmid + ymid * ymid);
      double r2 = sqrt(1 + x[inode2] * x[inode2] + y[inode2] * y[inode2]);
      double hx = x[inode2] - x[inode];

      area += hx
          * (y[inode] / (r1 * (1 + x[inode] * x[inode]))
              + 4.0 * ymid / (rm * (1 + xmid * xmid))
              + y[inode2] / (r2 * (1 + x[inode2] * x[inode2]))) / 6.0;

      int_x += hx
          * (x[inode] * y[inode] / (r1 * (1 + x[inode] * x[inode]))
              + 4.0 * xmid * ymid / (rm * (1 + xmid * xmid))
              + x[inode2] * y[inode2] / (r2 * (1 + x[inode2] * x[inode2])))
          / 6.0;

      int_y += -hx * (1.0 / r1 + 4.0 / rm + 1.0 / r2) / 6.0;
    }

    // get linear coeficients
    std::vector<double> rho_coefs(3);
    rval = mb->tag_get_data(linearCoefTag, &icell, 1, &rho_coefs[0]);

    // get barycenters
    std::vector<double> bary(3);
    rval = mb->tag_get_data(barycenterTag, &icell, 1, &bary[0]);
    double bary_x;
    double bary_y;
    CartVect bary_xyz(bary[0], bary[1], bary[2]);
    gnomonic_projection(bary_xyz, R, plane, bary_x, bary_y);
    //int rc = gnomonic_projection_test(bary_xyz, R, plane, bary_x, bary_y);

    // get cell average density
    double cell_rho;
    rval = mb->tag_get_data(rhoTag, &icell, 1, &cell_rho);

    // ave rho = \int rho^h(x,y) dV / area = (\int (Ax + By + C) dV) / area
    double rho_test1 = (rho_coefs[0] * int_x + rho_coefs[1] * int_y
        + rho_coefs[2] * area) / area;

    // ave rho = A*bary_x + B*bary_y + C
    double rho_test2 = rho_coefs[0] * bary_x + rho_coefs[1] * bary_y
        + rho_coefs[2];

    std::cout << cell_rho << "  " << rho_test1 << "  " << rho_test2 << "  "
        << cell_rho - rho_test1 << "\n";

  }
  return;
}


int reverse_gnomonic_projection_test(const double & c1, const double & c2,
    double R, int plane, CartVect & pos) {

  double x = c1;
  double y = c2;
  double r = sqrt(1.0 + x * x + y * y);

  switch (plane) {

  case 1: {
    pos[0] = R / r * R;
    pos[1] = R / r * x;
    pos[2] = R / r * y;
    break;
  }
  case 2: {
    pos[0] = -R / r * x;
    pos[1] = R / r * R;
    pos[2] = R / r * y;
    break;
  }
  case 3: {
    pos[0] = -R / r * R;
    pos[1] = -R / r * x;
    pos[2] = R / r * y;
    break;
  }
  case 4: {
    pos[0] = R / r * x;
    pos[1] = -R / r * R;
    pos[2] = R / r * y;
    break;
  }
  case 5: {
    pos[0] = R / r * y;
    pos[1] = R / r * x;
    pos[2] = -R / r * R;
    break;
  }
  case 6: {
    pos[0] = -R / r * y;
    pos[1] = R / r * x;
    pos[2] = R / r * R;
    break;
  }

  }

  return 0; // no error
}

void decide_gnomonic_plane_test(const CartVect & pos, int & plane) {

// This is from early version of Homme vorticity calculation in parvis
// Poles are reversed from Homme and Iulian version. 

//  Now has been changed for consistency

  double X = pos[0];
  double Y = pos[1];
  double Z = pos[2];
  double R = sqrt(X * X + Y * Y + Z * Z);
  X = X / R;
  Y = Y / R;
  Z = Z / R;

  if ((Y < X) & (Y > -X)) {
    if (Z > X) {
      plane = 6;
    } else if (Z < -X) {
      plane = 5;
    } else {
      plane = 1;
    }
  } else if ((Y > X) & (Y < -X)) {
    if (Z > -X) {
      plane = 6;
    } else if (Z < X) {
      plane = 5;
    } else {
      plane = 3;
    }
  } else if ((Y > X) & (Y > -X)) {
    if (Z > Y) {
      plane = 6;
    } else if (Z < -Y) {
      plane = 5;
    } else {
      plane = 2;
    }
  } else if ((Y < X) & (Y < -X)) {
    if (Z > -Y) {
      plane = 6;
    } else if (Z < Y) {
      plane = 5;
    } else {
      plane = 4;
    }
  } else {
    if (abs(X) < Z) {
      plane = 6;
    } else if (Z < -abs(X)) {
      plane = 5;
    } else if ((X > 0) & (Y > 0)) {
      plane = 1;
    } else if ((X < 0) & (Y > 0)) {
      plane = 2;
    } else if ((X < 0) & (Y < 0)) {
      plane = 3;
    } else {
      plane = 4;
    }
  }

  return;
}

moab::ErrorCode add_field_value(moab::Interface * mb,
    moab::EntityHandle euler_set, int /*rank*/, moab::Tag & tagTracer,
    moab::Tag & tagElem, moab::Tag & tagArea, int field_type) {
  moab::ErrorCode rval = MB_SUCCESS;

  /*
   * get all plys first, then vertices, then move them on the surface of the sphere
   *  radius is 1., most of the time
   *
   */
  moab::Range polygons;
  rval = mb->get_entities_by_dimension(euler_set, 2, polygons);
  if (MB_SUCCESS != rval)
    return rval;

  moab::Range connecVerts;
  rval = mb->get_connectivity(polygons, connecVerts);
  if (MB_SUCCESS != rval)
    return rval;

  void *data; // pointer to the LOC in memory, for each vertex
  int count;

  rval = mb->tag_iterate(tagTracer, connecVerts.begin(), connecVerts.end(),
      count, data);
  // here we are checking contiguity
  assert(count == (int ) connecVerts.size());
  double * ptr_DP = (double*) data;
  // lambda is for longitude, theta for latitude
  // param will be: (la1, te1), (la2, te2), b, c; hmax=1, r=1/2
  // nondivergent flow, page 5, case 1, (la1, te1) = (M_PI, M_PI/3)
  //                                    (la2, te2) = (M_PI, -M_PI/3)
  //                 la1,    te1    la2    te2     b     c  hmax  r
  if (field_type == 1) // quasi smooth
      {
    double params[] = { 5 * M_PI / 6.0, 0.0, 7 * M_PI / 6, 0.0, 0.1, 0.9, 1.,
        0.5 };
    for (Range::iterator vit = connecVerts.begin(); vit != connecVerts.end();
        vit++) {
      moab::EntityHandle oldV = *vit;
      moab::CartVect posi;
      rval = mb->get_coords(&oldV, 1, &(posi[0]));

      moab::SphereCoords sphCoord = cart_to_spherical(posi);

      ptr_DP[0] = quasi_smooth_field(sphCoord.lon, sphCoord.lat, params);

      ptr_DP++; // increment to the next node
    }
  } else if (2 == field_type) // smooth
      {
    moab::CartVect p1, p2;
    moab::SphereCoords spr;
    spr.R = 1;
    spr.lat = M_PI / 3;
    spr.lon = M_PI;
    p1 = spherical_to_cart(spr);
    spr.lat = -M_PI / 3;
    p2 = spherical_to_cart(spr);
    //                  x1,    y1,     z1,    x2,   y2,    z2,   h_max, b0
    double params[] = { p1[0], p1[1], p1[2], p2[0], p2[1], p2[2], 1, 5. };
    for (Range::iterator vit = connecVerts.begin(); vit != connecVerts.end();
        vit++) {
      moab::EntityHandle oldV = *vit;
      moab::CartVect posi;
      rval = mb->get_coords(&oldV, 1, &(posi[0]));

      moab::SphereCoords sphCoord = cart_to_spherical(posi);

      ptr_DP[0] = smooth_field(sphCoord.lon, sphCoord.lat, params);

      ptr_DP++; // increment to the next node
    }
  } else if (3 == field_type) // slotted
      {
    //                   la1, te1,   la2, te2,       b,   c,   r
    double params[] = { M_PI, M_PI / 3, M_PI, -M_PI / 3, 0.1, 0.9, 0.5 }; // no h_max
    for (Range::iterator vit = connecVerts.begin(); vit != connecVerts.end();
        vit++) {
      moab::EntityHandle oldV = *vit;
      moab::CartVect posi;
      rval = mb->get_coords(&oldV, 1, &(posi[0]));

      moab::SphereCoords sphCoord = cart_to_spherical(posi);

      ptr_DP[0] = slotted_cylinder_field(sphCoord.lon, sphCoord.lat, params);
      ;

      ptr_DP++; // increment to the next node
    }
  } else if (4 == field_type) // constant = 1
      {
    for (Range::iterator vit = connecVerts.begin(); vit != connecVerts.end();
        vit++) {
     /* moab::EntityHandle oldV = *vit;
      moab::CartVect posi;
      rval = mb->get_coords(&oldV, 1, &(posi[0]));

      moab::SphereCoords sphCoord = cart_to_spherical(posi);*/

      ptr_DP[0] = 1.0;

      ptr_DP++; // increment to the next node
    }
  }
  // add average value for quad/polygon (average corners)
  // do some averages

  Range::iterator iter = polygons.begin();
  while (iter != polygons.end()) {
    rval = mb->tag_iterate(tagElem, iter, polygons.end(), count, data);
    double * ptr = (double*) data;

    rval = mb->tag_iterate(tagArea, iter, polygons.end(), count, data);
    double * ptrArea = (double*) data;
    for (int i = 0; i < count; i++, iter++, ptr++, ptrArea++) {
      const moab::EntityHandle * conn = NULL;
      int num_nodes = 0;
      rval = mb->get_connectivity(*iter, conn, num_nodes);
      if (num_nodes == 0)
        return MB_FAILURE;
      std::vector<double> nodeVals(num_nodes);
      double average = 0.;
      rval = mb->tag_get_data(tagTracer, conn, num_nodes, &nodeVals[0]);
      for (int j = 0; j < num_nodes; j++)
        average += nodeVals[j];
      average /= num_nodes;
      *ptr = average;

    }

  }

  return MB_SUCCESS;
}
