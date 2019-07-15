//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/mesh/json/jsonResource.h"

// NOTE: If you forget to include jsonUUID.h, everything compiles and runs
//       correctly in debug mode, but UUID serialization breaks down in release
//       mode. My guess is that an nlohmann template specialization for UUIDs
//       needs to exist before it is first called; otherwise, a symbol is
//       generated for UUIDs and its use is continued in favor of the explicit
//       one encountered afterwards.
#include "smtk/common/json/jsonUUID.h"

#include "smtk/mesh/json/Interface.h"
#include "smtk/mesh/json/jsonHandleRange.h"
#include "smtk/mesh/json/jsonMeshInfo.h"

#include "smtk/mesh/core/ForEachTypes.h"

#include "smtk/resource/json/jsonResource.h"

namespace
{
const std::string dirName = "dirichlet";
const std::string neumName = "neumann";

class ForMeshset : public smtk::mesh::MeshForEach
{
public:
  ForMeshset(nlohmann::json& j)
    : smtk::mesh::MeshForEach()
    , m_json(j)
  {
  }

  void write(
    const smtk::mesh::MeshSet& mesh, nlohmann::json& j, bool writeMeshes, bool writeCellAndPoints)
  {
    j["cell_types"] = mesh.types().cellTypes().to_string();

    //needs to be by value since cells() will go out out of scope, and
    //we don't want a reference to a stack object that has gone out of scope
    if (writeMeshes)
    {
      j["meshIds"] = mesh.range();
    }

    if (writeCellAndPoints)
    {
      j["cells"] = mesh.cells().range();
      j["points"] = mesh.points().range();
    }

    j["id"] = mesh.id();
    j["modelEntityIds"] = mesh.modelEntityIds();

    std::vector<int> domainValues;
    for (auto& domain : mesh.domains())
    {
      domainValues.push_back(domain.value());
    }
    j["domains"] = domainValues;

    nlohmann::json bcs_json;

    std::vector<smtk::mesh::Dirichlet> dirichlets = mesh.dirichlets();
    for (std::size_t i = 0; i < dirichlets.size(); ++i)
    {
      nlohmann::json bc_json;
      bc_json["type"] = dirName;
      bc_json["value"] = dirichlets[i].value();
      bcs_json.push_back(bc_json);
    }
    std::vector<smtk::mesh::Neumann> neumanns = mesh.neumanns();
    for (std::size_t i = 0; i < neumanns.size(); ++i)
    {
      nlohmann::json bc_json;
      bc_json["type"] = neumName;
      bc_json["value"] = neumanns[i].value();
      bcs_json.push_back(bc_json);
    }

    j["boundary_conditions"] = bcs_json;
  }

  void forMesh(smtk::mesh::MeshSet& mesh) override
  {
    nlohmann::json j;

    const bool writeMeshes = false;
    const bool writeCellAndPoints = true;
    this->write(mesh, j, writeMeshes, writeCellAndPoints);

    m_json.push_back(j);
  }

private:
  nlohmann::json& m_json;
};
}

namespace smtk
{
namespace mesh
{
void to_json(nlohmann::json& j, const ResourcePtr& resource)
{
  smtk::resource::to_json(j, smtk::static_pointer_cast<smtk::resource::Resource>(resource));

  ForMeshset addInfoAboutResource(j);
  const bool writeMeshes = true;
  const bool writeCellAndPoints = false;

  nlohmann::json info_json;
  addInfoAboutResource.write(resource->meshes(), info_json, writeMeshes, writeCellAndPoints);
  j["info"] = info_json;

  //now walk through each meshset and dump  all the info related to it.
  nlohmann::json meshes_json;

  smtk::mesh::MeshSet meshes = resource->meshes();
  ForMeshset perMeshExportToJson(meshes_json);
  smtk::mesh::for_each(meshes, perMeshExportToJson);

  j["meshes"] = meshes_json;
}

void from_json(const nlohmann::json& j, ResourcePtr& resource)
{
  if (!resource)
  {
    // It's derived class's responsibility to create a valid mresource
    return;
  }

  auto temp = std::static_pointer_cast<smtk::resource::Resource>(resource);
  smtk::resource::from_json(j, temp);

  std::vector<smtk::mesh::json::MeshInfo> meshInfos = j;

  smtk::mesh::json::InterfacePtr interface =
    smtk::dynamic_pointer_cast<smtk::mesh::json::Interface>(resource->interface());

  if (interface)
  {
    interface->addMeshes(meshInfos);
  }
}
}
}
