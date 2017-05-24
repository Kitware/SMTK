//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/io/SaveJSON.h"
#include "smtk/io/SaveJSON.txx"

#include "smtk/common/ResourceSet.h"
#include "smtk/common/Version.h"

#include "smtk/model/Arrangement.h"
#include "smtk/model/Entity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"
#include "smtk/model/SessionIOJSON.h"
#include "smtk/model/SessionRegistrar.h"
#include "smtk/model/StoredResource.h"
#include "smtk/model/Tessellation.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/System.h"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"

#include "smtk/io/AttributeWriter.h"
#include "smtk/io/Logger.h"
#include "smtk/io/WriteMesh.h"
#include "smtk/io/mesh/MeshIO.h"

#include "cJSON.h"

#include <fstream>

#include <stdlib.h> // for free()

using namespace smtk::io;
using namespace smtk::common;
using namespace smtk::model;

// Some cJSON helpers
namespace
{
cJSON* cJSON_CreateLongArray(const long* ints, unsigned count);
cJSON* cJSON_CreateStringArray(const std::string* strings, unsigned count);
cJSON* cJSON_CreateUUIDArray(const smtk::common::UUID* uids, unsigned count);

cJSON* cJSON_AddAttributeSpec(cJSON* opEntry,
  const char* tagName,    // tag holding name of attribute
  const char* xmlTagName, // tag holding XML for attribute
  smtk::attribute::AttributePtr spec)
{
  if (spec)
  {
    smtk::attribute::SystemPtr tmpSys = smtk::attribute::System::create();
    tmpSys->setRefModelManager(spec->modelManager());
    tmpSys->copyAttribute(
      spec, static_cast<bool>(smtk::attribute::System::FORCE_COPY_ASSOCIATIONS));
    smtk::io::Logger log;
    smtk::io::AttributeWriter wri;
    wri.includeDefinitions(false);
    wri.includeInstances(true);
    wri.includeModelInformation(false);
    wri.includeViews(true); // now operator could specify views
    std::string xml;
    bool err = wri.writeContents(tmpSys, xml, log, true);
    if (!err)
    {
      cJSON_AddItemToObject(opEntry, tagName, cJSON_CreateString(spec->name().c_str()));
      cJSON_AddItemToObject(opEntry, xmlTagName, cJSON_CreateString(xml.c_str()));
    }
  }
  return opEntry;
}

cJSON* cJSON_AddOperator(smtk::model::OperatorPtr op, cJSON* opEntry)
{
  cJSON_AddItemToObject(opEntry, "name", cJSON_CreateString(op->name().c_str()));
  smtk::attribute::AttributePtr spec = op->specification();
  if (spec)
  {
    cJSON_AddAttributeSpec(opEntry, "spec", "specXML", spec);
  }
  /*
    cJSON_AddItemToObject(opEntry, "parameters",
      cJSON_CreateParameterArray(op->parameters()));
      */
  return opEntry;
}

/*
  cJSON* cJSON_CreateOperatorArray(const smtk::model::Operators& ops)
    {
    cJSON* a = cJSON_CreateArray();
    for (smtk::model::Operators::const_iterator it = ops.begin(); it != ops.end(); ++it)
      {
      cJSON* opEntry = cJSON_CreateObject();
      cJSON_AddItemToArray(a, opEntry);
      cJSON_AddOperator(*it, opEntry);
      }
    return a;
    }
    */

cJSON* cJSON_CreateUUIDArray(const smtk::common::UUID* uids, unsigned count)
{
  cJSON* a = cJSON_CreateArray();
  for (unsigned i = 0; i < count; ++i)
  {
    cJSON_AddItemToArray(a, cJSON_CreateString(uids[i].toString().c_str()));
  }
  return a;
}

cJSON* cJSON_CreateStringArray(const std::string* strings, unsigned count)
{
  cJSON* a = cJSON_CreateArray();
  for (unsigned i = 0; i < count; ++i)
  {
    cJSON_AddItemToArray(a, cJSON_CreateString(strings[i].c_str()));
  }
  return a;
}
cJSON* cJSON_CreateLongArray(const long* ints, unsigned count)
{
  // FIXME: This uses a hack to speed up creation of arrays with many entries
  // cJSON should provide a method to append given last element in array and
  // its append method should return each newly-created element.

  cJSON* a = cJSON_CreateArray();
  cJSON** loc = &a->child;
  for (unsigned i = 0; i < count; ++i)
  {
    if (ints[i] > 9007199254740991.0) //== 2^53 - 1, max integer-accurate double
    {
      std::cerr << "Error exporting array: integer value " << i << " (" << ints[i]
                << ") out of range for cJSON\n";
    }
    *loc = cJSON_CreateNumber(ints[i]);
    loc = &((*loc)->next); // fast way to append to cJSON array
  }
  return a;
}
}

