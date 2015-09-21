/*
 * Intx2MeshOnSphere.cpp
 *
 *  Created on: Oct 3, 2012
 */

#include "Intx2MeshOnSphere.hpp"
#include "moab/GeomUtil.hpp"
#include "MBTagConventions.hpp"
#ifdef MOAB_HAVE_MPI
#include "moab/ParallelComm.hpp"
#endif

namespace moab {


Intx2MeshOnSphere::Intx2MeshOnSphere(Interface * mbimpl):Intx2Mesh(mbimpl)
{
  // TODO Auto-generated constructor stub

}

Intx2MeshOnSphere::~Intx2MeshOnSphere()
{
  // TODO Auto-generated destructor stub
}

/*
 * return also the area for robustness verification
 */
double Intx2MeshOnSphere::setup_red_cell(EntityHandle red, int & nsRed){


  // get coordinates of the red quad, to decide the gnomonic plane
  double cellArea =0;

  int num_nodes;
  ErrorCode rval = mb->get_connectivity(red, redConn, num_nodes);

  if (MB_SUCCESS != rval )
    return 1;
  nsRed = num_nodes;
  // account for possible padded polygons
  while (redConn[nsRed-2]==redConn[nsRed-1] && nsRed>3)
    nsRed--;

  //CartVect coords[4];
  rval = mb->get_coords(redConn, nsRed, &(redCoords[0][0]));
  if (MB_SUCCESS != rval)
    return 1;
  CartVect middle = redCoords[0];
  for (int i=1; i<nsRed; i++)
    middle += redCoords[i];
  middle = 1./nsRed * middle;

  decide_gnomonic_plane(middle, plane);// output the plane
  for (int j = 0; j < nsRed; j++)
  {
    // populate coords in the plane for intersection
    // they should be oriented correctly, positively
    int rc = gnomonic_projection(redCoords[j],  R, plane, redCoords2D[2 * j],
        redCoords2D[2 * j + 1]);
    if (rc != 0)
      return 1;
  }

  for (int j=1; j<nsRed-1; j++)
    cellArea += area2D(&redCoords2D[0], &redCoords2D[2*j], &redCoords2D[2*j+2]);

  // take red coords in order and compute area in plane
  return cellArea;
}

/* the elements are convex for sure, then do a gnomonic projection of both,
 *  compute intersection in the plane, then go back to the sphere for the points
 *  */
int Intx2MeshOnSphere::computeIntersectionBetweenRedAndBlue(EntityHandle red, EntityHandle blue,
    double * P, int & nP, double & area, int markb[MAXEDGES], int markr[MAXEDGES],
    int & nsBlue, int & nsRed, bool check_boxes_first)
{
  // the area will be used from now on, to see how well we fill the red cell with polygons
  // the points will be at most 40; they will describe a convex patch, after the points will be ordered and
  // collapsed (eliminate doubles)

  //CartVect bluecoords[4];
  int num_nodes=0;
  ErrorCode rval = mb->get_connectivity(blue, blueConn, num_nodes);
  if (MB_SUCCESS != rval )
    return 1;
  nsBlue = num_nodes;
  // account for possible padded polygons
  while (blueConn[nsBlue-2]==blueConn[nsBlue-1] && nsBlue>3)
    nsBlue--;
  rval = mb->get_coords(blueConn, nsBlue, &(blueCoords[0][0]));
  if (MB_SUCCESS != rval)
    return 1;

  area = 0.;
  nP = 0; // number of intersection points we are marking the boundary of blue!
  if (check_boxes_first)
  {
    // look at the boxes formed with vertices; if they are far away, return false early
    // make sure the red is setup already
    setup_red_cell(red, nsRed); // we do not need area here
    if (!GeomUtil::bounding_boxes_overlap(redCoords, nsRed, blueCoords, nsBlue, box_error))
      return 0; // no error, but no intersection, decide early to get out
  }
  if (dbg_1)
  {
    std::cout << "red " << mb->id_from_handle(red) << "\n";
    for (int j = 0; j < nsRed; j++)
    {
      std::cout << redCoords[j] << "\n";
    }
    std::cout << "blue " << mb->id_from_handle(blue) << "\n";
    for (int j = 0; j < nsBlue; j++)
    {
      std::cout << blueCoords[j] << "\n";
    }
    mb->list_entities(&red, 1);
    mb->list_entities(&blue, 1);
  }

  for (int j=0; j<nsBlue; j++)
  {
    int rc = gnomonic_projection(blueCoords[j], R, plane, blueCoords2D[2 * j],
        blueCoords2D[2 * j + 1]);
    if (rc != 0)
      return 1;
  }
  if (dbg_1)
  {
    std::cout << "gnomonic plane: " << plane << "\n";
    std::cout << " red                                blue\n";
    for (int j = 0; j < nsRed; j++)
    {
      std::cout << redCoords2D[2 * j] << " " << redCoords2D[2 * j + 1] << "\n";
    }
    for (int j = 0; j < nsBlue; j++)
    {
      std::cout << blueCoords2D[2 * j] << " " << blueCoords2D[2 * j + 1] << "\n";
    }
  }

  int ret = EdgeIntersections2(blueCoords2D, nsBlue, redCoords2D, nsRed, markb, markr, P, nP);
  if (ret != 0)
    return 1; // some unforeseen error

  int side[MAXEDGES] = { 0 };// this refers to what side? blue or red?
  int extraPoints = borderPointsOfXinY2(blueCoords2D, nsBlue, redCoords2D, nsRed, &(P[2 * nP]), side, epsilon_area);
  if (extraPoints >= 1)
  {
    for (int k = 0; k < nsBlue; k++)
    {
      if (side[k])
      {
        // this means that vertex k of blue is inside convex red; mark edges k-1 and k in blue,
        //   as being "intersected" by red; (even though they might not be intersected by other edges,
        //   the fact that their apex is inside, is good enough)
        markb[k] = 1;
        markb[(k + nsBlue-1) % nsBlue] = 1; // it is the previous edge, actually, but instead of doing -1, it is
        // better to do modulo +3 (modulo 4)
        // null side b for next call
        side[k]=0;
      }
    }
  }
  nP += extraPoints;

  extraPoints = borderPointsOfXinY2(redCoords2D, nsRed, blueCoords2D, nsBlue, &(P[2 * nP]), side, epsilon_area);
  if (extraPoints >= 1)
  {
    for (int k = 0; k < nsRed; k++)
    {
      if (side[k])
      {
        // this is to mark that red edges k-1 and k are intersecting blue
        markr[k] = 1;
        markr[(k + nsRed-1) % nsRed] = 1; // it is the previous edge, actually, but instead of doing -1, it is
        // better to do modulo +3 (modulo 4)
        // null side b for next call
      }
    }
  }
  nP += extraPoints;

  // now sort and orient the points in P, such that they are forming a convex polygon
  // this will be the foundation of our new mesh
  // this works if the polygons are convex
  SortAndRemoveDoubles2(P, nP, epsilon_1); // nP should be at most 8 in the end ?
  // if there are more than 3 points, some area will be positive

  if (nP >= 3)
  {
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
int Intx2MeshOnSphere::findNodes(EntityHandle red, int nsRed, EntityHandle blue, int nsBlue,
    double * iP, int nP)
{
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
    EntityHandle v[2] = { redConn[i], redConn[(i + 1) % nsRed] };// this is fine even for padded polygons
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
    // project the point back on the sphere
    CartVect pos;
    reverse_gnomonic_projection(pp[0], pp[1], R, plane, pos);
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
      std::cout << " red quad: ";
      for (int j1 = 0; j1 < nsRed; j1++)
      {
        std::cout << redCoords2D[2 * j1] << " " << redCoords2D[2 * j1 + 1] << "\n";
      }
      std::cout << " a point pp is not on a red quad " << *pp << " " << pp[1]
          << " red quad " << mb->id_from_handle(red) << " \n";
      delete[] foundIds;
      return 1;
    }
  }
  if (dbg_1)
  {
    std::cout << " candidate polygon: nP" << nP <<  " plane: " << plane << "\n";
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

      std::cout << "Counting: " << counting << "\n";
      std::cout << " polygon " << mb->id_from_handle(polyNew) << "  nodes: " << nP << " :";
      for (int i1 = 0; i1 < nP; i1++)
        std::cout << " " << mb->id_from_handle(foundIds[i1]);
      std::cout << " plane: " << plane << "\n";
      std::vector<CartVect> posi(nP);
      mb->get_coords(foundIds, nP, &(posi[0][0]));
      for (int i1 = 0; i1 < nP; i1++)
        std::cout << foundIds[i1]<< " " << posi[i1] << "\n";

      std::stringstream fff;
      fff << "file0" <<  counting<< ".vtk";
         mb->write_mesh(fff.str().c_str(), &outSet, 1);
    }

  }
  //disable_debug();
  delete[] foundIds;
  foundIds = NULL;
  return 0;
}
bool Intx2MeshOnSphere::is_inside_element(double xyz[3], EntityHandle eh)
{
  int num_nodes;
  ErrorCode rval = mb->get_connectivity(eh, redConn, num_nodes);

  if (MB_SUCCESS != rval)
    return false;
  int nsRed = num_nodes;

  //CartVect coords[4];
  rval = mb->get_coords(redConn, num_nodes, &(redCoords[0][0]));
  if (MB_SUCCESS != rval)
    return 1;
  CartVect center(0.,0.,0.);
  for (int k=0; k<num_nodes; k++)
      center += redCoords[k];
  center = 1./num_nodes*center;
  decide_gnomonic_plane(center, plane);// output the plane
  for (int j = 0; j < nsRed; j++)
  {
    // populate coords in the plane for decision making
    // they should be oriented correctly, positively
    int rc = gnomonic_projection(redCoords[j],  R, plane, redCoords2D[2 * j],
        redCoords2D[2 * j + 1]);
    if (rc != 0)
      return false;
  }
  double pt[2];
  CartVect pos(xyz);
  int rc=gnomonic_projection(pos, R, plane, pt[0], pt[1]);
  if (rc != 0)
    return false;

  // now, is the projected point inside the red quad?
  // cslam utils
  if (point_in_interior_of_convex_polygon (redCoords2D, nsRed, pt))
    return true;
  return false;
}

