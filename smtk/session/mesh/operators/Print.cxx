//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/session/mesh/operators/Print.h"

#include "smtk/session/mesh/Resource.h"
#include "smtk/session/mesh/Session.h"
#include "smtk/session/mesh/Topology.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"

#include "smtk/mesh/core/Resource.h"
#include "smtk/mesh/utility/Metrics.h"

#include "smtk/model/Face.h"
#include "smtk/model/Model.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/Resource.h"

#include "smtk/session/mesh/operators/Print_xml.h"

using namespace smtk::model;
using namespace smtk::common;

namespace smtk
{
namespace session
{
namespace mesh
{

Print::Result Print::operateInternal()
{
  // Access the associated model entities
  auto associations = this->parameters()->associations();

  // Access the model resource associated with all of the input model entities
  smtk::session::mesh::Resource::Ptr resource =
    std::static_pointer_cast<smtk::session::mesh::Resource>(
      associations->valueAs<smtk::resource::Component>()->resource());

  // Access the model resource's associated mesh resource
  smtk::mesh::Resource::Ptr meshResource = resource->resource();

  // Access the model resource's associated topology
  smtk::session::mesh::Topology* topology = resource->session()->topology(resource);

  // For each model entity to be printed...
  for (auto it = associations->begin(); it != associations->end(); ++it)
  {
    //...access its associated topology element.
    auto elementIt = topology->m_elements.find(it->id());
    Topology::Element& element = elementIt->second;

    smtk::mesh::MeshSet meshset = element.m_mesh;

    smtkInfoMacro(
      this->log(),
      "Model Entity <" << it->id() << ">\n"
                       << "  name:         " << it->name()
                       << "\n"
                          "  # meshes:     "
                       << meshset.size()
                       << "\n"
                          "  # cells:      "
                       << meshset.cells().size()
                       << "\n"
                          "  # points:     "
                       << meshset.points().size()
                       << "\n"
                          "  # domains:    "
                       << meshset.domains().size()
                       << "\n"
                          "  # dirichlets: "
                       << meshset.dirichlets().size()
                       << "\n"
                          "  # neumanns:   "
                       << meshset.neumanns().size());
  }

  return this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  ;
}

const char* Print::xmlDescription() const
{
  return Print_xml;
}
} // namespace mesh
} // namespace session
} // namespace smtk
