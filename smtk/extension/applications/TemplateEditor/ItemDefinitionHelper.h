//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef ItemDefinitionHelper_h
#define ItemDefinitionHelper_h
#include <QStringList>

#include "smtk/attribute/DateTimeItemDefinition.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/DirectoryItemDefinition.h"
#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/MeshItemDefinition.h"
#include "smtk/attribute/MeshSelectionItemDefinition.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"
#include "smtk/attribute/RefItemDefinition.h"
#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/attribute/VoidItemDefinition.h"

#include "HandlerGroup.h"
#include "HandlerValue.h"
#include "HandlerVoid.h"

namespace ItemDefinitionHelper
{
using namespace smtk::attribute;

/**
   * Map attribute::Item::Type to corresponding Handler.  Handlers customize
   * UI and create concrete ItemDefinition.
   */
inline std::shared_ptr<HandlerItemDef> createHandler(const int type)
{
  switch (type)
  {
    case Item::DOUBLE:
    {
      auto ptr = std::make_shared<HandlerDouble>();
      return ptr;
    }
    case Item::GROUP:
    {
      auto ptr = std::make_shared<HandlerGroup>();
      return ptr;
    }
    case Item::INT:
    {
      auto ptr = std::make_shared<HandlerInt>();
      return ptr;
    }
    case Item::STRING:
    {
      auto ptr = std::make_shared<HandlerString>();
      return ptr;
    }
    case Item::VOID:
    case Item::FILE:
    case Item::DIRECTORY:
    case Item::COLOR:
    case Item::MODEL_ENTITY:
    case Item::MESH_SELECTION:
    case Item::MESH_ENTITY:
    case Item::DATE_TIME:
    case Item::ATTRIBUTE_REF:
    {
      auto ptr = std::make_shared<HandlerVoid>();
      return ptr;
    }
    default:
      std::cout << "Error: Unknown type!\n";
      return nullptr;
  }
}

/**
   * Create item definitions from enum type.
   */
inline ItemDefinitionPtr create(DefinitionPtr def, const int type, const std::string& name)
{
  switch (type)
  {
    case Item::ATTRIBUTE_REF:
      return RefItemDefinition::New(name);
    case Item::DOUBLE:
      return DoubleItemDefinition::New(name);
    case Item::GROUP:
      return GroupItemDefinition::New(name);
    case Item::INT:
      return IntItemDefinition::New(name);
    case Item::STRING:
      return StringItemDefinition::New(name);
    case Item::VOID:
      return VoidItemDefinition::New(name);
    case Item::FILE:
      return FileItemDefinition::New(name);
    case Item::DIRECTORY:
      return DirectoryItemDefinition::New(name);
    case Item::MODEL_ENTITY:
      return ModelEntityItemDefinition::New(name);
    case Item::MESH_SELECTION:
      return MeshSelectionItemDefinition::New(name);
    case Item::MESH_ENTITY:
      return MeshItemDefinition::New(name);
    case Item::DATE_TIME:
      return DateTimeItemDefinition::New(name);
    default:
      std::cout << "Error: Unknown type! Creating an Item::VOID\n";
      return VoidItemDefinition::New(name);
  }
};

/**
   * Arrange types in list of strings. Uses smtk::attribute::Item string types.
   * This assumes smtk::attribute::Item::Type is continuous and NUMBER_OF_TYPES
   * is the last type.
   */
inline QStringList getTypes()
{
  QStringList names;
  for (int type = 0; type < Item::NUMBER_OF_TYPES; type++)
  {
    names << QString::fromStdString(Item::type2String(static_cast<Item::Type>(type)));
  }

  return names;
};
}
#endif // ItemDefinitionHelper_h
