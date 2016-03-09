//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_io_ModelToMesh_h
#define __smtk_io_ModelToMesh_h

#include "smtk/CoreExports.h" // For SMTKCORE_EXPORT macro.
#include "smtk/PublicPointerDefs.h"

namespace smtk {
  namespace model
  {
    class Model;
  }

namespace io {
class SMTKCORE_EXPORT ModelToMesh
{
public:
  //By default will merge duplicate elements, and will use the default tolerance
  //specified by mergeCoincidentContactPoints
  ModelToMesh();

  //By default will merge duplicate elements
  bool isMergingEnabled() const { return this->m_mergeDuplicates; }
  void setIsMerging(bool m) { this->m_mergeDuplicates = m; }

  //Allow you to specify a custom tolerance for the merging of duplicate elements
  double getMergeTolerance() const { return this->m_tolerance; }
  void setMergeTolerance(double tol) { this->m_tolerance = tol; }

  //convert smtk::model to a collection
  smtk::mesh::CollectionPtr operator()(const smtk::mesh::ManagerPtr& meshManager,
                                       const smtk::model::ManagerPtr& modelManager) const;
  //convert smtk::model to a collection
  smtk::mesh::CollectionPtr operator()(const smtk::model::Model& model) const;

private:
  bool m_mergeDuplicates;
  double m_tolerance;
};

}
}

#endif
