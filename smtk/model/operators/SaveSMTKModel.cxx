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
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"

#include "smtk/io/SaveJSON.h"
#include "smtk/io/SaveJSON.txx"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Session.h"

#include "smtk/common/CompilerInformation.h"
#include "smtk/common/ResourceSet.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "boost/filesystem.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

#include "cJSON.h"
#include <fstream>

using namespace smtk::model;
using namespace boost::filesystem;

namespace smtk {
  namespace model {

OperatorResult SaveSMTKModel::operateInternal()
{
  smtk::attribute::FileItemPtr filenameItem = this->findFile("filename");
  smtk::attribute::IntItemPtr flagsItem = this->findInt("flags");
  smtk::attribute::StringItemPtr renameItem = this->findString("rename models");

  Models models = this->m_specification->associatedModelEntities<Models>();
  if (models.empty())
    {
    smtkErrorMacro(this->log(), "No valid models selected for export.");
    return this->createResult(OPERATION_FAILED);
    }

  std::string filename = filenameItem->value();
  if (filename.empty())
    {
    smtkErrorMacro(this->log(), "A filename must be provided.");
    return this->createResult(OPERATION_FAILED);
    }

  std::ofstream jsonFile(filename.c_str(), std::ios::trunc);
  if (!jsonFile.good())
    {
    smtkErrorMacro(this->log(), "Could not open file \"" << filename << "\".");
    return this->createResult(OPERATION_FAILED);
    }

  cJSON* top = cJSON_CreateObject();
  std::string smtkfilepath = path(filename).parent_path().string();
  std::string smtkfilename = path(filename).stem().string();

  smtk::attribute::ValueItemDefinition::ConstPtr renameDef =
    std::dynamic_pointer_cast<const smtk::attribute::ValueItemDefinition>(
      renameItem->definition());
  std::string renamePolicy = renameDef->discreteEnum(renameItem->discreteIndex(0));
  smtkDebugMacro(this->log(), "Rename policy: " << renamePolicy);

  // Add the output smtk model name to the model "smtk_url", so that the individual session can
  // use that name to construct a filename for saving native models of the session.
  Models::iterator modit;
  bool plural = models.size() > 1;
  int counter = 0;
  smtk::model::EntityRefArray modified;
  for(modit = models.begin(); modit != models.end(); ++modit, ++counter)
    {
    modit->setStringProperty("smtk_url", filename);

    if (renamePolicy != "none")
      {
      std::string oldfilename = filename;
      if (modit->hasStringProperty("url"))
        {
        oldfilename = path(modit->stringProperty("url")[0]).stem().string();
        }
      std::string oldmodelname = modit->name();
      bool matchDefault = false;
      bool matchPrevious = false;
      const std::string defaultprefix("model ");
      if (
        renamePolicy == "all" ||
        (matchDefault = std::equal(defaultprefix.begin(), defaultprefix.end(), oldmodelname.begin())) ||
        (matchPrevious = std::equal(oldfilename.begin(), oldfilename.end(), oldmodelname.begin())))
        {
        std::ostringstream newname;
        newname << smtkfilename;
        std::string suffix;
        if (matchDefault)
          {
          suffix = oldmodelname.substr(defaultprefix.size() - 1); // include space after prefix
          }
        else if (matchPrevious)
          {
          suffix = oldmodelname.substr(oldfilename.size());
          }
        if (!suffix.empty())
          {
          newname << suffix;
          }
        else if (plural)
          { // TODO: Use pedigree ID if present? and unique?
          newname << " " << counter;
          }
        smtkDebugMacro(this->log(), "Renaming " << oldmodelname << " to " << newname.str());
        modit->setName(newname.str());
        }
      }

    // we also want to write out the meshes to new "write_locations"
    std::vector<smtk::mesh::CollectionPtr> collections =
      this->manager()->meshes()->associatedCollections(*modit);
    std::vector<smtk::mesh::CollectionPtr>::const_iterator cit;
    for(cit = collections.begin(); cit != collections.end(); ++cit)
      {
      std::ostringstream outmeshname;
      outmeshname << smtkfilename << "_" << (*cit)->name() << ".h5m";
      std::string write_path = (path(smtkfilepath) / path(outmeshname.str())).string();
      smtk::common::FileLocation wfLocation(write_path, smtkfilepath);
      (*cit)->writeLocation(wfLocation);
      }

    modified.push_back(*modit); // All models saved should be marked clean.
    }

  smtk::io::SaveJSON::forManagerSessionPartial(
    this->session()->sessionId(),
    this->m_specification->associatedModelEntityIds(),
    top, this->manager(), true, smtkfilepath);

  smtk::common::ResourceSetPtr rset = this->manager()->resources();
  smtk::io::SaveJSON::fromResourceSet(top, rset);

  char* json = cJSON_Print(top);
  jsonFile << json;
  free(json);
  cJSON_Delete(top);
  jsonFile.close();

  // Now we need to do some work to reset the url property of models since
  // during export, the property may be changed to be relative path, and we want
  // to set it back to be absolute path to display
  if(!smtkfilepath.empty())
    {
    for(modit = models.begin(); modit != models.end(); ++modit)
      {
      if(modit->hasStringProperty("url"))
        {
        path url(modit->stringProperty("url")[0]);
        if (!url.string().empty() && !url.is_absolute())
          {
          url = smtkfilepath / url;
          url = canonical(url, smtkfilepath);
          // set the url property to be consistent with "modelFiles" record when written out
          modit->setStringProperty("url", url.string());
          }
        }

      // After a collection is written out successfully or not,
      // we want to clear the writeLocation so that it won't be re-written
      // unless explicitly set again.
      std::vector<smtk::mesh::CollectionPtr> collections =
        this->manager()->meshes()->associatedCollections(*modit);
      std::vector<smtk::mesh::CollectionPtr>::const_iterator cit;
      for(cit = collections.begin(); cit != collections.end(); ++cit)
        {
        (*cit)->writeLocation(std::string());
        }
      }
    }

  auto result = this->createResult(OPERATION_SUCCEEDED);
  this->addEntitiesToResult(result, modified, MODIFIED);
  return result;
}

void SaveSMTKModel::generateSummary(OperatorResult& res)
{
  std::ostringstream msg;
  int outcome = res->findInt("outcome")->value();
  smtk::attribute::FileItemPtr fitem = this->findFile("filename");
  msg << this->specification()->definition()->label();
  if (outcome == static_cast<int>(OPERATION_SUCCEEDED))
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

  } //namespace model
} // namespace smtk

#include "smtk/model/SaveSMTKModel_xml.h"

smtkImplementsModelOperator(
  SMTKCORE_EXPORT,
  smtk::model::SaveSMTKModel,
  save_smtk_model,
  "save smtk model",
  SaveSMTKModel_xml,
  smtk::model::Session);
