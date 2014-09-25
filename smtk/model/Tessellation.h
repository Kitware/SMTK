//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_Tessellation_h
#define __smtk_model_Tessellation_h

#include "smtk/SystemConfig.h"
#include "smtk/common/UUID.h"

#include "sparsehash/sparse_hash_map"

#include <vector>

namespace smtk {
  namespace model {

/**\brief Store geometric information related to model entities.
  *
  * This is currently used to store coordinates and connectivity of
  * a triangulation of the model entity for rendering.
  * However, it may also evolve to store information about the
  * underlying geometric construct being approximated.
  */
class SMTKCORE_EXPORT Tessellation
{
public:
  Tessellation();

  /// Direct access to the underlying point-coordinate storage
  std::vector<double>& coords()
    { return this->m_coords; }
  std::vector<double> const& coords() const
    { return this->m_coords; }

  /// Direct access to the underlying connectivity storage
  std::vector<int>& conn()
    { return this->m_conn; }
  std::vector<int> const& conn() const
    { return this->m_conn; }

  int addCoords(double* a);
  Tessellation& addCoords(double x, double y, double z);

  Tessellation& addPoint(double* a);
  Tessellation& addLine(double* a, double* b);
  Tessellation& addTriangle(double* a, double* b, double* c);

  Tessellation& addPoint(int ai);
  Tessellation& addLine(int ai, int bi);
  Tessellation& addTriangle(int ai, int bi, int ci);

  Tessellation& reset();

protected:
  std::vector<double> m_coords;
  std::vector<int> m_conn;
};

typedef google::sparse_hash_map<smtk::common::UUID,Tessellation> UUIDsToTessellations;
typedef google::sparse_hash_map<smtk::common::UUID,Tessellation>::iterator UUIDWithTessellation;

  } // model namespace
} // smtk namespace

#endif // __smtk_model_Tessellation_h
