//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_resource_Set_h
#define __smtk_resource_Set_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SystemConfig.h"

#include "smtk/resource/Resource.h"

#include <map>
#include <string>
#include <vector>

namespace smtk
{
namespace resource
{

struct Wrapper; // defined in Set.cxx

/**\brief A container for SMTK resources.
  *
  * A resource set provides methods for tracking the
  * type and location of resources used to prepare a
  * simulation input deck; and for loading the resources
  * as required when the input deck is being processed.
  */
class SMTKCORE_EXPORT Set
{
public:
  /// Identifies load-state of Resource
  enum State
  {
    NOT_LOADED = 0, // have link to file/uri, but resource not instantiated
    LOADED,         // resource instantiated and contents loaded
    LOAD_ERROR      // failed to load
  };

  /// Identifies resource role, used with attribute resources
  enum Role
  {
    NOT_DEFINED = 0, //!< for non-attribute, non-model resources
    TEMPLATE,        //!< resources storing attributes serving as templates
    SCENARIO,        //!< resources storing attributes serving as scenarios
    INSTANCE,       //!< resources storing attributes whose instances are created on demand by users
    MODEL_RESOURCE, //!< resources storing model geometry
    AUX_GEOM_RESOURCE, //!< resources storing auxiliary model geometry
  };

  Set();
  virtual ~Set();

  bool add(ResourcePtr resource, std::string id, std::string link = "", Role = NOT_DEFINED);

  bool addInfo(
    const std::string id, Resource::Type type, Role role, State state, std::string link = "");

  bool remove(const std::string& id);

  std::size_t numberOfResources() const;

  const std::vector<std::string> resourceIds() const;

  bool resourceInfo(
    std::string id, Resource::Type& type, Role& role, State& state, std::string& link) const;

  bool get(std::string id, ResourcePtr& resource) const;

  static std::string state2String(State state);
  static std::string role2String(Role role);
  static Role string2Role(const std::string s);

  std::string linkStartPath() const;
  void setLinkStartPath(const std::string path);

protected:
  std::vector<std::string> m_resourceIds;
  std::map<std::string, Wrapper*> m_resourceMap;
  std::string m_linkStartPath;

  Wrapper* getWrapper(std::string id) const;
};

} // namespace resource
} // namespace smtk

#endif // __smtk_resource_Set_h
