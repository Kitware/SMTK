//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#include <detail/fromRemus.h>

#include "smtk/model/Resource.h"

#include "smtk/attribute/Resource.h"

#include "smtk/common/UUID.h"

#include "smtk/io/AttributeReader.h"
#include "smtk/io/LoadJSON.h"
#include "smtk/io/Logger.h"
#include "smtk/io/SaveJSON.h"

namespace
{

//----------------------------------------------------------------------------
smtk::model::Model make_as_model(smtk::model::ResourcePtr& resource, const smtk::common::UUID& ent)
{
  return smtk::model::Model(resource, ent);
}

//----------------------------------------------------------------------------
smtk::model::Model make_as_model(smtk::model::ResourcePtr&, const smtk::model::EntityRef& ent)
{
  return smtk::model::Model(ent);
}

//----------------------------------------------------------------------------
template <typename T>
void load_selected_models(smtk::model::ResourcePtr& resource, const T& itemsTomesh,
  std::vector<detail::FacesOfModel>& modelsToMesh)
{

  modelsToMesh.reserve(itemsTomesh.size());
  for (auto i = itemsTomesh.cbegin(); i != itemsTomesh.cend(); ++i)
  {
    smtk::model::Model m = make_as_model(resource, *i);
    if (m.isValid())
    {
      detail::FacesOfModel fom;
      fom.m_model = m;

      //find all the faces for this model
      smtk::model::EntityIterator it;
      it.traverse(m, smtk::model::ITERATE_CHILDREN);
      for (it.begin(); !it.isAtEnd(); ++it)
      {
        if (it->isFace())
        {
          fom.m_faces.push_back(it->as<smtk::model::Face>());
        }
      }

      modelsToMesh.push_back(fom);
    }
  }
}
}

namespace detail
{

//----------------------------------------------------------------------------
Resources::Resources(
  smtk::model::ResourcePtr m, smtk::attribute::ResourcePtr s, const std::vector<FacesOfModel>& f)
  : m_model(m)
  , m_mesh(m->meshes())
  , m_attributes(s)
  , m_modelsToMesh(f)
{
}

//----------------------------------------------------------------------------
detail::Resources deserialize_smtk_model(const remus::proto::JobContent& jsonModelData,
  const remus::proto::JobContent& meshAttributeData,
  const remus::proto::JobContent& smtkModelIdsToMesh)
{
  //we should load in the test2D.json file as an smtk to model
  smtk::model::ResourcePtr resource = smtk::model::Resource::create();
  if (jsonModelData.dataSize() > 0 &&
    jsonModelData.sourceType() == remus::common::ContentSource::Memory)
  { //todo handle file based model data
    smtk::io::LoadJSON::intoModelResource(jsonModelData.data(), resource);
  }

  //now that the model is loaded apply the mesh attributes to the model
  smtk::attribute::ResourcePtr attResource = smtk::attribute::Resource::create();
  if (meshAttributeData.dataSize() > 0)
  {
    attResource->setRefModelResource(resource);
    smtk::io::AttributeReader reader;
    smtk::io::Logger inputLogger;
    reader.readContents(
      attResource, meshAttributeData.data(), meshAttributeData.dataSize(), inputLogger);
  }

  //need to make smtkModelIdsToMesh be optional
  std::vector<FacesOfModel> modelsToMesh;
  if (smtkModelIdsToMesh.dataSize() > 0)
  {
    //parse the contents of smtkModelIdsToMesh
    smtk::common::UUIDArray uuidsToMesh;
    cJSON* root = cJSON_Parse(smtkModelIdsToMesh.data());
    cJSON* ids = cJSON_GetObjectItem(root, "ids");
    smtk::io::LoadJSON::getUUIDArrayFromJSON(ids->child, uuidsToMesh);
    cJSON_Delete(root);
    load_selected_models(resource, uuidsToMesh, modelsToMesh);
  }
  else
  {
    smtk::model::EntityRefs modelEnts =
      resource->entitiesMatchingFlagsAs<smtk::model::EntityRefs>(smtk::model::MODEL_ENTITY);
    load_selected_models(resource, modelEnts, modelsToMesh);
  }

  return Resources(resource, attResource, modelsToMesh);
}
}
