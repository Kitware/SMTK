//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME smtkResourceComponent.h - Abstract base class for SMTK ResourceComponents
// .SECTION Description
//   A SMTK ResourceComponent is a peice of a Resource such as an Attribute, ModelEntity,
//  or MeshEntity
// .SECTION See Also

#ifndef smtk_common_ResourceComponent_h
#define smtk_common_ResourceComponent_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"
#include "smtk/SystemConfig.h"
#include "smtk/common/UUID.h"

#include <string>

namespace smtk
{
namespace common
{

class SMTKCORE_EXPORT ResourceComponent : smtkEnableSharedPtr(ResourceComponent)
{
  friend class Resource;

public:
  smtkTypeMacroBase(ResourceComponent);
  virtual ~ResourceComponent();
  virtual ResourcePtr resource() const = 0;

  const UUID& id() const { return this->m_id; }

protected:
  ResourceComponent(const UUID& myID);
  ResourceComponent();
  void setId(const UUID& myID) { this->m_id = myID; }

private:
  UUID m_id;
};
}
}

#endif // smtk_common_ResourceComponent_h
