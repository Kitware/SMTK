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

/**\class moab::EdgeSizeSimpleImplicit
  *
  * This is an simple example edge evaluator tha subdivides edges based
  * on their midpoint's distance to a simple, fixed-form implicit surface
  * written as \f$ x^T A x + B x + C \f$ where \f$x\f$ is a column vector of
  * holding the edge midpoint coordinates, \f$A\f$ is a symmetric 3x3 matrix,
  * \f$B\f$ is a 1x3 row vector, and \f$C\f$ is a scalar.
  * Whenever the implicit function divided by half of the edge length is smaller than
  * some minimum ratio (which defaults to 1), the edge is marked for subdivision.
  *
  * \author David Thompson
  *
  * \date 19 November 2007
  */
#ifndef MOAB_EDGE_SIZE_SIMPLE_IMPLICIT_HPP
#define MOAB_EDGE_SIZE_SIMPLE_IMPLICIT_HPP

#include "moabrefiner_export.h"

#include "EdgeSizeEvaluator.hpp"

namespace moab {

class MOABREFINER_EXPORT EdgeSizeSimpleImplicit : public EdgeSizeEvaluator
{
public:
  /// Construct an evaluator.
  EdgeSizeSimpleImplicit();
  /// Destruction is virtual so subclasses may clean up after refinement.
  virtual ~EdgeSizeSimpleImplicit();

  /** \brief Given an edge of length L, true when edge midpoint is within $\alpha^2$ of $\left(\frac{2f(x,y,z)}{L}\right)^2$.
    */
  virtual bool evaluate_edge(
    const double* p0, const void* t0,
    double* p1, void* t1,
    const double* p2, const void* t2 );

  /// Set the 10 coefficients of the implicit function. The vector contains the entries of A, followed by B, followed by C.
  virtual void set_implicit_function( double* coeffs );
  /// Get the 10 coefficients of the implicit function. The vector contains the entries of A, followed by B, followed by C.
  void get_implicit_function( double*& coeffs );

  /// Set the threshold ratio of function value to half-edge length that triggers subdivision.
  virtual void set_ratio( double r ) { this->ratio = r; }
  /// Get the threshold ratio of function value to half-edge length that triggers subdivision.
  double get_ratio() { return this->ratio; }

protected:
  double coeffA[6];
  double coeffB[3];
  double coeffC;
  double ratio;
};

} // namespace moab 

#endif // MOAB_EDGE_SIZE_SIMPLE_IMPLICIT_HPP