ErrorCode Intx2MeshOnSphere::update_tracer_data(EntityHandle out_set, Tag & tagElem, Tag & tagArea)
{
  EntityHandle dum = 0;

  Tag corrTag;
  ErrorCode rval = mb->tag_get_handle(CORRTAGNAME,
                                           1, MB_TYPE_HANDLE, corrTag,
                                           MB_TAG_DENSE, &dum); // it should have been created
  ERRORR(rval, "can't get correlation tag");

  Tag gid;
  rval = mb->tag_get_handle(GLOBAL_ID_TAG_NAME, 1, MB_TYPE_INTEGER, gid, MB_TAG_DENSE);
  ERRORR(rval,"can't get global ID tag" );

  // get all polygons out of out_set; then see where are they coming from
  Range polys;
  rval = mb->get_entities_by_dimension(out_set, 2, polys);
  ERRORR(rval, "can't get polygons out");

  // rs2 is the red range, arrival; rs1 is blue, departure;
  // there is a connection between rs1 and rs2, through the corrTag
  // corrTag is __correlation
  // basically, mb->tag_get_data(corrTag, &(redPoly), 1, &bluePoly);
  // also,  mb->tag_get_data(corrTag, &(bluePoly), 1, &redPoly);
  // we start from rs2 existing, then we have to update something

  // tagElem will have multiple tracers
  int numTracers = 0;
  rval = mb->tag_get_length(tagElem, numTracers);
  ERRORR(rval, "can't get number of tracers in simulation");
  if (numTracers < 1)
    ERRORR(MB_FAILURE, "no tracers data");
  std::vector<double>  currentVals(rs2.size()*numTracers);
  rval = mb->tag_get_data(tagElem, rs2, &currentVals[0]);
  ERRORR(rval, "can't get existing tracers values");

  // create new tuple list for tracers to other processors, from remote_cells
#ifdef MOAB_HAVE_MPI
  if (remote_cells)
  {
    int n = remote_cells->get_n();
    if (n>0) {
      remote_cells_with_tracers = new TupleList();
      remote_cells_with_tracers->initialize(2, 0, 1, numTracers, n); // tracers are in these tuples
      remote_cells_with_tracers->enableWriteAccess();
      for (int i=0; i<n; i++)
      {
        remote_cells_with_tracers->vi_wr[2*i]=remote_cells->vi_wr[2*i];
        remote_cells_with_tracers->vi_wr[2*i+1]=remote_cells->vi_wr[2*i+1];
        //    remote_cells->vr_wr[i] = 0.; will have a different tuple for communication
        remote_cells_with_tracers->vul_wr[i]=   remote_cells->vul_wr[i];// this is the corresponding red cell (arrival)
        for (int k=0; k<numTracers; k++)
          remote_cells_with_tracers->vr_wr[numTracers*i+k] = 0; // initialize tracers to be transported
        remote_cells_with_tracers->inc_n();
      }
    }
    delete remote_cells;
    remote_cells = NULL;
  }
#endif
  // for each polygon, we have 2 indices: red and blue parents
  // we need index blue to update index red?
  std::vector<double> newValues(rs2.size()*numTracers, 0.);// initialize with 0 all of them
  // area of the polygon * conc on red (old) current quantity
  // finaly, divide by the area of the red
  double check_intx_area=0.;
  for (Range::iterator it= polys.begin(); it!=polys.end(); it++)
  {
    EntityHandle poly=*it;
    int blueIndex, redIndex;
    rval =  mb->tag_get_data(blueParentTag, &poly, 1, &blueIndex);
    ERRORR(rval, "can't get blue tag");
    EntityHandle blue = rs1[blueIndex];
    rval =  mb->tag_get_data(redParentTag, &poly, 1, &redIndex);
    ERRORR(rval, "can't get red tag");
    //EntityHandle red = rs2[redIndex];
    // big assumption here, red and blue are "parallel" ;we should have an index from
    // blue to red (so a deformed blue corresponds to an arrival red)
    double areap = area_spherical_element(mb, poly, R);
    check_intx_area+=areap;
    // so the departure cell at time t (blueIndex) covers a portion of a redCell
    // that quantity will be transported to the redCell at time t+dt
    // the blue corresponds to a red arrival
    EntityHandle redArr;
    rval = mb->tag_get_data(corrTag, &blue, 1, &redArr);
    if (0==redArr || MB_TAG_NOT_FOUND==rval)
    {
#ifdef MOAB_HAVE_MPI
      if (!remote_cells_with_tracers)
        ERRORR( MB_FAILURE, "no remote cells, failure\n");
      // maybe the element is remote, from another processor
      int global_id_blue;
      rval = mb->tag_get_data(gid, &blue, 1, &global_id_blue);
      ERRORR(rval, "can't get arrival red for corresponding blue gid");
      // find the
      int index_in_remote = remote_cells_with_tracers->find(1, global_id_blue);
      if (index_in_remote==-1)
        ERRORR( MB_FAILURE, "can't find the global id element in remote cells\n");
      for (int k=0; k<numTracers; k++)
        remote_cells_with_tracers->vr_wr[index_in_remote*numTracers+k] +=
            currentVals[numTracers*redIndex+k]*areap;
#endif
    }
    else if (MB_SUCCESS==rval)
    {
      int arrRedIndex = rs2.index(redArr);
      if (-1 == arrRedIndex)
        ERRORR(MB_FAILURE, "can't find the red arrival index");
      for (int k=0; k<numTracers; k++)
        newValues[numTracers*arrRedIndex+k] += currentVals[redIndex*numTracers+k]*areap;
    }

    else
      ERRORR(rval, "can't get arrival red for corresponding ");
  }
  // now, send back the remote_cells_with_tracers to the processors they came from, with the updated values for
  // the tracer mass in a cell
#ifdef MOAB_HAVE_MPI
  if (remote_cells_with_tracers)
  {
    // so this means that some cells will be sent back with tracer info to the procs they were sent from
    (parcomm->proc_config().crystal_router())->gs_transfer(1, *remote_cells_with_tracers, 0);
    // now, look at the global id, find the proper "red" cell with that index and update its mass
    //remote_cells->print("remote cells after routing");
    int n = remote_cells_with_tracers->get_n();
    for (int j=0; j<n; j++)
    {
      EntityHandle redCell = remote_cells_with_tracers->vul_rd[j];// entity handle sent back
      int arrRedIndex = rs2.index(redCell);
      if (-1 == arrRedIndex)
        ERRORR(MB_FAILURE, "can't find the red arrival index");
      for (int k=0; k<numTracers; k++)
        newValues[arrRedIndex*numTracers+k] += remote_cells_with_tracers->vr_rd[j*numTracers+k];
    }
  }
#endif /* MOAB_HAVE_MPI */
  // now divide by red area (current)
  int j=0;
  Range::iterator iter = rs2.begin();
  void * data=NULL; //used for stored area
  int count =0;
  std::vector<double> total_mass_local(numTracers, 0.);
  while (iter != rs2.end())
  {
    rval = mb->tag_iterate(tagArea, iter, rs2.end(), count, data);
    ERRORR(rval, "can't tag iterate");
    double * ptrArea=(double*)data;
    for (int i=0; i<count; i++, iter++, j++, ptrArea++)
    {
      for (int k=0; k<numTracers; k++)
      {
        total_mass_local[k]+=newValues[j*numTracers+k];
        newValues[j*numTracers+k]/= (*ptrArea);
      }
    }
  }
  rval = mb->tag_set_data(tagElem, rs2, &newValues[0]);
  ERRORR(rval, "can't set new values tag");


#ifdef MOAB_HAVE_MPI
  std::vector<double> total_mass(numTracers,0.);
  double total_intx_area =0;
  int mpi_err = MPI_Reduce(&total_mass_local[0], &total_mass[0], numTracers, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  if (MPI_SUCCESS != mpi_err) return MB_FAILURE;
  // now reduce total area
  mpi_err = MPI_Reduce(&check_intx_area, &total_intx_area, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  if (MPI_SUCCESS != mpi_err) return MB_FAILURE;
  if (my_rank==0)
  {
    for (int k=0; k<numTracers; k++)
      std::cout <<"total mass now tracer k=" << k+1<<" "  << total_mass[k] << "\n";
    std::cout <<"check: total intersection area: (4 * M_PI * R^2): " << 4 * M_PI * R*R << " " << total_intx_area << "\n";
  }

  if (remote_cells_with_tracers)
  {
    delete remote_cells_with_tracers;
    remote_cells_with_tracers=NULL;
  }
#else
  for (int k=0; k<numTracers; k++)
        std::cout <<"total mass now tracer k=" << k+1<<" "  << total_mass_local[k] << "\n";
  std::cout <<"check: total intersection area: (4 * M_PI * R^2): " << 4 * M_PI * R*R << " " << check_intx_area << "\n";
#endif
  return MB_SUCCESS;
}
} /* namespace moab */
