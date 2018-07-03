//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/attribute/json/jsonHelperFunction.h"

#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/ComponentItemDefinition.h"
#include "smtk/attribute/DateTimeItemDefinition.h"
#include "smtk/attribute/DirectoryItemDefinition.h"
#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/attribute/FileSystemItemDefinition.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/MeshItemDefinition.h"
#include "smtk/attribute/MeshSelectionItemDefinition.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"
#include "smtk/attribute/RefItemDefinition.h"
#include "smtk/attribute/ResourceItemDefinition.h"
#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/attribute/ValueItemDefinition.h"

#include "smtk/attribute/json/jsonComponentItem.h"
#include "smtk/attribute/json/jsonDateTimeItem.h"
#include "smtk/attribute/json/jsonDirectoryItem.h"
#include "smtk/attribute/json/jsonDoubleItem.h"
#include "smtk/attribute/json/jsonFileItem.h"
#include "smtk/attribute/json/jsonFileSystemItem.h"
#include "smtk/attribute/json/jsonGroupItem.h"
#include "smtk/attribute/json/jsonIntItem.h"
#include "smtk/attribute/json/jsonItem.h"
#include "smtk/attribute/json/jsonMeshItem.h"
#include "smtk/attribute/json/jsonMeshSelectionItem.h"
#include "smtk/attribute/json/jsonModelEntityItem.h"
#include "smtk/attribute/json/jsonRefItem.h"
#include "smtk/attribute/json/jsonStringItem.h"
#include "smtk/attribute/json/jsonValueItem.h"

#include "smtk/attribute/json/jsonComponentItemDefinition.h"
#include "smtk/attribute/json/jsonDateTimeItemDefinition.h"
#include "smtk/attribute/json/jsonDirectoryItemDefinition.h"
#include "smtk/attribute/json/jsonDoubleItemDefinition.h"
#include "smtk/attribute/json/jsonFileItemDefinition.h"
#include "smtk/attribute/json/jsonFileSystemItemDefinition.h"
#include "smtk/attribute/json/jsonGroupItemDefinition.h"
#include "smtk/attribute/json/jsonIntItemDefinition.h"
#include "smtk/attribute/json/jsonItemDefinition.h"
#include "smtk/attribute/json/jsonMeshItemDefinition.h"
#include "smtk/attribute/json/jsonMeshSelectionItemDefinition.h"
#include "smtk/attribute/json/jsonModelEntityItemDefinition.h"
#include "smtk/attribute/json/jsonRefItemDefinition.h"
#include "smtk/attribute/json/jsonStringItemDefinition.h"
#include "smtk/attribute/json/jsonValueItemDefinition.h"

#include "smtk/PublicPointerDefs.h"

#include "nlohmann/json.hpp"

#include <string>

/**\brief Provide a way to serialize itemPtr
  */
