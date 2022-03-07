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
#include "smtk/attribute/CustomItem.h"
#include "smtk/attribute/CustomItemDefinition.h"
#include "smtk/attribute/DateTimeItemDefinition.h"
#include "smtk/attribute/DirectoryItemDefinition.h"
#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/attribute/FileSystemItemDefinition.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"
#include "smtk/attribute/ResourceItemDefinition.h"
#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/attribute/ValueItemDefinition.h"
#include "smtk/attribute/VoidItemDefinition.h"

#include "smtk/attribute/json/jsonComponentItem.h"
#include "smtk/attribute/json/jsonDateTimeItem.h"
#include "smtk/attribute/json/jsonDirectoryItem.h"
#include "smtk/attribute/json/jsonDoubleItem.h"
#include "smtk/attribute/json/jsonFileItem.h"
#include "smtk/attribute/json/jsonFileSystemItem.h"
#include "smtk/attribute/json/jsonGroupItem.h"
#include "smtk/attribute/json/jsonIntItem.h"
#include "smtk/attribute/json/jsonItem.h"
#include "smtk/attribute/json/jsonModelEntityItem.h"
#include "smtk/attribute/json/jsonReferenceItem.h"
#include "smtk/attribute/json/jsonResourceItem.h"
#include "smtk/attribute/json/jsonStringItem.h"
#include "smtk/attribute/json/jsonValueItem.h"
#include "smtk/attribute/json/jsonVoidItem.h"

#include "smtk/attribute/json/jsonComponentItemDefinition.h"
#include "smtk/attribute/json/jsonDateTimeItemDefinition.h"
#include "smtk/attribute/json/jsonDirectoryItemDefinition.h"
#include "smtk/attribute/json/jsonDoubleItemDefinition.h"
#include "smtk/attribute/json/jsonFileItemDefinition.h"
#include "smtk/attribute/json/jsonFileSystemItemDefinition.h"
#include "smtk/attribute/json/jsonGroupItemDefinition.h"
#include "smtk/attribute/json/jsonIntItemDefinition.h"
#include "smtk/attribute/json/jsonItemDefinition.h"
#include "smtk/attribute/json/jsonModelEntityItemDefinition.h"
#include "smtk/attribute/json/jsonReferenceItemDefinition.h"
#include "smtk/attribute/json/jsonResourceItemDefinition.h"
#include "smtk/attribute/json/jsonStringItemDefinition.h"
#include "smtk/attribute/json/jsonValueItemDefinition.h"
#include "smtk/attribute/json/jsonVoidItemDefinition.h"

#include "smtk/PublicPointerDefs.h"

#include "nlohmann/json.hpp"

#include <string>

#define PUGIXML_HEADER_ONLY
// NOLINTNEXTLINE(bugprone-suspicious-include)
#include "pugixml/src/pugixml.hpp"

/**\brief Provide a way to serialize itemPtr
  */
