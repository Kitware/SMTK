//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_jsonHelperFunction_h
#define smtk_attribute_jsonHelperFunction_h

#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/ItemDefinition.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/json/jsonItem.h"
#include "smtk/attribute/json/jsonItemDefinition.h"

#include "nlohmann/json.hpp"

#include <string>
/**\brief Provide a way to serialize itemPtr
  */
namespace smtk
{
namespace attribute
{
class SMTKCORE_EXPORT JsonHelperFunction
{
public:
  /**
   * @brief A helper function to dispatch itemDefinitionPtr process based on its type
   * @param j json to fill in
   * @param idef shared pointer to ItemDefinition
   */
  static void processItemDefinitionTypeToJson(
    nlohmann::json& j, const smtk::attribute::ItemDefinitionPtr& idef);

  /**
   * @brief A helper function to add an item definition into the itemDefinitionPtr
   * given a json iterator
   */
  static void processItemDefinitionTypeFromJson(const nlohmann::json& jItemDef,
    ValueItemDefinitionPtr& idef, const smtk::attribute::ResourcePtr& resPtr);

  static void processItemDefinitionTypeFromJson(const nlohmann::json& jItemDef, DefinitionPtr& idef,
    const smtk::attribute::ResourcePtr& resPtr,
    std::set<const smtk::attribute::ItemDefinition*>& convertedAttDefs);

  static void processItemDefinitionTypeFromJson(const nlohmann::json& jItemDef,
    GroupItemDefinitionPtr& idef, const smtk::attribute::ResourcePtr& resPtr);

  /**
   * @brief A helper function to dispatch itemPtr process based on its type
   */
  static void processItemTypeToJson(nlohmann::json& j, const smtk::attribute::ItemPtr& item);

  /**
   * @brief A helper function to fill an itemPtr given json
   */
  static void processItemTypeFromJson(const nlohmann::json& j, ItemPtr& itemPtr,
    std::vector<ItemExpressionInfo>& itemExpressionInfo, std::vector<AttRefInfo>& attRefInfo,
    const std::set<const smtk::attribute::ItemDefinition*>& convertedAttDefs);
};
}
}

#endif
