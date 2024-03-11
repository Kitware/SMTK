//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/markup/Subset.h"

#include "smtk/markup/IdNature.h"

#include "smtk/common/Paths.h"
#include "smtk/markup/Resource.h"
#include "smtk/resource/json/Helper.h"

#include "vtkDataObject.h"
#include "vtkDataSetReader.h"
#include "vtkNew.h"
#include "vtkPolyDataReader.h"
#include "vtkUnstructuredGridReader.h"
#include "vtkXMLImageDataReader.h"

using namespace smtk::string::literals; // for ""_token

namespace smtk
{
namespace markup
{

Subset::~Subset() = default;

void Subset::initialize(const nlohmann::json& data, smtk::resource::json::Helper& helper)
{
  auto resource = helper.resourceAs<smtk::markup::Resource>();
  m_idSpace = data["domain_name"].get<smtk::string::Token>();
  auto space = std::dynamic_pointer_cast<IdSpace>(resource->domains().find(m_idSpace));
  auto jpid = data["ids"];
  auto jprr = jpid["range"].get<AssignedIds::IdRange>();
  auto jpnn = jpid["nature"].get<std::string>();
  IdNature pnat = natureEnumerant(jpnn);

  m_ids = std::make_shared<smtk::markup::AssignedIds>(space, pnat, jprr[0], jprr[1], this);
}

bool Subset::setDomainName(smtk::string::Token idSpace)
{
  if (m_idSpace == idSpace)
  {
    return false;
  }
  auto domain = std::dynamic_pointer_cast<IdSpace>(
    this->parentResourceAs<smtk::markup::Resource>()->domains().find(idSpace));
  if (!domain)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "The domain of a discrete subset must be an IdSpace registered "
      "in the resource's domains().");
    return false;
  }
  if (m_ids && !m_ids->empty())
  {
    smtkWarningMacro(
      smtk::io::Logger::instance(),
      "Changing the subset's domain from "
        << m_idSpace.data() << " to " << idSpace.data() << " while " << m_ids->size()
        << " IDs are assigned. This reinterprets their meaning.");
  }
  m_idSpace = idSpace;
  return true;
}

std::shared_ptr<IdSpace> Subset::domain() const
{
  return std::dynamic_pointer_cast<IdSpace>(
    this->parentResourceAs<smtk::markup::Resource>()->domains().find(m_idSpace));
}

#if 0
ArcEndpointInterface<arcs::BoundariesToShapes, ConstArc, OutgoingArc> Subset::parents() const
{
  return this->outgoing<arcs::BoundariesToShapes>();
}

ArcEndpointInterface<arcs::BoundariesToShapes, NonConstArc, OutgoingArc> Subset::parents()
{
  return this->outgoing<arcs::BoundariesToShapes>();
}

ArcEndpointInterface<arcs::BoundariesToShapes, ConstArc, IncomingArc> Subset::children() const
{
  return this->incoming<arcs::BoundariesToShapes>();
}

ArcEndpointInterface<arcs::BoundariesToShapes, NonConstArc, IncomingArc> Subset::children()
{
  return this->incoming<arcs::BoundariesToShapes>();
}
#endif // 0

bool Subset::assign(
  const smtk::graph::Component::ConstPtr& source,
  smtk::resource::CopyOptions& options)
{
  bool ok = this->Superclass::assign(source, options);
  if (auto sourceSubset = std::dynamic_pointer_cast<const Subset>(source))
  {
    this->setDomainName(sourceSubset->domainName());
    ok &= this->copyAssignment(sourceSubset->ids(), m_ids);
  }
  else
  {
    ok = false;
  }
  return ok;
}

} // namespace markup
} // namespace smtk
