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

namespace ItemDefinitionHelper
{
using namespace smtk::attribute;

/**
   * Map int type to ItemDef concrete type.
   */
inline ItemDefinitionPtr create(DefinitionPtr def, const int type, const std::string& name)
{
  switch (type)
  {
    case Item::ATTRIBUTE_REF:
      return def->addItemDefinition<RefItemDefinition>(name);
    case Item::DOUBLE:
      return def->addItemDefinition<DoubleItemDefinition>(name);
    case Item::GROUP:
      return def->addItemDefinition<GroupItemDefinition>(name);
    case Item::INT:
      return def->addItemDefinition<IntItemDefinition>(name);
    case Item::STRING:
      return def->addItemDefinition<StringItemDefinition>(name);
    case Item::VOID:
      return def->addItemDefinition<VoidItemDefinition>(name);
    case Item::FILE:
      return def->addItemDefinition<FileItemDefinition>(name);
    case Item::DIRECTORY:
      return def->addItemDefinition<DirectoryItemDefinition>(name);
    case Item::MODEL_ENTITY:
      return def->addItemDefinition<ModelEntityItemDefinition>(name);
    case Item::MESH_SELECTION:
      return def->addItemDefinition<MeshSelectionItemDefinition>(name);
    case Item::MESH_ENTITY:
      return def->addItemDefinition<MeshItemDefinition>(name);
    case Item::DATE_TIME:
      return def->addItemDefinition<DateTimeItemDefinition>(name);
    default:
      std::cout << "Error: Unknown type! Creating an Item::VOID\n";
      return def->addItemDefinition<VoidItemDefinition>(name);
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

/**
   * Map int type to corresponding instanced ui form.
   */
//ItemDefinitionHelper::createUi(props.type);
}
#endif // ItemDefinitionHelper_h
