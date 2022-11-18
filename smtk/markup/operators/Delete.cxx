//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/io/Logger.h"

#include "smtk/markup/operators/Delete.h"

#include "smtk/markup/Resource.h"

#include "smtk/operation/MarkGeometry.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/resource/Manager.h"

#include "smtk/common/Paths.h"
#include "smtk/common/StringUtil.h"

#include <sstream>

// On Windows MSVC 2015+, something is included that defines
// a macro named ERROR to be 0. This causes smtkErrorMacro()
// to expand into garbage (because smtk::io::Logger::ERROR
// gets expanded to smtk::io::Logger::0).
#ifdef ERROR
#undef ERROR
#endif

namespace smtk
{
namespace markup
{

Delete::Delete() = default;

bool Delete::ableToOperate()
{
  if (!this->Superclass::ableToOperate())
  {
    return false;
  }

  auto deleteDeps = this->parameters()->findVoid("delete dependents")->isEnabled();
  if (deleteDeps)
  {
    return true;
  }
  std::set<smtk::graph::Component*> deps;
  bool haveExternalDeps = this->insertDependencies<smtk::markup::Resource>(deps);
  return !haveExternalDeps;
}

Delete::Result Delete::operateInternal()
{
  m_result = this->createResult(Delete::Outcome::FAILED);

  auto assocs = this->parameters()->associations();
  auto deleteDeps = this->parameters()->findVoid("delete dependents")->isEnabled();

  std::set<smtk::graph::Component*> dels;
  bool haveExternalDeps = this->insertDependencies<smtk::markup::Resource>(dels);
  if (haveExternalDeps && !deleteDeps)
  {
    smtkErrorMacro(this->log(), "ableToOperate() failed to prevent improper deletion.");
    return m_result;
  }

  // Combine the requested and dependent components into one set
  // by adding associations into dels:
  std::size_t nn = assocs->numberOfValues();
  for (std::size_t ii = 0; ii < nn; ++ii)
  {
    if (!assocs->isSet(ii))
    {
      continue;
    }
    auto* comp = dynamic_cast<Component*>(assocs->value(ii).get());
    dels.insert(comp);
  }

  auto expunged = m_result->findComponent("expunged");
  for (const auto& del : dels)
  {
    auto sdel = del->shared_from_this();
    expunged->appendValue(sdel);
    // Remove all arcs connected to this component:
    del->disconnect();
    // Clear the component's geometry:
    smtk::operation::MarkGeometry().erase(sdel);
    // Remove the component from the resource (it is now owned by
    // the expunged item, so it is not destroyed yet):
    sdel->parentResourceAs<smtk::markup::Resource>()->erase(del->id());
  }
  // TODO: Remove properties
  // TODO: Remove links

  m_result->findInt("outcome")->setValue(static_cast<int>(Delete::Outcome::SUCCEEDED));
  return m_result;
}

const char* Delete::xmlDescription() const
{
  using namespace smtk::common;
  static thread_local std::string parentXML = this->Superclass::xmlDescription();
  StringUtil::replaceAll(parentXML, "smtk::graph::ResourceBase", "smtk::markup::Resource");
  StringUtil::replaceAll(
    parentXML, "smtk::view::SubphraseGenerator", "smtk::markup::SubphraseGenerator");
  return parentXML.c_str();
}

} // namespace markup
} // namespace smtk
