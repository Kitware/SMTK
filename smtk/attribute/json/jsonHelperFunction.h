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
#include "smtk/attribute/RefItemDefinition.h"

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
using ItemExpressionDefInfo = std::pair<smtk::attribute::ValueItemDefinitionPtr, std::string>;

using AttRefDefInfo = std::pair<smtk::attribute::RefItemDefinitionPtr, std::string>;

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
  static void processItemDefinitionTypeFromJson(const nlohmann::json::iterator& iter,
    ValueItemDefinitionPtr& idef, const smtk::attribute::CollectionPtr& colPtr,
    std::vector<ItemExpressionDefInfo>& expressionDefInfo,
    std::vector<AttRefDefInfo>& attRefDefInfo);

  static void processItemDefinitionTypeFromJson(const nlohmann::json::iterator& iter,
    DefinitionPtr& idef, const smtk::attribute::CollectionPtr& colPtr,
    std::vector<ItemExpressionDefInfo>& expressionDefInfo,
    std::vector<AttRefDefInfo>& attRefDefInfo);

  static void processItemDefinitionTypeFromJson(const nlohmann::json::iterator& iter,
    GroupItemDefinitionPtr& idef, const smtk::attribute::CollectionPtr& colPtr,
    std::vector<ItemExpressionDefInfo>& expressionDefInfo,
    std::vector<AttRefDefInfo>& attRefDefInfo);

  /**
   * @brief A helper function to dispatch itemPtr process based on its type
   */
  static void processItemTypeToJson(nlohmann::json& j, const smtk::attribute::ItemPtr& item);

  /**
   * @brief A helper function to fill an itemPtr given json
   */
  static void processItemTypeFromJson(const nlohmann::json& j, ItemPtr& itemPtr,
    std::vector<smtk::attribute::ItemExpressionInfo>& itemExpressionInfo,
    std::vector<smtk::attribute::AttRefInfo>& attRefInfo);
};
}
}

#endif
