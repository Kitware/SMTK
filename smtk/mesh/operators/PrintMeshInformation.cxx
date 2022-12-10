//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/mesh/operators/PrintMeshInformation.h"

#include "smtk/mesh/core/Component.h"
#include "smtk/mesh/core/MeshSet.h"
#include "smtk/mesh/core/Resource.h"

#include "smtk/model/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/IntItem.h"

#include "smtk/mesh/operators/PrintMeshInformation_xml.h"

namespace smtk
{
namespace mesh
{

smtk::mesh::PrintMeshInformation::Result PrintMeshInformation::operateInternal()
{
  smtk::attribute::ReferenceItem::Ptr meshItem = this->parameters()->associations();

  for (std::size_t i = 0; i < meshItem->numberOfValues(); i++)
  {
    smtk::mesh::Component::Ptr meshComponent = meshItem->valueAs<smtk::mesh::Component>(i);
    smtk::mesh::MeshSet meshset = meshComponent->mesh();

    smtkInfoMacro(
      this->log(),
      "Mesh Component <" << meshComponent->id() << ">\n"
                         << "  name:         " << meshset.name()
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
}

const char* PrintMeshInformation::xmlDescription() const
{
  return PrintMeshInformation_xml;
}

} //namespace mesh
} // namespace smtk
