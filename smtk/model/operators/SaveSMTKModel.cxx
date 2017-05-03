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
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/attribute/VoidItem.h"

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

namespace smtk
{
namespace model
{

class SaveSMTKModel::Internals
{
public:
  std::map<smtk::model::EntityRef, smtk::model::StringData> m_modelChanges;
  std::map<smtk::mesh::CollectionPtr, smtk::model::StringData> m_meshChanges;
  std::map<std::string, std::string> m_copyFiles;
  std::map<std::string, std::string> m_saveModels;
  std::map<smtk::mesh::CollectionPtr, std::string> m_saveMeshes;
  bool m_undoEdits;
  bool m_didCopy;

  std::string m_smtkFilename;
  std::string m_embedDir;
};

SaveSMTKModel::SaveSMTKModel()
{
  this->m_data = new SaveSMTKModel::Internals;
}

void SaveSMTKModel::extractChanges()
{
  this->m_data->m_undoEdits = this->findVoid("undo edits")->isEnabled();

  smtk::attribute::GroupItemPtr propEdits = this->findGroup("property edits");
  std::size_t numEntities = propEdits->numberOfGroups();
  for (std::size_t ii = 0; ii < numEntities; ++ii)
  {
    EntityRef ent = propEdits->findAs<smtk::attribute::ModelEntityItem>(ii, "edit entity")->value();
    if (ent.isValid())
    {
      this->m_data->m_modelChanges[ent] = StringData();
      smtk::attribute::GroupItemPtr valuePairs =
        propEdits->findAs<smtk::attribute::GroupItem>(ii, "value pairs");
      std::size_t numKeys = valuePairs->numberOfGroups();
      for (std::size_t jj = 0; jj < numKeys; ++jj)
      {
        std::string propKey =
          valuePairs->findAs<smtk::attribute::StringItem>(jj, "edit property")->value();
        smtk::attribute::StringItem::Ptr vals =
          valuePairs->findAs<smtk::attribute::StringItem>(jj, "edit values");
        this->m_data->m_modelChanges[ent][propKey] = StringList(vals->begin(), vals->end());
      }
    }
    /*
		smtk::mesh::CollectionPtr coll = mgr->meshManager()->collection(ent.entity());
    if (coll)
    {
      this->m_data->m_meshChanges[ent] = StringData();
      smtk::attribute::GroupItemPtr valuePairs =
        propEdits->findAs<smtk::attribute::GroupItemPtr>(ii, "value pairs");
      std::size_t numKeys = valuePairs->numberOfGroups();
      for (std::size_t jj = 0; jj < numKeys; ++jj)
      {
        std::string propKey =
		  		valuePairs->findAs<smtk::attribute::StringItem>(jj, "edit property")->value();
        smtk::attribute::StringItem::Ptr vals =
          valuePairs->findAs<smtk::attribute::StringItem>(jj, "edit values");
        this->m_data->m_meshChanges[ent][propKey] = StringList(vals->begin(), vals->end());
      }
		}
    */
  }

  smtk::attribute::StringItemPtr copyFilesItem = this->findString("copy files");
  int numFiles = static_cast<int>(copyFilesItem->numberOfValues()) / 2;
  for (int ii = 0; ii < numFiles; ++ii)
  {
    this->m_data->m_copyFiles[copyFilesItem->value(2 * ii)] = copyFilesItem->value(2 * ii + 1);
  }

  smtk::attribute::StringItemPtr saveModelsItem = this->findString("save models");
  int numModels = static_cast<int>(saveModelsItem->numberOfValues()) / 2;
  for (int ii = 0; ii < numModels; ++ii)
  {
    this->m_data->m_saveModels[saveModelsItem->value(2 * ii)] = saveModelsItem->value(2 * ii + 1);
  }

  smtk::attribute::MeshItemPtr saveMeshesItem = this->findMesh("save meshes");
  smtk::attribute::StringItemPtr saveMeshURLsItem = this->findString("save mesh urls");
  int numMeshes = static_cast<int>(saveMeshesItem->numberOfValues());
  for (int ii = 0; ii < numMeshes; ++ii)
  {
    smtk::mesh::CollectionPtr coll = saveMeshesItem->value(ii).collection();
    this->m_data->m_saveMeshes[coll] = saveMeshURLsItem->value(ii);
  }
}

bool SaveSMTKModel::applyChanges(smtk::model::EntityRefs& modified)
{
  bool ok = true;
  // I. Ensure this->m_data's output path and filename are updated from the operator attribute.
  Models models = this->m_specification->associatedModelEntities<Models>();
  this->m_data->m_smtkFilename = this->findFile("filename")->value(0);
  this->m_data->m_embedDir =
    ::boost::filesystem::path(this->m_data->m_smtkFilename).parent_path().string();

  // II. Apply changes from this->m_data (or unapply)
  //     a. Update string properties in the model system (name, url, smtk_url):
  std::map<smtk::model::EntityRef, smtk::model::StringData>::iterator mcit;
  for (mcit = this->m_data->m_modelChanges.begin(); mcit != this->m_data->m_modelChanges.end();
       ++mcit)
  {
    EntityRef ent(mcit->first);
    if (!this->m_data->m_undoEdits)
    {
      modified.insert(modified.end(), ent);
    }
    StringData::iterator kvit;
    for (kvit = mcit->second.begin(); kvit != mcit->second.end(); ++kvit)
    {
      if (kvit->second.empty())
      { // Property did not exist before; remove it.
        if (ent.hasStringProperty(kvit->first))
        {
          kvit->second = ent.stringProperty(kvit->first);
          ent.removeStringProperty(kvit->first);
        }
      }
      else
      { // Swap value between kvit and entity.
        if (ent.hasStringProperty(kvit->first))
        {
          StringList tmp(kvit->second);
          kvit->second = ent.stringProperty(kvit->first);
          ent.setStringProperty(kvit->first, tmp);
        }
        else
        {
          ent.setStringProperty(kvit->first, kvit->second);
          kvit->second.clear();
        }
      }
    }
  }

  // II. b. Update mesh locations.
  // TODO: Changes to mesh names?
  std::map<smtk::mesh::CollectionPtr, std::string>::iterator smit;
  for (smit = this->m_data->m_saveMeshes.begin(); smit != this->m_data->m_saveMeshes.end(); ++smit)
  {
    if (smit->first)
    {
      std::string prevURL = smit->first->writeLocation().absolutePath();
      ::boost::filesystem::path nextURL(smit->second);
      if (nextURL.is_relative())
      {
        nextURL = this->m_data->m_embedDir / nextURL;
      }
      smit->first->writeLocation(nextURL.string());
      smit->second = prevURL;
    }
  }

  // III. Copy files as instructed. This is only done once per operation.
  if (!this->m_data->m_didCopy)
  {
    this->m_data->m_didCopy = true;
    std::map<std::string, std::string>::const_iterator cfit;
    for (cfit = this->m_data->m_copyFiles.begin(); cfit != this->m_data->m_copyFiles.end(); ++cfit)
    {
      ::boost::filesystem::path src(cfit->first);
      ::boost::filesystem::path dst(cfit->second);
      if (::boost::filesystem::exists(src))
      {
        if (dst.is_relative())
        {
          dst = this->m_data->m_embedDir / dst;
        }
        // std::cout << "Copying data from " << src << " to " << dst << "\n";
        ::boost::filesystem::create_directories(dst.parent_path());
        ::boost::filesystem::copy_file(
          src, dst, ::boost::filesystem::copy_option::overwrite_if_exists);
      }
    }
  }
  return ok;
}

OperatorResult SaveSMTKModel::operateInternal()
{
  smtk::attribute::FileItemPtr filenameItem = this->findFile("filename");

  this->m_data->m_didCopy = false;

  // Make changes required by operator
  EntityRefs modif;
  this->extractChanges();
  bool ok = this->applyChanges(modif);

  Models models = this->m_specification->associatedModelEntities<Models>();
  if (models.empty())
  {
    smtkErrorMacro(this->log(), "No valid models selected for export.");
    return this->createResult(OPERATION_FAILED);
  }

  std::ofstream jsonFile(this->m_data->m_smtkFilename.c_str(), std::ios::trunc);
  if (!jsonFile.good())
  {
    smtkErrorMacro(this->log(), "Could not open file \"" << this->m_data->m_smtkFilename << "\".");
    return this->createResult(OPERATION_FAILED);
  }

  cJSON* top = cJSON_CreateObject();
  /*
  std::string smtkfilepath = path(filename).parent_path().string();
  std::string smtkfilename = path(filename).stem().string();

  smtk::io::SaveJSON::forManagerSessionPartial(this->session()->sessionId(),
    this->m_specification->associatedModelEntityIds(), top, this->manager(), true, smtkfilepath);
    */
  if (smtk::io::SaveJSON::save(top, models, /* renameModels */ false, this->m_data->m_embedDir))
  {
    ok = true;
  }

  smtk::common::ResourceSetPtr rset = this->manager()->resources();
  smtk::io::SaveJSON::fromResourceSet(top, rset);

  if (ok)
  {
    char* json = cJSON_Print(top);
    jsonFile << json;
    free(json);
    cJSON_Delete(top);
    jsonFile.close();
  }

  if (this->m_data->m_undoEdits)
  { // Revert temporary changes
    this->applyChanges(modif);
  }

  auto result = this->createResult(OPERATION_SUCCEEDED);
  // Note that modified entities will be marked clean in
  // the model manager (i.e., unmodified since last save):
  for (auto ent : modif)
  {
    this->addEntityToResult(result, ent, MODIFIED);
  }
  result->findVoid("cleanse entities")->setIsEnabled(this->m_data->m_undoEdits);
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

smtkImplementsModelOperator(SMTKCORE_EXPORT, smtk::model::SaveSMTKModel, save_smtk_model,
  "save smtk model", SaveSMTKModel_xml, smtk::model::Session);