namespace
{
template<typename itemDefPtr>
void processItemDef(
  const nlohmann::json& itemDef,
  itemDefPtr& idef,
  const smtk::attribute::ResourcePtr& resPtr,
  std::set<const smtk::attribute::ItemDefinition*>& convertedAttDefs)
{
  //iter format: {type: [{def1}, {def2}] }
  smtk::attribute::Item::Type citype = smtk::attribute::Item::string2Type(itemDef.at("Type"));
  std::string citemName = itemDef.at("Name");
  smtk::attribute::ItemDefinitionPtr cidef;

  switch (citype)
  {
    case smtk::attribute::Item::AttributeRefType:
      if ((cidef =
             idef->template addItemDefinition<smtk::attribute::ComponentItemDefinition>(citemName)))
      {
        convertedAttDefs.insert(cidef.get());
        smtk::attribute::ComponentItemDefinitionPtr temp =
          smtk::dynamic_pointer_cast<smtk::attribute::ComponentItemDefinition>(cidef);
        smtk::attribute::processFromRefItemDef(itemDef, temp);
      }
      break;
    case smtk::attribute::Item::DoubleType:
      if ((cidef =
             idef->template addItemDefinition<smtk::attribute::DoubleItemDefinition>(citemName)))
      {
        smtk::attribute::DoubleItemDefinitionPtr temp =
          smtk::dynamic_pointer_cast<smtk::attribute::DoubleItemDefinition>(cidef);
        smtk::attribute::from_json(itemDef, temp, resPtr);
      }
      break;
    case smtk::attribute::Item::DirectoryType:
      if ((cidef =
             idef->template addItemDefinition<smtk::attribute::DirectoryItemDefinition>(citemName)))
      {
        smtk::attribute::DirectoryItemDefinitionPtr temp =
          smtk::dynamic_pointer_cast<smtk::attribute::DirectoryItemDefinition>(cidef);
        smtk::attribute::from_json(itemDef, temp);
      }
      break;
    case smtk::attribute::Item::FileType:
      if ((cidef =
             idef->template addItemDefinition<smtk::attribute::FileItemDefinition>(citemName)))
      {
        smtk::attribute::FileItemDefinitionPtr temp =
          smtk::dynamic_pointer_cast<smtk::attribute::FileItemDefinition>(cidef);
        smtk::attribute::from_json(itemDef, temp);
      }
      break;
    case smtk::attribute::Item::GroupType:
      if ((cidef =
             idef->template addItemDefinition<smtk::attribute::GroupItemDefinition>(citemName)))
      {
        smtk::attribute::GroupItemDefinitionPtr temp =
          smtk::dynamic_pointer_cast<smtk::attribute::GroupItemDefinition>(cidef);
        smtk::attribute::from_json(itemDef, temp, resPtr);
      }
      break;
    case smtk::attribute::Item::IntType:
      if ((cidef = idef->template addItemDefinition<smtk::attribute::IntItemDefinition>(citemName)))
      {
        smtk::attribute::IntItemDefinitionPtr temp =
          smtk::dynamic_pointer_cast<smtk::attribute::IntItemDefinition>(cidef);
        smtk::attribute::from_json(itemDef, temp, resPtr);
      }
      break;
    case smtk::attribute::Item::StringType:
      if ((cidef =
             idef->template addItemDefinition<smtk::attribute::StringItemDefinition>(citemName)))
      {
        smtk::attribute::StringItemDefinitionPtr temp =
          smtk::dynamic_pointer_cast<smtk::attribute::StringItemDefinition>(cidef);
        smtk::attribute::from_json(itemDef, temp, resPtr);
      }
      break;
    case smtk::attribute::Item::ModelEntityType:
      if ((cidef = idef->template addItemDefinition<smtk::attribute::ModelEntityItemDefinition>(
             citemName)))
      {
        smtk::attribute::ModelEntityItemDefinitionPtr temp =
          smtk::dynamic_pointer_cast<smtk::attribute::ModelEntityItemDefinition>(cidef);
        smtk::attribute::from_json(itemDef, temp);
      }
      break;
    case smtk::attribute::Item::VoidType:
      if ((cidef =
             idef->template addItemDefinition<smtk::attribute::VoidItemDefinition>(citemName)))
      {
        smtk::attribute::VoidItemDefinitionPtr temp =
          smtk::dynamic_pointer_cast<smtk::attribute::VoidItemDefinition>(cidef);
        smtk::attribute::from_json(itemDef, temp);
      }
      break;
    case smtk::attribute::Item::MeshEntityType:
      if ((cidef =
             idef->template addItemDefinition<smtk::attribute::ComponentItemDefinition>(citemName)))
      {
        auto temp = smtk::dynamic_pointer_cast<smtk::attribute::ComponentItemDefinition>(cidef);
        smtk::attribute::processFromMeshItemDef(itemDef, temp);
      }
      break;
    case smtk::attribute::Item::DateTimeType:
      if ((cidef =
             idef->template addItemDefinition<smtk::attribute::DateTimeItemDefinition>(citemName)))
      {
        smtk::attribute::DateTimeItemDefinitionPtr temp =
          smtk::dynamic_pointer_cast<smtk::attribute::DateTimeItemDefinition>(cidef);
        smtk::attribute::from_json(itemDef, temp);
      }
      break;
    case smtk::attribute::Item::ComponentType:
      if ((cidef =
             idef->template addItemDefinition<smtk::attribute::ComponentItemDefinition>(citemName)))
      {
        smtk::attribute::ComponentItemDefinitionPtr temp =
          smtk::dynamic_pointer_cast<smtk::attribute::ComponentItemDefinition>(cidef);
        smtk::attribute::from_json(itemDef, temp, resPtr);
      }
      break;
    case smtk::attribute::Item::ReferenceType:
      if ((cidef =
             idef->template addItemDefinition<smtk::attribute::ReferenceItemDefinition>(citemName)))
      {
        smtk::attribute::ComponentItemDefinitionPtr temp =
          smtk::dynamic_pointer_cast<smtk::attribute::ComponentItemDefinition>(cidef);
        smtk::attribute::from_json(itemDef, temp, resPtr);
      }
      break;
    case smtk::attribute::Item::ResourceType:
      if ((cidef =
             idef->template addItemDefinition<smtk::attribute::ResourceItemDefinition>(citemName)))
      {
        smtk::attribute::ResourceItemDefinitionPtr temp =
          smtk::dynamic_pointer_cast<smtk::attribute::ResourceItemDefinition>(cidef);
        smtk::attribute::from_json(itemDef, temp, resPtr);
      }
      break;
    default:
      auto typeNameJson = itemDef.find("TypeName");
      if (
        typeNameJson != itemDef.end() &&
        resPtr->customItemDefinitionFactory().contains(typeNameJson->get<std::string>()))
      {
        cidef = std::shared_ptr<smtk::attribute::ItemDefinition>(
          resPtr->customItemDefinitionFactory().createFromName(
            typeNameJson->get<std::string>(), itemDef.at("Name").get<std::string>()));
        (*static_cast<smtk::attribute::CustomItemBaseDefinition*>(cidef.get())) << itemDef;
        idef->addItemDefinition(cidef);
      }
  }
}
} // namespace

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
  nlohmann::json& j,
  const smtk::attribute::ItemDefinitionPtr& idef)
{
  switch (idef->type())
  {
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
    case Item::VoidType:
    {
      smtk::attribute::VoidItemDefinitionPtr temp =
        smtk::dynamic_pointer_cast<VoidItemDefinition>(idef);
      smtk::attribute::to_json(j, temp);
    }
    break;
    case Item::DateTimeType:
    {
      smtk::attribute::DateTimeItemDefinitionPtr temp =
        smtk::dynamic_pointer_cast<DateTimeItemDefinition>(idef);
      smtk::attribute::to_json(j, temp);
    }
    break;
    case Item::ReferenceType:
    {
      smtk::attribute::ComponentItemDefinitionPtr temp =
        smtk::dynamic_pointer_cast<ComponentItemDefinition>(idef);
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
    case Item::ResourceType:
    {
      smtk::attribute::ResourceItemDefinitionPtr temp =
        smtk::dynamic_pointer_cast<ResourceItemDefinition>(idef);
      smtk::attribute::to_json(j, temp);
    }
    break;
    default:
      // For definitions that do not have a corresponding type entry in
      // Item::Type, attempt to handle them as custom definitions.
      const CustomItemBaseDefinition* customItemDefinition =
        dynamic_cast<const CustomItemBaseDefinition*>(idef.get());
      if (customItemDefinition)
      {
        (*customItemDefinition) >> j;
      }
  }
}

void JsonHelperFunction::processItemDefinitionTypeFromJson(
  const nlohmann::json& jItemDef,
  DefinitionPtr& idef,
  const ResourcePtr& resPtr,
  std::set<const smtk::attribute::ItemDefinition*>& convertedAttDefs)
{
  processItemDef<smtk::attribute::DefinitionPtr>(jItemDef, idef, resPtr, convertedAttDefs);
}

void JsonHelperFunction::processItemDefinitionTypeFromJson(
  const nlohmann::json& jItemDef,
  GroupItemDefinitionPtr& gdef,
  const ResourcePtr& resPtr)
{
  std::set<const smtk::attribute::ItemDefinition*> convertedAttDefs;
  processItemDef<smtk::attribute::GroupItemDefinitionPtr>(jItemDef, gdef, resPtr, convertedAttDefs);
}

void JsonHelperFunction::processItemDefinitionTypeFromJson(
  const nlohmann::json& jItemDef,
  ValueItemDefinitionPtr& idef,
  const ResourcePtr& resPtr)
{
  std::set<const smtk::attribute::ItemDefinition*> convertedAttDefs;
  processItemDef<smtk::attribute::ValueItemDefinitionPtr>(jItemDef, idef, resPtr, convertedAttDefs);
}

void JsonHelperFunction::processItemDefinitionTypeFromJson(
  const nlohmann::json& jItemDef,
  ReferenceItemDefinitionPtr& idef,
  const ResourcePtr& resPtr)
{
  std::set<const smtk::attribute::ItemDefinition*> convertedAttDefs;
  processItemDef<smtk::attribute::ReferenceItemDefinitionPtr>(
    jItemDef, idef, resPtr, convertedAttDefs);
}

void JsonHelperFunction::processItemTypeToJson(nlohmann::json& j, const ItemPtr& item)
{
  switch (item->type())
  {
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
    case Item::VoidType:
    {
      smtk::attribute::VoidItemPtr temp = smtk::dynamic_pointer_cast<VoidItem>(item);
      smtk::attribute::to_json(j, temp);
    }
    break;
    case Item::DateTimeType:
    {
      smtk::attribute::DateTimeItemPtr temp = smtk::dynamic_pointer_cast<DateTimeItem>(item);
      smtk::attribute::to_json(j, temp);
    }
    break;
    case Item::ReferenceType:
    {
      smtk::attribute::ReferenceItemPtr temp = smtk::dynamic_pointer_cast<ReferenceItem>(item);
      smtk::attribute::to_json(j, temp);
    }
    break;
    case Item::ResourceType:
    {
      smtk::attribute::ResourceItemPtr temp = smtk::dynamic_pointer_cast<ResourceItem>(item);
      smtk::attribute::to_json(j, temp);
    }
    break;
    case Item::ComponentType:
    {
      smtk::attribute::ComponentItemPtr temp = smtk::dynamic_pointer_cast<ComponentItem>(item);
      smtk::attribute::to_json(j, temp);
    }
    break;
    default:
      // For items that do not have a corresponding type entry in
      // Item::Type, attempt to handle them as custom items.
      const CustomItemBase* customItem = dynamic_cast<const CustomItemBase*>(item.get());
      if (customItem)
      {
        (*customItem) >> j;
      }
  }
}

void JsonHelperFunction::processItemTypeFromJson(
  const nlohmann::json& j,
  ItemPtr& itemPtr,
  std::vector<ItemExpressionInfo>& itemExpressionInfo,
  std::vector<AttRefInfo>& attRefInfo,
  const std::set<const smtk::attribute::ItemDefinition*>& convertedAttDefs)
{
  switch (itemPtr->type())
  {
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
    case smtk::attribute::Item::DateTimeType:
    {
      smtk::attribute::DateTimeItemPtr temp =
        smtk::dynamic_pointer_cast<smtk::attribute::DateTimeItem>(itemPtr);
      smtk::attribute::from_json(j, temp);
    }
    break;
    case smtk::attribute::Item::ReferenceType:
    {
      smtk::attribute::ComponentItemPtr temp =
        smtk::dynamic_pointer_cast<smtk::attribute::ComponentItem>(itemPtr);
      smtk::attribute::from_json(j, temp, itemExpressionInfo, attRefInfo);
    }
    break;
    case smtk::attribute::Item::ResourceType:
    {
      smtk::attribute::ResourceItemPtr temp =
        smtk::dynamic_pointer_cast<smtk::attribute::ResourceItem>(itemPtr);
      smtk::attribute::from_json(j, temp, itemExpressionInfo, attRefInfo);
    }
    break;
    case smtk::attribute::Item::ComponentType:
    {
      smtk::attribute::ComponentItemPtr temp =
        smtk::dynamic_pointer_cast<smtk::attribute::ComponentItem>(itemPtr);

      if (convertedAttDefs.find(itemPtr->definition().get()) != convertedAttDefs.end())
      {
        processFromRefItemSpec(j, temp, attRefInfo);
      }
      else
      {
        auto tj = j.find("Type");
        if (
          tj != j.end() &&
          (smtk::attribute::Item::string2Type(*tj) == smtk::attribute::Item::AttributeRefType))
        {
          processFromRefItemSpec(j, temp, attRefInfo);
        }
        else
        {
          smtk::attribute::from_json(j, temp, itemExpressionInfo, attRefInfo);
        }
      }
    }
    break;
    case smtk::attribute::Item::VoidType:
    {
      smtk::attribute::VoidItemPtr temp =
        smtk::dynamic_pointer_cast<smtk::attribute::VoidItem>(itemPtr);
      smtk::attribute::from_json(j, temp);
    }
    break;
    default:
    {
      if (
        smtk::attribute::CustomItemBase* item =
          dynamic_cast<smtk::attribute::CustomItemBase*>(itemPtr.get()))
      {
        (*item) << j;
      }
    }
  }
}
} // namespace attribute
} // namespace smtk