namespace smtk
{
namespace io
{

using smtk::common::UUID;

cJSON* SaveJSON::fromUUIDs(const UUIDs& uids)
{
  cJSON* a = cJSON_CreateArray();
  for (UUIDs::const_iterator it = uids.begin(); it != uids.end(); ++it)
  {
    cJSON_AddItemToArray(a, cJSON_CreateString(it->toString().c_str()));
  }
  return a;
}

int SaveJSON::fromModelManager(cJSON* json, ManagerPtr modelMgr, JSONFlags sections)
{
  int status = 0;
  if (!json || !modelMgr)
  {
    std::cerr << "Invalid arguments.\n";
    return status;
  }

  cJSON* body = cJSON_CreateObject();
  cJSON* sess = cJSON_CreateObject();
  cJSON* mesh = cJSON_CreateObject();
  switch (json->type)
  {
    case cJSON_Object:
      cJSON_AddItemToObject(json, "topo", body);
      cJSON_AddItemToObject(json, "sessions", sess);
      cJSON_AddItemToObject(json, "mesh_collections", mesh);
      break;
    case cJSON_Array:
      cJSON_AddItemToArray(json, body);
      cJSON_AddItemToArray(json, sess);
      break;
    case cJSON_NULL:
    case cJSON_Number:
    case cJSON_String:
    default:
      std::cerr << "Invalid toplevel JSON type (" << json->type << ").\n";
      return status;
      break;
  }

  cJSON* mtyp = cJSON_CreateString("Manager");
  cJSON_AddItemToObject(json, "type", mtyp);
  status = SaveJSON::forManager(body, sess, mesh, modelMgr, sections);

  return status;
}

std::string SaveJSON::fromModelManager(ManagerPtr modelMgr, JSONFlags sections)
{
  cJSON* top = cJSON_CreateObject();
  SaveJSON::fromModelManager(top, modelMgr, sections);
  char* json = cJSON_Print(top);
  std::string result(json);
  free(json);
  cJSON_Delete(top);
  return result;
}

bool SaveJSON::fromModelManagerToFile(smtk::model::ManagerPtr modelMgr, const char* filename)
{
  if (!filename || !modelMgr)
    return false;

  std::ofstream file(filename);
  file << SaveJSON::fromModelManager(modelMgr, JSON_DEFAULT);
  return true;
}

int SaveJSON::fromResourceSet(cJSON* pnode, smtk::common::ResourceSetPtr& rset)
{
  if (!pnode || pnode->type != cJSON_Object || !rset)
  {
    return 0;
  }

  cJSON* jset = cJSON_CreateObject();
  cJSON_AddItemToObject(pnode, "resource set", jset);
  if (!rset->linkStartPath().empty())
  {
    cJSON_AddItemToObject(jset, "path prefix", cJSON_CreateString(rset->linkStartPath().c_str()));
  }
  std::vector<std::string> rids = rset->resourceIds();
  for (auto rid : rids)
  {
    smtk::common::ResourcePtr rsrc;
    smtk::model::StoredResourcePtr srsrc;
    if (rset->get(rid, rsrc))
    {
      smtk::common::Resource::Type rsrcType;
      smtk::common::ResourceSet::ResourceRole rsrcRole;
      smtk::common::ResourceSet::ResourceState rsrcState;
      std::string rsrcLink;

      cJSON* jsrc = cJSON_CreateObject();
      cJSON_AddItemToObject(jset, rid.c_str(), jsrc);
      if (rset->resourceInfo(rid, rsrcType, rsrcRole, rsrcState, rsrcLink))
      {
        cJSON_AddItemToObject(
          jsrc, "type", cJSON_CreateString(smtk::common::Resource::type2String(rsrcType).c_str()));
        cJSON_AddItemToObject(jsrc, "role",
          cJSON_CreateString(smtk::common::ResourceSet::role2String(rsrcRole).c_str()));
        cJSON_AddItemToObject(jsrc, "state",
          cJSON_CreateString(smtk::common::ResourceSet::state2String(rsrcState).c_str()));

        if ((srsrc = smtk::dynamic_pointer_cast<smtk::model::StoredResource>(rsrc)))
        {
          cJSON_AddItemToObject(jsrc, "url", cJSON_CreateString(srsrc->url().c_str()));
          const smtk::model::EntityRefs& children(srsrc->entities());
          if (!children.empty())
          { // Append entities contained in the file to the resource description:
            cJSON* jents = cJSON_CreateArray();
            cJSON_AddItemToObject(jsrc, "entities", jents);
            cJSON** jchild = &(jents->child);
            for (auto child : children)
            {
              *jchild = cJSON_CreateString(child.entity().toString().c_str());
              jchild = &((*jchild)->next);
            }
          }
        }
      }
    }
  }
  return 1;
}

bool SaveJSON::canSaveModels(const smtk::model::Models& models)
{
  std::set<smtk::model::Manager::Ptr> mgrs;
  for (auto model : models)
  {
    if (!model.isValid())
    {
      continue;
    }
    if (mgrs.find(model.manager()) == mgrs.end())
    {
      (void)model.manager()->resources(); // force computeResources to be called
    }
    if (!model.hasStringProperty("smtk_url"))
    {
      return false;
    }
  }
  return true;
}

int SaveJSON::save(
  cJSON* pnode, const smtk::model::Models& models, bool renameModels, const std::string& embedDir)
{
  (void)renameModels; // FIXME

  int status = 1;
  std::set<smtk::model::Manager::Ptr> mgrs;
  smtk::model::SessionIOJSON::Ptr delegate;
  std::map<smtk::model::SessionRef, smtk::model::SessionIOJSON::Ptr> delegates;
  std::map<smtk::model::SessionRef, smtk::model::SessionIOJSON::Ptr>::iterator delegateIter;
  for (auto model : models)
  {
    if (!model.isValid())
    {
      continue;
    }
    // If we have never seen this model's manager, save the
    // models in this manager that have been requested:
    if (mgrs.find(model.manager()) == mgrs.end())
    {
      smtk::common::ResourceSetPtr rset = model.manager()->resources();
      if (!embedDir.empty())
      {
        rset->setLinkStartPath(embedDir);
      }
      std::vector<std::string> rids = rset->resourceIds();
      for (auto rsrcId : rids)
      {
        smtk::common::ResourcePtr rsrc;
        smtk::model::StoredResourcePtr srsrc;
        smtk::model::SessionRef sref;
        if (rset->get(rsrcId, rsrc) && (srsrc = smtk::dynamic_pointer_cast<StoredResource>(rsrc)) &&
          (srsrc->entities().find(model) != srsrc->entities().end()) &&
          (sref = srsrc->session()).isValid())
        {
          // Get an I/O delegate for this session
          delegateIter = delegates.find(sref);
          if (delegateIter != delegates.end())
          {
            delegate = delegateIter->second;
          }
          else
          {
            delegate = smtk::dynamic_pointer_cast<smtk::model::SessionIOJSON>(
              sref.session()->createIODelegate("json"));
            delegate->setReferencePath(rset->linkStartPath());
            delegates[sref] = delegate;
          }
          // Tell the delegate to save this resource for us
          if (delegate)
          {
            //delegate->saveResource(pnode, model, srsrc);
            delegate->saveResource(model, rset, srsrc);
          }
        }
      }
    }
  }

  // Now we've written out CAD files and auxiliary geometry as required.
  // We still need to save JSON data. Do so on a per-session basis, which
  // we can do with our fancy "delegates" map.
  for (delegateIter = delegates.begin(); delegateIter != delegates.end(); ++delegateIter)
  {
    cJSON* sess = cJSON_CreateObject();
    cJSON_AddItemToObject(pnode, delegateIter->first.entity().toString().c_str(), sess);
    cJSON_AddStringToObject(sess, "type", "session");
    cJSON_AddStringToObject(sess, "name", delegateIter->first.session()->name().c_str());

    delegate = delegateIter->second;
    if (delegate)
    {
      status &= delegate->saveJSON(sess, delegateIter->first, models);
    }

    // Add mesh info to the session JSON node for models in this session:
    smtk::io::WriteMesh write;
    for (auto model : models)
    {
      if (model.owningSession() == delegateIter->first)
      {
        smtk::mesh::ManagerPtr meshMgr = model.manager()->meshes();
        smtk::common::UUIDs cids = meshMgr->associatedCollectionIds(model);
        for (auto cid : cids)
        {
          smtk::mesh::CollectionPtr coll = meshMgr->collection(cid);
          std::string meshFile;
          if (coll && coll->isModified() &&
            !(meshFile = coll->writeLocation().absolutePath()).empty())
          {
            // Write the mesh. This should reset coll->isModified() so it only
            // happens once even if multiple models have meshes in the same collection.
            bool ok = write(meshFile, coll, smtk::io::mesh::Subset::EntireCollection);
            if (!ok)
            {
              smtkErrorMacro(model.manager()->log(), "Could not write mesh ("
                  << coll->name() << ") to \"" << meshFile << "\".");
            }
          }
        }
        smtk::io::SaveJSON::forModelMeshes(model.entity(), sess, model.manager());
      }
    }

    //SaveJSON::addMeshesRecord(delegateIter->first.manager(), models, sess);
    SaveJSON::addModelsRecord(delegateIter->first.manager(), models, sess);
  }

  //status &= SaveJSON::forOperatorDefinitions(session->operatorSystem(), sess);
  return status;
}

int SaveJSON::forManager(
  cJSON* dict, cJSON* sess, cJSON* mesh, ManagerPtr modelMgr, JSONFlags sections)
{
  if (!dict || !modelMgr)
  {
    return 0;
  }
  int status = 1;
  UUIDWithEntity it;

  if (sections == JSON_NOTHING)
    return status;

  for (it = modelMgr->topology().begin(); it != modelMgr->topology().end(); ++it)
  {
    if ((it->second.entityFlags() & SESSION) && !(sections & JSON_SESSIONS))
      continue;

    cJSON* curChild = cJSON_CreateObject();
    {
      std::string suid = it->first.toString();
      cJSON_AddItemToObject(dict, suid.c_str(), curChild);
    }
    if (sections & JSON_ENTITIES)
    {
      status &= SaveJSON::forManagerEntity(it, curChild, modelMgr);
      status &= SaveJSON::forManagerArrangement(
        modelMgr->arrangements().find(it->first), curChild, modelMgr);
    }
    if (sections & JSON_TESSELLATIONS)
      status &= SaveJSON::forManagerTessellation(it->first, curChild, modelMgr);
    if (sections & JSON_ANALYSISMESH)
      status &= SaveJSON::forManagerAnalysis(it->first, curChild, modelMgr);
    if (sections & JSON_PROPERTIES)
    {
      status &= SaveJSON::forManagerFloatProperties(it->first, curChild, modelMgr);
      status &= SaveJSON::forManagerStringProperties(it->first, curChild, modelMgr);
      status &= SaveJSON::forManagerIntegerProperties(it->first, curChild, modelMgr);
    }
  }

  if (sections & JSON_SESSIONS)
  {
    smtk::model::SessionRefs sessions = modelMgr->sessions();
    for (smtk::model::SessionRefs::iterator bit = sessions.begin(); bit != sessions.end(); ++bit)
    {
      status &= SaveJSON::forManagerSession(bit->entity(), sess, modelMgr);
    }
  }

  if (sections & JSON_MESHES)
  {
    smtk::mesh::ManagerPtr meshPtr = modelMgr->meshes();
    status &= SaveJSON::forManagerMeshes(meshPtr, mesh, modelMgr);
  }
  return status;
}

int SaveJSON::forManagerEntity(UUIDWithEntity& entry, cJSON* entRec, ManagerPtr model)
{
  (void)model;
  cJSON* ent = cJSON_CreateNumber(entry->second.entityFlags());
  cJSON* dim = cJSON_CreateNumber(entry->second.dimension());
  cJSON_AddItemToObject(entRec, "e", ent);
  cJSON_AddItemToObject(entRec, "d", dim);
  if (!entry->second.relations().empty())
  {
    cJSON_AddItemToObject(
      entRec, "r", cJSON_CreateUUIDArray(&entry->second.relations()[0],
                     static_cast<unsigned int>(entry->second.relations().size())));
  }
  /*
  if (entry->second.entityFlags() & MODEL_ENTITY)
    SaveJSON::forModelOperators(entry->first, entRec, model);
    */
  return 1;
}

int SaveJSON::forManagerArrangement(
  const UUIDWithArrangementDictionary& entry, cJSON* dict, ManagerPtr model)
{
  if (entry == model->arrangements().end())
  {
    return 0;
  }
  ArrangementKindWithArrangements it;
  cJSON* arrNode = cJSON_CreateObject();
  cJSON_AddItemToObject(dict, "a", arrNode);
  for (it = entry->second.begin(); it != entry->second.end(); ++it)
  {
    Arrangements& arr(it->second);
    if (!arr.empty())
    {
      cJSON* kindNode = cJSON_CreateArray();
      cJSON_AddItemToObject(
        arrNode, smtk::model::AbbreviationForArrangementKind(it->first).c_str(), kindNode);
      Arrangements::iterator ait;
      for (ait = arr.begin(); ait != arr.end(); ++ait)
      {
        if (!ait->details().empty())
        {
          cJSON_AddItemToArray(kindNode,
            cJSON_CreateIntArray(&(ait->details()[0]), static_cast<int>(ait->details().size())));
        }
      }
    }
  }
  return 1;
}

int SaveJSON::forManagerTessellation(const smtk::common::UUID& uid, cJSON* dict, ManagerPtr model)
{
  UUIDWithTessellation tessIt = model->tessellations().find(uid);
  if (tessIt == model->tessellations().end() || tessIt->second.coords().empty())
  { // No tessellation? Not a problem.
    return 1;
  }
  //  "metadata": { "formatVersion" : 3 },
  //  "vertices": [ 0,0,0, 0,0,1, 1,0,1, 1,0,0, ... ],
  //  "normals":  [ 0,1,0, ... ],
  //  "faces": [
  //    0, 0,1,2, // tri
  //    1, 0,1,2,3, // quad
  //    32, 0,1,2, // tri w/ per-vert norm
  cJSON* tess = cJSON_CreateObject();
  //cJSON* meta = cJSON_CreateObject();
  cJSON* fmt = cJSON_CreateObject();
  cJSON_AddItemToObject(fmt, "formatVersion", cJSON_CreateNumber(3));
  //cJSON_AddItemToObject(meta, "metadata", fmt);
  //cJSON_AddItemToObject(tess, "3js", meta);
  cJSON_AddItemToObject(tess, "metadata", fmt);
  cJSON_AddItemToObject(tess, "vertices", cJSON_CreateDoubleArray(&tessIt->second.coords()[0],
                                            static_cast<int>(tessIt->second.coords().size())));
  cJSON_AddItemToObject(tess, "faces",
    cJSON_CreateIntArray(tessIt->second.conn().empty() ? NULL : &tessIt->second.conn()[0],
                          static_cast<int>(tessIt->second.conn().size())));
  cJSON_AddItemToObject(dict, "t", tess);
  return 1;
}

int SaveJSON::forManagerAnalysis(const smtk::common::UUID& uid, cJSON* dict, ManagerPtr model)
{
  UUIDWithTessellation meshIt = model->analysisMesh().find(uid);
  if (meshIt == model->analysisMesh().end() || meshIt->second.coords().empty())
  { // No tessellation? Not a problem.
    return 1;
  }
  //  "metadata": { "formatVersion" : 3 },
  //  "vertices": [ 0,0,0, 0,0,1, 1,0,1, 1,0,0, ... ],
  //  "normals":  [ 0,1,0, ... ],
  //  "faces": [
  //    0, 0,1,2, // tri
  //    1, 0,1,2,3, // quad
  //    32, 0,1,2, // tri w/ per-vert norm
  cJSON* mesh = cJSON_CreateObject();
  //cJSON* meta = cJSON_CreateObject();
  cJSON* fmt = cJSON_CreateObject();
  cJSON_AddItemToObject(fmt, "formatVersion", cJSON_CreateNumber(3));
  //cJSON_AddItemToObject(meta, "metadata", fmt);
  //cJSON_AddItemToObject(tess, "3js", meta);
  cJSON_AddItemToObject(mesh, "metadata", fmt);
  cJSON_AddItemToObject(mesh, "vertices", cJSON_CreateDoubleArray(&meshIt->second.coords()[0],
                                            static_cast<int>(meshIt->second.coords().size())));
  cJSON_AddItemToObject(mesh, "faces",
    cJSON_CreateIntArray(meshIt->second.conn().empty() ? NULL : &meshIt->second.conn()[0],
                          static_cast<int>(meshIt->second.conn().size())));
  cJSON_AddItemToObject(dict, "m", mesh);
  return 1;
}

int SaveJSON::forFloatData(cJSON* dict, const FloatData& fdata)
{
  cJSON* pdict = cJSON_CreateObject();
  cJSON_AddItemToObject(dict, "f", pdict);
  PropertyNameWithConstFloats entry;
  for (entry = fdata.begin(); entry != fdata.end(); ++entry)
  {
    if (entry->second.empty())
    {
      continue;
    }
    cJSON_AddItemToObject(pdict, entry->first.c_str(),
      cJSON_CreateDoubleArray(&entry->second[0], static_cast<int>(entry->second.size())));
  }
  return 1;
}

int SaveJSON::forStringData(cJSON* dict, const StringData& sdata)
{
  cJSON* pdict = cJSON_CreateObject();
  cJSON_AddItemToObject(dict, "s", pdict);
  PropertyNameWithConstStrings entry;
  for (entry = sdata.begin(); entry != sdata.end(); ++entry)
  {
    if (entry->second.empty())
    {
      continue;
    }
    cJSON_AddItemToObject(pdict, entry->first.c_str(),
      cJSON_CreateStringArray(&entry->second[0], static_cast<unsigned int>(entry->second.size())));
  }
  return 1;
}

int SaveJSON::forIntegerData(cJSON* dict, const IntegerData& idata)
{
  cJSON* pdict = cJSON_CreateObject();
  cJSON_AddItemToObject(dict, "i", pdict);
  PropertyNameWithConstIntegers entry;
  for (entry = idata.begin(); entry != idata.end(); ++entry)
  {
    if (entry->second.empty())
    {
      continue;
    }
    cJSON_AddItemToObject(pdict, entry->first.c_str(),
      cJSON_CreateLongArray(&entry->second[0], static_cast<unsigned int>(entry->second.size())));
  }
  return 1;
}

int SaveJSON::forManagerFloatProperties(
  const smtk::common::UUID& uid, cJSON* dict, ManagerPtr model)
{
  int status = 1;
  UUIDWithFloatProperties entIt = model->floatProperties().find(uid);
  if (entIt == model->floatProperties().end() || entIt->second.empty())
  { // No properties is not an error
    return status;
  }
  return SaveJSON::forFloatData(dict, entIt->second);
}

int SaveJSON::forManagerStringProperties(
  const smtk::common::UUID& uid, cJSON* dict, ManagerPtr modelManager)
{
  int status = 1;
  UUIDWithStringProperties entIt = modelManager->stringProperties().find(uid);
  if (entIt == modelManager->stringProperties().end() || entIt->second.empty())
  { // No properties is not an error
    return status;
  }
  return SaveJSON::forStringData(dict, entIt->second);
}

int SaveJSON::forManagerIntegerProperties(
  const smtk::common::UUID& uid, cJSON* dict, ManagerPtr model)
{
  int status = 1;
  UUIDWithIntegerProperties entIt = model->integerProperties().find(uid);
  if (entIt == model->integerProperties().end() || entIt->second.empty())
  { // No properties is not an error
    return status;
  }
  return SaveJSON::forIntegerData(dict, entIt->second);
}

int SaveJSON::forManagerSession(const smtk::common::UUID& uid, cJSON* node, ManagerPtr modelMgr,
  bool writeNativeModels, const std::string& refPath)
{
  int status = 1;
  SessionPtr session = SessionRef(modelMgr, uid).session();
  if (!session)
    return status;

  cJSON* sess = cJSON_CreateObject();
  cJSON_AddItemToObject(node, uid.toString().c_str(), sess);
  cJSON_AddStringToObject(sess, "type", "session");
  cJSON_AddStringToObject(sess, "name", session->name().c_str());
  SessionIOJSONPtr delegate =
    smtk::dynamic_pointer_cast<SessionIOJSON>(session->createIODelegate("json"));
  if (delegate)
  {
    delegate->setReferencePath(refPath);
    status &= delegate->exportJSON(modelMgr, session, sess, writeNativeModels);
  }

  smtk::model::Models modelsOfSession = SessionRef(modelMgr, session).models<smtk::model::Models>();
  SaveJSON::addModelsRecord(modelMgr, modelsOfSession, sess);
  SaveJSON::addMeshesRecord(modelMgr, modelsOfSession, sess);

  status &= SaveJSON::forOperatorDefinitions(session->operatorSystem(), sess);
  return status;
}

int SaveJSON::forManagerSessionPartial(const smtk::common::UUID& sessionid,
  const smtk::common::UUIDs& modelIds, cJSON* node, ManagerPtr modelMgr, bool writeNativeModels,
  const std::string& refPath)
{
  int status = 1;
  SessionPtr session = SessionRef(modelMgr, sessionid).session();
  if (!session)
    return status;

  cJSON* sess = cJSON_CreateObject();
  cJSON_AddItemToObject(node, sessionid.toString().c_str(), sess);
  cJSON_AddStringToObject(sess, "type", "session");
  cJSON_AddStringToObject(sess, "name", session->name().c_str());

  SessionIOJSONPtr delegate =
    smtk::dynamic_pointer_cast<SessionIOJSON>(session->createIODelegate("json"));
  if (delegate)
  {
    delegate->setReferencePath(refPath);
    status &= delegate->exportJSON(modelMgr, session, modelIds, sess, writeNativeModels);
  }
  SaveJSON::addModelsRecord(modelMgr, modelIds, sess);
  SaveJSON::addMeshesRecord(modelMgr, modelIds, sess);
  status &= SaveJSON::forOperatorDefinitions(session->operatorSystem(), sess);
  return status;
}

/*
int SaveJSON::forModelOperators(const smtk::common::UUID& uid, cJSON* entRec, ManagerPtr modelMgr)
{
  smtk::model::Model mod(modelMgr, uid);
  smtk::model::Operators ops(mod.operators());
  cJSON_AddItemToObject(entRec, "ops",
    cJSON_CreateOperatorArray(ops));`
  return 1; // SaveJSON::forOperators(ops, entRec);
} */

int SaveJSON::forOperatorDefinitions(smtk::attribute::SystemPtr opSys, cJSON* entRec)
{
  smtk::io::Logger log;
  smtk::io::AttributeWriter wri;
  wri.includeDefinitions(true);
  wri.includeInstances(false);
  wri.includeModelInformation(false);
  wri.includeViews(true); // now operator could specify views
  std::string xml;
  bool err = wri.writeContents(opSys, xml, log, true);
  if (!err)
  {
    cJSON_AddItemToObject(entRec, "ops", cJSON_CreateString(xml.c_str()));
  }
  /*
  std::vector<smtk::attribute::DefinitionPtr> ops;
  opSys.derivedDefinitions(
    opSys.findDefinition("operator"), ops);
  if (!ops.empty())
    {
    cJSON_AddItemToObject(entRec, "ops",
      cJSON_CreateOperatorArray(ops));
    }
    */
  return 1;
}

int SaveJSON::forOperator(OperatorPtr op, cJSON* entRec)
{
  cJSON_AddOperator(op, entRec);
  return 1;
}

int SaveJSON::forOperator(smtk::attribute::AttributePtr op, cJSON* entRec)
{
  cJSON_AddItemToObject(entRec, "name", cJSON_CreateString(op->type().c_str()));
  cJSON_AddAttributeSpec(entRec, "spec", "specXML", op);
  return 1;
}

int SaveJSON::forOperatorResult(OperatorResult res, cJSON* entRec)
{
  cJSON_AddItemToObject(entRec, "name", cJSON_CreateString(res->type().c_str()));
  cJSON_AddAttributeSpec(entRec, "result", "resultXML", res);
  EntityRefs ents = res->modelEntitiesAs<EntityRefs>("created");
  EntityRefs mdfs = res->modelEntitiesAs<EntityRefs>("modified");
  EntityRefs meshents = res->modelEntitiesAs<EntityRefs>("mesh_created");

  ents.insert(mdfs.begin(), mdfs.end());
  ents.insert(meshents.begin(), meshents.end());
  if (!ents.empty())
  {
    // If the operator reports new/modified entities, transcribe the affected models.
    // TODO: In the future, this may be more conservative (i.e., fewer records
    //       would be included to save time and memory) than ITERATE_MODELS.
    cJSON* records = cJSON_CreateObject();
    SaveJSON::forEntities(records, ents, smtk::model::ITERATE_MODELS, JSON_CLIENT_DATA);
    cJSON_AddItemToObject(entRec, "records", records);
  }

  smtk::attribute::MeshItemPtr modifiedMeshes = res->findMesh("mesh_modified");
  // Also export JSON meshes as a new node "mesh_records"
  if (!meshents.empty() || modifiedMeshes)
  {
    // get all collections associated with the input entities
    smtk::common::UUIDs collectionIds;
    smtk::mesh::ManagerPtr meshMgr = res->modelManager()->meshes();
    smtk::model::EntityRefs::const_iterator iter;
    for (iter = meshents.begin(); iter != meshents.end(); ++iter)
    {
      smtk::common::UUIDs cids = meshMgr->associatedCollectionIds(*iter);
      collectionIds.insert(cids.begin(), cids.end());
    }
    if (modifiedMeshes)
    {
      smtk::attribute::MeshItem::const_mesh_it it;
      for (it = modifiedMeshes->begin(); it != modifiedMeshes->end(); ++it)
      {
        collectionIds.insert(it->collection()->entity());
      }
    }
    if (collectionIds.size() > 0)
    {
      cJSON* mesh_records = cJSON_CreateObject();
      SaveJSON::forMeshCollections(mesh_records, collectionIds, meshMgr);
      cJSON_AddItemToObject(entRec, "mesh_records", mesh_records);
    }
  }

  return 1;
}

/// Serialize a session's list of dangling entities held in the given \a modelMgr.
int SaveJSON::forDanglingEntities(
  const smtk::common::UUID& sessionId, cJSON* node, ManagerPtr modelMgr)
{
  if (!modelMgr || !node || node->type != cJSON_Object)
    return 0;
  SessionPtr session = SessionRef(modelMgr, sessionId).session();
  if (!session)
    return 0;

  cJSON* danglers = cJSON_CreateObject();
  cJSON* darray = cJSON_CreateObject();
  cJSON_AddItemToObject(node, "danglingEntities", danglers);
  cJSON_AddItemToObject(danglers, "sessionId", cJSON_CreateString(sessionId.toString().c_str()));
  cJSON_AddItemToObject(danglers, "entities", darray);
  DanglingEntities::const_iterator it;
  for (it = session->danglingEntities().begin(); it != session->danglingEntities().end(); ++it)
  {
    if (it->first.manager() == modelMgr)
      cJSON_AddItemToObject(
        darray, it->first.entity().toString().c_str(), cJSON_CreateNumber(it->second));
  }
  return 1;
}

/**\brief Serialize a description of a Remus model worker.
  *
  * This populates an empty JSON Object (\a wdesc) with
  * data required for a Remus server to start an smtk-model-worker
  * process for use by a RemusRemoteSession instance.
  */
int SaveJSON::forModelWorker(cJSON* wdesc, const std::string& meshTypeIn,
  const std::string& meshTypeOut, smtk::model::SessionPtr session, const std::string& engine,
  const std::string& site, const std::string& root, const std::string& workerPath,
  const std::string& requirementsFileName)
{
  if (!wdesc || wdesc->type != cJSON_Object)
    return 0;

  // Base worker requirements
  cJSON_AddItemToObject(wdesc, "InputType", cJSON_CreateString(meshTypeIn.c_str()));
  cJSON_AddItemToObject(wdesc, "OutputType", cJSON_CreateString(meshTypeOut.c_str()));
  cJSON_AddItemToObject(wdesc, "ExecutableName", cJSON_CreateString(workerPath.c_str()));

  // External requirements information
  cJSON_AddItemToObject(wdesc, "File", cJSON_CreateString(requirementsFileName.c_str()));
  cJSON_AddItemToObject(wdesc, "FileFormat", cJSON_CreateString("XML"));

  // Additional requirements for SMTK model worker
  cJSON* argArray = cJSON_CreateArray();
  cJSON_AddItemToArray(argArray, cJSON_CreateString("-rwfile=@SELF@"));
  cJSON_AddItemToArray(argArray, cJSON_CreateString(("-kernel=" + session->name()).c_str()));
  if (!engine.empty())
    cJSON_AddItemToArray(argArray, cJSON_CreateString(("-engine=" + engine).c_str()));
  if (!site.empty())
    cJSON_AddItemToArray(argArray, cJSON_CreateString(("-site=" + site).c_str()));
  if (!root.empty())
    cJSON_AddItemToArray(argArray, cJSON_CreateString(("-root=" + root).c_str()));
  cJSON_AddItemToObject(wdesc, "Arguments", argArray);

  // TODO: Handle workers that can support multiple kernels as well as
  //       multiple engines.
  cJSON* tag = cJSON_Parse(SessionRegistrar::sessionTags(session->name()).c_str());
  if (!tag)
    return 0;

  cJSON_AddItemToObject(wdesc, "Tag", tag);
  cJSON_AddItemToObject(tag, "default_kernel", cJSON_CreateString(session->name().c_str()));
  cJSON_AddItemToObject(tag, "default_engine", cJSON_CreateString(engine.c_str()));
  cJSON_AddItemToObject(tag, "site", cJSON_CreateString(site.c_str()));
  cJSON_AddItemToObject(
    tag, "smtk_version", cJSON_CreateString(smtk::common::Version::number().c_str()));
  return 1;
}

/**\brief Serialize all the smtk::mesh associated with a given smtk::model.
  *
  * This populates an empty JSON Object (\a mdesc) with
  * data required to recreate the smtk::mesh Collections
  * associated with the given smtk::model.
  */
int SaveJSON::forManagerMeshes(
  smtk::mesh::ManagerPtr meshes, cJSON* mdesc, smtk::model::ManagerPtr modelMgr)
{
  (void)modelMgr;
  //current issue is that a mesh Manager needs to know where to write
  //these collections to disk.

  if (!mdesc || mdesc->type != cJSON_Object)
  {
    return 0;
  }

  //step 0. get all collections in the manager
  int status = 1;
  std::vector<smtk::mesh::CollectionPtr> collections;
  typedef smtk::mesh::Manager::const_iterator cit;
  for (cit it = meshes->collectionBegin(); it != meshes->collectionEnd(); ++it)
  {
    status &= forSingleCollection(mdesc, it->second);
  }

  return status;
}

/**\brief Serialize input mesh Collections.
  *
  * This creates and populate an JSON Object "mesh_collections"
  * and add it to the parent json node (\a pnode) with
  * data required to recreate the mesh Collections
  * associated with the given \a collectionIds of mesh manager (\a meshMgr)
  */
int SaveJSON::forMeshCollections(
  cJSON* pnode, const smtk::common::UUIDs& collectionIds, smtk::mesh::ManagerPtr meshMgr)
{
  if (!pnode || pnode->type != cJSON_Object)
  {
    return 0;
  }

  if (collectionIds.empty())
  {
    return 0;
  }
  int status = 1;
  cJSON* mesh = cJSON_GetObjectItem(pnode, "mesh_collections");
  if (!mesh)
  {
    mesh = cJSON_CreateObject();
    cJSON_AddItemToObject(pnode, "mesh_collections", mesh);
  }

  smtk::common::UUIDs::const_iterator cit;
  for (cit = collectionIds.begin(); cit != collectionIds.end(); ++cit)
  {
    status &= forSingleCollection(mesh, meshMgr->collection(*cit));
  }

  return status;
}

/**\brief Serialize all the mesh collections associated with given \a modelid.
  *
  * This creates and populate an JSON Object "mesh_collections"
  * and add it to the parent json node (\a pnode) with
  * all mesh collections associated with the given \a modelid.
  */
int SaveJSON::forModelMeshes(
  const smtk::common::UUID& modelid, cJSON* pnode, smtk::model::ManagerPtr modelMgr)
{
  if (!pnode || pnode->type != cJSON_Object)
  {
    return 0;
  }
  smtk::mesh::ManagerPtr meshMgr = modelMgr->meshes();
  smtk::model::Model model(modelMgr, modelid);
  if (!model.isValid() || !meshMgr)
  {
    return 0;
  }

  smtk::common::UUIDs cids = meshMgr->associatedCollectionIds(model);
  return SaveJSON::forMeshCollections(pnode, cids, meshMgr);
}

namespace
{

template <typename T>
void writeIntegerValues(cJSON* parent, std::vector<T> const& values, std::string name)
{
  cJSON* a = cJSON_CreateArray();
  for (std::size_t i = 0; i < values.size(); ++i)
  {
    cJSON_AddItemToArray(a, cJSON_CreateNumber(values[i].value()));
  }
  cJSON_AddItemToObject(parent, name.c_str(), a);
}

void writeHandleValues(cJSON* parent, smtk::mesh::HandleRange const& values, std::string name)
{
  //need to implement a free method like:
  cJSON* json = smtk::mesh::to_json(values);
  cJSON_AddItemToObject(parent, name.c_str(), json);
}

void writeBoundaryConditions(cJSON* parent, const smtk::mesh::MeshSet& mesh)
{
  cJSON* boundaryJson = cJSON_CreateObject();
  cJSON_AddItemToObject(parent, "boundary_conditions", boundaryJson);

  std::size_t index = 0;
  std::stringstream buffer;

  //list out the dirichlets that this mesheset contains
  std::vector<smtk::mesh::Dirichlet> dirichlets = mesh.dirichlets();
  for (std::size_t i = 0; i < dirichlets.size(); ++i)
  {
    cJSON* conditionJson = cJSON_CreateObject();
    cJSON_AddItemToObject(conditionJson, "value", cJSON_CreateNumber(dirichlets[i].value()));
    cJSON_AddItemToObject(conditionJson, "type", cJSON_CreateString("dirichlet"));

    //convert the index to a string.
    buffer << index++;
    std::string sindex = buffer.str();
    cJSON_AddItemToObject(boundaryJson, sindex.c_str(), conditionJson);

    buffer.str("");
  }

  //list out the neumanns that this mesheset contains
  std::vector<smtk::mesh::Neumann> neumanns = mesh.neumanns();
  for (std::size_t i = 0; i < neumanns.size(); ++i)
  {
    cJSON* conditionJson = cJSON_CreateObject();
    cJSON_AddItemToObject(conditionJson, "value", cJSON_CreateNumber(neumanns[i].value()));
    cJSON_AddItemToObject(conditionJson, "type", cJSON_CreateString("neumann"));

    //convert the index to a string.
    buffer << index++;
    std::string sindex = buffer.str();
    cJSON_AddItemToObject(boundaryJson, sindex.c_str(), conditionJson);
  }
}

void writeUUIDValues(cJSON* parent, smtk::common::UUIDArray const& values, std::string name)
{
  cJSON_AddItemToObject(parent, name.c_str(),
    cJSON_CreateUUIDArray(&values[0], static_cast<unsigned int>(values.size())));
}

class ForMeshset : public smtk::mesh::MeshForEach
{
public:
  ForMeshset(cJSON* json)
    : smtk::mesh::MeshForEach()
    , m_json(json)
    , m_index(0)
  {
  }