namespace
{
using ItemExpressionDefInfo = std::pair<smtk::attribute::ValueItemDefinitionPtr, std::string>;

using AttRefDefInfo = std::pair<smtk::attribute::RefItemDefinitionPtr, std::string>;

template <typename itemDefPtr>
void processItemDef(const nlohmann::json::iterator& iter, itemDefPtr& idef,
  const smtk::attribute::ResourcePtr& resPtr, std::vector<ItemExpressionDefInfo>& expressionDefInfo,
  std::vector<AttRefDefInfo>& attRefDefInfo)
{
  //iter format: {type: [{def1}, {def2}] }
  smtk::attribute::Item::Type citype = smtk::attribute::Item::string2Type(iter.key());
  json itemDefList = iter.value();
  for (auto itemDef = itemDefList.begin(); itemDef != itemDefList.end(); itemDef++)
  {
    std::string citemName = (*itemDef).at("Name");
    smtk::attribute::ItemDefinitionPtr cidef;

    switch (citype)
    {
      case smtk::attribute::Item::AttributeRefType:
        if ((cidef =
                idef->template addItemDefinition<smtk::attribute::RefItemDefinition>(citemName)))
        {
          smtk::attribute::RefItemDefinitionPtr temp =
            smtk::dynamic_pointer_cast<smtk::attribute::RefItemDefinition>(cidef);
          smtk::attribute::from_json((*itemDef), temp, resPtr, attRefDefInfo);
        }
        break;
      case smtk::attribute::Item::DoubleType:
        if ((cidef =
                idef->template addItemDefinition<smtk::attribute::DoubleItemDefinition>(citemName)))
        {
          smtk::attribute::DoubleItemDefinitionPtr temp =
            smtk::dynamic_pointer_cast<smtk::attribute::DoubleItemDefinition>(cidef);
          smtk::attribute::from_json((*itemDef), temp, resPtr, expressionDefInfo, attRefDefInfo);
        }
        break;
      case smtk::attribute::Item::DirectoryType:
        if ((cidef = idef->template addItemDefinition<smtk::attribute::DirectoryItemDefinition>(
               citemName)))
        {
          smtk::attribute::DirectoryItemDefinitionPtr temp =
            smtk::dynamic_pointer_cast<smtk::attribute::DirectoryItemDefinition>(cidef);
          smtk::attribute::from_json((*itemDef), temp);
        }
        break;
      case smtk::attribute::Item::FileType:
        if ((cidef =
                idef->template addItemDefinition<smtk::attribute::FileItemDefinition>(citemName)))
        {
          smtk::attribute::FileItemDefinitionPtr temp =
            smtk::dynamic_pointer_cast<smtk::attribute::FileItemDefinition>(cidef);
          smtk::attribute::from_json((*itemDef), temp);
        }
        break;
      case smtk::attribute::Item::GroupType:
        if ((cidef =
                idef->template addItemDefinition<smtk::attribute::GroupItemDefinition>(citemName)))
        {
          smtk::attribute::GroupItemDefinitionPtr temp =
            smtk::dynamic_pointer_cast<smtk::attribute::GroupItemDefinition>(cidef);
          smtk::attribute::from_json((*itemDef), temp, resPtr, expressionDefInfo, attRefDefInfo);
        }
        break;
      case smtk::attribute::Item::IntType:
        if ((cidef =
                idef->template addItemDefinition<smtk::attribute::IntItemDefinition>(citemName)))
        {
          smtk::attribute::IntItemDefinitionPtr temp =
            smtk::dynamic_pointer_cast<smtk::attribute::IntItemDefinition>(cidef);
          smtk::attribute::from_json((*itemDef), temp, resPtr, expressionDefInfo, attRefDefInfo);
        }
        break;
      case smtk::attribute::Item::StringType:
        if ((cidef =
                idef->template addItemDefinition<smtk::attribute::StringItemDefinition>(citemName)))
        {
          smtk::attribute::StringItemDefinitionPtr temp =
            smtk::dynamic_pointer_cast<smtk::attribute::StringItemDefinition>(cidef);
          smtk::attribute::from_json((*itemDef), temp, resPtr, expressionDefInfo, attRefDefInfo);
        }
        break;
      case smtk::attribute::Item::ModelEntityType:
        if ((cidef = idef->template addItemDefinition<smtk::attribute::ModelEntityItemDefinition>(
               citemName)))
        {
          smtk::attribute::ModelEntityItemDefinitionPtr temp =
            smtk::dynamic_pointer_cast<smtk::attribute::ModelEntityItemDefinition>(cidef);
          smtk::attribute::from_json((*itemDef), temp);
        }
        break;
      case smtk::attribute::Item::VoidType:
        if ((cidef =
                idef->template addItemDefinition<smtk::attribute::VoidItemDefinition>(citemName)))
        {
          // Treat it as ItemDef
          smtk::attribute::ItemDefinitionPtr temp =
            smtk::dynamic_pointer_cast<smtk::attribute::ItemDefinition>(cidef);
          smtk::attribute::from_json((*itemDef), temp);
        }
        break;
      case smtk::attribute::Item::MeshSelectionType:
        if ((cidef = idef->template addItemDefinition<smtk::attribute::MeshSelectionItemDefinition>(
               citemName)))
        {
          //QUESTION: When parsing, valueItemDef and groupItemDef treat it as a simple ItemDef in xml parser
          smtk::attribute::MeshSelectionItemDefinitionPtr temp =
            smtk::dynamic_pointer_cast<smtk::attribute::MeshSelectionItemDefinition>(cidef);
          smtk::attribute::from_json((*itemDef), temp);
        }
        break;
      case smtk::attribute::Item::MeshEntityType:
        if ((cidef =
                idef->template addItemDefinition<smtk::attribute::MeshItemDefinition>(citemName)))
        {
          smtk::attribute::MeshItemDefinitionPtr temp =
            smtk::dynamic_pointer_cast<smtk::attribute::MeshItemDefinition>(cidef);
          smtk::attribute::from_json((*itemDef), temp);
        }
        break;
      case smtk::attribute::Item::DateTimeType:
        if ((cidef = idef->template addItemDefinition<smtk::attribute::DateTimeItemDefinition>(
               citemName)))
        {
          smtk::attribute::DateTimeItemDefinitionPtr temp =
            smtk::dynamic_pointer_cast<smtk::attribute::DateTimeItemDefinition>(cidef);
          smtk::attribute::from_json((*itemDef), temp);
        }
        break;
      case smtk::attribute::Item::ComponentType:
        if ((cidef = idef->template addItemDefinition<smtk::attribute::ComponentItemDefinition>(
               citemName)))
        {
          smtk::attribute::ComponentItemDefinitionPtr temp =
            smtk::dynamic_pointer_cast<smtk::attribute::ComponentItemDefinition>(cidef);
          smtk::attribute::from_json((*itemDef), temp);
        }
        break;
      default:;
    }
  }
}
}

