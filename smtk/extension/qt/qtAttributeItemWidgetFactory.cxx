//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtAttributeItemWidgetFactory.h"

#include "smtk/extension/qt/qtAttributeRefItem.h"
#include "smtk/extension/qt/qtDateTimeItem.h"
#include "smtk/extension/qt/qtFileItem.h"
#include "smtk/extension/qt/qtGroupItem.h"
#include "smtk/extension/qt/qtInputsItem.h"
#include "smtk/extension/qt/qtMeshItem.h"
#include "smtk/extension/qt/qtMeshSelectionItem.h"
#include "smtk/extension/qt/qtModelEntityItem.h"
#include "smtk/extension/qt/qtVoidItem.h"

#include "smtk/attribute/DateTimeItem.h"
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/MeshSelectionItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/RefItem.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/VoidItem.h"

using namespace smtk::extension;
using namespace smtk::attribute;

/**\brief Create a widget that illustrates an item referencing another attribute.
  *
  * The referenced attribute will be displayed inline as if it were a child of the
  * current attribute's item.
  * Subclasses might wish to provide other representations, such as a
  * clickable "link" to the referenced attribute.
  */
qtItem* qtAttributeItemWidgetFactory::createRefItemWidget(
  RefItemPtr item, QWidget* p, qtBaseView* bview, Qt::Orientation orient)
{
  return new qtAttributeRefItem(smtk::dynamic_pointer_cast<RefItem>(item), p, bview, orient);
}

/**\brief Create a widget that illustrates an item whose value is a primitive type.
  *
  */
qtItem* qtAttributeItemWidgetFactory::createValueItemWidget(
  ValueItemPtr item, QWidget* p, qtBaseView* bview, Qt::Orientation orient)
{
  return new qtInputsItem(smtk::dynamic_pointer_cast<ValueItem>(item), p, bview, orient);
}

/**\brief Create a widget that illustrates an item whose value is a directory.
  *
  */
qtItem* qtAttributeItemWidgetFactory::createDirectoryItemWidget(
  DirectoryItemPtr item, QWidget* p, qtBaseView* bview, Qt::Orientation orient)
{
  return new qtFileItem(smtk::dynamic_pointer_cast<DirectoryItem>(item), p, bview, orient);
}

/**\brief Create a widget that illustrates an item whose value is a file.
  *
  */
qtItem* qtAttributeItemWidgetFactory::createFileItemWidget(
  FileItemPtr item, QWidget* p, qtBaseView* bview, Qt::Orientation orient)
{
  return new qtFileItem(smtk::dynamic_pointer_cast<FileItem>(item), p, bview, orient);
}

/**\brief Create a widget that illustrates an item whose value is a group of model entities.
  *
  */
qtItem* qtAttributeItemWidgetFactory::createGroupItemWidget(
  GroupItemPtr item, QWidget* p, qtBaseView* bview, Qt::Orientation orient)
{
  return new qtGroupItem(smtk::dynamic_pointer_cast<GroupItem>(item), p, bview, orient);
}

/**\brief Create a widget that illustrates an item that is either enabled/present or disabled/absent but has no value.
  *
  */
qtItem* qtAttributeItemWidgetFactory::createVoidItemWidget(
  VoidItemPtr item, QWidget* p, qtBaseView* bview, Qt::Orientation orient)
{
  (void)orient;
  return new qtVoidItem(smtk::dynamic_pointer_cast<VoidItem>(item), p, bview);
}

/**\brief Create a widget that illustrates an item whose value is a geometric model entity.
  *
  */
qtItem* qtAttributeItemWidgetFactory::createModelEntityItemWidget(
  ModelEntityItemPtr item, QWidget* p, qtBaseView* bview, Qt::Orientation orient)
{
  return new qtModelEntityItem(item, p, bview, orient);
}

/**\brief Create a widget that illustrates an item whose value is a set of model geometric selections.
  *
  */
qtItem* qtAttributeItemWidgetFactory::createMeshSelectionItemWidget(
  MeshSelectionItemPtr item, QWidget* p, qtBaseView* bview, Qt::Orientation orient)
{
  return new qtMeshSelectionItem(
    smtk::dynamic_pointer_cast<MeshSelectionItem>(item), p, bview, orient);
}

/**\brief Create a widget that illustrates an item whose value is a set of geometric mesh entities.
  *
  */
qtItem* qtAttributeItemWidgetFactory::createMeshItemWidget(
  MeshItemPtr item, QWidget* p, qtBaseView* bview, Qt::Orientation orient)
{
  return new qtMeshItem(
    smtk::dynamic_pointer_cast<MeshItem>(item), p, bview, orient);
}

/**\brief Create a widget that illustrates an item whose value is a date-time
  *
  */
qtItem* qtAttributeItemWidgetFactory::createDateTimeItemWidget(
  DateTimeItemPtr item, QWidget* p, qtBaseView* bview, Qt::Orientation orient)
{
  return new qtDateTimeItem(smtk::dynamic_pointer_cast<DateTimeItem>(item), p, bview, orient);
}
