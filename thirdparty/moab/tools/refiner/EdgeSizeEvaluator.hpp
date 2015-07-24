/*
 * MOAB, a Mesh-Oriented datABase, is a software component for creating,
 * storing and accessing finite element mesh data.
 * 
 * Copyright 2004 Sandia Corporation.  Under the terms of Contract
 * DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government
 * retains certain rights in this software.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 */

/**\class moab::EdgeSizeEvaluator
  *
  * This is an abstract class that embodies the rule used during edge-based mesh
  * refinement to decide whether an edge should be subdivided or not.
  * Subclasses must implement the pure virtual evaluate_edge() function.
  *
  * \author David Thompson
  *
  * \date 19 November 2007
  */
#ifndef MOAB_EDGE_SIZE_EVALUATOR_HPP
#define MOAB_EDGE_SIZE_EVALUATOR_HPP

#include "RefinerTagManager.hpp"

namespace moab {

class EdgeSizeEvaluator
{
public:
  EdgeSizeEvaluator();
  virtual ~EdgeSizeEvaluator();

  virtual bool evaluate_edge(
    const double* p0, const void* t0,
    double* p1, void* t1,
    const double* p2, const void* t2 ) = 0;

  void set_tag_manager( RefinerTagManager* tmgr ) { this->tag_manager = tmgr; }
  RefinerTagManager* get_tag_manager() { return this->tag_manager; }

protected:
  RefinerTagManager* tag_manager;
};

}

#endif // MOAB_EDGE_SIZE_EVALUATOR_HPP
