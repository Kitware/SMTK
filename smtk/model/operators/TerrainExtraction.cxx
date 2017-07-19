//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/operators/TerrainExtraction.h"

#include "smtk/model/EntityRef.h"

#include "smtk/common/Color.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include <cstddef> // for size_t

using smtk::attribute::StringItem;

namespace smtk
{
namespace model
{

smtk::model::OperatorResult TerrainExtraction::operateInternal()
{
}

} //namespace model
} // namespace smtk

#include "smtk/model/TerrainExtraction_xml.h"

smtkImplementsModelOperator(SMTKCORE_EXPORT, smtk::model::TerrainExtraction, terrain_extraction,
  "terrain extraction", TerrainExtraction_xml, smtk::model::Session);
