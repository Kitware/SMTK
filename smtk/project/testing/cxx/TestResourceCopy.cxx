//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Registrar.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/common/UUID.h"
#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/EntityTypeBits.h"
#include "smtk/model/Resource.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Operation.h"
#include "smtk/operation/operators/ReadResource.h"
#include "smtk/operation/operators/WriteResource.h"
#include "smtk/resource/Component.h"
#include "smtk/resource/Manager.h"
#include "smtk/session/vtk/Registrar.h"
#include "smtk/session/vtk/operators/Import.h"
#include "smtk/session/vtk/operators/Write.h"

#include <boost/filesystem.hpp>

#include <algorithm>
#include <iostream>
#include <string>

namespace
{
const std::string DATA_ROOT = SMTK_DATA_DIR;
const std::string TEMP_ROOT = SMTK_SCRATCH_DIR;
const std::string ORIG_ROOT = TEMP_ROOT + "/orig";
const std::string COPY_ROOT = TEMP_ROOT + "/copy";
const int OP_SUCCESS = static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED);
smtk::common::UUID origAttUUID = smtk::common::UUID::null();
smtk::common::UUID origModelUUID = smtk::common::UUID::null();

// clang-format off
const std::string ATT_TEMPLATE =
  "<SMTK_AttributeResource Version=\"3\">\n"
    "<Definitions>\n"
     "<AttDef Type=\"model-assoc\">\n"
      "<AssociationsDef Name=\"assoc\">\n"
        "<MembershipMask>face</MembershipMask>\n"
      "</AssociationsDef>\n"
      "<ItemDefinitions>\n"
        "<Resource Name=\"resource-ref\" LockType=\"DoNotLock\">\n"
          "<Accepts>\n"
            "<Resource Name=\"smtk::model::Resource\" />\n"
          "</Accepts>\n"
        "</Resource>"
        "<Component Name=\"face-ref\" LockType=\"DoNotLock\">\n"
          "<Accepts>\n"
            "<Resource Name=\"smtk::model::Resource\" Filter=\"face\" />\n"
          "</Accepts>\n"
        "</Component>\n"
      "</ItemDefinitions>\n"
     "</AttDef>\n"

     "<AttDef Type=\"attribute-assoc\">\n"
      "<AssociationsDef Name=\"assoc\">\n"
        "<Accepts>\n"
          "<Resource Name=\"smtk::attribute::Resource\" Filter=\"attribute[type='model-assoc']\" />\n"
        "</Accepts>\n"
      "</AssociationsDef>\n"
      "<ItemDefinitions>\n"
        "<Resource Name=\"resource-ref\" LockType=\"DoNotLock\">\n"
          "<Accepts>\n"
            "<Resource Name=\"smtk::attribute::Resource\" />\n"
          "</Accepts>\n"
        "</Resource>"
        "<Component Name=\"attribute-ref\" LockType=\"DoNotLock\">\n"
          "<Accepts>\n"
            "<Resource Name=\"smtk::attribute::Resource\" Filter=\"attribute[type='model-assoc']\" />\n"
          "</Accepts>\n"
        "</Component>\n"
      "</ItemDefinitions>\n"
     "</AttDef>\n"
    "</Definitions>\n"
  "</SMTK_AttributeResource>\n";
// clang-format on

// Function for sorting model entities by uuid
bool compareModelEntity(smtk::model::EntityRef left, smtk::model::EntityRef right)
{
  // Use pedigree id if available
  const std::string propName = "pedigree id";
  if (left.hasIntegerProperty(propName) && right.hasIntegerProperty(propName))
  {
    int leftId = left.integerProperty(propName).front();
    int rightId = right.integerProperty(propName).front();
    return leftId < rightId;
  }
  // (else) use UUID
  std::cout << "Warning: model entities missing " << propName << " property" << std::endl;
  return left.entity() < right.entity();
}

class ResourceBuilder
{
public:
  void importResources();
  void buildAttributes() const;
  void writeResources(
    smtk::operation::ManagerPtr, const std::string& folder, bool setFileItem = false);
  void copyResources(const std::string& folder);

protected:
  smtk::attribute::ResourcePtr m_attResource;
  smtk::model::ResourcePtr m_modelResource;
}; // class ResourceBuilder

class ResourceChecker
{
public:
  void readResources(smtk::operation::ManagerPtr opManager, const std::string& folder);
  void checkResources();

protected:
  smtk::attribute::ResourcePtr m_attResource;
  smtk::model::ResourcePtr m_modelResource;
}; // class ResourceBuilder

