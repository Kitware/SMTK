/*
 * Intx2MeshInPlane.cpp
 *
 *  Created on: Oct 24, 2012
 *      Author: iulian
 */

#include "Intx2MeshInPlane.hpp"
#include "moab/GeomUtil.hpp"
namespace moab {
Intx2MeshInPlane::Intx2MeshInPlane(Interface * mbimpl):Intx2Mesh(mbimpl){

}

Intx2MeshInPlane::~Intx2MeshInPlane() {
  // TODO Auto-generated destructor stub
}

double Intx2MeshInPlane::setup_red_cell(EntityHandle red, int & nsRed)
{
  // the points will be at most ?; they will describe a convex patch, after the points will be ordered and
  // collapsed (eliminate doubles)
  // the area is not really required
  // get coordinates of the red quad
  double cellArea =0;
  int num_nodes;
  ErrorCode rval = mb->get_connectivity(red, redConn, num_nodes);

  nsRed = num_nodes;

  rval = mb->get_coords(redConn, num_nodes, &(redCoords[0][0]));
  if (MB_SUCCESS != rval)
    return 1.; // it should be an error

  for (int j = 0; j < nsRed; j++) {
    // populate coords in the plane for intersection
    // they should be oriented correctly, positively
    redCoords2D[2 * j] = redCoords[j][0]; // x coordinate,
    redCoords2D[2 * j + 1] = redCoords[j][1]; // y coordinate
  }
  for (int j=1; j<nsRed-1; j++)
    cellArea += area2D(&redCoords2D[0], &redCoords2D[2*j], &redCoords2D[2*j+2]);

  return cellArea;
}
int Intx2MeshInPlane::computeIntersectionBetweenRedAndBlue(EntityHandle red,
    EntityHandle blue, double * P, int & nP, double & area, int markb[MAXEDGES],
    int markr[MAXEDGES], int & nsBlue, int & nsRed, bool check_boxes_first) {

  int num_nodes = 0;
  ErrorCode rval = mb->get_connectivity(blue, blueConn, num_nodes);
  if (MB_SUCCESS != rval)
    return 1;
  nsBlue = num_nodes;
  rval = mb->get_coords(blueConn, num_nodes, &(blueCoords[0][0]));
  if (MB_SUCCESS != rval)
    return 1;

  area = 0.;
  nP = 0; // number of intersection points we are marking the boundary of blue!
  if (check_boxes_first) {
    setup_red_cell(red, nsRed); // we do not need area here
    // look at the boxes formed with vertices; if they are far away, return false early
    if (!GeomUtil::bounding_boxes_overlap(redCoords, nsRed, blueCoords, nsBlue,
        box_error))
      return 0; // no error, but no intersection, decide early to get out
  }
  if (dbg_1) {
    std::cout << "red " << mb->id_from_handle(red) << "\n";
    for (int j = 0; j < nsRed; j++) {
      std::cout << redCoords[j] << "\n";
    }
    std::cout << "blue " << mb->id_from_handle(blue) << "\n";
    for (int j = 0; j < nsBlue; j++) {
      std::cout << blueCoords[j] << "\n";
    }
    mb->list_entities(&red, 1);
    mb->list_entities(&blue, 1);
  }

  for (int j = 0; j < nsBlue; j++) {
    blueCoords2D[2 * j] = blueCoords[j][0]; // x coordinate,
    blueCoords2D[2 * j + 1] = blueCoords[j][1]; // y coordinate
  }
  if (dbg_1) {
    //std::cout << "gnomonic plane: " << plane << "\n";
    std::cout << " red \n";
    for (int j = 0; j < nsRed; j++) {
      std::cout << redCoords2D[2 * j] << " " << redCoords2D[2 * j + 1] << "\n ";
    }
    std::cout << " blue\n";
    for (int j = 0; j < nsBlue; j++) {
      std::cout << blueCoords2D[2 * j] << " " << blueCoords2D[2 * j + 1]
          << "\n";
    }
  }

  int ret = EdgeIntersections2(blueCoords2D, nsBlue, redCoords2D, nsRed, markb,
      markr, P, nP);
  if (ret != 0)
    return 1; // some unforeseen error
  if (dbg_1) {
    for (int k = 0; k < 3; k++) {
      std::cout << " markb, markr: " << k << " " << markb[k] << " " << markr[k]
          << "\n";
    }
  }

  int side[MAXEDGES] = { 0 }; // this refers to what side? blue or red?
  int extraPoints = borderPointsOfXinY2(blueCoords2D, nsBlue, redCoords2D,
      nsRed, &(P[2 * nP]), side, epsilon_area);
  if (extraPoints >= 1) {
    for (int k = 0; k < nsBlue; k++) {
      if (side[k]) {
        // this means that vertex k of blue is inside convex red; mark edges k-1 and k in blue,
        //   as being "intersected" by red; (even though they might not be intersected by other edges,
        //   the fact that their apex is inside, is good enough)
        markb[k] = 1;
        markb[(k + nsBlue - 1) % nsBlue] = 1; // it is the previous edge, actually, but instead of doing -1, it is
        // better to do modulo +3 (modulo 4)
        // null side b for next call
        side[k] = 0;
      }
    }
  }
  if (dbg_1) {
    for (int k = 0; k < 3; k++) {
      std::cout << " markb, markr: " << k << " " << markb[k] << " " << markr[k]
          << "\n";
    }
  }
  nP += extraPoints;

  extraPoints = borderPointsOfXinY2(redCoords2D, nsRed, blueCoords2D, nsBlue,
      &(P[2 * nP]), side, epsilon_area);
  if (extraPoints >= 1) {
    for (int k = 0; k < nsRed; k++) {
      if (side[k]) {
        // this is to mark that red edges k-1 and k are intersecting blue
        markr[k] = 1;
        markr[(k + nsRed - 1) % nsRed] = 1; // it is the previous edge, actually, but instead of doing -1, it is
        // better to do modulo +3 (modulo 4)
        // null side b for next call
      }
    }
  }
  if (dbg_1) {
    for (int k = 0; k < 3; k++) {
      std::cout << " markb, markr: " << k << " " << markb[k] << " " << markr[k]
          << "\n";
    }
  }
  nP += extraPoints;

  // now sort and orient the points in P, such that they are forming a convex polygon
  // this will be the foundation of our new mesh
  // this works if the polygons are convex
  SortAndRemoveDoubles2(P, nP, epsilon_1); // nP should be at most 8 in the end ?
  // if there are more than 3 points, some area will be positive

  if (nP >= 3) {
    for (int k = 1; k < nP - 1; k++)
      area += area2D(P, &P[2 * k], &P[2 * k + 2]);
  }

  return 0; // no error
}

// this method will also construct the triangles/polygons in the new mesh
// if we accept planar polygons, we just save them
// also, we could just create new vertices every time, and merge only in the end;
// could be too expensive, and the tolerance for merging could be an
// interesting topic
int Intx2MeshInPlane::findNodes(EntityHandle red, int nsRed, EntityHandle blue, int nsBlue,
    double * iP, int nP)
{
  // except for gnomonic projection, everything is the same as spherical intx
  // start copy
  // first of all, check against red and blue vertices
  //
  if (dbg_1)
  {
    std::cout << "red, blue, nP, P " << mb->id_from_handle(red) << " "
        << mb->id_from_handle(blue) << " " << nP << "\n";
    for (int n = 0; n < nP; n++)
      std::cout << " \t" << iP[2 * n] << "\t" << iP[2 * n + 1] << "\n";

  }

  // get the edges for the red triangle; the extra points will be on those edges, saved as
  // lists (unordered)
  std::vector<EntityHandle> redEdges(nsRed);//
  int i = 0;
  for (i = 0; i < nsRed; i++)
  {
    EntityHandle v[2] = { redConn[i], redConn[(i + 1) % nsRed] };
    std::vector<EntityHandle> adj_entities;
    ErrorCode rval = mb->get_adjacencies(v, 2, 1, false, adj_entities,
        Interface::INTERSECT);
    if (rval != MB_SUCCESS || adj_entities.size() < 1)
      return 0; // get out , big error
    redEdges[i] = adj_entities[0]; // should be only one edge between 2 nodes
  }
  // these will be in the new mesh, mbOut
  // some of them will be handles to the initial vertices from blue or red meshes (lagr or euler)

  EntityHandle * foundIds = new EntityHandle[nP];
  for (i = 0; i < nP; i++)
  {
    double * pp = &iP[2 * i]; // iP+2*i
    //
    CartVect pos(pp[0], pp[1], 0.);
    int found = 0;
    // first, are they on vertices from red or blue?
    // priority is the red mesh (mb2?)
    int j = 0;
    EntityHandle outNode = (EntityHandle) 0;
    for (j = 0; j < nsRed && !found; j++)
    {
      //int node = redTri.v[j];
      double d2 = dist2(pp, &redCoords2D[2 * j]);
      if (d2 < epsilon_1)
      {

        foundIds[i] = redConn[j]; // no new node
        found = 1;
        if (dbg_1)
          std::cout << "  red node j:" << j << " id:"
              << mb->id_from_handle(redConn[j]) << " 2d coords:" << redCoords2D[2 * j] << "  "
              << redCoords2D[2 * j + 1] << " d2: " << d2 << " \n";
      }
    }

    for (j = 0; j < nsBlue && !found; j++)
    {
      //int node = blueTri.v[j];
      double d2 = dist2(pp, &blueCoords2D[2 * j]);
      if (d2 < epsilon_1)
      {
        // suspect is blueConn[j] corresponding in mbOut

        foundIds[i] = blueConn[j]; // no new node
        found = 1;
        if (dbg_1)
          std::cout << "  blue node " << j << " "
              << mb->id_from_handle(blueConn[j]) << " d2:" << d2 << " \n";
      }

    }
    if (!found)
    {
      // find the edge it belongs, first, on the red element
      //
      for (j = 0; j < nsRed; j++)
      {
        int j1 = (j + 1) % nsRed;
        double area = area2D(&redCoords2D[2 * j], &redCoords2D[2 * j1], pp);
        if (dbg_1)
          std::cout << "   edge " << j << ": "
              << mb->id_from_handle(redEdges[j]) << " " << redConn[j] << " "
              << redConn[j1] << "  area : " << area << "\n";
        if (fabs(area) < epsilon_1/2)
        {
          // found the edge; now find if there is a point in the list here
          //std::vector<EntityHandle> * expts = extraNodesMap[redEdges[j]];
          int indx = -1;
          indx = RedEdges.index(redEdges[j]);
          std::vector<EntityHandle> * expts = extraNodesVec[indx];
          // if the points pp is between extra points, then just give that id
          // if not, create a new point, (check the id)
          // get the coordinates of the extra points so far
          int nbExtraNodesSoFar = expts->size();
          if (nbExtraNodesSoFar>0)
          {
            CartVect * coords1 = new CartVect[nbExtraNodesSoFar];
            mb->get_coords(&(*expts)[0], nbExtraNodesSoFar, &(coords1[0][0]));
            //std::list<int>::iterator it;
            for (int k = 0; k < nbExtraNodesSoFar && !found; k++)
            {
              //int pnt = *it;
              double d2 = (pos - coords1[k]).length_squared();
              if (d2 < epsilon_1)
              {
                found = 1;
                foundIds[i] = (*expts)[k];
                if (dbg_1)
                  std::cout << " found node:" << foundIds[i] << std::endl;
              }
            }
            delete[] coords1;
          }

          if (!found)
          {
            // create a new point in 2d (at the intersection)
            //foundIds[i] = m_num2dPoints;
            //expts.push_back(m_num2dPoints);
            // need to create a new node in mbOut
            // this will be on the edge, and it will be added to the local list
            mb->create_vertex(pos.array(), outNode);
            (*expts).push_back(outNode);
            foundIds[i] = outNode;
            found = 1;
            if (dbg_1)
              std::cout << " new node: " << outNode << std::endl;
          }

        }
      }
    }
    if (!found)
    {
      std::cout << " red polygon: ";
      for (int j1 = 0; j1 < nsRed; j1++)
      {
        std::cout << redCoords2D[2 * j1] << " " << redCoords2D[2 * j1 + 1] << "\n";
      }
      std::cout << " a point pp is not on a red polygon " << *pp << " " << pp[1]
          << " red polygon " << mb->id_from_handle(red) << " \n";
      delete[] foundIds;
      return 1;
    }
  }
  if (dbg_1)
  {
    std::cout << " candidate polygon: nP " << nP << "\n";
    for (int i1 = 0; i1 < nP; i1++)
            std::cout << iP[2 * i1] << " " << iP[2 * i1 + 1] << " " << foundIds[i1] << "\n";
  }
  // first, find out if we have nodes collapsed; shrink them
  // we may have to reduce nP
  // it is possible that some nodes are collapsed after intersection only
  // nodes will always be in order (convex intersection)
  correct_polygon(foundIds, nP);
  // now we can build the triangles, from P array, with foundIds
  // we will put them in the out set
  if (nP >= 3)
  {
    EntityHandle polyNew;
    mb->create_element(MBPOLYGON, foundIds, nP, polyNew);
    mb->add_entities(outSet, &polyNew, 1);

    // tag it with the index ids from red and blue sets
    int id = rs1.index(blue); // index starts from 0
    mb->tag_set_data(blueParentTag, &polyNew, 1, &id);
    id = rs2.index(red);
    mb->tag_set_data(redParentTag, &polyNew, 1, &id);

    counting++;
    mb->tag_set_data(countTag, &polyNew, 1, &counting);

    if (dbg_1)
    {

      std::cout << "Count: " << counting+1 << "\n";
      std::cout << " polygon " << mb->id_from_handle(polyNew) << "  nodes: " << nP << " :";
      for (int i1 = 0; i1 < nP; i1++)
        std::cout << " " << mb->id_from_handle(foundIds[i1]);
      std::cout << "\n";
      std::vector<CartVect> posi(nP);
      mb->get_coords(foundIds, nP, &(posi[0][0]));
      for (int i1 = 0; i1 < nP; i1++)
        std::cout << iP[2 * i1] << " " << iP[2 * i1 + 1] << " " << posi[i1] << "\n";

      std::stringstream fff;
      fff << "file0" <<  counting<< ".vtk";
          mb->write_mesh(fff.str().c_str(), &outSet, 1);
    }

  }
  delete[] foundIds;
  foundIds = NULL;
  return 0;
  // end copy
}
bool Intx2MeshInPlane::is_inside_element(double xyz[3], EntityHandle eh)
{
  int num_nodes;
  ErrorCode rval = mb->get_connectivity(eh, redConn, num_nodes);

  if (MB_SUCCESS != rval)
    return false;
  int nsides = num_nodes;

  //CartVect coords[4];
  rval = mb->get_coords(redConn, num_nodes, &(redCoords[0][0]));
  if (MB_SUCCESS != rval)
    return 1;

  for (int j = 0; j < nsides; j++)
  {
    // populate coords in the plane for decision making
    // they should be oriented correctly, positively
    redCoords2D[2 * j]     = redCoords[j][0];
    redCoords2D[2 * j + 1] = redCoords[j][1];
  }

  double pt[2]={xyz[0], xyz[1]};// xy plane only
  // now, is the projected point inside the red quad?
  // cslam utils
  if (point_in_interior_of_convex_polygon (redCoords2D, num_nodes, pt))
    return true;
  return false;
}
} // end namespace moab
