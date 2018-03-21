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
#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DateTimeItem.h"
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/MeshSelectionItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/VoidItem.h"

#include <QFrame>
#include <QLabel>
#include <QPointer>
#include <QVBoxLayout>

#include <stdlib.h> // for atexit()

using namespace smtk::attribute;
using namespace smtk::extension;

qtAttributeItemWidgetFactory* qtAttribute::s_factory = NULL;

class qtAttributeInternals
{
public:
  qtAttributeInternals(smtk::attribute::AttributePtr myAttribute, QWidget* p, qtBaseView* myView)
  {
    m_parentWidget = p;
    m_attribute = myAttribute;
    m_view = myView;
  }
  ~qtAttributeInternals() {}
  smtk::attribute::WeakAttributePtr m_attribute;
  QPointer<QWidget> m_parentWidget;
  QList<smtk::extension::qtItem*> m_items;
  QPointer<qtBaseView> m_view;
};

qtAttribute::qtAttribute(smtk::attribute::AttributePtr myAttribute, QWidget* p, qtBaseView* myView)
{
  m_internals = new qtAttributeInternals(myAttribute, p, myView);
  m_widget = NULL;
  m_useSelectionManager = false;
  this->createWidget();
}

qtAttribute::~qtAttribute()
{
  // First Clear all the items
  for (int i = 0; i < m_internals->m_items.count(); i++)
  {
    delete m_internals->m_items.value(i);
  }

  m_internals->m_items.clear();
  if (m_widget)
  {
    delete m_widget;
  }

  delete m_internals;
}

void qtAttribute::createWidget()
{
  if (!this->attribute() ||
    (!this->attribute()->numberOfItems() && !this->attribute()->associations()))
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
  if (m_internals->m_view)
  {
    for (i = 0; i < n; i++)
    {
      if (m_internals->m_view->displayItem(att->item(static_cast<int>(i))))
      {
        numShowItems++;
      }
    }
    // also check associations
    if (m_internals->m_view->displayItem(att->associations()))
    {
      numShowItems++;
    }
  }
  else // show everything
  {
    numShowItems = static_cast<int>(att->associations() ? n + 1 : n);
  }
  if (numShowItems == 0)
  {
    return;
  }

  QFrame* attFrame = new QFrame(this->parentWidget());
  attFrame->setFrameShape(QFrame::Box);
  m_widget = attFrame;

  QVBoxLayout* layout = new QVBoxLayout(m_widget);
  layout->setMargin(3);
  m_widget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
}

void qtAttribute::addItem(qtItem* child)
{
  if (!m_internals->m_items.contains(child))
  {
    m_internals->m_items.append(child);
    // When the item is modified so is the attribute that uses it
    connect(child, SIGNAL(modified()), this, SLOT(onItemModified()));
  }
}

QList<qtItem*>& qtAttribute::items() const
{
  return m_internals->m_items;
}

void qtAttribute::showAdvanceLevelOverlay(bool show)
{
  for (int i = 0; i < m_internals->m_items.count(); i++)
  {
    m_internals->m_items.value(i)->showAdvanceLevelOverlay(show);
  }
}

void qtAttribute::createBasicLayout(bool includeAssociations)
{
  //If there is no main widget there is nothing to show
  if (!m_widget)
  {
    return;
  }
  QLayout* layout = m_widget->layout();
  qtItem* qItem = NULL;
  smtk::attribute::AttributePtr att = this->attribute();
  // If there are model assocications for the attribute, create UI for them if requested.
  // This will be the same widget used for ModelEntityItem.
  if (includeAssociations && att->associations())
  {
    qItem = this->createItem(att->associations(), m_widget, m_internals->m_view);
    if (qItem && qItem->widget())
    {
      layout->addWidget(qItem->widget());
      this->addItem(qItem);
    }
  }
  // Now go through all child items and create ui components.
  std::size_t i, n = att->numberOfItems();
  for (i = 0; i < n; i++)
  {
    qItem = this->createItem(att->item(static_cast<int>(i)), m_widget, m_internals->m_view);
    if (qItem && qItem->widget())
    {
      layout->addWidget(qItem->widget());
      this->addItem(qItem);
    }
  }
}

