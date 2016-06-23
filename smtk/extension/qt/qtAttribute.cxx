//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtAttribute.h"

#include "smtk/extension/qt/qtAttributeItemWidgetFactory.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/extension/qt/qtModelEntityItem.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/MeshSelectionItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/VoidItem.h"

#include <QPointer>
#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>

#include <stdlib.h> // for atexit()

using namespace smtk::attribute;
using namespace smtk::extension;

qtAttributeItemWidgetFactory* qtAttribute::s_factory = NULL;

//----------------------------------------------------------------------------
class qtAttributeInternals
{
public:
  qtAttributeInternals(smtk::attribute::AttributePtr myAttribute, QWidget* p,
    qtBaseView* myView)
  {
  this->m_parentWidget = p;
  this->m_attribute = myAttribute;
  this->m_view = myView;
  }
  ~qtAttributeInternals()
  {
  }
 smtk::attribute::WeakAttributePtr m_attribute;
 QPointer<QWidget> m_parentWidget;
 QList<smtk::extension::qtItem*> m_items;
 QPointer<qtBaseView> m_view;
};

//----------------------------------------------------------------------------
qtAttribute::qtAttribute(smtk::attribute::AttributePtr myAttribute, QWidget* p,
   qtBaseView* myView)
{
  this->m_internals  = new qtAttributeInternals(myAttribute, p, myView);
  this->m_widget = NULL;
  this->createWidget();
}

//----------------------------------------------------------------------------
qtAttribute::~qtAttribute()
{
  // First Clear all the items
  for(int i=0; i < this->m_internals->m_items.count(); i++)
    {
    delete this->m_internals->m_items.value(i);
    }

  this->m_internals->m_items.clear();
  if (this->m_widget)
    {
    delete this->m_widget;
    }

  delete this->m_internals;
}

//----------------------------------------------------------------------------
void qtAttribute::createWidget()
{
  if(!this->attribute() || !this->attribute()->numberOfItems())
    {
    return;
    }

  if (!qtAttribute::s_factory)
    {
    qtAttribute::setItemWidgetFactory(new qtAttributeItemWidgetFactory());
    }

  int numShowItems = 0;
  smtk::attribute::AttributePtr att = this->attribute();
  std::size_t i, n = att->numberOfItems();
  if(this->m_internals->m_view)
    {
    for (i = 0; i < n; i++)
      {
      if(this->m_internals->m_view->displayItem(att->item(static_cast<int>(i))))
        {
        numShowItems++;
        }
      }
    }
  else // show everything
    {
    numShowItems = n;
    }
  if(numShowItems == 0)
    {
    return;
    }

  QFrame* attFrame = new QFrame(this->parentWidget());
  attFrame->setFrameShape(QFrame::Box);
  this->m_widget = attFrame;

  QVBoxLayout* layout = new QVBoxLayout(this->m_widget);
  layout->setMargin(3);
  this->m_widget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
}

//----------------------------------------------------------------------------
void qtAttribute::addItem(qtItem* child)
{
  if(!this->m_internals->m_items.contains(child))
    {
    this->m_internals->m_items.append(child);
    }
}

//----------------------------------------------------------------------------
QList<qtItem*>& qtAttribute::items() const
{
  return this->m_internals->m_items;
}

//----------------------------------------------------------------------------
void qtAttribute::showAdvanceLevelOverlay(bool show)
{
  for(int i=0; i < this->m_internals->m_items.count(); i++)
    {
    this->m_internals->m_items.value(i)->showAdvanceLevelOverlay(show);
    }
}

//----------------------------------------------------------------------------
void qtAttribute::createBasicLayout(bool includeAssociations)
{
  //If there is no main widget there is nothing to show
  if (!this->m_widget)
    {
    return;
    }

  QLayout* layout = this->m_widget->layout();
  qtItem* qItem = NULL;
  smtk::attribute::AttributePtr att = this->attribute();
  // If there are model assocications for the attribute, create UI for them if requested.
  // This will be the same widget used for ModelEntityItem.
  if(includeAssociations && att->associations())
    {
    qItem = this->createItem(att->associations(), this->m_widget,
      this->m_internals->m_view);
    if(qItem && qItem->widget())
      {
      layout->addWidget(qItem->widget());
      this->addItem(qItem);
      }
    }
  // Now go through all child items and create ui components.
  std::size_t i, n = att->numberOfItems();
  for (i = 0; i < n; i++)
    {
    qItem = this->createItem(att->item(static_cast<int>(i)), this->m_widget,
      this->m_internals->m_view);
    if(qItem && qItem->widget())
      {
      layout->addWidget(qItem->widget());
      this->addItem(qItem);
      }
    }
}
//----------------------------------------------------------------------------
void qtAttribute::onRequestEntityAssociation()
{
  foreach(qtItem* item, this->m_internals->m_items)
    {
    if(qtModelEntityItem* mitem = qobject_cast<qtModelEntityItem*>(item))
      mitem->onRequestEntityAssociation();
    }
}

