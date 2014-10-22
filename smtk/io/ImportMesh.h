//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_io_ImportMesh_h
#define __smtk_io_ImportMesh_h

#include "smtk/SMTKCoreExports.h" // For SMTKCORE_EXPORT macro.
#include "smtk/PublicPointerDefs.h"

#include "smtk/common/UUID.h"

#include <utility>
/**\brief Import an SMTK mesh from a file
  *
  */

namespace smtk {
  namespace io {

class SMTKCORE_EXPORT ImportMesh
{
public:

  static smtk::common::UUID intoManager(const std::string& filePath,
                                        smtk::mesh::ManagerPtr manager);

};

  }
}

#endif