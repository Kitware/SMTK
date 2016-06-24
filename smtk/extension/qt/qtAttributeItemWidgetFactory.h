//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_extension_qt_qtAttributeItemWidgetFactory_h
#define __smtk_extension_qt_qtAttributeItemWidgetFactory_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/PublicPointerDefs.h"

#include <QWidget>

namespace smtk {
  namespace extension {

class qtItem;
class qtBaseView;

/**\brief Create widgets for attribute items.
  *
  * This class serves as a delegate to qtAttribute for creating the qtItem widgets
  * that allow users to modify items owned by an attribute.
  *
  * As a view requests the qtAttribute for widgets to expose items owned by
  * the attribute, this factory is called to create the widgets.
  * The factory can then inspect the item and view to determine
  * what widget the item should be presented with.
  *
  * An instance of this class (or a subclass) is owned by the qtAttribute class
  * (as a static member variable, not on a per-qtAttribute-instance basis).
  * Subclasses may be used to provide application-specific interactions for
  * setting item values.
  */
class SMTKQTEXT_EXPORT qtAttributeItemWidgetFactory
{
public:
  virtual ~qtAttributeItemWidgetFactory() { }

  virtual qtItem* createRefItemWidget(attribute::RefItemPtr item, QWidget* p, qtBaseView* bview, Qt::Orientation orient);
  virtual qtItem* createValueItemWidget(attribute::ValueItemPtr item, QWidget* p, qtBaseView* bview, Qt::Orientation orient);
  virtual qtItem* createDirectoryItemWidget(attribute::DirectoryItemPtr item, QWidget* p, qtBaseView* bview, Qt::Orientation orient);
  virtual qtItem* createFileItemWidget(attribute::FileItemPtr item, QWidget* p, qtBaseView* bview, Qt::Orientation orient);
  virtual qtItem* createGroupItemWidget(attribute::GroupItemPtr item, QWidget* p, qtBaseView* bview, Qt::Orientation orient);
  virtual qtItem* createVoidItemWidget(attribute::VoidItemPtr item, QWidget* p, qtBaseView* bview, Qt::Orientation orient);
  virtual qtItem* createModelEntityItemWidget(attribute::ModelEntityItemPtr item, QWidget* p, qtBaseView* bview, Qt::Orientation orient);
  virtual qtItem* createMeshSelectionItemWidget(attribute::MeshSelectionItemPtr item, QWidget* p, qtBaseView* bview, Qt::Orientation orient);
  virtual qtItem* createMeshItemWidget(attribute::MeshItemPtr item, QWidget* p, qtBaseView* bview, Qt::Orientation orient);
};

  } // namespace extension
} // namespace smtk

#endif // __smtk_extension_qt_qtAttributeItemWidgetFactory_h
