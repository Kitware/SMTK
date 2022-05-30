//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/PublicPointerDefs.h"

#include "smtk/operation/operators/ReadResource.h"

#include "smtk/model/Resource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/ReferenceItemDefinition.h"
#include "smtk/attribute/Registrar.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/UnsetValueError.h"

#include "smtk/session/polygon/Registrar.h"

#include "smtk/plugin/Registry.h"

#include "smtk/io/Logger.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Registrar.h"
#include "smtk/resource/Manager.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <algorithm>

using namespace smtk::attribute;

static std::vector<char*> dataArgs;

int unitUnsetValueError(int argc, char* argv[])
{
  // Load in a model file so we can associate model entities to test attributes.
  if (argc < 2)
  {
    std::string testFile;
    testFile = SMTK_DATA_DIR;
    testFile += "/model/2d/smtk/epic-trex-drummer.smtk";
    dataArgs.push_back(strdup(argv[0]));
    dataArgs.push_back(strdup(testFile.c_str()));
    dataArgs.push_back(nullptr);
    argc = 2;
    argv = dataArgs.data();
  }
  auto resourceManager = smtk::resource::Manager::create();
  auto operationManager = smtk::operation::Manager::create();
  operationManager->registerResourceManager(resourceManager);

  auto attributeRegistry = smtk::plugin::
    Registry<smtk::attribute::Registrar, smtk::resource::Manager, smtk::operation::Manager>(
      resourceManager, operationManager);
  auto operationRegistry = smtk::plugin::
    Registry<smtk::operation::Registrar, smtk::resource::Manager, smtk::operation::Manager>(
      resourceManager, operationManager);
  auto polygonRegistry = smtk::plugin::
    Registry<smtk::session::polygon::Registrar, smtk::resource::Manager, smtk::operation::Manager>(
      resourceManager, operationManager);

  for (int i = 1; i < argc; i++)
  {
    auto read = operationManager->create<smtk::operation::ReadResource>();
    read->parameters()->findFile("filename")->setValue(argv[i]);
    read->operate();
  }

  smtk::attribute::ResourcePtr resourcePtr = resourceManager->create<smtk::attribute::Resource>();
  smtk::attribute::Resource& resource(*resourcePtr.get());

  // Create an attribute
  smtk::attribute::DefinitionPtr definition = resource.createDefinition("Def");
  definition->setBriefDescription("Sample Definition");
  definition->setDetailedDescription("Sample Definition for testing\nThere is not much here!");

  // Add a ReferenceItemDefinition
  auto referenceItemDefinition =
    definition->addItemDefinition<smtk::attribute::ReferenceItemDefinition>("Faces");
  referenceItemDefinition->setIsOptional(true);
  referenceItemDefinition->setIsExtensible(true);
  referenceItemDefinition->setNumberOfRequiredValues(2);
  referenceItemDefinition->setMaxNumberOfValues(0);
  referenceItemDefinition->setAcceptsEntries("smtk::model::Resource", "face", true);

  // Lets test creating an attribute by passing in the expression definition explicitly
  smtk::attribute::AttributePtr attribute = resource.createAttribute("attribute", definition);

  auto referenceItem =
    smtk::dynamic_pointer_cast<smtk::attribute::ReferenceItem>(attribute->find("Faces"));
  referenceItem->setIsEnabled(true);
  referenceItem->setNumberOfValues(3);

  auto basicRsrc = resourceManager->get(argv[1]);
  test(basicRsrc != nullptr, "Failed to access basic resource");
  auto modelRsrc = std::dynamic_pointer_cast<smtk::model::Resource>(basicRsrc);
  test(modelRsrc != nullptr, "Failed to cast model resource");
  auto allFaces =
    modelRsrc->entitiesMatchingFlagsAs<smtk::model::EntityRefArray>(smtk::model::FACE);
  std::cout << "Model " << modelRsrc->id().toString() << " has " << allFaces.size() << " faces\n";

  for (std::size_t i = 0; i < referenceItem->numberOfValues() - 1; ++i)
  {
    referenceItem->setValue(i, allFaces[i].component());
  }

  bool errorCaught = false;

  try
  {
    for (auto it = referenceItem->begin(); it != referenceItem->end(); ++it)
    {
      std::cout << (*it)->name() << std::endl;
    }
  }
  catch (smtk::attribute::UnsetValueError&)
  {
    errorCaught = true;
  }

  test(errorCaught, "Failed to catch \"UnsetValueError\" exception");

  std::size_t numberOfUnsetValues = 0;
  for (auto it = referenceItem->begin(); it != referenceItem->end(); ++it)
  {
    if (!it.isSet())
    {
      ++numberOfUnsetValues;
    }
  }

  test(numberOfUnsetValues == 1, "Incorrect number of unset values counted through iteration");

  while (!dataArgs.empty())
  {
    char* val = dataArgs.back();
    if (val)
    {
      free(val);
    }
    dataArgs.pop_back();
  }
  return 0;
}