  void write(
    const smtk::mesh::MeshSet& mesh, cJSON* parent, bool writeMeshes, bool writeCellAndPoints)
  {
    std::string cell_bit_types = mesh.types().cellTypes().to_string();

    cJSON_AddStringToObject(parent, "cell_types", cell_bit_types.c_str());

    //needs to be by value since cells() will go out out of scope, and
    //we don't want a reference to a stack object that has gone out of scope
    if (writeMeshes)
    {
      smtk::mesh::HandleRange meshes = mesh.range();
      //note we uses meshIds, since 'meshes' is used by the actual mesh dict
      writeHandleValues(parent, meshes, std::string("meshIds"));
    }

    if (writeCellAndPoints)
    {
      smtk::mesh::HandleRange cells = mesh.cells().range();
      writeHandleValues(parent, cells, std::string("cells"));

      smtk::mesh::HandleRange points = mesh.points().range();
      writeHandleValues(parent, points, std::string("points"));
    }

    //list out the domains that this mesheset contains
    std::vector<smtk::mesh::Domain> domains = mesh.domains();
    writeIntegerValues(parent, domains, std::string("domains"));

    //write out the boundary conditions of this meshset
    writeBoundaryConditions(parent, mesh);

    //list out the model associations that this mesheset contains
    smtk::common::UUIDArray modelEntityIds = mesh.modelEntityIds();
    writeUUIDValues(parent, modelEntityIds, std::string("modelEntityIds"));
  }

