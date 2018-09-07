//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "Write.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/session/vtk/Resource.h"
#include "smtk/session/vtk/Session.h"
#include "smtk/session/vtk/Write_xml.h"
#include "smtk/session/vtk/json/jsonResource.h"
#include "smtk/session/vtk/operators/Export.h"

#include "smtk/common/Paths.h"

#include "smtk/model/SessionIOJSON.h"

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>

using namespace smtk::model;

namespace
{
static void RetrievePreservedUUID(vtkDataObject* data, std::vector<smtk::common::UUID>& uuids)
{
  if (!data)
    return;

  vtkInformation* info = data->GetInformation();
  uuids.push_back(
    smtk::common::UUID(std::string(info->Get(smtk::session::vtk::Session::SMTK_UUID_KEY()))));
}

static void RetrievePreservedUUIDsRecursive(
  vtkDataObject* data, std::vector<smtk::common::UUID>& uuids)
{
  RetrievePreservedUUID(data, uuids);

  vtkMultiBlockDataSet* mbds = vtkMultiBlockDataSet::SafeDownCast(data);
  if (mbds)
  {
    int nb = mbds->GetNumberOfBlocks();
    for (int i = 0; i < nb; ++i)
    {
      RetrievePreservedUUIDsRecursive(mbds->GetBlock(i), uuids);
    }
  }
}
}

namespace smtk
{
namespace session
{
namespace vtk
{

bool Write::ableToOperate()
{
  if (!this->Superclass::ableToOperate())
  {
    return false;
  }

  if (this->parameters()->associations()->numberOfValues() < 1)
  {
    return false;
  }

  return true;
}

Write::Result Write::operateInternal()
{
  auto resourceItem = this->parameters()->associations();

  smtk::session::vtk::Resource::Ptr rsrc =
    std::dynamic_pointer_cast<smtk::session::vtk::Resource>(resourceItem->objectValue());

  // Serialize resource into a set of JSON records:
  smtk::model::SessionIOJSON::json j = rsrc;

  if (j.is_null())
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  std::vector<smtk::common::UUID> preservedUUIDs;
  smtk::common::UUIDs modelIds = rsrc->entitiesMatchingFlags(smtk::model::MODEL_ENTITY);
  for (auto& id : modelIds)
  {
    smtk::model::Model dataset = smtk::model::Model(rsrc, id);
    EntityHandle handle = rsrc->session()->toEntity(dataset);
    vtkMultiBlockDataSet* mbds = handle.object<vtkMultiBlockDataSet>();
    RetrievePreservedUUIDsRecursive(mbds, preservedUUIDs);
  }

  std::vector<std::string> preservedUUIDsStr;
  for (auto& id : preservedUUIDs)
  {
    preservedUUIDsStr.push_back(id.toString());
  }
  j["preservedUUIDs"] = preservedUUIDsStr;

  std::string fileDirectory = smtk::common::Paths::directory(rsrc->location()) + "/";

  std::vector<std::string> modelFiles;

  for (auto& id : modelIds)
  {
    smtk::model::Model dataset = smtk::model::Model(rsrc, id);

    std::string modelFile =
      fileDirectory + id.toString() + rsrc->session()->defaultFileExtension(dataset);

    static const bool exportToExodus = false;
    if (exportToExodus)
    {
      Export::Ptr exportOp = Export::create();
      exportOp->parameters()->findString("filetype")->setValue("");

      exportOp->parameters()->associate(dataset.entityRecord());
      exportOp->parameters()->findFile("filename")->setValue(modelFile);
      Result exportOpResult = exportOp->operate();

      if (exportOpResult->findInt("outcome")->value() != static_cast<int>(Outcome::SUCCEEDED))
      {
        smtkErrorMacro(log(), "Cannot export file \"" << modelFile << "\".");
        return this->createResult(smtk::operation::Operation::Outcome::FAILED);
      }
    }
    else
    {
      std::string url = dataset.stringProperty("url")[0];
      if (!boost::filesystem::is_regular_file(url))
      {
        smtkErrorMacro(log(), "Cannot copy file \"" << url << "\".");
        return this->createResult(smtk::operation::Operation::Outcome::FAILED);
      }
      if (!boost::filesystem::is_regular_file(modelFile))
      {
        boost::filesystem::copy_file(url, modelFile);
      }
    }

    modelFiles.push_back(id.toString() + rsrc->session()->defaultFileExtension(dataset));
  }

  j["modelFiles"] = modelFiles;

  // Write JSON records to the specified URL:
  bool ok = smtk::model::SessionIOJSON::saveModelRecords(j, rsrc->location());

  return ok ? this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED)
            : this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
}

const char* Write::xmlDescription() const
{
  return Write_xml;
}

bool write(const smtk::resource::ResourcePtr& resource)
{
  Write::Ptr write = Write::create();
  write->parameters()->associate(resource);
  Write::Result result = write->operate();
  return (result->findInt("outcome")->value() == static_cast<int>(Write::Outcome::SUCCEEDED));
}

} // namespace vtk
} // namespace session
} // namespace smtk
