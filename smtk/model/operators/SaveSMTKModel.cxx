//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/operators/SaveSMTKModel.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/Metadata.h"

#include "smtk/model/Manager.h"

#include "smtk/model/SaveSMTKModel_xml.h"

#include "smtk/model/json/jsonResource.h"

#include <fstream>

using namespace smtk::model;
using json = nlohmann::json;

SaveSMTKModel::SaveSMTKModel()
{
}

smtk::operation::Operation::Result SaveSMTKModel::operateInternal()
{
  auto params = this->parameters();
  auto fileItem = params->findFile("filename");
  auto setFilename = fileItem->isEnabled();
  auto rsrcItem = params->findResource("resources");

  if (rsrcItem->numberOfValues() < 1)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(), "At least one resource must be selected for saving.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  if (setFilename && rsrcItem->numberOfValues() != fileItem->numberOfValues())
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(), "Number of filenames must match number of resources.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  /*
  std::ifstream file(filename, std::ios::out);
  if (!file.good())
  {
    smtkErrorMacro(smtk::io::Logger::instance(),
      "Could not open file \"" << filename << "\" for writing.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }
  */

  int rr = 0;
  for (auto rit = rsrcItem->begin(); rit != rsrcItem->end(); ++rit, ++rr)
  {
    auto rsrc = *rit;
    auto rsrcMgr = rsrc->manager();
    if (!rsrcMgr)
    {
      smtkErrorMacro(smtk::io::Logger::instance(), "Resource "
          << rsrc->uniqueName() << " \"" << rsrc->location()
          << "\" has no manager"
             " which is required in order to discern how it should be saved.");
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }

    auto rsrcMetaMeta = rsrcMgr->metadata();
    auto& rsrcMeta = rsrcMetaMeta.get<smtk::resource::IndexTag>();
    auto meta = rsrcMeta.find(rsrc->index());
    if (meta == rsrcMeta.end())
    {
      smtkErrorMacro(smtk::io::Logger::instance(), "Resource "
          << rsrc->uniqueName() << " \"" << rsrc->location()
          << "\" has no metadata"
             " registered with its manager, which is required in order to discern how it should be "
             "saved.");
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }

    if (!meta->write)
    {
      smtkErrorMacro(smtk::io::Logger::instance(), "Resource metadata for "
          << rsrc->uniqueName() << " has a null write method.");
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }

    if (rsrc->location().empty())
    {
      auto filename = setFilename ? fileItem->value(rr) : "";
      if (filename.empty())
      {
        smtkErrorMacro(smtk::io::Logger::instance(), "An empty filename is not allowed.");
        return this->createResult(smtk::operation::Operation::Outcome::FAILED);
      }
      rsrc->setLocation(filename);
    }

    if (!meta->write(rsrc))
    {
      // The writer will have logged an error message.
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }
  }
  return this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
}

const char* SaveSMTKModel::xmlDescription() const
{
  return SaveSMTKModel_xml;
}

void SaveSMTKModel::generateSummary(SaveSMTKModel::Result& res)
{
  std::ostringstream msg;
  int outcome = res->findInt("outcome")->value();
  smtk::attribute::FileItemPtr fitem = this->parameters()->findFile("filename");
  msg << this->parameters()->definition()->label();
  if (outcome == static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    msg << ": wrote \"" << fitem->value(0) << "\"";
    smtkInfoMacro(this->log(), msg.str());
  }
  else
  {
    msg << ": failed to write \"" << fitem->value(0) << "\"";
    smtkErrorMacro(this->log(), msg.str());
  }
}