  void writeProperties(const smtk::mesh::CollectionPtr& collection, cJSON* parent)
  {
    smtk::mesh::MeshFloatData* fProperties = collection->properties<smtk::mesh::MeshFloatData>();
    smtk::mesh::MeshStringData* sProperties = collection->properties<smtk::mesh::MeshStringData>();
    smtk::mesh::MeshIntegerData* iProperties =
      collection->properties<smtk::mesh::MeshIntegerData>();
    // if there are no properties, just return
    if (!(fProperties && fProperties->size() > 0) && !(sProperties && sProperties->size() > 0) &&
      !(iProperties && iProperties->size() > 0))
    {
      return;
    }
    std::string meshid("meshid");
    cJSON* jsonProperties = cJSON_CreateObject();
    cJSON_AddItemToObject(parent, "properties", jsonProperties);
    if (fProperties && fProperties->size() > 0)
    {
      for (smtk::mesh::MeshFloatData::const_iterator it = fProperties->begin();
           it != fProperties->end(); ++it)
      {
        cJSON* jMesh = smtk::mesh::to_json(it->first.range());
        cJSON_AddItemToObject(jsonProperties, meshid.c_str(), jMesh);
        SaveJSON::forFloatData(jMesh, it->second);
      }
    }
    if (sProperties && sProperties->size() > 0)
    {
      for (smtk::mesh::MeshStringData::const_iterator it = sProperties->begin();
           it != sProperties->end(); ++it)
      {
        cJSON* jMesh = smtk::mesh::to_json(it->first.range());
        cJSON_AddItemToObject(jsonProperties, meshid.c_str(), jMesh);
        SaveJSON::forStringData(jMesh, it->second);
      }
    }
    if (iProperties && iProperties->size() > 0)
    {
      for (smtk::mesh::MeshIntegerData::const_iterator it = iProperties->begin();
           it != iProperties->end(); ++it)
      {
        cJSON* jMesh = smtk::mesh::to_json(it->first.range());
        cJSON_AddItemToObject(jsonProperties, meshid.c_str(), jMesh);
        SaveJSON::forIntegerData(jMesh, it->second);
      }
    }
  }

