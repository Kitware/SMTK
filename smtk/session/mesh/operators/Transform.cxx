//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/session/mesh/operators/Transform.h"

#include "smtk/session/mesh/Resource.h"
#include "smtk/session/mesh/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"

#include "smtk/common/CompilerInformation.h"

#include "smtk/mesh/core/Component.h"
#include "smtk/mesh/core/Resource.h"
#include "smtk/mesh/operators/Transform.h"

#include "smtk/model/Model.h"

#include "smtk/operation/MarkGeometry.h"

#include "smtk/session/mesh/operators/Transform_xml.h"

using namespace smtk::model;
using namespace smtk::common;

namespace smtk
{
namespace session
{
namespace mesh
{

Transform::Result Transform::operateInternal()
{
  // Access the associated model.
  smtk::model::Model model = this->parameters()->associatedModelEntities<smtk::model::Models>()[0];
  if (!model.isValid())
  {
    smtkErrorMacro(this->log(), "Invalid model.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  smtk::session::mesh::Resource::Ptr resource =
    std::static_pointer_cast<smtk::session::mesh::Resource>(model.component()->resource());
  smtk::session::mesh::Session::Ptr session = resource->session();

  // Access the underlying mesh resource for the model.
  smtk::mesh::ResourcePtr meshResource = resource->resource();
  if (meshResource == nullptr || !meshResource->isValid())
  {
    smtkErrorMacro(this->log(), "No mesh resource associated with this model.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // Access the transform values
  smtk::attribute::DoubleItem::Ptr scaleItem = this->parameters()->findDouble("scale");
  smtk::attribute::DoubleItem::Ptr rotateItem = this->parameters()->findDouble("rotate");
  smtk::attribute::DoubleItem::Ptr translateItem = this->parameters()->findDouble("translate");

  // Construct a mesh transform operation.
  auto transform = smtk::mesh::Transform::create();

  // Associate it with the model's underlying mesh.
  transform->parameters()->associate(smtk::mesh::Component::create(meshResource->meshes()));

  // Access the mesh transform operation's transform values.
  smtk::attribute::DoubleItem::Ptr scaleItem_ = transform->parameters()->findDouble("scale");
  smtk::attribute::DoubleItem::Ptr rotateItem_ = transform->parameters()->findDouble("rotate");
  smtk::attribute::DoubleItem::Ptr translateItem_ =
    transform->parameters()->findDouble("translate");

  // Transfer the transform values from this operation to the mesh operation.
  for (std::size_t i = 0; i < 3; ++i)
  {
    scaleItem_->setValue(i, scaleItem->value(i));
    rotateItem_->setValue(i, rotateItem->value(i));
    translateItem_->setValue(i, translateItem->value(i));
  }

  // Execute the transform.
  transform->operate(this->childKey());

  // Access the attribute associated with the modified meshes.
  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  // Access the attribute associated with the modified model.
  smtk::attribute::ComponentItem::Ptr modified = result->findComponent("modified");

  // Assign the model as modified.
  modified->appendValue(model.component());

  // Construct a MarkGeometry instance.
  smtk::operation::MarkGeometry markGeometry(resource);

  // Access the model resource's associated topology.
  smtk::session::mesh::Topology* topology = resource->session()->topology(resource);

  std::function<void(const smtk::common::UUID&)> mark;

  mark = [&](const smtk::common::UUID& id) {
    markGeometry.markModified(resource->find(id));
    auto elementIt = topology->m_elements.find(id);
    if (elementIt == topology->m_elements.end())
    {
      return;
    }
    Topology::Element& element = elementIt->second;

    for (const smtk::common::UUID& childId : element.m_children)
    {
      mark(childId);
    }
  };

  mark(model.entity());

  return result;
}

const char* Transform::xmlDescription() const
{
  return Transform_xml;
}
} // namespace mesh
} // namespace session
} // namespace smtk
