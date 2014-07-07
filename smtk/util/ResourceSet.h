/*=========================================================================

Copyright (c) 1998-2014 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/
// .NAME ResourceSet.h - Container for SMTK resources
// .SECTION Description
// .SECTION See Also

#ifndef __ResourceSet_h
#define __ResourceSet_h

#include "smtk/SMTKCoreExports.h"
#include "smtk/util/Resource.h"
#include "smtk/util/SystemConfig.h"
#include "smtk/PublicPointerDefs.h"

#include <map>
#include <string>
#include <vector>

struct ResourceWrapper;  // defined in ResourceSet.cxx

namespace smtk {
  namespace util {

class SMTKCORE_EXPORT ResourceSet
{
 public:
  /// Identifies load-state of Resource
  enum ResourceState
  {
    NOT_LOADED = 0,   // have link to file/uri, but resource not instantiated
    LOADED,           // resource instantiated and contents loaded
    LOAD_ERROR        // failed to load
  };

  /// Identifies resource role, used with attribute resources
  enum ResourceRole
  {
    NOT_DEFINED = 0,  // for non-attribute resources
    TEMPLATE,
    SCENARIO,
    INSTANCE
  };

  ResourceSet();
  virtual ~ResourceSet();

  bool addResource(smtk::util::ResourcePtr resource,
                   std::string id,
                   std::string link="",
                   ResourceRole=NOT_DEFINED);

  bool addResourceInfo(const std::string id,
                       smtk::util::Resource::Type type,
                       ResourceRole role,
                       ResourceState state,
                       std::string link="");

  unsigned numberOfResources() const;

  const std::vector<std::string> resourceIds() const;

  bool resourceInfo(std::string id,
                    smtk::util::Resource::Type& type,
                    ResourceRole& role,
                    ResourceState& state,
                    std::string& link) const;

  bool get(std::string id,
           smtk::util::ResourcePtr& resource) const;

  static std::string state2String(ResourceState state);
  static std::string role2String(ResourceRole role);
  static ResourceRole string2Role(const std::string s);

  std::string linkStartPath() const;
  void setLinkStartPath(const std::string path);

 protected:
  std::vector<std::string> m_resourceIds;
  std::map<std::string, ResourceWrapper*> m_resourceMap;
  std::string m_linkStartPath;

  ResourceWrapper *getWrapper(std::string id) const;
};

  }  // namespace util
}  // namespace smtk

#endif  /* __ResourceSet_h */