//----------------------------------------------------------------------------
smtk::attribute::AttributePtr qtAttribute::attribute()
{
  return this->m_internals->m_attribute.lock();
}

//----------------------------------------------------------------------------
QWidget* qtAttribute::parentWidget()
{
  return this->m_internals->m_parentWidget;
}

//----------------------------------------------------------------------------
qtItem* qtAttribute::createItem(smtk::attribute::ItemPtr item, QWidget* pW,
  qtBaseView* bview, Qt::Orientation enVectorItemOrient)
{
  if(bview && (!bview->displayItem(item)))
    {
    return NULL;
    }

  qtItem* aItem = NULL;
  switch (item->type())
    {
    case smtk::attribute::Item::ATTRIBUTE_REF: // This is always inside valueItem ???
      aItem = qtAttribute::s_factory->createRefItemWidget(
        smtk::dynamic_pointer_cast<RefItem>(item), pW, bview, enVectorItemOrient);
      break;
    case smtk::attribute::Item::DOUBLE:
    case smtk::attribute::Item::INT:
    case smtk::attribute::Item::STRING:
      aItem = qtAttribute::s_factory->createValueItemWidget(
        smtk::dynamic_pointer_cast<ValueItem>(item), pW, bview, enVectorItemOrient);
      break;
    case smtk::attribute::Item::DIRECTORY:
      aItem = qtAttribute::s_factory->createDirectoryItemWidget(
        smtk::dynamic_pointer_cast<DirectoryItem>(item), pW, bview, enVectorItemOrient);
      break;
    case smtk::attribute::Item::FILE:
      aItem = qtAttribute::s_factory->createFileItemWidget(
        smtk::dynamic_pointer_cast<FileItem>(item), pW, bview, enVectorItemOrient);
      break;
    case smtk::attribute::Item::GROUP:
      aItem = qtAttribute::s_factory->createGroupItemWidget(
        smtk::dynamic_pointer_cast<GroupItem>(item), pW, bview, enVectorItemOrient);
      break;
    case smtk::attribute::Item::VOID:
      aItem = qtAttribute::s_factory->createVoidItemWidget(
        smtk::dynamic_pointer_cast<VoidItem>(item), pW, bview, enVectorItemOrient);
      break;
    case smtk::attribute::Item::MODEL_ENTITY:
      aItem = qtAttribute::s_factory->createModelEntityItemWidget(
        smtk::dynamic_pointer_cast<ModelEntityItem>(item), pW, bview, enVectorItemOrient);
      break;
    case smtk::attribute::Item::MESH_SELECTION:
      aItem = qtAttribute::s_factory->createMeshSelectionItemWidget(
        smtk::dynamic_pointer_cast<MeshSelectionItem>(item), pW, bview, enVectorItemOrient);
      break;
    case smtk::attribute::Item::MESH_ENTITY:
      aItem = qtAttribute::s_factory->createMeshItemWidget(
        smtk::dynamic_pointer_cast<MeshItem>(item), pW, bview, enVectorItemOrient);
      break;
    default:
      //this->m_errorStatus << "Error: Unsupported Item Type: " <<
      // smtk::attribute::Item::type2String(item->type()) << "\n";
      break;
    }
  return aItem;
}

// Used with atexit() to prevent leakage:
static void cleanupItemFactory()
{
  qtAttribute::setItemWidgetFactory(NULL);
}

/**\brief Set the factory to be used for all future items created.
 *
 * This method is ignored if \a f is ignored.
 */
void qtAttribute::setItemWidgetFactory(qtAttributeItemWidgetFactory* f)
{
  if (f == qtAttribute::s_factory)
    return;

  delete qtAttribute::s_factory;
  qtAttribute::s_factory = f;
  static bool once = false;
  if (!once && qtAttribute::s_factory)
    {
    once = true;
    atexit(cleanupItemFactory);
    }
}

/// Return the factory currently being used to create widgets for child items.
qtAttributeItemWidgetFactory* qtAttribute::itemWidgetFactory()
{
  return qtAttribute::s_factory;
}
