//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/polygon/SessionIOJSON.h"

#include "smtk/common/CompilerInformation.h"

#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/common/UUID.h"
#include "smtk/io/ExportJSON.h"
#include "smtk/io/ExportJSON.txx"
#include "smtk/io/ImportJSON.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Operator.h"

#include "smtk/bridge/polygon/Session.h"
#include "smtk/bridge/polygon/internal/Edge.h"
#include "smtk/bridge/polygon/internal/Model.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "boost/filesystem.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

#include "cJSON.h"

using namespace boost::filesystem;

namespace smtk {
  namespace bridge {
    namespace polygon {

/**\brief Decode information from \a sessionRec for the given \a mgr.
  *
  * Subclasses should return 1 on success and 0 on failure.
  */
int SessionIOJSON::importJSON(
  smtk::model::ManagerPtr mgr,
  const smtk::model::SessionPtr& session,
  cJSON* sessionRec,
  bool loadNativeModels)
{
  smtk::bridge::polygon::Session::Ptr psession =
    smtk::dynamic_pointer_cast<Session>(session);

  smtk::common::UUIDs models =
    mgr->entitiesMatchingFlags(smtk::model::MODEL_ENTITY);

  cJSON* modelsObj = cJSON_GetObjectItem(sessionRec, "models");
  if (!modelsObj)
    {
    smtkInfoMacro(mgr->log(), "Expecting a \"models\" entry!");
    return 0;
    }

  std::map<smtk::common::UUID, std::string> existingURLs;
  cJSON* modelentry;
  // import all native models model entites, should only have meta info
  for (modelentry = modelsObj->child; modelentry; modelentry = modelentry->next)
    {
    if (!modelentry->string || !modelentry->string[0])
      continue;

    smtk::common::UUID modelid = smtk::common::UUID(modelentry->string);
    if (modelid.isNull())
      {
      smtkInfoMacro(mgr->log(), "Invalid model uuid, skipping!");
      continue;
      }
    // import native model if the model does not exist;
    // NOTE: what should we do if it already exists? erase then re-load
    // the original model from file (stored in string property "url")?
    // Else, just import meta info
    if(models.find(modelid) == models.end())
      {
      // find the model entry, and get the native model file name if it exists,
      // by looking at "url" property
      for (cJSON* curChild = modelentry->child; curChild; curChild = curChild->next)
        {
        if (!curChild->string || !curChild->string[0])
          {
          continue;
          }
        // find the model id in dictionary
        if (smtk::common::UUID(curChild->string) != modelid)
          {
          continue;
          }
        // failed to load properties is still OK
        smtk::io::ImportJSON::ofManagerStringProperties(modelid, curChild, mgr);
        break;
        }

      }
    else if(mgr->hasStringProperty(modelid, "url"))
      {
      smtk::model::StringList const& nprop(mgr->stringProperty(modelid, "url"));
      if (!nprop.empty())
        {
        existingURLs[modelid] = nprop[0];
        }
      }
    }

  std::set<smtk::model::Model> newModels;
  // Create internal data structures for polygon session if requested:
  cJSON* idata = cJSON_GetObjectItem(sessionRec, "internal");
  if (loadNativeModels && idata && idata->type == cJSON_Object)
    {
    std::map<internal::Id, internal::Id> childParentMap;
    std::map<internal::Id, internal::pmodel::Ptr> parentAddrMap;
    for (cJSON* entry = idata->child; entry; entry = entry->next)
      {
      cJSON* trec;
      if (
        !entry->string ||
        !entry->string[0] ||
        entry->type != cJSON_Object ||
        !(trec = cJSON_GetObjectItem(entry, "type")))
        {
        continue;
        }
      internal::Id uid = internal::Id(entry->string);
      std::string etype = trec->valuestring;
      internal::EntityPtr eptr;
      if (etype == "model")
        {
        smtk::model::Model newModel(mgr, uid);
        newModels.insert(newModel);
        internal::pmodel::Ptr mod = this->deserializeModel(entry, newModel);
        parentAddrMap[uid] = mod;
        mod->setSession(psession.get());
        eptr = mod;
        }
      else if (etype == "face")
        {
        this->deserializeFace(entry, smtk::model::Face(mgr, uid));
        }
      else if (etype == "edge")
        {
        eptr = this->deserializeEdge(entry, smtk::model::Edge(mgr, uid));
        trec = cJSON_GetObjectItem(entry, "parent");
        if (trec && trec->type == cJSON_String && trec->valuestring && trec->valuestring[0])
          {
          childParentMap[uid] = internal::Id(trec->valuestring);
          }
        }
      else if (etype == "vertex")
        {
        eptr = this->deserializeVertex(entry, smtk::model::Vertex(mgr, uid));
        trec = cJSON_GetObjectItem(entry, "parent");
        if (trec && trec->type == cJSON_String && trec->valuestring && trec->valuestring[0])
          {
          childParentMap[uid] = internal::Id(trec->valuestring);
          }
        }
      if (eptr)
        {
        eptr->setId(uid);
        psession->addStorage(uid, eptr);
        }
      }

    // Add "parent" entries to session storage entries in childParentMap
    // Do this after processing all "internal" entries since models may
    // appear after their children (JSON allows dict item shuffling).
    //
    // Also, make sure model vertices are registered with their parent
    // model's point-to-id lookup map.
    internal::EntityIdToPtr::const_iterator sit;
    for (sit = psession->beginStorage(); sit != psession->endStorage(); ++sit)
      {
      internal::Id parentId = childParentMap[sit->first];
      if (parentId)
        {
        internal::pmodel::Ptr parentAddr = parentAddrMap[parentId];
        sit->second->setParent(parentAddr.get());
        internal::vertex::Ptr vert = smtk::dynamic_pointer_cast<internal::vertex>(sit->second);
        if (vert)
          {
          parentAddr->addVertexIndex(vert);
          }
        }
      }
    }

  int status = this->loadModelsRecord(mgr, sessionRec);
  status &= this->loadMeshesRecord(mgr, sessionRec);
  // recover "url" property for models already loaded
  std::map<smtk::common::UUID, std::string>::const_iterator mit;
  for(mit = existingURLs.begin(); mit != existingURLs.end(); ++mit)
    {
    mgr->setStringProperty(mit->first, "url", mit->second);
    }

  // set the SessionRef for the newModels
  smtk::model::SessionRef sess(mgr, psession->sessionId());
  std::set<smtk::model::Model>::const_iterator modit;
  for(modit = newModels.begin(); modit != newModels.end(); ++modit)
    {
    sess.addModel(*modit);
    }

  return status;
}

/**\brief Encode information into \a sessionRec for the given \a mgr.
  *
  * Subclasses should return 1 on success and 0 on failure.
  */
int SessionIOJSON::exportJSON(
  smtk::model::ManagerPtr mgr,
  const smtk::model::SessionPtr& session,
  cJSON* sessionRec,
  bool writeNativeModels)
{
  (void)mgr;
  (void)session;
  (void)sessionRec;
  (void)writeNativeModels;
  return 1;
}

/**\brief Encode information into \a sessionRec for the given \a modelId of the \a mgr.
  *
  * Subclasses should return 1 on success and 0 on failure.
  */
int SessionIOJSON::exportJSON(
  smtk::model::ManagerPtr mgr,
  const smtk::model::SessionPtr& session,
  const smtk::common::UUIDs& modelIds,
  cJSON* sessionRec,
  bool writeNativeModels)
{
  SessionPtr psession = smtk::dynamic_pointer_cast<Session>(session);
  if (writeNativeModels)
    {
    cJSON* idata = cJSON_CreateObject();
    cJSON_AddItemToObject(sessionRec, "internal", idata);
    cJSON_AddItemToObject(idata, "version", cJSON_CreateString("1"));
    internal::EntityIdToPtr::const_iterator sit;
    cJSON* entry;
    for (sit = psession->beginStorage(); sit != psession->endStorage(); ++sit)
      {
      // Only write the entity if its owning model is one in the list we're given:
      if (
        (!sit->second->parent() && modelIds.find(sit->second->id()) != modelIds.end()) || // We are a model and listed in modelIds
        (sit->second->parent() && modelIds.find(sit->second->parent()->id()) != modelIds.end())) // Our parent is in modelIds
        {
        internal::pmodel::Ptr pmodel;
        internal::edge::Ptr edge;
        internal::vertex::Ptr vertex;
        if ((pmodel = smtk::dynamic_pointer_cast<internal::pmodel>(sit->second)))
          {
          smtk::model::Model mm(mgr, pmodel->id());
          entry = this->serializeModel(pmodel, mm);
          // Now, because faces have no polygon-session storage but must be free cells
          // in models if they exist, we also serialize them here:
          smtk::model::Faces ff(mm.cellsAs<smtk::model::Faces>());
          for (smtk::model::Faces::iterator fit = ff.begin(); fit != ff.end(); ++fit)
            {
            cJSON_AddItemToObject(
              idata, fit->entity().toString().c_str(), this->serializeFace(*fit));
            }
          }
        else if ((edge = smtk::dynamic_pointer_cast<internal::edge>(sit->second)))
          {
          entry = this->serializeEdge(edge, smtk::model::Edge(mgr, edge->id()));
          }
        else if ((vertex = smtk::dynamic_pointer_cast<internal::vertex>(sit->second)))
          {
          entry = this->serializeVertex(vertex, smtk::model::Vertex(mgr, vertex->id()));
          }
        if (entry)
          {
          cJSON_AddItemToObject(idata, sit->second->id().toString().c_str(), entry);
          }
        }
      else
        {
        std::cout
          << "Ignoring " << sit->second
          << " id " << sit->second->id().toString()
          << " parent " << sit->second->parent()
          << " parent id " << (sit->second->parent() ? sit->second->parent()->id().toString().c_str() : "nope")
          << " type " << sit->second->classname()
          << "\n";
        }
      }

    /*
    smtk::common::UUIDs::const_iterator modit;
    cJSON* idata = cJSON_CreateObject();
    cJSON_AddItemToObject(sessionRec, "internal", idata);
    for (modit = modelIds.begin(); modit != modelIds.end(); ++modit)
      {
      smtk::model::Model model(mgr, *modit);
      std::cout << "Add to cJSON for " << model.name() << "\n";
      internal::pmodel::Ptr imodel = psession->findStorage<internal::pmodel>(model.entity());
      cJSON* modelEntry = this->serializeModel(imodel);
      cJSON_AddItemToObject(idata, model.entity().toString().c_str(), modelEntry);

      }
      */
    }

  return 1;
}

cJSON* SessionIOJSON::serializeModel(internal::pmodel::Ptr model, const smtk::model::Model& mod)
{
  (void)mod;
  cJSON* result = cJSON_CreateObject();
  cJSON_AddItemToObject(result, "type", cJSON_CreateString("model"));
  cJSON_AddItemToObject(result, "origin", cJSON_CreateDoubleArray(model->origin(), 3));
  cJSON_AddItemToObject(result, "x axis", cJSON_CreateDoubleArray(model->xAxis(), 3));
  cJSON_AddItemToObject(result, "y axis", cJSON_CreateDoubleArray(model->yAxis(), 3));
  cJSON_AddItemToObject(result, "z axis", cJSON_CreateDoubleArray(model->zAxis(), 3));
  cJSON_AddItemToObject(result, "i axis", cJSON_CreateDoubleArray(model->iAxis(), 3));
  cJSON_AddItemToObject(result, "j axis", cJSON_CreateDoubleArray(model->jAxis(), 3));
  cJSON_AddItemToObject(result, "feature size", cJSON_CreateNumber(model->featureSize()));
  // Encode model scale carefully since cJSON cannot store large integers faithfully:
  int modelScaleBytes[8];
  long long mscale = static_cast<long long>(model->modelScale());
  for (int i = 0; i < 8; ++i)
    {
    modelScaleBytes[7 - i] = mscale & 0xff;
    mscale >>= 8;
    }
  cJSON_AddItemToObject(result, "model scale", cJSON_CreateIntArray(modelScaleBytes, 8));
  return result;
}

cJSON* SessionIOJSON::serializeFace(const smtk::model::Face& face)
{
  cJSON* result = cJSON_CreateObject();
  cJSON_AddItemToObject(result, "type", cJSON_CreateString("face"));
  smtk::io::ExportJSON::forManagerTessellation(face.entity(), result, face.manager());
  return result;
}

cJSON* SessionIOJSON::serializeEdge(internal::EdgePtr edge, const smtk::model::Edge& e)
{
  cJSON* result = cJSON_CreateObject();
  cJSON_AddItemToObject(result, "type", cJSON_CreateString("edge"));
  if (edge->parent())
    {
    cJSON_AddItemToObject(result, "parent", cJSON_CreateString(edge->parent()->id().toString().c_str()));
    }
  std::size_t np = edge->pointsSize();
  const int stride = 2 /* coords per pt */ * 4 /* ints per coord */;
  std::vector<long> ptdata(np * stride, 0);
  internal::PointSeq::const_iterator pit = edge->pointsBegin();
  for (std::size_t i = 0; i < np; ++i, ++pit)
    {
    internal::Coord c = pit->x();
    unsigned long long& uc(*reinterpret_cast<unsigned long long*>(&c));
    ptdata[i * stride + 0] = uc & 0xffff;
    uc >>= 16;
    ptdata[i * stride + 1] = uc & 0xffff;
    uc >>= 16;
    ptdata[i * stride + 2] = uc & 0xffff;
    uc >>= 16;
    ptdata[i * stride + 3] = uc & 0xffff;

    c = pit->y();
    ptdata[i * stride + 4] = uc & 0xffff;
    uc >>= 16;
    ptdata[i * stride + 5] = uc & 0xffff;
    uc >>= 16;
    ptdata[i * stride + 6] = uc & 0xffff;
    uc >>= 16;
    ptdata[i * stride + 7] = uc & 0xffff;
    }
  cJSON_AddItemToObject(result, "points", smtk::io::ExportJSON::createIntegerArray(ptdata));
  smtk::io::ExportJSON::forManagerTessellation(e.entity(), result, e.manager());
  return result;
}

cJSON* SessionIOJSON::serializeVertex(internal::VertexPtr vert, const smtk::model::Vertex& v)
{
  cJSON* result = cJSON_CreateObject();
  cJSON_AddItemToObject(result, "type", cJSON_CreateString("vertex"));
  if (vert->parent())
    {
    cJSON_AddItemToObject(result, "parent", cJSON_CreateString(vert->parent()->id().toString().c_str()));
    }
  // Store exact integer point coordinates safely:
  std::vector<long> ptdata(8, 0);
  internal::Coord c = vert->point().x();
  ptdata[0] = c & 0xffff;
  c >>= 16;
  ptdata[1] = c & 0xffff;
  c >>= 16;
  ptdata[2] = c & 0xffff;
  c >>= 16;
  ptdata[3] = c & 0xffff;
  c = vert->point().y();
  ptdata[4] = c & 0xffff;
  c >>= 16;
  ptdata[5] = c & 0xffff;
  c >>= 16;
  ptdata[6] = c & 0xffff;
  c >>= 16;
  ptdata[7] = c & 0xffff;
  cJSON_AddItemToObject(result, "point", smtk::io::ExportJSON::createIntegerArray(ptdata));
  cJSON* iearr = cJSON_CreateArray();
  cJSON_AddItemToObject(result, "edges", iearr);
  // Store CCW-ordered list of incident-edge records:
  internal::vertex::incident_edges::const_iterator ieit;
  cJSON** loc = &iearr->child;
  for (ieit = vert->edgesBegin(); ieit != vert->edgesEnd(); ++ieit)
    {
    *loc = this->serializeIncidentEdgeRecord(*ieit);
    loc = &((*loc)->next); // fast way to append to cJSON array
    }
  // Store tessellation to avoid boost re-compute.
  smtk::io::ExportJSON::forManagerTessellation(v.entity(), result, v.manager());
  return result;
}

cJSON* SessionIOJSON::serializeIncidentEdgeRecord(const internal::vertex::incident_edge_data& rec)
{
  cJSON* result = cJSON_CreateObject();
  if (rec.edgeId())
    {
    cJSON_AddItemToObject(result, "edge", cJSON_CreateString(rec.edgeId().toString().c_str()));
    cJSON_AddItemToObject(result, "edgeout", cJSON_CreateBool(rec.isEdgeOutgoing()));
    }
  if (rec.clockwiseFaceId())
    {
    cJSON_AddItemToObject(result, "cwface", cJSON_CreateString(rec.clockwiseFaceId().toString().c_str()));
    }
  return result;
}

internal::pmodel::Ptr SessionIOJSON::deserializeModel(cJSON* record, const smtk::model::Model& m)
{
  (void)m;
  internal::pmodel::Ptr result;
  std::vector<double> origin;
  std::vector<double> xAxis;
  std::vector<double> yAxis;
  std::vector<double> zAxis;
  std::vector<double> iAxis;
  std::vector<double> jAxis;
  std::vector<long> msData;
  double featureSize;
  long long modelScale;
  cJSON* item;

  bool ok = true;

  ok &= (item = cJSON_GetObjectItem(record, "origin")) && smtk::io::ImportJSON::getRealArrayFromJSON(item, origin) == 3;
  ok &= (item = cJSON_GetObjectItem(record, "x axis")) && smtk::io::ImportJSON::getRealArrayFromJSON(item, xAxis) == 3;
  ok &= (item = cJSON_GetObjectItem(record, "y axis")) && smtk::io::ImportJSON::getRealArrayFromJSON(item, yAxis) == 3;
  ok &= (item = cJSON_GetObjectItem(record, "z axis")) && smtk::io::ImportJSON::getRealArrayFromJSON(item, zAxis) == 3;
  ok &= (item = cJSON_GetObjectItem(record, "i axis")) && smtk::io::ImportJSON::getRealArrayFromJSON(item, iAxis) == 3;
  ok &= (item = cJSON_GetObjectItem(record, "j axis")) && smtk::io::ImportJSON::getRealArrayFromJSON(item, jAxis) == 3;
  ok &= (item = cJSON_GetObjectItem(record, "model scale")) && smtk::io::ImportJSON::getIntegerArrayFromJSON(item, msData) == 8;
  ok &= (item = cJSON_GetObjectItem(record, "feature size")) && item->type == cJSON_Number && (featureSize = item->valuedouble) > 0;

  if (ok)
    {
    modelScale = 0;
    for (int i = 0; i < 8; ++i)
      {
      modelScale += (msData[7 - i] << (8 * i));
      }
    ok &= modelScale > 0;
    }

  if (ok)
    {
    result = internal::pmodel::create();
    result->restoreModel(
      origin, xAxis, yAxis, zAxis, iAxis, jAxis,
      featureSize, modelScale);
    }

  return result;
}

void SessionIOJSON::deserializeFace(cJSON* record, const smtk::model::Face& face)
{
  // Fetch tessellation
  smtk::io::ImportJSON::ofManagerTessellation(face.entity(), record, face.manager());
}

internal::EdgePtr SessionIOJSON::deserializeEdge(cJSON* record, const smtk::model::Edge& e)
{
  internal::edge::Ptr result;
  std::vector<long> ptdata;
  cJSON* item;

  bool ok = true;

  ok &= (item = cJSON_GetObjectItem(record, "points")) && smtk::io::ImportJSON::getIntegerArrayFromJSON(item, ptdata) > 0;
  ok &= (item = cJSON_GetObjectItem(record, "t")) && item->type == cJSON_Object;

  if (ok)
    {
    result = internal::edge::create();
    const int stride = 2 /* coords per pt */ * 4 /* ints per coord */;
    std::size_t np = ptdata.size() / stride;
    std::vector<long>::const_iterator cit = ptdata.begin();
    for (std::size_t i = 0; i < np; ++i)
      {
      internal::Coord xy[2] = {0, 0};
      for (int j = 0; j < 2; ++j)
        {
        unsigned long long& uc(*reinterpret_cast<unsigned long long*>(&xy[j]));
        uc = *cit;
        ++cit;
        uc |= (static_cast<unsigned long long>(*cit) << 16);
        ++cit;
        uc |= (static_cast<unsigned long long>(*cit) << 32);
        ++cit;
        uc |= (static_cast<unsigned long long>(*cit) << 48);
        ++cit;
        }
      result->points().insert(result->pointsEnd(), internal::Point(xy[0], xy[1]));
      }

    // Fetch tessellation
    smtk::io::ImportJSON::ofManagerTessellation(e.entity(), record, e.manager());
    }

  return result;
}

internal::VertexPtr SessionIOJSON::deserializeVertex(cJSON* record, const smtk::model::Vertex& v)
{
  internal::vertex::Ptr result;
  std::vector<long> ptdata;
  cJSON* item;

  bool ok = true;

  ok &= (item = cJSON_GetObjectItem(record, "t")) && item->type == cJSON_Object;
  ok &= (item = cJSON_GetObjectItem(record, "point")) && smtk::io::ImportJSON::getIntegerArrayFromJSON(item, ptdata) > 0;
  ok &= (item = cJSON_GetObjectItem(record, "edges")) && item->type == cJSON_Array;

  if (ok)
    {
    // Create an internal vertex record
    result = internal::vertex::create();

    // Set the integer point coordinates
    const int stride = 2 /* coords per pt */ * 4 /* ints per coord */;
    std::size_t np = ptdata.size() / stride;
    std::vector<long>::const_iterator cit = ptdata.begin();
    if (np > 0)
      {
      internal::Coord xy[2] = {0, 0};
      for (int j = 0; j < 2; ++j)
        {
        unsigned long long& uc(*reinterpret_cast<unsigned long long*>(&xy[j]));
        uc = *cit;
        ++cit;
        uc |= (static_cast<unsigned long long>(*cit) << 16);
        ++cit;
        uc |= (static_cast<unsigned long long>(*cit) << 32);
        ++cit;
        uc |= (static_cast<unsigned long long>(*cit) << 48);
        ++cit;
        }
      result->point() = internal::Point(xy[0], xy[1]);
      }

    // Deserialize the CCW-ordered list of incident edges:
    for (cJSON* er = item->child; er; er = er->next)
      {
      result->dangerousAppendEdge(this->deserializeIncidentEdgeRecord(er));
      }

    // Fetch tessellation
    smtk::io::ImportJSON::ofManagerTessellation(v.entity(), record, v.manager());
    }
  return result;
}

internal::vertex::incident_edge_data SessionIOJSON::deserializeIncidentEdgeRecord(cJSON* record)
{
  bool haveEdge = false;
  bool haveFace = false;
  internal::Id edgeId;
  internal::Id faceId;
  bool edgeOut;
  cJSON* entry;
  if ((entry = cJSON_GetObjectItem(record, "edge")))
    {
    edgeId = smtk::common::UUID(entry->valuestring);
    entry = cJSON_GetObjectItem(record, "edgeout");
    edgeOut = (entry ? entry->type == cJSON_True : false);
    haveEdge = true;
    }
  if ((entry = cJSON_GetObjectItem(record, "cwface")))
    {
    faceId = smtk::common::UUID(entry->valuestring);
    haveFace = true;
    }
  return (
    haveEdge && haveFace ? internal::vertex::incident_edge_data(edgeId, edgeOut, faceId) : (
      haveEdge ? internal::vertex::incident_edge_data(edgeId, edgeOut) : (
        haveFace ? internal::vertex::incident_edge_data(faceId) : (
          internal::vertex::incident_edge_data()))));
}

    } // namespace polygon
  } // namespace bridge
} // namespace smtk
