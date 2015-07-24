/*
 * Intx2MeshOnSphere.hpp
 *
 *  Created on: Oct 3, 2012
 *      Author: iulian
 */

#ifndef INTX2MESHONSPHERE_HPP_
#define INTX2MESHONSPHERE_HPP_

#include "Intx2Mesh.hpp"

namespace moab {

class Intx2MeshOnSphere: public moab::Intx2Mesh
{
public:
  Intx2MeshOnSphere(Interface * mbimpl);
  virtual ~Intx2MeshOnSphere();

  void SetRadius(double radius) { R=radius ;}

  double setup_red_cell(EntityHandle red, int & nsRed);

  // main method to intersect meshes on a sphere

  int computeIntersectionBetweenRedAndBlue(EntityHandle red, EntityHandle blue,
          double * P, int & nP, double & area, int markb[MAXEDGES], int markr[MAXEDGES],
          int & nsBlue, int & nsRed, bool check_boxes_first=false);

  int findNodes(EntityHandle red, int nsRed, EntityHandle blue, int nsBlue,
      double * iP, int nP);

  bool is_inside_element(double xyz[3], EntityHandle eh);

  ErrorCode update_tracer_data(EntityHandle out_set, Tag & tagElem, Tag & tagArea);

private:
  int plane; // current gnomonic plane
  double R; // radius of the sphere


};

} /* namespace moab */
#endif /* INTX2MESHONSPHERE_HPP_ */
