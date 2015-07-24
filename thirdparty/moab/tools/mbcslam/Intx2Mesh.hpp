/*
 * Intx2Mesh.hpp
 *
 *  Created on: Oct 2, 2012
 *
 */

#ifndef INTX2MESH_HPP_
#define INTX2MESH_HPP_

#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "moab/Core.hpp"
#include "moab/Interface.hpp"
#include "moab/Range.hpp"
#include "moab/CartVect.hpp"

// these are intersection utils
#include "CslamUtils.hpp"

namespace moab {

#define ERRORR(rval, str) \
    if (MB_SUCCESS != rval) {std::cout << str << "\n"; return rval;}

#define ERRORV(rval, str) \
    if (MB_SUCCESS != rval) {std::cout << str << "\n"; return ;}

#ifdef MOAB_HAVE_MPI
// forward declarations
class ParallelComm;
class TupleList;
#endif

class Intx2Mesh
{
public:
  Intx2Mesh(Interface * mbimpl);
  virtual ~Intx2Mesh();

  ErrorCode intersect_meshes(EntityHandle mbs1, EntityHandle mbs2,
       EntityHandle & outputSet);

  // mark could be (3 or 4, depending on type: ) no, it could go to 10
  // no, it will be MAXEDGES = 10
  // this is pure abstract, this needs to be implemented by
  // all derivations
  // the max number of intersection points could be 2*MAXEDGES
  // so P must be dimensioned to 4*MAXEDGES (2*2*MAXEDGES)
  // so, if you intersect 2 convex polygons with MAXEDGES , you will get a convex polygon
  // with 2*MAXEDGES, at most
  // will also return the number of nodes of red and blue elements
  virtual int computeIntersectionBetweenRedAndBlue(EntityHandle red,
      EntityHandle blue, double * P, int & nP, double & area,
      int markb[MAXEDGES], int markr[MAXEDGES], int & nsidesBlue,
      int & nsidesRed, bool check_boxes_first=false)=0;

  // this is also abstract
  virtual int findNodes(EntityHandle red, int nsRed, EntityHandle blue, int nsBlue,
      double * iP, int nP)=0;

  // this is also computing the area of the red cell in plane (gnomonic plane for sphere)
  // setting the local variables:
  // this will be done once per red cell, and once per localQueue for blue cells
  //  const EntityHandle * redConn;
  // CartVect redCoords[MAXEDGES];
  // double redCoords2D[MAXEDGES2]; // these are in plane

  virtual double setup_red_cell(EntityHandle red, int & nsRed)= 0;

  virtual void createTags();
  ErrorCode GetOrderedNeighbors(EntityHandle set, EntityHandle quad,
      EntityHandle neighbors[MAXEDGES]);

  void SetErrorTolerance(double eps) { epsilon_1=eps; epsilon_area = eps*eps/2;}

  //void SetEntityType (EntityType tp) { type=tp;}

  // clean some memory allocated
  void clean();

  // this will depend on the problem and element type; return true if on the border edge too
  virtual bool is_inside_element(double xyz[3], EntityHandle eh) = 0;
  void set_box_error(double berror)
   {box_error = berror;}

  ErrorCode create_departure_mesh_2nd_alg(EntityHandle & euler_set, EntityHandle & covering_lagr_set);

  // in this method, used in parallel, each departure elements are already created, and at their positions
  // the covering_set is output, will contain the departure cells that cover the euler set; some of these
  // departure cells might come from different processors
  // so the covering_set contains some elements from lagr_set and some elements that come from other procs
  // we need to keep track of what processors "sent" the elements so we know were to
  // send back the info about the tracers masses

  ErrorCode create_departure_mesh_3rd_alg(EntityHandle & lagr_set, EntityHandle & covering_set);

  ErrorCode build_processor_euler_boxes(EntityHandle euler_set, Range & local_verts);

  void correct_polygon(EntityHandle * foundIds, int & nP);
#ifdef MOAB_HAVE_MPI
  ErrorCode correct_intersection_points_positions();
#endif
  void enable_debug()  {dbg_1 = 1;}
  void disable_debug() {dbg_1 = 0;}
protected: // so it can be accessed in derived classes, InPlane and OnSphere
  Interface * mb;

  EntityHandle mbs1;
  EntityHandle mbs2;
  Range rs1;// range set 1 (departure set, lagrange set, blue set, manufactured set)
  Range rs2;// range set 2 (arrival set, euler set, red set, initial set)

  EntityHandle outSet; // will contain intersection

  // tags used in computation, advancing front
  Tag RedFlagTag; // to mark red quads already considered

  Range RedEdges; //

  // red parent and blue parent tags
  // these will be on the out mesh
  Tag redParentTag;
  Tag blueParentTag;
  Tag countTag;

  //EntityType type; // this will be tri, quad or MBPOLYGON...

  const EntityHandle * redConn;
  const EntityHandle * blueConn;
  CartVect redCoords[MAXEDGES];
  CartVect blueCoords[MAXEDGES];
  double redCoords2D[MAXEDGES2]; // these are in plane
  double blueCoords2D[MAXEDGES2]; // these are in plane

  std::ofstream mout_1[6]; // some debug files
  int dbg_1;
  // for each red edge, we keep a vector of extra nodes, coming from intersections
  // use the index in RedEdges range, instead of a map, as before
  // std::map<EntityHandle, std::vector<EntityHandle> *> extraNodesMap;
  // so the extra nodes on each red edge are kept track of
  std::vector<std::vector<EntityHandle> *> extraNodesVec;

  double epsilon_1;
  double epsilon_area;

  std::vector<double> allBoxes;
  double box_error;
  /* \brief Local root of the kdtree */
  EntityHandle localRoot;
  Range localEnts;// this range is for local elements of interest, euler cells
  unsigned int my_rank;

#ifdef MOAB_HAVE_MPI
  ParallelComm * parcomm;
  TupleList * remote_cells; // not used anymore for communication, just a container
  TupleList * remote_cells_with_tracers; // these will be used now to update tracers on remote procs
  std::map<int, EntityHandle> globalID_to_eh;// needed for parallel, mostly
#endif
  int max_edges; // maximum number of edges in the euler set
  int counting;



};

} /* namespace moab */
#endif /* INTX2MESH_HPP_ */
