//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/mesh/WriteOperator.h"

#include "smtk/bridge/mesh/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/common/CompilerInformation.h"

#include "smtk/io/WriteMesh.h"

#include "smtk/mesh/Manager.h"

#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"

using namespace smtk::model;
using namespace smtk::common;

namespace smtk {
namespace bridge {
namespace mesh {

void breakMaterialsByAssociation(const smtk::mesh::CollectionPtr& c)
{
  //for each association we iterate the meshsets
  smtk::model::EntityRefArray refs = c->meshes().modelEntities();

  int domain = 0;
  for(auto&& ref : refs)
    {
    smtk::mesh::MeshSet m = c->findAssociatedMeshes(ref);
    c->setDomainOnMeshes(m, smtk::mesh::Domain(domain++));
    }
}

smtk::model::OperatorResult WriteOperator::operateInternal()
{
  smtk::attribute::FileItem::Ptr filePathItem =
    this->specification()->findFile("filename");

  std::string filePath = filePathItem->value();

  smtk::model::Models datasets =
    this->specification()->associatedModelEntities<smtk::model::Models>();
  if (datasets.empty())
    {
    smtkErrorMacro(this->log(), "No models to save.");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  smtk::model::Model dataset = datasets[0];

  smtk::mesh::CollectionPtr collection =
    this->activeSession()->meshManager()
    ->findCollection( dataset.entity() )->second;

  if ( !collection->isValid() )
    {
    smtkErrorMacro(this->log(), "No collection associated with this model.");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  breakMaterialsByAssociation(collection);

  bool writeSuccess = smtk::io::writeMesh( filePath, collection );

  if ( !writeSuccess )
    {
    smtkErrorMacro(this->log(), "Collection failed to write.");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  smtk::model::OperatorResult result = this->createResult(
    smtk::model::OPERATION_SUCCEEDED);
  return result;
}

} // namespace mesh
} //namespace bridge
} // namespace smtk

#include "smtk/bridge/mesh/WriteOperator_xml.h"
#include "smtk/bridge/mesh/Exports.h"

smtkImplementsModelOperator(
  SMTKMESHSESSION_EXPORT,
  smtk::bridge::mesh::WriteOperator,
  mesh_write,
  "write",
  WriteOperator_xml,
  smtk::bridge::mesh::Session);
