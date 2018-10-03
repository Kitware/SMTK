//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/mesh/json/jsonMeshInfo.h"

#include "smtk/common/json/jsonUUID.h"

#include "smtk/mesh/core/QueryTypes.h"
#include "smtk/mesh/json/MeshInfo.h"
#include "smtk/mesh/json/jsonHandleRange.h"

namespace
{
const std::string dirName = "dirichlet";
const std::string neumName = "neumann";
}

namespace smtk
{
namespace mesh
{
namespace json
{
void to_json(nlohmann::json& /*j*/, const std::vector<MeshInfo>& /*meshInfos*/)
{
  // smtk::mesh::HandleRange meshIds;
  // for (auto& meshInfo : meshInfos)
  // {
  //   meshIds.insert(meshInfo.mesh());
  // }

  // j["meshIds"] = meshIds;

  // nlohmann::json meshes_json;
  // for (auto& meshInfo : meshInfos)
  // {
  //   nlohmann::json mesh_json;
  //   mesh_json["cells"] = meshInfo.cells();
  //   mesh_json["points"] = meshInfo.points();

  //   mesh_json["id"] = meshInfo.id();
  //   mesh_json["modelEntityIds"] = meshInfo.modelUUIDS();

  //   std::vector<int> domainValues;
  //   if (meshInfo.domains().empty() == false)
  //   {
  //     for (std::size_t i = 0; i < meshInfo.domains().size(); i++)
  //     {
  //       domainValues.push_back(meshInfo.domains().at(i).value());
  //     }
  //   }
  //   mesh_json["domains"] = domainValues;

  //   nlohmann::json bcs_json;
  //   for (auto& bc : meshInfo.dirichlets())
  //   {
  //     nlohmann::json bc_json;
  //     bc_json["type"] = dirName;
  //     bc_json["value"] = bc.value();
  //     bcs_json.push_back(bc_json);
  //   }
  //   for (auto& bc : meshInfo.neumanns())
  //   {
  //     nlohmann::json bc_json;
  //     bc_json["type"] = neumName;
  //     bc_json["value"] = bc.value();
  //     bcs_json.push_back(bc_json);
  //   }

  //   mesh_json["boundary_conditions"] = bcs_json;

  //   meshes_json.push_back(mesh_json);
  // }

  // j["meshes"] = meshes_json;
}

void from_json(const nlohmann::json& /*j*/, std::vector<MeshInfo>& /*meshInfos*/)
{
  //extract the ids of all the meshes and make it a smtk::mesh::Handle

  // smtk::mesh::HandleRange meshIds = j.at("meshIds");

  // nlohmann::json meshes_json = j.at("meshes");

  // auto meshIdIter = smtk::mesh::rangeElementsBegin(meshIds);
  // for (nlohmann::json::iterator mesh_json_it = meshes_json.begin();
  //      mesh_json_it != meshes_json.end(); ++mesh_json_it, ++meshIdIter)
  // {
  //   nlohmann::json mesh_json = *mesh_json_it;

  //   //get the Typeset from the json
  //   std::string cell_bit_types = mesh_json.at("cell_types");
  //   smtk::mesh::TypeSet types(smtk::mesh::CellTypes(cell_bit_types), false, true);

  //   //get the points and cells of the mesh
  //   smtk::mesh::HandleRange cells = mesh_json.at("cells");
  //   smtk::mesh::HandleRange points = mesh_json.at("points");

  //   //get the uuid of the mesh
  //   smtk::common::UUID id = mesh_json.at("id");

  //   //get any model entity ids that are associated to this mesh
  //   smtk::common::UUIDArray modelEnts = mesh_json.at("modelEntityIds");

  //   std::vector<smtk::mesh::Domain> domains;
  //   std::vector<int> domainValues = mesh_json.at("domains");
  //   for (auto domainValue : domainValues)
  //   {
  //     domains.push_back(smtk::mesh::Domain(domainValue));
  //   }

  //   std::vector<smtk::mesh::Dirichlet> dirichlets;
  //   std::vector<smtk::mesh::Neumann> neumanns;

  //   for (auto& bc : mesh_json.at("boundary_conditions"))
  //   {
  //     std::string bcType = bc.at("type");
  //     int value = bc.at("value");

  //     if (bcType == dirName)
  //     {
  //       dirichlets.push_back(smtk::mesh::Dirichlet(value));
  //     }
  //     else if (bcType == neumName)
  //     {
  //       neumanns.push_back(smtk::mesh::Neumann(value));
  //     }
  //   }

  //   smtk::mesh::json::MeshInfo meshInfo(*meshIdIter, id, cells, points, types);
  //   //specify the optional parts of the mesh info
  //   if (!modelEnts.empty())
  //   {
  //     meshInfo.set(modelEnts);
  //   }
  //   if (!domains.empty())
  //   {
  //     meshInfo.set(domains);
  //   }
  //   if (!dirichlets.empty())
  //   {
  //     meshInfo.set(dirichlets);
  //   }
  //   if (!neumanns.empty())
  //   {
  //     meshInfo.set(neumanns);
  //   }

  //   meshInfos.push_back(meshInfo);
  // }
}
}
}
}
