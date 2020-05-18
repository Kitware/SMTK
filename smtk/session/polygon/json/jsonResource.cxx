//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/session/polygon/json/jsonResource.h"

#include "smtk/session/polygon/Resource.h"

#include "smtk/session/polygon/internal/Edge.h"
#include "smtk/session/polygon/internal/Model.h"
#include "smtk/session/polygon/internal/Vertex.h"

#include "smtk/session/polygon/json/jsonEdge.h"
#include "smtk/session/polygon/json/jsonModel.h"
#include "smtk/session/polygon/json/jsonVertex.h"

#include "smtk/model/Face.h"
#include "smtk/model/json/jsonResource.h"
#include "smtk/model/json/jsonTessellation.h"

// Define how polygon resources are serialized.
namespace smtk
{
namespace session
{
namespace polygon
{
using json = nlohmann::json;
void to_json(json& j, const smtk::session::polygon::Resource::Ptr& resource)
{
  smtk::model::to_json(j, std::static_pointer_cast<smtk::model::Resource>(resource));

  smtk::session::polygon::Session::Ptr psession = resource->polygonSession();

  json jinternal = json::object();

  jinternal["version"] = "1";

  smtk::session::polygon::internal::EntityIdToPtr::const_iterator sit;

  json entry;

  for (sit = psession->beginStorage(); sit != psession->endStorage(); ++sit)
  {
    smtk::session::polygon::internal::pmodel::Ptr pmodel;
    smtk::session::polygon::internal::edge::Ptr edge;
    smtk::session::polygon::internal::vertex::Ptr vertex;
    if ((pmodel =
           smtk::dynamic_pointer_cast<smtk::session::polygon::internal::pmodel>(sit->second)))
    {
      smtk::model::Model mm(resource, pmodel->id());
      entry = pmodel;

      // Now, because faces have no polygon-session storage but must be free cells
      // in models if they exist, we also serialize them here:
      smtk::model::Faces ff(mm.cellsAs<smtk::model::Faces>());
      for (smtk::model::Faces::iterator fit = ff.begin(); fit != ff.end(); ++fit)
      {
        json jface = json::object();
        jface["type"] = "face";
        jface["t"] = *(fit->hasTessellation());
        jinternal[fit->entity().toString()] = jface;
      }
    }
    else if ((edge =
                smtk::dynamic_pointer_cast<smtk::session::polygon::internal::edge>(sit->second)))
    {
      entry = edge;
      entry["t"] = *(smtk::model::Edge(resource, sit->second->id()).hasTessellation());
    }
    else if ((vertex =
                smtk::dynamic_pointer_cast<smtk::session::polygon::internal::vertex>(sit->second)))
    {
      entry = vertex;
      entry["t"] = *(smtk::model::Vertex(resource, sit->second->id()).hasTessellation());
    }
    if (!entry.is_null())
    {
      jinternal[sit->second->id().toString()] = entry;
    }
  }

  j["internal"] = jinternal;
}

void from_json(const json& j, smtk::session::polygon::Resource::Ptr& resource)
{
  if (resource == nullptr)
  {
    resource = smtk::session::polygon::Resource::create();
  }
  auto temp = std::static_pointer_cast<smtk::model::Resource>(resource);
  smtk::model::from_json(j, temp);

  // For now, we always create a new session when a new polygon resource is created
  smtk::session::polygon::Session::Ptr psession = smtk::session::polygon::Session::create();
  resource->setSession(psession);

  smtk::common::UUIDs models = resource->entitiesMatchingFlags(smtk::model::MODEL_ENTITY);

  json modelsObj;
  try
  {
    modelsObj = j.at("models");
  }
  catch (std::exception&)
  {
    smtkInfoMacro(resource->log(), "Expecting a \"models\" entry!");
    return;
  }

  std::map<smtk::common::UUID, std::string> existingURLs;
  // import all native models model entites, should only have meta info
  for (json::iterator modelentry = modelsObj.begin(); modelentry != modelsObj.end(); ++modelentry)
  {
    smtk::common::UUID modelid = smtk::common::UUID(std::string(modelentry.key()));
    if (modelid.isNull())
    {
      smtkInfoMacro(resource->log(), "Invalid model uuid, skipping!");
      continue;
    }
    // import native model if the model does not exist;
    // NOTE: what should we do if it already exists? erase then re-load
    // the original model from file (stored in string property "url")?
    // Else, just import meta info
    if (models.find(modelid) == models.end())
    {
      // find the model entry, and get the native model file name if it exists,
      // by looking at "url" property
      for (json::iterator curChild = modelentry->begin(); curChild != modelentry->end(); ++curChild)
      {
        // find the model id in dictionary
        if (smtk::common::UUID(curChild.key()) != modelid)
        {
          continue;
        }
        // failed to load properties is still OK
        json stringNode;
        try
        {
          stringNode = (*curChild).at("s");
        }
        catch (std::exception&)
        {
        }
        // Missing floating-point property map is not an error.
        if (!stringNode.is_null())
        {
          for (json::iterator stringProp = stringNode.begin(); stringProp != stringNode.end();
               ++stringProp)
          {
            smtk::model::StringList value = stringProp.value();
            resource->setStringProperty(modelid, stringProp.key(), value);
          }
        }
        break;
      }
    }
    else if (resource->hasStringProperty(modelid, "url"))
    {
      smtk::model::StringList const& nprop(resource->stringProperty(modelid, "url"));
      if (!nprop.empty())
      {
        existingURLs[modelid] = nprop[0];
      }
    }

    std::set<smtk::model::Model> newModels;
    // Create internal data structures for polygon session if requested:
    json jinternal;
    try
    {
      jinternal = j.at("internal");
    }
    catch (std::exception&)
    {
      std::cout << "Polygon resource does not have a valid internal json object" << std::endl;
      return;
    }
    std::map<smtk::session::polygon::internal::Id, smtk::session::polygon::internal::Id>
      childParentMap;
    std::map<smtk::session::polygon::internal::Id, smtk::session::polygon::internal::pmodel::Ptr>
      parentAddrMap;
    for (json::iterator entry = jinternal.begin(); entry != jinternal.end(); ++entry)
    {
      std::string etype;
      try
      {
        etype = (*entry).at("type").get<std::string>();
      }
      catch (std::exception&)
      {
        continue;
      }
      smtk::session::polygon::internal::Id uid = smtk::session::polygon::internal::Id(entry.key());
      smtk::session::polygon::internal::EntityPtr eptr;
      if (etype == "model")
      {
        smtk::model::Model newModel(resource, uid);
        newModels.insert(newModel);
        smtk::session::polygon::internal::pmodel::Ptr mod = (*entry);
        parentAddrMap[uid] = mod;
        mod->setSession(psession);
        eptr = mod;
      }
      else if (etype == "face")
      {
      }
      else if (etype == "edge")
      {
        smtk::session::polygon::internal::edge::Ptr edge = (*entry);
        eptr = edge;
        try
        {
          if (!(*entry).at("parent").is_null())
          {
            smtk::session::polygon::internal::Id tmp = (*entry).at("parent");
            childParentMap[uid] = smtk::session::polygon::internal::Id(tmp);
          }
        }
        catch (std::exception&)
        {
        }
      }
      else if (etype == "vertex")
      {
        smtk::session::polygon::internal::vertex::Ptr vertex = (*entry);
        eptr = vertex;
        try
        {
          if (!(*entry).at("parent").is_null())
          {
            smtk::session::polygon::internal::Id tmp = (*entry).at("parent");
            childParentMap[uid] = smtk::session::polygon::internal::Id(tmp);
          }
        }
        catch (std::exception&)
        {
        }
      }

      // read tessellation information if it exists
      {
        auto jtess = entry->find("t");
        if (jtess != entry->end())
        {
          // Now extract graphics primitives from the JSON data.
          // We should fetch the metadata->formatVersion and verify it,
          // but I don't think it makes any difference to the fields
          // we rely on... yet.
          smtk::model::UUIDsToTessellations::iterator tessIt = resource->tessellations().find(uid);
          if (tessIt == resource->tessellations().end())
          {
            smtk::model::Tessellation blank;
            tessIt = resource->tessellations()
                       .insert(std::pair<smtk::common::UUID, smtk::model::Tessellation>(uid, blank))
                       .first;
          }
          tessIt->second = *jtess;
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
    smtk::session::polygon::internal::EntityIdToPtr::const_iterator sit;
    for (sit = psession->beginStorage(); sit != psession->endStorage(); ++sit)
    {
      smtk::session::polygon::internal::Id parentId = childParentMap[sit->first];
      if (parentId)
      {
        smtk::session::polygon::internal::pmodel::Ptr parentAddr = parentAddrMap[parentId];
        sit->second->setParent(parentAddr.get());
        smtk::session::polygon::internal::vertex::Ptr vert =
          smtk::dynamic_pointer_cast<smtk::session::polygon::internal::vertex>(sit->second);
        if (vert)
        {
          parentAddr->addVertexIndex(vert);
        }
      }
    }
  }
}
} // namespace polygon
} // namespace session
} // namespace smtk
