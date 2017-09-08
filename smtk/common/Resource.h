//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME smtkResource.h - Abstract base class for SMTK resources
// .SECTION Description
//   An SMTK resource is one of: attribute manager, model, mesh
// .SECTION See Also

#ifndef smtk_common_Resource_h
#define smtk_common_Resource_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"
#include "smtk/SystemConfig.h"
#include "smtk/common/UUID.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <string>

namespace smtk
{
namespace common
{
class ResourceManager;

class SMTKCORE_EXPORT Resource : smtkEnableSharedPtr(Resource)
{
  friend class ResourceManager;

public:
  smtkTypeMacroBase(Resource);
  virtual ~Resource();
  std::string location() const { return this->m_url; }

  ResourceManager* manager() const { return this->m_manager; }

  const UUID& id() const { return this->m_id; }

  /// Identifies resource type
  enum Type
  {
    ATTRIBUTE = 0,
    MODEL,
    MESH, // future
    NUMBER_OF_TYPES
  };

  virtual Resource::Type resourceType() const = 0;
  virtual ResourceComponentPtr find(const UUID& compId) const = 0;

  static std::string type2String(Resource::Type t);
  static Resource::Type string2Type(const std::string& s);

protected:
  Resource(const UUID& myID, ResourceManager* manager);
  Resource(ResourceManager* manager);
  void setId(const UUID& myID) { this->m_id = myID; }

  void setLocation(const std::string& url) { this->m_url = url; }

private:
  UUID m_id;
  std::string m_url;
  ResourceManager* m_manager;
};
}
}

#endif // smtk_common_Resource_h
