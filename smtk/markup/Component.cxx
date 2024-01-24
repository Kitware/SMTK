//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/markup/Component.h"

#include "smtk/markup/Group.h"
#include "smtk/markup/Label.h"
#include "smtk/markup/Resource.h"

#include "smtk/resource/json/Helper.h"

namespace smtk
{
namespace markup
{

struct Component::ModifyName
{
  ModifyName(const std::string& nextName)
    : m_name(nextName)
  {
  }

  void operator()(Component::Ptr& c) { c->m_name = m_name; }

  std::string m_name;
};

Component::~Component() = default;

void Component::initialize(const nlohmann::json& data, smtk::resource::json::Helper& helper)
{
  (void)helper;
  std::cout << "init comp " << this << " with " << data.dump(2) << "\n";
  auto it = data.find("name");
  if (it != data.end())
  {
    this->setName(it->get<std::string>());
  }
}

Component::Index Component::index() const
{
  return std::type_index(typeid(*this)).hash_code();
}

std::string Component::name() const
{
  return m_name;
}

bool Component::setName(const std::string& name)
{
  if (name == m_name)
  {
    return false;
  }

  auto owner = std::dynamic_pointer_cast<smtk::markup::Resource>(this->resource());
  if (owner)
  {
    // We are not allowed to set our name directly since
    // our owning resource has indexed us by name.
    this->properties().get<std::string>()["name"] = name;
    return owner->modifyComponent(*this, ModifyName(name));
  }
  m_name = name;
  this->properties().get<std::string>()["name"] = name;
  return true;
}

ArcEndpointInterface<arcs::GroupsToMembers, ConstArc, IncomingArc> Component::groups() const
{
  return this->incoming<arcs::GroupsToMembers>();
}

ArcEndpointInterface<arcs::GroupsToMembers, NonConstArc, IncomingArc> Component::groups()
{
  return this->incoming<arcs::GroupsToMembers>();
}

ArcEndpointInterface<arcs::LabelsToSubjects, ConstArc, IncomingArc> Component::labels() const
{
  return this->incoming<arcs::LabelsToSubjects>();
}

ArcEndpointInterface<arcs::LabelsToSubjects, NonConstArc, IncomingArc> Component::labels()
{
  return this->incoming<arcs::LabelsToSubjects>();
}

ArcEndpointInterface<arcs::URLsToImportedData, ConstArc, IncomingArc> Component::importedFrom()
  const
{
  return this->incoming<arcs::URLsToImportedData>();
}

ArcEndpointInterface<arcs::URLsToImportedData, NonConstArc, IncomingArc> Component::importedFrom()
{
  return this->incoming<arcs::URLsToImportedData>();
}

ArcEndpointInterface<arcs::OntologyIdentifiersToIndividuals, ConstArc, IncomingArc>
Component::ontologyClasses() const
{
  return this->incoming<arcs::OntologyIdentifiersToIndividuals>();
}

ArcEndpointInterface<arcs::OntologyIdentifiersToIndividuals, NonConstArc, IncomingArc>
Component::ontologyClasses()
{
  return this->incoming<arcs::OntologyIdentifiersToIndividuals>();
}

} // namespace markup
} // namespace smtk