  void forMesh(smtk::mesh::MeshSet& mesh)
  {
    cJSON* meshJson = cJSON_CreateObject();

    const bool writeMeshes = false;
    const bool writeCellAndPoints = true;
    this->write(mesh, meshJson, writeMeshes, writeCellAndPoints);

    //convert the index to a string. assign it as the name of the meshset
    std::stringstream buffer;
    buffer << this->m_index++;
    std::string sindex = buffer.str();

    cJSON_AddItemToObject(this->m_json, sindex.c_str(), meshJson);
  }

private:
  cJSON* m_json;
  int m_index;
};
}
/**\brief Serialize a single mesh collection
  *
  */
int SaveJSON::forSingleCollection(cJSON* mdesc, smtk::mesh::CollectionPtr collection)
{
  cJSON* jsonCollection = cJSON_CreateObject();

  std::string collectionUUID = collection->entity().toString();
  cJSON_AddItemToObject(mdesc, collectionUUID.c_str(), jsonCollection);

  cJSON_AddItemToObject(jsonCollection, "formatVersion", cJSON_CreateNumber(1));

  cJSON_AddStringToObject(jsonCollection, "name", collection->name().c_str());
  //associated model uuid of the collection
  if (!collection->associatedModel().isNull())
  {
    cJSON_AddStringToObject(
      jsonCollection, "associatedModel", collection->associatedModel().toString().c_str());
  }

  std::string interfaceName = collection->interfaceName();
  cJSON_AddStringToObject(jsonCollection, "type", interfaceName.c_str());

  bool writeMeshFileOk = true;
  //now actually write out the collection to disk, and add the location keyword
  //if we have a file.
  //This allows the importer to use the location keyword to figure out
  //if it should use the type, or fall back to the json interface
  if (!collection->writeLocation().empty())
  {
    // if there is a reference path, write out the relative path to it;
    // otherwise, write out the absolute path
    const std::string& fileWriteLocation = collection->writeLocation().referencePath().empty()
      ? collection->writeLocation().absolutePath()
      : collection->writeLocation().relativePath();

    cJSON_AddStringToObject(jsonCollection, "location", fileWriteLocation.c_str());
    writeMeshFileOk = smtk::io::writeMesh(collection, mesh::Subset::EntireCollection);
  }

  //currently we only write out the modification state after the possibility
  //of a file write. This is done so that meshes that have a file location
  //have a modified state of false
  bool isModified = collection->isModified();
  cJSON_AddBoolToObject(jsonCollection, "modified", isModified);

  ///now to dump everything inside the collection by reusing the class
  //that writes out a single meshset, but instead pass it all meshsets
  ForMeshset addInfoAboutCollection(jsonCollection);
  const bool writeMeshes = true;
  const bool writeCellAndPoints = false;
  addInfoAboutCollection.write(
    collection->meshes(), jsonCollection, writeMeshes, writeCellAndPoints);

  ///wirte out mesh properties in the collection.
  addInfoAboutCollection.writeProperties(collection, jsonCollection);

  //now walk through each meshset and dump  all the info related to it.
  cJSON* jsonMeshes = cJSON_CreateObject();
  cJSON_AddItemToObject(jsonCollection, "meshes", jsonMeshes);

  smtk::mesh::MeshSet meshes = collection->meshes();
  ForMeshset perMeshExportToJson(jsonMeshes);
  smtk::mesh::for_each(meshes, perMeshExportToJson);

  return writeMeshFileOk ? 1 : 0;
}

/**\brief Add records for \a modelIds to its parent \a sessionRec.
  *
  * This will add a "models" record to \a sessionRec, and all models
  * will be added as children of "models"
  */
int SaveJSON::addModelsRecord(
  const smtk::model::ManagerPtr modelMgr, const smtk::common::UUIDs& modelIds, cJSON* sessionRec)
{
  smtk::model::Models models;
  smtk::model::EntityRef::EntityRefsFromUUIDs(models, modelMgr, modelIds);
  return SaveJSON::addModelsRecord(modelMgr, models, sessionRec);
}

int SaveJSON::addModelsRecord(
  const smtk::model::ManagerPtr modelMgr, const smtk::model::Models& inModels, cJSON* sessionRec)
{
  // This static method's signature matches the other "add###Record" methods,
  // but parameter <modelMgr> is unused. To remove "unused parameter" warnings,
  // we therefore cast <modelMgr> to void rather than remove the ManagerPtr from
  // the signature.
  (void)modelMgr;

  cJSON* jmodels = cJSON_CreateObject();
  cJSON_AddItemToObject(sessionRec, "models", jmodels);

  // add record for each model
  smtk::model::Models::const_iterator modit;
  for (modit = inModels.begin(); modit != inModels.end(); ++modit)
  {
    //smtk::model::Model model(modelMgr, *modit);
    cJSON* jmodel = cJSON_CreateObject();

    // Write out all entities of the model, only the meta data
    smtk::model::Models currentmodels;
    currentmodels.push_back(*modit);
    SaveJSON::forEntities(jmodel, currentmodels, smtk::model::ITERATE_MODELS,
      static_cast<smtk::io::JSONFlags>(smtk::io::JSON_ENTITIES | smtk::io::JSON_PROPERTIES));

    cJSON_AddItemToObject(jmodels, modit->entity().toString().c_str(), jmodel);
  }
  return 1;
}

/**\brief Add records for meshes of \a modelIds to its parent \a sessionRec.
  *
  * This will add a "mesh_collections" record to \a sessionRec, and all meshes
  * will be added as children of "mesh_collections"
  */
int SaveJSON::addMeshesRecord(
  const smtk::model::ManagerPtr modelMgr, const smtk::common::UUIDs& modelIds, cJSON* sessionRec)
{
  smtk::model::Models models;
  smtk::model::EntityRef::EntityRefsFromUUIDs(models, modelMgr, modelIds);
  return SaveJSON::addMeshesRecord(modelMgr, models, sessionRec);
}

int SaveJSON::addMeshesRecord(
  const smtk::model::ManagerPtr modelMgr, const smtk::model::Models& inModels, cJSON* sessionRec)
{
  // Add record for each model
  smtk::model::Models::const_iterator modit;
  for (modit = inModels.begin(); modit != inModels.end(); ++modit)
  {
    // Write out related mesh collections.
    // When writing a single collection, all its MeshSets will also be written out.
    smtk::io::SaveJSON::forModelMeshes(modit->entity(), sessionRec, modelMgr);
  }
  return 1;
}

/**\brief Export log records into a cJSON array.
  *
  * You must pass in a valid cJSON array (\a logrecordarray).
  * Log entries will be appended to the end of it (and will
  * not overwrite any pre-existing array entries).
  *
  * If you specify \a start and \a end, then only the specified
  * subset of records from \a log will be added to
  * \a logrecordarray.
  * As with C++ iterators, \a start and \a end describe a
  * half-open interval.
  */
int SaveJSON::forLog(
  cJSON* logrecordarray, const smtk::io::Logger& log, std::size_t start, std::size_t end)
{
  if (!logrecordarray || logrecordarray->type != cJSON_Array)
    return -1;

  // Figure out where to stop writing entries:
  int numberOfRecords = 0;
  std::size_t finish = log.numberOfRecords();
  if (end < finish)
    finish = end;
  // Advance to the end of the array.
  cJSON* lastEntry = logrecordarray->child;
  while (lastEntry && lastEntry->next)
    lastEntry = lastEntry->next;
  cJSON* first = NULL;
  for (; start < finish; ++start)
  {
    const smtk::io::Logger::Record& rec(log.record(start));
    if (rec.message.empty())
      continue;

    cJSON* entry = cJSON_CreateArray();
    ++numberOfRecords;

    cJSON_AddItemToArray(entry, cJSON_CreateNumber(static_cast<int>(rec.severity)));
    cJSON_AddItemToArray(entry, cJSON_CreateString(rec.message.c_str()));
    if (!rec.fileName.empty())
      cJSON_AddItemToArray(entry, cJSON_CreateString(rec.fileName.c_str()));
    if (rec.lineNumber)
      cJSON_AddItemToArray(entry, cJSON_CreateNumber(rec.lineNumber));

    // Append cJSON entry to lastEntry:
    if (lastEntry)
      lastEntry->next = entry;
    else
      first = entry;
    entry->prev = lastEntry;
    lastEntry = entry;
  }
  if (!logrecordarray->child && first)
    logrecordarray->child = first;
  return numberOfRecords;
}

/**\brief Create a JSON-RPC request object.
  *
  * This variant stores a valid pointer in \a params for you to populate.
  *
  * Note that \a paramsType can be used to control the type of structure
  * created to hold parameters. JSON-RPC v2.0 allows for either an Array
  * or an Object structure (while v1 required an Array).
  * The value should be either cJSON_Object or cJSON_Array.
  * If unspecified, cJSON_Array is used.
  * Although not part of the JSON-RPC spec, True, False, and NULL parameters
  * are also accepted by this call.
  */
cJSON* SaveJSON::createRPCRequest(
  const std::string& method, cJSON*& params, const std::string& reqId, int paramsType)
{
  cJSON* rpcReq = cJSON_CreateObject();
  cJSON_AddItemToObject(rpcReq, "jsonrpc", cJSON_CreateString("2.0"));
  cJSON_AddItemToObject(rpcReq, "method", cJSON_CreateString(method.c_str()));
  cJSON_AddItemToObject(rpcReq, "id", cJSON_CreateString(reqId.c_str()));
  switch (paramsType)
  {
    case cJSON_Array:
      params = cJSON_CreateArray();
      break;
    case cJSON_Object:
      params = cJSON_CreateObject();
      break;
    case cJSON_True:
      params = cJSON_CreateTrue();
      break;
    case cJSON_False:
      params = cJSON_CreateFalse();
      break;
    case cJSON_NULL:
      params = cJSON_CreateNull();
      break;
    default:
      // TODO: Should we emit an error here?
      return rpcReq;
      break;
  }
  cJSON_AddItemToObject(rpcReq, "params", params);
  return rpcReq;
}

/**\brief Create a JSON-RPC request object.
  *
  * This variant stores a single string parameter, \a params.
  */
cJSON* SaveJSON::createRPCRequest(
  const std::string& method, const std::string& params, const std::string& reqId)
{
  cJSON* paramObj;
  cJSON* rpcReq = SaveJSON::createRPCRequest(method, paramObj, reqId, cJSON_Array);
  cJSON_AddItemToArray(paramObj, cJSON_CreateString(params.c_str()));
  return rpcReq;
}

/**\brief Copy \a arr into a JSON array of strings.
  *
  * You are responsible for managing the memory allocated for the returned
  * object, either by calling cJSON_Delete on it or adding it to another
  * cJSON node that is eventually deleted.
  */
cJSON* SaveJSON::createStringArray(const std::vector<std::string>& arr)
{
  return cJSON_CreateStringArray(&arr[0], static_cast<unsigned>(arr.size()));
}

/**\brief Copy \a arr into a JSON array of UUIDs.
  *
  * You are responsible for managing the memory allocated for the returned
  * object, either by calling cJSON_Delete on it or adding it to another
  * cJSON node that is eventually deleted.
  */
cJSON* SaveJSON::createUUIDArray(const std::vector<smtk::common::UUID>& arr)
{
  return cJSON_CreateUUIDArray(&arr[0], static_cast<unsigned>(arr.size()));
}

/**\brief Copy \a arr into a JSON array of integers.
  *
  * You are responsible for managing the memory allocated for the returned
  * object, either by calling cJSON_Delete on it or adding it to another
  * cJSON node that is eventually deleted.
  */
cJSON* SaveJSON::createIntegerArray(const std::vector<long>& arr)
{
  return cJSON_CreateLongArray(&arr[0], static_cast<unsigned>(arr.size()));
}
}
}