void ResourceBuilder::importResources()
{
  auto logger = smtk::io::Logger::instance();

  {
    // Initialize the attribute resource
    m_attResource = smtk::attribute::Resource::create();
    smtk::io::AttributeReader reader;
    bool status = reader.readContents(m_attResource, ATT_TEMPLATE, logger);
    test(!status, "Could not read attribute template");
    smtkTest(m_attResource->setName("attributes"), "failed to set attribute resource name");

    std::string modelPath(DATA_ROOT);
    modelPath += "/model/3d/netcdf/pillbox.ncdf";
    auto importOp = smtk::session::vtk::Import::create();
    importOp->parameters()->findFile("filename")->setValue(modelPath);
    auto importResult = importOp->operate();
    bool importSuccess = importResult->findInt("outcome")->value(0) == OP_SUCCESS;
    smtkTest(importSuccess, "import model failed: " << modelPath);
    auto resource = importResult->findResource("resource")->value();
    m_modelResource = std::dynamic_pointer_cast<smtk::model::Resource>(resource);
    smtkTest(!!m_modelResource, "model not imported");
    smtkTest(m_modelResource->setName("model"), "failed to set model resource name");
  }

  // Sanity check shared pointer use counts
  smtkTest(m_attResource.use_count() == 1, "m_attResource use_count is "
      << m_attResource.use_count()) smtkTest(m_modelResource.use_count() == 1,
    "m_modelResource use_count is " << m_modelResource.use_count())

    origAttUUID = m_attResource->id();
  origModelUUID = m_modelResource->id();
} // ResourceBuilder::importResources()

void ResourceBuilder::buildAttributes() const
{
  // Associate attribute resource to model resource
  m_attResource->associate(m_modelResource);

  // Get list of model entities
  std::vector<smtk::model::EntityRef> modelFaces =
    m_modelResource->findEntitiesOfType(smtk::model::FACE);
  std::sort(modelFaces.begin(), modelFaces.end(), compareModelEntity);
  std::size_t faceIndex = 0;

  // Create and build model-association attribute
  auto modelAssocAtt = m_attResource->createAttribute("model-assoc", "model-assoc");
  auto face0 = modelFaces.at(faceIndex);
  smtkTest(
    modelAssocAtt->associateEntity(face0), "failed to associate model face 0: " << face0.name());
  faceIndex++;

  auto faceCompItem = modelAssocAtt->findComponent("face-ref");
  auto face1 = modelFaces.at(faceIndex);
  smtkTest(
    faceCompItem->setValue(face1.component()), "failed to assign model face 1: " << face1.name());
  faceIndex++;

  auto modelResourceItem = modelAssocAtt->findResource("resource-ref");
  smtkTest(modelResourceItem->setValue(m_modelResource), "failed to assign model resource item");

  // Create and build attribute-association attribute
  auto attributeAssocAtt = m_attResource->createAttribute("attribute-assoc", "attribute-assoc");
  smtkTest(attributeAssocAtt->associate(modelAssocAtt), "failed to associate attribute: ");

  auto attributeCompItem = attributeAssocAtt->findComponent("attribute-ref");
  smtkTest(attributeCompItem->setValue(modelAssocAtt), "failed to assign attribute: ");

  auto attResourceItem = attributeAssocAtt->findResource("resource-ref");
  smtkTest(attResourceItem->setValue(m_attResource), "failed to assign attribute resource item");
}

void ResourceBuilder::copyResources(const std::string& folder)
{
  // Modify the resources as needed to effect a copy
  auto att_uuid = smtk::common::UUID::random();
  smtkTest(m_attResource->setId(att_uuid), "failed to set attribute resource UUID");

  auto model_uuid = smtk::common::UUID::random();
  smtkTest(m_modelResource->setId(model_uuid), "failed to set model resource UUID");

  smtkTest(m_attResource->hasAssociations(), "lost model resource association");
  smtk::resource::ResourceSet assocs = m_attResource->associations();
  smtkTest(assocs.size() == 1, "wrong number of associations");

  // Set locations (before serializing)
  std::string attPath = std::string(folder) + "/attributes.smtk";
  smtkTest(m_attResource->setLocation(attPath), "failed to set attribute resource location");

  std::string modelPath = std::string(folder) + "/model.smtk";
  smtkTest(m_modelResource->setLocation(modelPath), "failed to set model resource location");
} // ResourceBuilder::copyResources()

