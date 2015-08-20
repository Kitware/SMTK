/*
 * Intx2MeshInPlane.hpp
 *
 *  Created on: Oct 24, 2012
 *      Author: iulian
 */

#ifndef INTX2MESHINPLANE_HPP_
#define INTX2MESHINPLANE_HPP_

#include "Intx2Mesh.hpp"
namespace moab {

class Intx2MeshInPlane: public moab::Intx2Mesh {
public:
  Intx2MeshInPlane(Interface * mbimpl);
  virtual ~Intx2MeshInPlane();

  double setup_red_cell(EntityHandle red, int & nsRed);

  int computeIntersectionBetweenRedAndBlue(EntityHandle red, EntityHandle blue,
      double * P, int & nP, double & area, int markb[MAXEDGES], int markr[MAXEDGES],
      int & nsBlue, int & nsRed, bool check_boxes_first=false);

  int findNodes(EntityHandle red, int nsRed, EntityHandle blue, int nsBlue, double * iP, int nP);

  bool is_inside_element(double xyz[3], EntityHandle eh);
};
} // end namespace moab
#endif /* INTX2MESHINPLANE_HPP_ */
