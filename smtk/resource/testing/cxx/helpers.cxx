//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/resource/testing/cxx/helpers.h"

#include "smtk/model/Registrar.h"
#include "smtk/model/Session.h"
#include "smtk/model/SessionRef.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/operation/operators/ReadResource.h"

#include "smtk/plugin/Registry.h"

#include "smtk/resource/Metadata.h"

namespace smtk
{
namespace resource
{
namespace testing
{

ResourceArray
loadTestResources(smtk::resource::Manager::Ptr resourceManager, int argc, char* argv[])
{
  auto registry = smtk::plugin::addToManagers<smtk::model::Registrar>(resourceManager);

  ResourceArray result;
  if (argc < 2)
  {
    std::cerr << "No files to load.\n";
    return result;
  }

  smtk::operation::ReadResource::Ptr read = smtk::operation::ReadResource::create();
  read->parameters()->findFile("filename")->setValue(argv[1]);
  auto opResult = read->operate();
  result.push_back(opResult->findResource("resourcesCreated")->value(0));

  return result;
}
} // namespace testing
} // namespace resource
} // namespace smtk