void ResourceBuilder::writeResources(
  smtk::operation::ManagerPtr opManager, const std::string& folder, bool setFileItem)
{
  // Write attribute resource
  std::string attPath = std::string(folder) + "/attributes.smtk";
  auto attWriteOp = opManager->create<smtk::operation::WriteResource>();
  attWriteOp->parameters()->associate(m_attResource);
  if (setFileItem)
  {
    attWriteOp->parameters()->findFile("filename")->setIsEnabled(true);
    attWriteOp->parameters()->findFile("filename")->setValue(attPath);
  }
  auto attWriteResult = attWriteOp->operate();
  bool attWriteSuccess = attWriteResult->findInt("outcome")->value(0) == OP_SUCCESS;
  smtkTest(attWriteSuccess, "write attributes failed: " << attPath);

  // Write model resource
  std::string modelPath = std::string(folder) + "/model.smtk";
  auto modelWriteOp = opManager->create<smtk::operation::WriteResource>();
  modelWriteOp->parameters()->associate(m_modelResource);
  if (setFileItem)
  {
    modelWriteOp->parameters()->findFile("filename")->setIsEnabled(true);
    modelWriteOp->parameters()->findFile("filename")->setValue(modelPath);
  }
  auto modelWriteResult = modelWriteOp->operate();
  bool modelWriteSuccess = modelWriteResult->findInt("outcome")->value(0) == OP_SUCCESS;
  smtkTest(modelWriteSuccess, "write model failed: " << modelPath);

  // Sanity check shared pointer use counts
  smtkTest(m_attResource.use_count() == 1, "m_attResource use_count is "
      << m_attResource.use_count()) smtkTest(m_modelResource.use_count() == 1,
    "m_modelResource use_count is " << m_modelResource.use_count())
} // ResourceBuilder::writeResources()

void ResourceChecker::readResources(
  smtk::operation::ManagerPtr opManager, const std::string& folder)
{
  std::string attPath = std::string(folder) + "/attributes.smtk";
  auto attReadOp = opManager->create<smtk::operation::ReadResource>();
  attReadOp->parameters()->findFile("filename")->setValue(attPath);
  auto attReadResult = attReadOp->operate();
  bool attReadSuccess = attReadResult->findInt("outcome")->value(0) == OP_SUCCESS;
  smtkTest(attReadSuccess, "read attributes failed: " << attPath);
  auto attResource = attReadResult->findResource("resource")->value();
  m_attResource = std::dynamic_pointer_cast<smtk::attribute::Resource>(attResource);
  smtkTest(!!m_attResource, "attribute resource not read");

  std::string modelPath = std::string(folder) + "/model.smtk";
  auto modelReadOp = opManager->create<smtk::operation::ReadResource>();
  modelReadOp->parameters()->findFile("filename")->setValue(modelPath);
  auto modelReadResult = modelReadOp->operate();
  bool modelReadSuccess = modelReadResult->findInt("outcome")->value(0) == OP_SUCCESS;
  smtkTest(modelReadSuccess, "read model failed: " << modelPath);
  auto modelResource = modelReadResult->findResource("resource")->value();
  m_modelResource = std::dynamic_pointer_cast<smtk::model::Resource>(modelResource);
  smtkTest(!!m_modelResource, "model resource not read");
} // ResourceChecker::readResources()

