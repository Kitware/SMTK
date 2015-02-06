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

#include "smtk/SMTKCoreExports.h" // For SMTKCORE_EXPORT macro.
#include "smtk/PublicPointerDefs.h"

namespace smtk {
namespace io {
class SMTKCORE_EXPORT ModelToMesh
{
public:
  //convert smtk::model to a collection
  smtk::mesh::CollectionPtr operator()(const smtk::mesh::ManagerPtr& meshManager,
                                       const smtk::model::ManagerPtr& modelManager) const;
};

}
}

#endif