smtk::attribute::AttributePtr qtAttribute::attribute()
{
  return m_internals->m_attribute.lock();
}

QWidget* qtAttribute::parentWidget()
{
  return m_internals->m_parentWidget;
}

qtItem* qtAttribute::createItem(
  smtk::attribute::ItemPtr item, QWidget* pW, qtBaseView* bview, Qt::Orientation enVectorItemOrient)
{
  if (bview && (!bview->displayItem(item)))
  {
    return NULL;
  }

  qtItem* aItem = NULL;
  switch (item->type())
  {
    case smtk::attribute::Item::AttributeRefType: // This is always inside valueItem ???
      aItem = qtAttribute::s_factory->createRefItemWidget(
        smtk::dynamic_pointer_cast<RefItem>(item), pW, bview, enVectorItemOrient);
      break;
    case smtk::attribute::Item::DoubleType:
    case smtk::attribute::Item::IntType:
    case smtk::attribute::Item::StringType:
      aItem = qtAttribute::s_factory->createValueItemWidget(
        smtk::dynamic_pointer_cast<ValueItem>(item), pW, bview, enVectorItemOrient);
      break;
    case smtk::attribute::Item::DirectoryType:
      aItem = qtAttribute::s_factory->createDirectoryItemWidget(
        smtk::dynamic_pointer_cast<DirectoryItem>(item), pW, bview, enVectorItemOrient);
      break;
    case smtk::attribute::Item::FileType:
      aItem = qtAttribute::s_factory->createFileItemWidget(
        smtk::dynamic_pointer_cast<FileItem>(item), pW, bview, enVectorItemOrient);
      break;
    case smtk::attribute::Item::GroupType:
      aItem = qtAttribute::s_factory->createGroupItemWidget(
        smtk::dynamic_pointer_cast<GroupItem>(item), pW, bview, enVectorItemOrient);
      break;
    case smtk::attribute::Item::VoidType:
      aItem = qtAttribute::s_factory->createVoidItemWidget(
        smtk::dynamic_pointer_cast<VoidItem>(item), pW, bview, enVectorItemOrient);
      break;
    case smtk::attribute::Item::DateTimeType:
      aItem = qtAttribute::s_factory->createDateTimeItemWidget(
        smtk::dynamic_pointer_cast<DateTimeItem>(item), pW, bview, enVectorItemOrient);
      break;
    case smtk::attribute::Item::ComponentType:
      aItem = qtAttribute::s_factory->createComponentItemWidget(
        smtk::dynamic_pointer_cast<ComponentItem>(item), pW, bview, enVectorItemOrient);
      break;
    default:
      //m_errorStatus << "Error: Unsupported Item Type: " <<
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
  {
    return;
  }

  delete qtAttribute::s_factory;
  qtAttribute::s_factory = f;
  static bool once = false;
  if (!once && qtAttribute::s_factory)
  {
    once = true;
    atexit(cleanupItemFactory);
  }
}

/* Slot for properly emitting signals when an attribute's item is modified */
void qtAttribute::onItemModified()
{
  // are we here due to a signal?
  QObject* sobject = this->sender();
  if (sobject == NULL)
  {
    return;
  }
  auto iobject = qobject_cast<smtk::extension::qtItem*>(sobject);
  if (iobject == NULL)
  {
    return;
  }
  emit this->itemModified(iobject);
  emit this->modified();
}

/// Return the factory currently being used to create widgets for child items.
qtAttributeItemWidgetFactory* qtAttribute::itemWidgetFactory()
{
  return qtAttribute::s_factory;
}
