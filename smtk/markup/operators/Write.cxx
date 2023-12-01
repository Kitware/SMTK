//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/markup/operators/Write.h"

#include "smtk/markup/Resource.h"
#include "smtk/markup/operators/Write_xml.h"

#include "smtk/view/Manager.h"
#include "smtk/view/UIElementState.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/markup/json/jsonResource.h"

#include "smtk/resource/json/Helper.h"

#include "smtk/common/Paths.h"

#include "vtkDataObject.h"
#include "vtkDataSetWriter.h"
#include "vtkImageData.h"
#include "vtkNew.h"
#include "vtkXMLImageDataWriter.h"

using namespace smtk::model;

namespace smtk
{
namespace markup
{

bool Write::ableToOperate()
{
  if (!this->Superclass::ableToOperate())
  {
    return false;
  }

  auto associations = this->parameters()->associations();
  if (associations->numberOfValues() != 1 || !associations->isSet())
  {
    smtkWarningMacro(this->log(), "A resource must be provided.");
    return false;
  }

  auto resource = associations->valueAs<smtk::markup::Resource>();
  if (!resource || resource->location().empty())
  {
    smtkWarningMacro(this->log(), "Resource must have a valid location and be a markup resource.");
    return false;
  }

  // TODO: We could also check that the location is writable.

  return true;
}

Write::Result Write::operateInternal()
{
  auto resourceItem = this->parameters()->associations();

  smtk::markup::Resource::Ptr rsrc =
    std::dynamic_pointer_cast<smtk::markup::Resource>(resourceItem->value());

  std::string fileDirectory;
  if (smtk::common::Paths::isRelative(rsrc->location()))
  {
    // If the resource itself has a relative location, it is relative to
    // the application's current working directory:
    fileDirectory = smtk::common::Paths::currentDirectory() + "/";
  }
  else
  {
    // Otherwise, fetch the directory to which the resource will be written
    // in order to locate other files there.
    fileDirectory = smtk::common::Paths::directory(rsrc->location()) + "/";
  }
  std::string smtkDirectory = fileDirectory + smtk::common::Paths::stem(rsrc->location()) + "/";
  bool ok = smtk::common::Paths::createDirectory(smtkDirectory);
  if (!ok)
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // Write geometric (and other) data attached to URLs into smtkDirectory.
  auto urls = rsrc->filterAs<std::set<URL::Ptr>>("'" + smtk::common::typeName<URL>() + "'");
  std::set<URL*> modifiedLocations; // Keep track of URLs whose locations we set.
  for (const auto& url : urls)
  {
    // Some URLs are for locations used to import data, not the
    // resting place for data in our internally-preferred format.
    // For example, image data read from a nifti file will be
    // stored to disk in VTK's vti format. There will be a URL
    // attached to the ImageData via a URLsToData arc and another
    // URL attached to the ImageData via a URLsToImportedData arc.
    // We only care about the former.
    if (url->data().empty())
    {
      continue;
    }
    const auto* dataNode = url->data().node();
    auto dataFilename = url->location();
    if (dataFilename.data().empty())
    {
      modifiedLocations.insert(url.get());
      std::string relativePath = smtk::common::Paths::stem(rsrc->location()) + "/" +
        dataNode->name() + url->extensionForType();
      std::string fullPath = fileDirectory + relativePath;
      url->setLocation(relativePath);
      dataFilename = fullPath;
    }
    else if (smtk::common::Paths::isRelative(dataFilename.data()))
    {
      std::string fullPath = fileDirectory + dataFilename.data();
      dataFilename = fullPath;
    }
    ok = this->writeData(dataNode, dataFilename.data(), url->type());
    if (!ok)
    {
      break;
    }
  }
  if (!ok)
  {
    // Reset any previously-blank URLs.
    for (auto* url : modifiedLocations)
    {
      url->setLocation(smtk::string::Token());
    }
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  auto& helper = smtk::resource::json::Helper::pushInstance(rsrc);
  helper.setManagers(this->managers());

  // Serialize resource into a set of JSON records:
  nlohmann::json j = rsrc;

  // Save the JSON configurations of all the UI Elements in the
  // View Manager.
  auto managers = this->managers();
  if (managers)
  {
    auto viewManager = this->managers()->get<smtk::view::Manager::Ptr>();
    if (viewManager)
    {
      nlohmann::json js;
      auto& elementStateMap = viewManager->elementStateMap();
      for (auto& element : elementStateMap)
      {
        js[element.first.data()] = element.second->configuration();
      }
      j["ui_state"] = js;
    }
  }

  if (j.is_null())
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // Write JSON records to the specified URL:
  std::ofstream jsonFile(rsrc->location(), std::ios::out | std::ios::trunc);
  ok = jsonFile.good() && !jsonFile.fail();
  if (ok)
  {
    jsonFile << j;
    jsonFile.close();
  }

  smtk::resource::json::Helper::popInstance();

  return ok ? this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED)
            : this->createResult(smtk::operation::Operation::Outcome::FAILED);
}

const char* Write::xmlDescription() const
{
  return Write_xml;
}

void Write::markModifiedResources(Write::Result& /*unused*/)
{
  auto resourceItem = this->parameters()->associations();
  for (auto rit = resourceItem->begin(); rit != resourceItem->end(); ++rit)
  {
    auto resource = std::dynamic_pointer_cast<smtk::resource::Resource>(*rit);

    // Set the resource as unmodified from its persistent (i.e. on-disk) state
    resource->setClean(true);
  }
}

bool Write::writeData(
  const Component* dataNode,
  const std::string& filename,
  smtk::string::Token mimeType)
{
  (void)mimeType;
  if (!dataNode)
  {
    return false;
  }

  if (const auto* udata = dynamic_cast<const UnstructuredData*>(dataNode))
  {
    // Could use mimeType to determine file format (e.g., XML or Legacy)
    vtkNew<vtkDataSetWriter> wri;
    wri->SetFileName(filename.c_str());
    wri->SetInputDataObject(udata->shapeData());
    wri->Write();
    return true;
  }
  else if (const auto* idata = dynamic_cast<const ImageData*>(dataNode))
  {
    // Could use mimeType to determine file format (e.g., XML or Legacy)
    vtkNew<vtkXMLImageDataWriter> wri;
    wri->SetFileName(filename.c_str());
    wri->SetInputDataObject(idata->shapeData());
    wri->Write();
    return true;
  }
  smtkWarningMacro(
    this->log(), "Unhandled data node-type \"" << dataNode->typeName() << "\". Ignoring");
  return false;
}

bool write(
  const smtk::resource::ResourcePtr& resource,
  const std::shared_ptr<smtk::common::Managers>& managers)
{
  Write::Ptr write = Write::create();
  write->setManagers(managers);
  write->parameters()->associate(resource);
  Write::Result result = write->operate();
  return (result->findInt("outcome")->value() == static_cast<int>(Write::Outcome::SUCCEEDED));
}

} // namespace markup
} // namespace smtk
