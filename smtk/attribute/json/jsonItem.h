//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_jsonItem_h
#define smtk_attribute_jsonItem_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Item.h"

#include "nlohmann/json.hpp"
#include "smtk/CoreExports.h"

#include <string>
using json = nlohmann::json;

/**\brief Provide a way to serialize itemPtr
  */
namespace smtk
{
namespace attribute
{

struct AttRefInfo
{
  smtk::attribute::ComponentItemPtr item;
  int pos;
  std::string attName;
};

struct ItemExpressionInfo
{
  smtk::attribute::ValueItemPtr item;
  int pos;
  std::string expName;
};

SMTKCORE_EXPORT void to_json(json& j, const smtk::attribute::ItemPtr& itemPtr);

SMTKCORE_EXPORT void from_json(const json& j, smtk::attribute::ItemPtr& itemPtr);
}
}

#endif
