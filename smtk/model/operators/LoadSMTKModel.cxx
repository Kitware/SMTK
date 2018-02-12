//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/operators/LoadSMTKModel.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/Metadata.h"

#include "smtk/model/Manager.h"

#include "smtk/model/LoadSMTKModel_xml.h"

#include "nlohmann/json.hpp"

#include <fstream>

using json = nlohmann::json;
using namespace smtk::model;

LoadSMTKModel::LoadSMTKModel()
{
}

smtk::operation::Operation::Result LoadSMTKModel::operateInternal()
{
  auto params = this->parameters();
  auto fileItem = params->findFile("filename");
  auto filename = fileItem->value(0);

  json j;
  // Scope so file is only open for a short time:
  {
    std::ifstream file(filename, std::ios::in);
    if (!file.good())
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(), "Could not open file \"" << filename << "\" for writing.");
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }
    j = json::parse(file);
  }

  /*
  int rr = 0;
  for (auto rit = rsrcItem->begin(); rit != rsrcItem->end(); ++rit, ++rr)
  {
    auto rsrc = *rit;
    auto rsrcMgr = rsrc->manager();
    if (!rsrcMgr)
    {
      smtkErrorMacro(smtk::io::Logger::instance(),
        "Resource " << rsrc->uniqueName() << " \"" << rsrc->location() << "\" has no manager"
        " which is required in order to discern how it should be read.");
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }

    auto rsrcMetaMeta = rsrcMgr->metadata();
    auto& rsrcMeta = rsrcMetaMeta.get<smtk::resource::IndexTag>();
    auto meta = rsrcMeta.find(rsrc->index());
    if (meta == rsrcMeta.end())
    {
      smtkErrorMacro(smtk::io::Logger::instance(),
        "Resource " << rsrc->uniqueName() << " \"" << rsrc->location() << "\" has no metadata"
        " registered with its manager, which is required in order to discern how it should be read.");
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }

    if (!meta->read)
    {
      smtkErrorMacro(smtk::io::Logger::instance(),
        "Resource metadata for " << rsrc->uniqueName() << " has a null read method.");
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }

    std::string filename =  fileItem->value(rr);

    if (filename.empty())
    {
        smtkErrorMacro(smtk::io::Logger::instance(), "An empty filename is not allowed.");
        return this->createResult(smtk::operation::Operation::Outcome::FAILED);
      rsrc->setLocation(filename);
    }

    if (!meta->write(rsrc))
    {
      // The writer will have logged an error message.
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }
  }
*/
  return this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
}

const char* LoadSMTKModel::xmlDescription() const
{
  return LoadSMTKModel_xml;
}

void LoadSMTKModel::generateSummary(LoadSMTKModel::Result& res)
{
  int outcome = res->findInt("outcome")->value();
  smtk::attribute::FileItemPtr fitem = this->parameters()->findFile("filename");
  std::string label = this->parameters()->definition()->label();
  if (outcome == static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    smtkInfoMacro(this->log(), label << ": loaded \"" << fitem->value(0) << "\"");
  }
  else
  {
    smtkErrorMacro(this->log(), label << ": failed to load \"" << fitem->value(0) << "\"");
  }
}
