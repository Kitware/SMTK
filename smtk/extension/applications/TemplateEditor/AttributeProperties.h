#ifndef AttributeProperties_h
#define AttributeProperties_h
#include <string>

#include <QModelIndex>

#include "smtk/PublicPointerDefs.h"

/**
 * \brief Container for parameters to create an attribute definition.
 */
struct DefProperties
{
  DefProperties(){};

  std::string Type;
  std::string BaseType;
  std::string Label;
  bool IsUnique;
  bool IsAbstract;
};

/**
 * \brief Container for parameters to create an item definition.
 * TODO Different subclasses need to define properties for different concrete
 * ItemDefinitions.
 */
struct ItemDefProperties
{
  ItemDefProperties(){};

  smtk::attribute::DefinitionPtr Definition;
  std::string Name;
  std::string Type;
  QModelIndex ParentNode;
};

#endif // AttributeProperties_h