void ResourceChecker::checkResources()
{
#ifndef NDEBUG
  std::cout << "ATTRIBUTE RESOURCE:" << std::endl;
  std::cout << "  name:            " << m_attResource->name() << "\n";
  std::cout << "  location:        " << m_attResource->location() << "\n";
  std::cout << "  hasAssociations: " << m_attResource->hasAssociations() << "\n";
  std::cout << "  use_count:       " << m_attResource.use_count() << "\n";

  std::cout << "MODEL RESOURCE:" << std::endl;
  std::cout << "  name:            " << m_modelResource->name() << "\n";
  std::cout << "  location:        " << m_modelResource->location() << "\n";
  std::cout << "  use_count:       " << m_modelResource.use_count() << "\n";

  std::cout << std::endl;
#endif

  // Check that UUIDs are different
  smtkTest(!origAttUUID.isNull(), "forgot to set origAttUUID");
  smtkTest(origAttUUID != m_attResource->id(), "attribute resource UUID unchanged");
  smtkTest(!origModelUUID.isNull(), "forgot to set origModelUUID");
  smtkTest(origModelUUID != m_modelResource->id(), "model resource UUID unchanged");

  // Check locations
  std::string attPath = std::string(COPY_ROOT) + "/attributes.smtk";
  smtkTest(attPath == m_attResource->location(), "wrong attribute resource location");
  std::string modelPath = std::string(COPY_ROOT) + "/model.smtk";
  smtkTest(modelPath == m_modelResource->location(), "wrong model resource location");

  // Check resource association
  std::set<smtk::resource::ResourcePtr> assocs = m_attResource->associations();
  smtkTest(assocs.size() == 1, "should have 1 resource association, not " << assocs.size());
  smtkTest(assocs.count(m_modelResource) == 1, "model association is wrong");

  // Get list of model entities
  std::vector<smtk::model::EntityRef> modelFaces =
    m_modelResource->findEntitiesOfType(smtk::model::FACE);
  std::sort(modelFaces.begin(), modelFaces.end(), compareModelEntity);
  std::size_t faceIndex = 0;

  // Check model associations and reference items
  auto modelAssocAtt = m_attResource->findAttribute("model-assoc");
  smtkTest(modelAssocAtt != nullptr, "failed to find model-assoc");
  auto face = modelFaces.at(faceIndex);
  smtkTest(
    modelAssocAtt->isEntityAssociated(face), "missing association to model face" << face.name());
  faceIndex++;

  auto faceCompItem = modelAssocAtt->findComponent("face-ref");
  auto faceComp1 = faceCompItem->value();
  auto face1 = modelFaces.at(faceIndex);
  smtkTest(faceComp1 == face1.component(), "missing component item face 1: " << face1.name());
  smtkTest(std::dynamic_pointer_cast<smtk::model::Entity>(faceComp1) == face1.entityRecord(),
    "missing component item face 1:" << face1.name());
  faceIndex++;

  auto modelResourceItem = modelAssocAtt->findResource("resource-ref");
  smtkTest(modelResourceItem->value() == m_modelResource, "missing resource item");

  // Check attribute associations and reference items
  auto attributeAssocAtt = m_attResource->findAttribute("attribute-assoc");
  smtkTest(attributeAssocAtt != nullptr, "failed to find attribute-assoc");
  smtkTest(
    attributeAssocAtt->isObjectAssociated(modelAssocAtt), "missing association to model-assoc att");

  auto attCompItem = attributeAssocAtt->findComponent("attribute-ref");
  auto attComp1 = attCompItem->value();
  smtkTest(attComp1 == modelAssocAtt, "missing attribute component");

  auto attResourceItem = attributeAssocAtt->findResource("resource-ref");
  smtkTest(attResourceItem->value() == m_attResource, "missing resource item");
} // ResourceChecker::checkResources()
} // namespace

int TestResourceCopy(int /*unused*/, char** const /*unused*/)
{
  // Clear out orig and copy folders
  std::vector<std::string> folderList = { ORIG_ROOT, COPY_ROOT };
  for (auto folder : folderList)
  {
    boost::filesystem::path p(folder);
    boost::filesystem::remove_all(p);
    boost::filesystem::create_directory(p);
  }

  // Initialize smtk managers
  auto resManager = smtk::resource::Manager::create();
  auto opManager = smtk::operation::Manager::create();
  smtk::attribute::Registrar::registerTo(resManager);
  smtk::attribute::Registrar::registerTo(opManager);
  smtk::session::vtk::Registrar::registerTo(resManager);
  smtk::session::vtk::Registrar::registerTo(opManager);
  smtk::operation::Registrar::registerTo(opManager);
  opManager->registerResourceManager(resManager);

  {
    ResourceBuilder builder;
    builder.importResources();
    builder.buildAttributes();
    builder.writeResources(opManager, ORIG_ROOT, true);
    builder.copyResources(COPY_ROOT);
    builder.writeResources(opManager, COPY_ROOT);
  }

  // Make sure that resource manager is empty
  smtkTest(resManager->empty(), "Original resources still held by resource manager");

  {
    ResourceChecker checker;
    checker.readResources(opManager, COPY_ROOT);
    checker.checkResources();
  }

  // If everything passed, delete the resource files
  for (auto folder : folderList)
  {
    boost::filesystem::path p(folder);
    boost::filesystem::remove_all(p);
  }

  return 0;
}
