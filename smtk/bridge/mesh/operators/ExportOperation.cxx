//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/mesh/operators/ExportOperation.h"

#include "smtk/bridge/mesh/Resource.h"
#include "smtk/bridge/mesh/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/io/WriteMesh.h"

#include "smtk/mesh/core/Manager.h"

#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"

#include "smtk/bridge/mesh/ExportOperation_xml.h"

using namespace smtk::model;
using namespace smtk::common;

namespace smtk
{
namespace bridge
{
namespace mesh
{

void breakMaterialsByAssociation(const smtk::mesh::CollectionPtr& c)
{
  //for each association we iterate the meshsets
  smtk::model::EntityRefArray refs;
  c->meshes().modelEntities(refs);

  int domain = 0;
  for (auto&& ref : refs)
  {
    smtk::mesh::MeshSet m = c->findAssociatedMeshes(ref);
    c->setDomainOnMeshes(m, smtk::mesh::Domain(domain++));
  }
}

smtk::bridge::mesh::ExportOperation::Result ExportOperation::operateInternal()
{
  smtk::attribute::FileItem::Ptr filePathItem = this->parameters()->findFile("filename");

  std::string filePath = filePathItem->value();

  smtk::model::Models datasets = this->parameters()->associatedModelEntities<smtk::model::Models>();
  if (datasets.empty())
  {
    smtkErrorMacro(this->log(), "No models to save.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  smtk::model::Model dataset = datasets[0];

  smtk::bridge::mesh::Resource::Ptr resource =
    std::static_pointer_cast<smtk::bridge::mesh::Resource>(dataset.component()->resource());
  smtk::bridge::mesh::Session::Ptr session = resource->session();

  smtk::mesh::CollectionPtr collection =
    session->meshManager()->findCollection(dataset.entity())->second;

  if (!collection->isValid())
  {
    smtkErrorMacro(this->log(), "No collection associated with this model.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  breakMaterialsByAssociation(collection);

  bool writeSuccess = smtk::io::writeMesh(filePath, collection);

  if (!writeSuccess)
  {
    smtkErrorMacro(this->log(), "Collection failed to write.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  return result;
}

const char* ExportOperation::xmlDescription() const
{
  return ExportOperation_xml;
}

} // namespace mesh
} //namespace bridge
} // namespace smtk
