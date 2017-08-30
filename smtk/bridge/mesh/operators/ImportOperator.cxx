//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/mesh/operators/ImportOperator.h"

#include "smtk/bridge/mesh/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/io/ImportMesh.h"

#include "smtk/mesh/Metrics.h"

#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"

#include "smtk/common/CompilerInformation.h"

using namespace smtk::model;
using namespace smtk::common;

namespace smtk
{
namespace bridge
{
namespace mesh
{

smtk::model::OperatorResult ImportOperator::operateInternal()
{
  // Get the read file name
  smtk::attribute::FileItem::Ptr filePathItem = this->specification()->findFile("filename");
  std::string filePath = filePathItem->value();

  smtk::attribute::StringItem::Ptr labelItem = this->specification()->findString("label");
  std::string label = labelItem->value();

  smtk::attribute::VoidItem::Ptr hierarchyItem =
    this->specification()->findVoid("construct hierarchy");
  bool constructHierarchy = hierarchyItem->isEnabled();

  // Get the collection from the file
  smtk::mesh::CollectionPtr collection =
    smtk::io::importMesh(filePath, this->activeSession()->meshManager(), label);

  if (!collection || !collection->isValid())
  {
    // The file was not correctly read.
    return this->createResult(smtk::model::OPERATION_FAILED);
  }

  auto format = smtk::io::meshFileFormat(filePath);
  if (format.Name == "exodus")
  {
    this->activeSession()->facade()["domain"] = "Element Block";
    this->activeSession()->facade()["dirichlet"] = "Node Set";
    this->activeSession()->facade()["neumann"] = "Side Set";
  }

  // Assign its model manager to the one associated with this session
  collection->setModelManager(this->manager());

  // Construct the topology
  this->activeSession()->addTopology(Topology(collection, constructHierarchy));

  // Determine the model's dimension
  int dimension = int(smtk::mesh::highestDimension(collection->meshes()));

  // Our collections will already have a UUID, so here we create a model given
  // the model manager and uuid
  smtk::model::Model model =
    this->manager()->insertModel(collection->entity(), dimension, dimension);

  // Declare the model as "dangling" so it will be transcribed
  this->session()->declareDanglingEntity(model);

  collection->associateToModel(model.entity());

  // Set the model's session to point to the current session
  model.setSession(smtk::model::SessionRef(this->manager(), this->activeSession()->sessionId()));

  // If we don't call "transcribe" ourselves, it never gets called.
  this->activeSession()->transcribe(model, smtk::model::SESSION_EVERYTHING, false);

  smtk::model::OperatorResult result = this->createResult(smtk::model::OPERATION_SUCCEEDED);

  smtk::attribute::ModelEntityItem::Ptr resultModels = result->findModelEntity("model");
  resultModels->setValue(model);

  smtk::attribute::ModelEntityItem::Ptr created = result->findModelEntity("created");
  created->setNumberOfValues(1);
  created->setValue(model);
  created->setIsEnabled(true);

  result->findModelEntity("mesh_created")->setValue(model);

  return result;
}

} // namespace mesh
} //namespace bridge
} // namespace smtk

#include "smtk/bridge/mesh/Exports.h"
#include "smtk/bridge/mesh/ImportOperator_xml.h"

smtkImplementsModelOperator(SMTKMESHSESSION_EXPORT, smtk::bridge::mesh::ImportOperator, mesh_import,
  "import", ImportOperator_xml, smtk::bridge::mesh::Session);
