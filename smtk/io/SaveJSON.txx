#ifndef __smtk_io_SaveJSON_txx
#define __smtk_io_SaveJSON_txx

#include "smtk/io/SaveJSON.h"

#include "smtk/common/FileLocation.h"

#include "smtk/io/Helpers.h"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"

#include "smtk/model/EntityIterator.h"

#include "boost/filesystem.hpp"

#include "cJSON.h"

namespace smtk
{
namespace io
{

template <typename T>
bool SaveJSON::prepareToSave(const smtk::model::Models& modelsToSave,
  const std::string& mode,     // "save", "save as", or "save a copy"
  const std::string& filename, // only used when mode == "save as" or "save a copy"
  const std::string& renamePolicy, bool embedData,
  T& obj // structure whose ivars will contain changes to be made before/during/after saving.
  )
{
  int ok = true; // Assume it's OK to save unless we find a problem.
  if ((mode == "save as" || mode == "save a copy") && filename.empty())
  {
    ok = false;
    obj.m_saveErrors << "<li><span style=\"color:" << obj.m_errorColor << ";\">"
                     << "You must provide a name for the output SMTK file."
                     << "</span></li>\n";
  }
  int modelCounter = 0;
  std::set<std::string> preExistingFilenames;
  std::set<smtk::mesh::CollectionPtr> meshCollections;
  obj.m_smtkFilename.clear();
  ::boost::filesystem::path fullSMTKPath;
  for (auto model : modelsToSave)
  {
    obj.m_modelChanges[model] = smtk::model::StringData();
    // I. Determine the name of the ".smtk" file for the current model
    //    Once this is determined, we also know the "embedding"
    //    directory (i.e., the directory where package files must go).
    ::boost::filesystem::path prevSMTKPath;
    ::boost::filesystem::path prevEmbedDir;
    if (model.hasStringProperty("smtk_url"))
    {
      prevSMTKPath = model.stringProperty("smtk_url")[0];
      prevEmbedDir = prevSMTKPath.parent_path();
    }
    if (mode == "save" && !model.hasStringProperty("smtk_url"))
    {
      ok = false;
      obj.m_saveErrors << "<li><span style=\"color:" << obj.m_errorColor << ";\">" << model.name()
                       << " has never been saved before. Use \"save as\" or \"save a copy\""
                       << "</span></li>\n";
      continue;
    }
    ::boost::filesystem::path smtkPath(
      mode == "save" ? model.stringProperty("smtk_url")[0] : filename);
    if (fullSMTKPath.empty())
    {
      fullSMTKPath = smtkPath;
    }
    else if (fullSMTKPath != smtkPath)
    {
      ok = false;
      obj.m_saveErrors << "<li><span style=\"color:" << obj.m_errorColor << ";\">"
                       << "Models with different, pre-existing SMTK files were chosen. "
                       << "Currently, only saving to a single SMTK file is supported."
                       << "</span></li>\n";
    }

    ::boost::filesystem::path embedDir = smtkPath.parent_path();
    obj.m_embedDir = embedDir.string();

    std::string smtkStem = smtkPath.stem().string();
    bool haveStem = true;
    if (smtkStem.empty())
    {
      haveStem = false;
      smtkStem = "smtk_model";
    }
    obj.m_modelChanges[model]["smtk_url"] = smtk::model::StringList(1, smtkPath.string());
    preExistingFilenames.insert(smtkPath.filename().string());

    // II. If the model has a session-specific URL, determine
    //     its path relative to the embedding dir. This may also
    //     involve rewriting the URL if we are packaging.
    std::string defaultFileExtension = model.owningSession().defaultFileExtension(model);
    if (!defaultFileExtension.empty() && model.hasStringProperty("url"))
    {
      std::string modelURL = model.stringProperty("url")[0];
      std::string relPath;
      ::boost::filesystem::path url = ::boost::filesystem::weakly_canonical(modelURL);
      if (url.is_relative())
      {
        url = prevEmbedDir / url;
      }
      bool alreadyEmbedded =
        Helpers::isDirectoryASubdirectory(embedDir.string(), url.parent_path().string(), relPath);
      if (embedData)
      {
        /*
        std::cout
          << "Is URL <" << url.parent_path().string() << "> inside EmbedDir <" << embedDir << ">? "
          << (alreadyEmbedded ? "Y" : "N")
          << "   --- " << relPath << "\n";
          */
        if (ok && !alreadyEmbedded)
        {
          std::string embeddedURL;
          if (obj.m_saveModels.find(modelURL) == obj.m_saveModels.end())
          { // We haven't seen this model URL before.
            embeddedURL = smtk::io::Helpers::uniqueFilename(url.filename().string(),
              preExistingFilenames, smtkStem, defaultFileExtension, obj.m_embedDir);
            obj.m_saveModels[modelURL] = embeddedURL;
            smtk::model::SessionRef sref = model.owningSession();
            if (!sref.op("write") && !sref.op("export") && !sref.op("save"))
            {
              // We don't have an operator to save the model, so
              // assume it hasn't changed and copy the file to its
              // embedded location (if it exists in its original location).
              ::boost::filesystem::path prevURL(modelURL);
              if (prevURL.is_relative() && model.hasStringProperty("smtk_url"))
              {
                prevURL = prevEmbedDir / modelURL;
              }
              if (::boost::filesystem::exists(prevURL))
              {
                obj.m_copyFiles[prevURL.string()] = (embedDir / embeddedURL).string();
              }
            }
          }
          else
          { // Another model was stored in the same file URL as this model; look up the new URL.
            embeddedURL = obj.m_saveModels[modelURL];
          }
          if (!embeddedURL.empty())
          {
            obj.m_modelChanges[model]["url"] = smtk::model::StringList(1, embeddedURL);
            //::boost::filesystem::path url = ::boost::filesystem::canonical(modelURL, embedDir);
            obj.m_saveErrors << "<li>Save " << url.filename().string() << " to "
                             << (embedDir / embeddedURL).string() << "</li>";
          }
        }
      }
      else
      {
        if (ok && model.isModified())
        {
          std::string embeddedURL = smtk::io::Helpers::uniqueFilename(url.filename().string(),
            preExistingFilenames, smtkStem, defaultFileExtension, obj.m_embedDir);
          obj.m_saveModels[(embedDir / embeddedURL).string()] = embeddedURL;
          obj.m_saveErrors << "<li>Save " << url.filename().string() << " to "
                           << (alreadyEmbedded
                                  ? (::boost::filesystem::path(relPath) / url.filename()).string()
                                  : url.string())
                           << "</li>";
        }
      }
    }
    else if (ok && model.isModified() && !defaultFileExtension.empty())
    {
      std::string embeddedURL = smtk::io::Helpers::uniqueFilename(
        smtkStem, preExistingFilenames, smtkStem, defaultFileExtension, obj.m_embedDir);
      obj.m_saveModels[(embedDir / embeddedURL).string()] = embeddedURL;
      obj.m_modelChanges[model]["url"] = smtk::model::StringList(1, embeddedURL);
    }

    // III. If the model has auxiliary geometry, determine
    //      the disposition of each. For now, this means
    //      deciding whether to copy it or not. In the future,
    //      auxiliary geometry may also be marked dirty and need
    //      to be saved.
    if (ok && embedData)
    {
      smtk::model::AuxiliaryGeometries auxGeoms = model.auxiliaryGeometry();
      for (auto aux : auxGeoms)
      {
        std::string relPath;
        ::boost::filesystem::path url = ::boost::filesystem::weakly_canonical(aux.url());
        if (url.is_relative())
        {
          url = embedDir / url;
        }
        bool alreadyEmbedded =
          Helpers::isDirectoryASubdirectory(embedDir.string(), url.parent_path().string(), relPath);
        std::cout << "Is URL <" << url.parent_path().string() << "> inside EmbedDir <" << embedDir
                  << ">? " << (alreadyEmbedded ? "Y" : "N") << "   --- " << relPath << "\n";
        /*
          */
        if (!alreadyEmbedded)
        {
          std::string embedRel = Helpers::uniqueFilename(url.filename().string(),
            preExistingFilenames, "aux", url.extension().string(), embedDir.string());
          obj.m_modelChanges[aux] = smtk::model::StringData();
          obj.m_modelChanges[aux]["url"] = smtk::model::StringList(1, embedRel);
          obj.m_copyFiles[url.string()] = embedRel;
          obj.m_saveErrors << "<li>Copy " << url.string() << " to "
                           << (embedDir / embedRel).string() << "</li>";
        }
      }
    }
    /*
    else
    { // TODO: check whether aux geom needs to be saved to its source location.
    }
    */

    // IV. Determine whether or not to rename the model.
    if (renamePolicy != "none" && haveStem)
    {
      std::string oldfilename = filename;
      if (model.hasStringProperty("url"))
      {
        oldfilename = ::boost::filesystem::path(model.stringProperty("url")[0]).stem().string();
      }
      std::string oldmodelname = model.name();
      bool matchDefault = false;
      bool matchPrevious = false;
      const std::string defaultPrefix("Model ");
      if (renamePolicy == "all" || (matchDefault = std::equal(defaultPrefix.begin(),
                                      defaultPrefix.end(), oldmodelname.begin())) ||
        (matchPrevious = std::equal(oldfilename.begin(), oldfilename.end(), oldmodelname.begin())))
      {
        std::ostringstream newname;
        newname << smtkStem;
        std::string suffix;
        if (matchDefault)
        {
          suffix = oldmodelname.substr(defaultPrefix.size() - 1); // include space after prefix
        }
        else if (matchPrevious)
        {
          suffix = oldmodelname.substr(oldfilename.size());
        }
        if (!suffix.empty() && modelsToSave.size() > 1)
        {
          newname << suffix;
        }
        else if (modelsToSave.size() > 1)
        { // TODO: Use pedigree ID if present? and unique?
          newname << " " << modelCounter++;
        }
        obj.m_modelChanges[model]["name"] = smtk::model::StringList(1, newname.str());
        obj.m_saveErrors << "<li>Renaming " << model.name() << " to " << newname.str() << "</li>";
      }
    }

    // V. Determine the disposition of each mesh of the model.
    smtk::mesh::ManagerPtr meshMgr = model.manager()->meshes();
    std::vector<smtk::mesh::CollectionPtr> collections = meshMgr->associatedCollections(model);
    for (auto coll : collections)
    {
      if (meshCollections.find(coll) == meshCollections.end())
      {
        meshCollections.insert(coll);
        std::string meshURL = coll->writeLocation().absolutePath();
        //std::cout << "mesh coll " << coll << " abs " << meshURL << " rel " << coll->writeLocation().relativePath() << "\n";
        bool badMeshURL = false;
        bool alreadyEmbedded = false;
        bool haveRecord = false;
        if (meshURL.empty())
        {
          badMeshURL = true;
          // Create a URL relative to embedDir:
          meshURL = (haveStem ? smtkStem : "smtk_mesh");
          if (!coll->name().empty())
          {
            meshURL += "_" + coll->name();
          }
          meshURL += ".h5m";
        }
        else
        {
          std::string relPath;
          ::boost::filesystem::path url = ::boost::filesystem::weakly_canonical(meshURL);
          alreadyEmbedded = Helpers::isDirectoryASubdirectory(
            embedDir.string(), url.parent_path().string(), relPath);
          meshURL = (::boost::filesystem::path(relPath) / url.filename()).string();
        }
        if (badMeshURL || (embedData && !alreadyEmbedded))
        {
          meshURL = smtk::io::Helpers::uniqueFilename(meshURL, preExistingFilenames,
            (haveStem ? smtkStem : "smtk_mesh"), ".h5m", obj.m_embedDir);
          if (ok)
          {
            haveRecord = true;
            obj.m_saveMeshes[coll] = meshURL;
            obj.m_saveErrors << "<li>Save " << coll->name() << " to "
                             << (embedDir / meshURL).string() << "</li>";
          }
        }
        if (ok && !haveRecord && coll->isModified())
        { // The URL did not need a rewrite but the mesh still needs saving
          ::boost::filesystem::path url = ::boost::filesystem::relative(meshURL, embedDir);
          obj.m_saveMeshes[coll] = url.string();
          obj.m_saveErrors << "<li>Save " << coll->name() << " to " << url.string() << "</li>";
        }
      }
    }
  }
  obj.m_smtkFilename = fullSMTKPath.filename().string();
  return ok;
}

/**\brief Populate the \a json node with the record(s) related to given \a entities.
  *
  */
template <typename T>
int SaveJSON::forEntities(
  cJSON* json, const T& entities, smtk::model::IteratorStyle relatedEntities, JSONFlags sections)
{
  using namespace smtk::model;

  if (!json || sections == JSON_NOTHING)
    return 1;

  EntityIterator iter;
  iter.traverse(entities.begin(), entities.end(), relatedEntities);
  int status = 1;
  for (iter.begin(); !iter.isAtEnd(); ++iter)
  {
    // Pull the first entry off the queue.
    EntityRef ent = *iter;

    // Generate JSON for the queued entity
    ManagerPtr modelMgr = ent.manager();
    UUIDWithEntity it = modelMgr->topology().find(ent.entity());
    if ((it == ent.manager()->topology().end()) ||
      ((it->second.entityFlags() & SESSION) && !(sections & JSON_SESSIONS)))
      continue;

    cJSON* curChild = cJSON_CreateObject();
    {
      std::string suid = it->first.toString();
      cJSON_AddItemToObject(json, suid.c_str(), curChild);
    }
    if (sections & JSON_ENTITIES)
    {
      status &= SaveJSON::forManagerEntity(it, curChild, modelMgr);
      status &= SaveJSON::forManagerArrangement(
        modelMgr->arrangements().find(it->first), curChild, modelMgr);
    }
    if (sections & JSON_TESSELLATIONS)
      status &= SaveJSON::forManagerTessellation(it->first, curChild, modelMgr);
    if (sections & JSON_PROPERTIES)
    {
      status &= SaveJSON::forManagerFloatProperties(it->first, curChild, modelMgr);
      status &= SaveJSON::forManagerStringProperties(it->first, curChild, modelMgr);
      status &= SaveJSON::forManagerIntegerProperties(it->first, curChild, modelMgr);
    }
  }
  return status;
}

/**\brief Populate the \a json node with the record(s) related to given \a entities.
  *
  */
template <typename T>
std::string SaveJSON::forEntities(
  const T& entities, smtk::model::IteratorStyle relatedEntities, JSONFlags sections)
{
  using namespace smtk::model;

  cJSON* top = cJSON_CreateObject();
  SaveJSON::forEntities(top, entities, relatedEntities, sections);
  char* json = cJSON_Print(top);
  std::string result(json);
  free(json);
  cJSON_Delete(top);
  return result;
}

} // namespace io
} // namespace smtk

#endif // __smtk_io_SaveJSON_txx
