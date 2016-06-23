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
#include "smtk/mesh/json/Readers.h"
#include "smtk/mesh/json/Interface.h"
#include "smtk/mesh/json/MeshInfo.h"

#include "smtk/io/ImportJSON.h"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"

#include <stdio.h>
#include <string.h>

#if defined(_WIN32) && !defined(__CYGWIN__)
# define snprintf(buf, cnt, fmt, ...) _snprintf_s(buf, cnt, cnt, fmt, __VA_ARGS__)
#endif

namespace
{
  //stolen from ImportJSON
  int cJSON_GetStringValue(cJSON* valItem, std::string& val)
    {
    switch (valItem->type)
      {
    case cJSON_Number:
        {
        char valtext[64];
        snprintf(valtext, 64, "%.17g", valItem->valuedouble);
        val = valtext;
        }
      return 0;
    case cJSON_String:
      if (valItem->valuestring && valItem->valuestring[0])
        {
        val = valItem->valuestring;
        return 0;
        }
    default:
      break;
      }
    return 1;
    }
}

namespace smtk {
namespace mesh {
namespace json {

namespace detail
{
  bool parse_meshInfo(cJSON* info,
                      std::vector< smtk::mesh::json::MeshInfo>& result)
  {
  //extract the ids of all the meshes and make it a smtk::mesh::Handle
  smtk::mesh::HandleRange meshIds = smtk::mesh::from_json( cJSON_GetObjectItem(info, "meshIds") );
  if(meshIds.empty())
  {
    return false;
  }

  //iterate all the meshes in info
  cJSON* meshesNode = cJSON_GetObjectItem(info, "meshes");
  if(!meshesNode)
    {
    return false;
    }

  int meshIdIdx = 0;
  std::size_t validMeshesLoaded=0;
  //each child represents a single MeshInfo
  for(cJSON* child = meshesNode->child;
      child != NULL;
      child = child->next, ++meshIdIdx)
    {
    //get the Typeset from the json
    std::string cell_bit_types;
    cJSON_GetStringValue( cJSON_GetObjectItem(child, "cell_types"), cell_bit_types);
    smtk::mesh::TypeSet types( smtk::mesh::CellTypes(cell_bit_types), false, true );

    //get the points and cells of the mesh
    smtk::mesh::HandleRange cells = smtk::mesh::from_json( cJSON_GetObjectItem(child, "cells") );
    smtk::mesh::HandleRange points = smtk::mesh::from_json( cJSON_GetObjectItem(child, "points") );

    //get any model entity ids that are associated to this mesh
    smtk::common::UUIDArray modelEnts;
    {
    cJSON* modelEntNode = cJSON_GetObjectItem(child,"modelEntityIds");
    if(modelEntNode && modelEntNode->child)
      { //need to pass the array, not the key: [] node
      smtk::io::ImportJSON::getUUIDArrayFromJSON(modelEntNode->child,
                                                 modelEnts);
      }
    }

    std::vector<smtk::mesh::Domain> domains;
    { //read in the domains
    cJSON* domainNode = cJSON_GetObjectItem(child,"domains");
    if(domainNode && domainNode->child)
      {
      std::vector< long > values;
      smtk::io::ImportJSON::getIntegerArrayFromJSON(domainNode->child,
                                                    values);
      domains.reserve( values.size() );
      for(std::size_t i=0; i < values.size(); ++i)
        {
        domains.push_back(smtk::mesh::Domain(values[i]));
        }
      }
    }

    std::vector<smtk::mesh::Dirichlet> dirichlets;
    std::vector<smtk::mesh::Neumann> neumanns;
    { //read in the boundary conditions

    const std::string dirName = "dirichlet";
    const std::string neumName = "neumann";

    cJSON* bcsNode = cJSON_GetObjectItem(child,"boundary_conditions");
    for( cJSON* bc = bcsNode->child; bc != NULL; bc=bc->next )
        {
        cJSON* bcTypeNode = cJSON_GetObjectItem(bc, "type");
        std::string bcType;
        cJSON_GetStringValue(bcTypeNode, bcType);

        cJSON* bcValueNode = cJSON_GetObjectItem(bc, "value");
        int value = bcValueNode->valueint;

        if(bcType == dirName)
          {
          dirichlets.push_back( smtk::mesh::Dirichlet(value) );
          }
        else if(bcType == neumName)
          {
          neumanns.push_back( smtk::mesh::Neumann(value) );
          }
        }
    }

    const bool valid = (!cells.empty() && !points.empty());
    if(valid)
      {
      smtk::mesh::json::MeshInfo minfo(meshIds[meshIdIdx],
                                      cells,
                                      points,
                                      types);
      //specify the optional parts of the mesh info
      if(!modelEnts.empty())
        { minfo.set( modelEnts ); }
      if(!domains.empty())
        { minfo.set( domains ); }
      if(!dirichlets.empty())
        { minfo.set( dirichlets ); }
      if(!neumanns.empty())
        { minfo.set( neumanns ); }

      result.push_back(minfo);
      validMeshesLoaded++;
      }
    }

  return validMeshesLoaded == meshIds.size();
  }
}

//Import everything in a json string into a new collection
smtk::mesh::CollectionPtr import(cJSON* child,
                                 const smtk::mesh::ManagerPtr& manager)
{
  std::vector< smtk::mesh::json::MeshInfo> result;
  const bool valid = detail::parse_meshInfo(child, result);

  if(!valid)
    {
    return smtk::mesh::CollectionPtr();
    }

  smtk::mesh::json::InterfacePtr interface = smtk::mesh::json::make_interface();
  interface->addMeshes(result);

  return manager->makeCollection(interface);
}


//Import everything in a json string into an existing collection
bool import(cJSON* child,  const smtk::mesh::CollectionPtr& c)
{
  std::vector< smtk::mesh::json::MeshInfo> result;
  const bool valid = detail::parse_meshInfo(child, result);

  if(!valid)
    {
    return false;
    }

  smtk::mesh::json::InterfacePtr interface =
    smtk::dynamic_pointer_cast< smtk::mesh::json::Interface >(c->interface());

  if(!interface)
    {
    return false;
    }

  interface->addMeshes(result);
  return true;
}


}
}
}
