//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_mesh_Facade_h
#define __smtk_session_mesh_Facade_h

#include "smtk/bridge/mesh/Exports.h"

#include <string>

namespace smtk
{
namespace bridge
{
namespace mesh
{

/**\brief A facade for naming model entities from mesh properties.

   Models generated from a mesh set have the same properties as mesh sets. Mesh
   set properties are "domain" for material properties, "dirichlet" for point-
   centric properties, and "neumann" for cell-centric properties. These labels
   are often ill-fitting for many use cases, so we provide an interface to
   change them. This class is simply a map connecting the native property names
   to user-definable names.
  */
struct SMTKMESHSESSION_EXPORT Facade
{
  Facade();
  virtual ~Facade();

  std::string& operator[](const std::string& key);

private:
  struct Internals;
  Internals* m_internal;
};
}
}
}

#endif
