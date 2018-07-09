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
#ifndef filigree_detail_fromRemus_h
#define filigree_detail_fromRemus_h

#include "remus/proto/JobContent.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/model/Face.h"
#include "smtk/model/Model.h"

namespace detail
{

struct FacesOfModel
{
  smtk::model::Model m_model;
  std::vector<smtk::model::Face> m_faces;
};

struct Resources
{
  Resources() {}

  Resources(smtk::model::ResourcePtr m, smtk::attribute::ResourcePtr s,
    const std::vector<FacesOfModel>& fom);

  bool valid() const { return ((!!m_model) && (!!m_attributes)); }

  smtk::model::ResourcePtr m_model;
  smtk::mesh::ManagerPtr m_mesh;
  smtk::attribute::ResourcePtr m_attributes;
  std::vector<FacesOfModel> m_modelsToMesh;
};
//----------------------------------------------------------------------------
//converts all the remus job content into a smtk model instance with an
//associated smtk attribute resource that represents the meshing controls.
//
//We support meshing a subset of all the models inside the resource through
//the smtkModelIdsToMesh data
//
detail::Resources deserialize_smtk_model(const remus::proto::JobContent& jsonModelData,
  const remus::proto::JobContent& meshAttributeData,
  const remus::proto::JobContent& smtkModelIdsToMesh);
}
#endif