namespace smtk
{
namespace attribute
{
/**
   * @brief A helper function to dispatch itemDefinitionPtr process based on its type
   * @param j json to fill in
   * @param idef shared pointer to ItemDefinition
   */
void JsonHelperFunction::processItemDefinitionTypeToJson(
  nlohmann::json& j, const smtk::attribute::ItemDefinitionPtr& idef)
{
  switch (idef->type())
  {
    case Item::AttributeRefType:
    {
      smtk::attribute::RefItemDefinitionPtr temp =
        smtk::dynamic_pointer_cast<RefItemDefinition>(idef);
      smtk::attribute::to_json(j, temp);
    }
    break;
    case Item::DoubleType:
    {
      smtk::attribute::DoubleItemDefinitionPtr temp =
        smtk::dynamic_pointer_cast<DoubleItemDefinition>(idef);
      smtk::attribute::to_json(j, temp);
    }
    break;
    case Item::DirectoryType:
    {
      smtk::attribute::DirectoryItemDefinitionPtr temp =
        smtk::dynamic_pointer_cast<DirectoryItemDefinition>(idef);
      smtk::attribute::to_json(j, temp);
    }
    break;
    case Item::FileType:
    {
      smtk::attribute::FileItemDefinitionPtr temp =
        smtk::dynamic_pointer_cast<FileItemDefinition>(idef);
      smtk::attribute::to_json(j, temp);
    }
    break;
    case Item::GroupType:
    {
      smtk::attribute::GroupItemDefinitionPtr temp =
        smtk::dynamic_pointer_cast<GroupItemDefinition>(idef);
      smtk::attribute::to_json(j, temp);
    }
    break;
    case Item::IntType:
    {
      smtk::attribute::IntItemDefinitionPtr temp =
        smtk::dynamic_pointer_cast<IntItemDefinition>(idef);
      smtk::attribute::to_json(j, temp);
    }
    break;
    case Item::StringType:
    {
      smtk::attribute::StringItemDefinitionPtr temp =
        smtk::dynamic_pointer_cast<StringItemDefinition>(idef);
      smtk::attribute::to_json(j, temp);
    }
    break;
    case Item::ModelEntityType:
    {
      smtk::attribute::ModelEntityItemDefinitionPtr temp =
        smtk::dynamic_pointer_cast<ModelEntityItemDefinition>(idef);
      smtk::attribute::to_json(j, temp);
    }
    break;
    case Item::MeshSelectionType:
    {
      smtk::attribute::MeshSelectionItemDefinitionPtr temp =
        smtk::dynamic_pointer_cast<MeshSelectionItemDefinition>(idef);
      smtk::attribute::to_json(j, temp);
    }
    break;
    case Item::MeshEntityType:
    {
      smtk::attribute::MeshItemDefinitionPtr temp =
        smtk::dynamic_pointer_cast<MeshItemDefinition>(idef);
      smtk::attribute::to_json(j, temp);
    }
    break;
    case Item::VoidType:
      // Nothing to do!
      break;
    case Item::DateTimeType:
    {
      smtk::attribute::DateTimeItemDefinitionPtr temp =
        smtk::dynamic_pointer_cast<DateTimeItemDefinition>(idef);
      smtk::attribute::to_json(j, temp);
    }
    break;
    case Item::ComponentType:
    {
      smtk::attribute::ComponentItemDefinitionPtr temp =
        smtk::dynamic_pointer_cast<ComponentItemDefinition>(idef);
      smtk::attribute::to_json(j, temp);
    }
    break;
    default:;
  }
}

void JsonHelperFunction::processItemDefinitionTypeFromJson(const nlohmann::json::iterator& iter,
  DefinitionPtr& idef, const ResourcePtr& resPtr,
  std::vector<ItemExpressionDefInfo>& expressionDefInfo, std::vector<AttRefDefInfo>& attRefDefInfo)
{
  processItemDef<smtk::attribute::DefinitionPtr>(
    iter, idef, resPtr, expressionDefInfo, attRefDefInfo);
}

void JsonHelperFunction::processItemDefinitionTypeFromJson(const nlohmann::json::iterator& iter,
  GroupItemDefinitionPtr& idef, const ResourcePtr& resPtr,
  std::vector<ItemExpressionDefInfo>& expressionDefInfo, std::vector<AttRefDefInfo>& attRefDefInfo)
{
  processItemDef<smtk::attribute::GroupItemDefinitionPtr>(
    iter, idef, resPtr, expressionDefInfo, attRefDefInfo);
}

void JsonHelperFunction::processItemDefinitionTypeFromJson(const nlohmann::json::iterator& iter,
  ValueItemDefinitionPtr& idef, const ResourcePtr& resPtr,
  std::vector<ItemExpressionDefInfo>& expressionDefInfo, std::vector<AttRefDefInfo>& attRefDefInfo)
{
  processItemDef<smtk::attribute::ValueItemDefinitionPtr>(
    iter, idef, resPtr, expressionDefInfo, attRefDefInfo);
}

void JsonHelperFunction::processItemTypeToJson(nlohmann::json& j, const ItemPtr& item)
{
  switch (item->type())
  {
    case Item::AttributeRefType:
    {
      smtk::attribute::RefItemPtr temp = smtk::dynamic_pointer_cast<RefItem>(item);
      smtk::attribute::to_json(j, temp);
    }
    break;
    case Item::DoubleType:
    {
      smtk::attribute::DoubleItemPtr temp = smtk::dynamic_pointer_cast<DoubleItem>(item);
      smtk::attribute::to_json(j, temp);
    }
    break;
    case Item::DirectoryType:
    {
      smtk::attribute::DirectoryItemPtr temp = smtk::dynamic_pointer_cast<DirectoryItem>(item);
      smtk::attribute::to_json(j, temp);
    }
    break;
    case Item::FileType:
    {
      smtk::attribute::FileItemPtr temp = smtk::dynamic_pointer_cast<FileItem>(item);
      smtk::attribute::to_json(j, temp);
    }
    break;
    case Item::GroupType:
    {
      smtk::attribute::GroupItemPtr temp = smtk::dynamic_pointer_cast<GroupItem>(item);
      smtk::attribute::to_json(j, temp);
    }
    break;
    case Item::IntType:
    {
      smtk::attribute::IntItemPtr temp = smtk::dynamic_pointer_cast<IntItem>(item);
      smtk::attribute::to_json(j, temp);
    }
    break;
    case Item::StringType:
    {
      smtk::attribute::StringItemPtr temp = smtk::dynamic_pointer_cast<StringItem>(item);
      smtk::attribute::to_json(j, temp);
    }
    break;
    case Item::ModelEntityType:
    {
      smtk::attribute::ModelEntityItemPtr temp = smtk::dynamic_pointer_cast<ModelEntityItem>(item);
      smtk::attribute::to_json(j, temp);
    }
    break;
    case Item::MeshSelectionType:
    {
      smtk::attribute::MeshSelectionItemPtr temp =
        smtk::dynamic_pointer_cast<MeshSelectionItem>(item);
      smtk::attribute::to_json(j, temp);
    }
    break;
    case Item::MeshEntityType:
    {
      smtk::attribute::MeshItemPtr temp = smtk::dynamic_pointer_cast<MeshItem>(item);
      smtk::attribute::to_json(j, temp);
    }
    break;
    case Item::VoidType:
      // Nothing to do!
      break;
    case Item::DateTimeType:
    {
      smtk::attribute::DateTimeItemPtr temp = smtk::dynamic_pointer_cast<DateTimeItem>(item);
      smtk::attribute::to_json(j, temp);
    }
    break;
    case Item::ComponentType:
    {
      smtk::attribute::ComponentItemPtr temp = smtk::dynamic_pointer_cast<ComponentItem>(item);
      smtk::attribute::to_json(j, temp);
    }
    break;
    default:;
  }
}

void JsonHelperFunction::processItemTypeFromJson(const nlohmann::json& j, ItemPtr& itemPtr,
  std::vector<ItemExpressionInfo>& itemExpressionInfo, std::vector<AttRefInfo>& attRefInfo)
{
  switch (itemPtr->type())
  {
    case smtk::attribute::Item::AttributeRefType:
    {
      smtk::attribute::RefItemPtr temp =
        smtk::dynamic_pointer_cast<smtk::attribute::RefItem>(itemPtr);
      smtk::attribute::from_json(j, temp, attRefInfo);
    }
    break;
    case smtk::attribute::Item::DoubleType:
    {
      smtk::attribute::DoubleItemPtr temp =
        smtk::dynamic_pointer_cast<smtk::attribute::DoubleItem>(itemPtr);
      smtk::attribute::from_json(j, temp, itemExpressionInfo, attRefInfo);
    }
    break;
    case smtk::attribute::Item::DirectoryType:
    {
      smtk::attribute::DirectoryItemPtr temp =
        smtk::dynamic_pointer_cast<smtk::attribute::DirectoryItem>(itemPtr);
      smtk::attribute::from_json(j, temp);
    }
    break;
    case smtk::attribute::Item::FileType:
    {
      smtk::attribute::FileItemPtr temp =
        smtk::dynamic_pointer_cast<smtk::attribute::FileItem>(itemPtr);
      smtk::attribute::from_json(j, temp);
    }
    break;
    case smtk::attribute::Item::GroupType:
    {
      smtk::attribute::GroupItemPtr temp =
        smtk::dynamic_pointer_cast<smtk::attribute::GroupItem>(itemPtr);
      smtk::attribute::from_json(j, temp, itemExpressionInfo, attRefInfo);
    }
    break;
    case smtk::attribute::Item::IntType:
    {
      smtk::attribute::IntItemPtr temp =
        smtk::dynamic_pointer_cast<smtk::attribute::IntItem>(itemPtr);
      smtk::attribute::from_json(j, temp, itemExpressionInfo, attRefInfo);
    }
    break;
    case smtk::attribute::Item::StringType:
    {
      smtk::attribute::StringItemPtr temp =
        smtk::dynamic_pointer_cast<smtk::attribute::StringItem>(itemPtr);
      smtk::attribute::from_json(j, temp, itemExpressionInfo, attRefInfo);
    }
    break;
    case smtk::attribute::Item::ModelEntityType:
    {
      smtk::attribute::ModelEntityItemPtr temp =
        smtk::dynamic_pointer_cast<smtk::attribute::ModelEntityItem>(itemPtr);
      smtk::attribute::from_json(j, temp);
    }
    break;
    case smtk::attribute::Item::MeshSelectionType:
    {
      smtk::attribute::MeshSelectionItemPtr temp =
        smtk::dynamic_pointer_cast<smtk::attribute::MeshSelectionItem>(itemPtr);
      smtk::attribute::from_json(j, temp);
    }
    break;
    case smtk::attribute::Item::MeshEntityType:
    {
      smtk::attribute::MeshItemPtr temp =
        smtk::dynamic_pointer_cast<smtk::attribute::MeshItem>(itemPtr);
      smtk::attribute::from_json(j, temp);
    }
    break;
    case smtk::attribute::Item::DateTimeType:
    {
      smtk::attribute::DateTimeItemPtr temp =
        smtk::dynamic_pointer_cast<smtk::attribute::DateTimeItem>(itemPtr);
      smtk::attribute::from_json(j, temp);
    }
    break;
    case smtk::attribute::Item::ComponentType:
    {
      smtk::attribute::ComponentItemPtr temp =
        smtk::dynamic_pointer_cast<smtk::attribute::ComponentItem>(itemPtr);
      smtk::attribute::from_json(j, temp);
    }
    break;
    case smtk::attribute::Item::VoidType:
      // Nothing to do!
      break;
    default:;
  }
}
}
}
