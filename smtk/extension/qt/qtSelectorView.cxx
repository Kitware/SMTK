//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtSelectorView.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/io/AttributeWriter.h"
#include "smtk/io/Logger.h"

#include "smtk/view/Configuration.h"

#include <QApplication>
#include <QFile>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QSize>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QVariant>

#include <algorithm>

using namespace smtk::attribute;
using namespace smtk::extension;

class qtSelectorViewInternals
{
public:
  ~qtSelectorViewInternals() { delete this->m_qtSelectorAttribute; }

  QList<smtk::extension::qtBaseView*> ChildViews;
  std::vector<smtk::view::ConfigurationPtr> m_views;
  QList<int> m_viewEnumIdices;
  smtk::attribute::AttributePtr m_selectorAttribute;
  smtk::extension::qtAttribute* m_qtSelectorAttribute;
  smtk::attribute::ValueItemPtr m_selectorItem;
};

qtBaseView* qtSelectorView::createViewWidget(const smtk::view::Information& info)
{
  if (qtBaseAttributeView::validateInformation(info))
  {
    auto* view = new qtSelectorView(info);
    view->buildUI();
    return view;
  }
  return nullptr; // Information is not suitable for this View
}

qtSelectorView::qtSelectorView(const smtk::view::Information& info)
  : qtBaseAttributeView(info)
{
  m_internals = new qtSelectorViewInternals;
}

qtSelectorView::~qtSelectorView()
{
  this->clearChildViews();
  delete m_internals;
}

void qtSelectorView::createWidget()
{
  smtk::view::ConfigurationPtr view = this->configuration();
  if (!view)
  {
    return;
  }
  this->clearChildViews();
  this->Widget = new QFrame(this->parentWidget());
  this->Widget->setObjectName(view->name().c_str());

  // First we need to create the basic layout and the
  // View Selection Mechanism
  if (!this->createSelector())
  {
    return;
  }

  // Now lets add the children Views
  this->createChildren();
}

bool qtSelectorView::createSelector()
{
  //create the layout for the frame area
  smtk::view::ConfigurationPtr view = this->configuration();
  QVBoxLayout* layout = new QVBoxLayout(this->Widget);
  layout->setMargin(0);
  this->Widget->setLayout(layout);

  // Get the Selector Attribute
  smtk::attribute::ResourcePtr resource = this->attributeResource();
  std::string attName, defName;
  view->details().attribute("SelectorName", attName);
  view->details().attribute("SelectorType", defName);

  smtk::attribute::DefinitionPtr attDef;
  m_internals->m_selectorAttribute = resource->findAttribute(attName);
  if (!m_internals->m_selectorAttribute)
  {
    attDef = resource->findDefinition(defName);
    if (!attDef)
    {
      std::cerr << "Selector Type: " << defName << " could not be found!\n";
      return false;
    }
    m_internals->m_selectorAttribute = resource->createAttribute(attName, attDef);
    this->attributeCreated(m_internals->m_selectorAttribute);
  }
  else
  {
    attDef = m_internals->m_selectorAttribute->definition();
  }

  // Is the first item discrete?
  if (!m_internals->m_selectorAttribute->numberOfItems())
  {
    std::cerr << "Selector Attribute: " << attName << " has no items!\n";
    return false;
  }
  m_internals->m_selectorItem =
    dynamic_pointer_cast<ValueItem>(m_internals->m_selectorAttribute->item(0));
  if ((m_internals->m_selectorItem == nullptr) || (!m_internals->m_selectorItem->isDiscrete()))
  {
    std::cerr << "Selector Attribute: " << attName << " does not have a discrete item!\n";
    return false;
  }

  // OK Now lets create a qtAttribute for the selector
  int labelWidth =
    this->uiManager()->getWidthOfAttributeMaxLabel(attDef, this->uiManager()->advancedFont());

  this->setFixedLabelWidth(labelWidth);
  smtk::view::Configuration::Component comp; // Right now not being used
  m_internals->m_qtSelectorAttribute =
    new qtAttribute(m_internals->m_selectorAttribute, comp, this->widget(), this, true);
  m_internals->m_qtSelectorAttribute->createBasicLayout(true);
  layout->addWidget(m_internals->m_qtSelectorAttribute->widget());
  QObject::connect(
    m_internals->m_qtSelectorAttribute,
    &qtAttribute::modified,
    this,
    &qtSelectorView::selectionChanged);
  return true;
}

void qtSelectorView::refreshSelector()
{
  if (m_internals->m_qtSelectorAttribute == nullptr)
  {
    return; // there is nothing to refresh
  }
  m_internals->m_qtSelectorAttribute->removeItems();
  m_internals->m_qtSelectorAttribute->createBasicLayout(true);
}

bool qtSelectorView::isEmpty() const
{
  return !this->displayItem(m_internals->m_selectorItem);
}

bool qtSelectorView::createChildren()
{
  smtk::view::ConfigurationPtr view = this->configuration();
  smtk::attribute::ResourcePtr resource = this->attributeResource();

  // We need the selector item's definition in order to get the enumeration info
  auto selItemDef =
    dynamic_pointer_cast<const ValueItemDefinition>(m_internals->m_selectorItem->definition());

  int viewsIndex;
  viewsIndex = view->details().findChild("Views");
  if (viewsIndex == -1)
  {
    // there are no children views
    return false;
  }
  smtk::view::Configuration::Component& viewsComp = view->details().child(viewsIndex);
  std::size_t i, n = viewsComp.numberOfChildren();
  smtk::view::ConfigurationPtr v;
  qtBaseView* qtView;

  for (i = 0; i < n; i++)
  {
    if (viewsComp.child(i).name() != "View")
    {
      // not a view component - skip it
      std::cerr << "Skipping Child: " << viewsComp.child(i).name() << " should be View\n";
      continue;
    }
    // Get the title
    std::string t;
    viewsComp.child(i).attribute("Title", t);
    v = resource->findView(t);
    if (!v)
    {
      // No such View by that name in attribute resource
      continue;
    }

    // Do we have an enum value?
    std::string enumVal;
    if (!viewsComp.child(i).attribute("Enum", enumVal))
    {
      std::cerr << "View: " << t << " does not have a Enum Value";
      continue;
    }
    // Is this a valid Enum value?
    std::size_t enumIndex;
    if (!selItemDef->getEnumIndex(enumVal, enumIndex))
    {
      std::cerr << "View: " << t << "'s Enum Value: " << enumVal << " is not valid\n";
      continue;
    }
    // Setup the information for the new child view based off of
    // this one
    auto vinfo = m_viewInfo;
    vinfo.insertOrAssign<smtk::view::ConfigurationPtr>(v);
    vinfo.insertOrAssign<QWidget*>(this->Widget);

    qtView = this->uiManager()->createView(vinfo);
    if (qtView)
    {
      this->addChildView(qtView, static_cast<int>(enumIndex));
      // Should this view be visible?
      qtView->widget()->setVisible(
        m_internals->m_selectorItem->isSet() &&
        (m_internals->m_selectorItem->discreteIndex() == static_cast<int>(enumIndex)));
    }
  }
  return true;
}

void qtSelectorView::getChildView(const std::string& viewType, QList<qtBaseView*>& views)
{
  Q_FOREACH (qtBaseView* childView, m_internals->ChildViews)
  {
    if (childView->configuration()->type() == viewType)
    {
      views.append(childView);
    }
    else if (childView->configuration()->type() == "Group")
    {
      qobject_cast<qtSelectorView*>(childView)->getChildView(viewType, views);
    }
  }
}

qtBaseView* qtSelectorView::getChildView(int pageIndex)
{
  if (pageIndex >= 0 && pageIndex < m_internals->ChildViews.count())
  {
    return m_internals->ChildViews.value(pageIndex);
  }
  return nullptr;
}

void qtSelectorView::addChildView(qtBaseView* child, int viewEnumIndex)
{
  if (!m_internals->ChildViews.contains(child))
  {
    m_internals->ChildViews.append(child);
    m_internals->m_viewEnumIdices.append(viewEnumIndex);
    QFrame* frame = dynamic_cast<QFrame*>(this->Widget);
    if (!frame || !child || !child->configuration())
    {
      return;
    }

    QLayout* vLayout = this->Widget->layout();
    vLayout->addWidget(child->widget());
    vLayout->setAlignment(Qt::AlignTop);
    QObject::connect(child, &qtBaseView::modified, this, &qtBaseView::modified);
  }
}

const QList<qtBaseView*>& qtSelectorView::childViews() const
{
  return m_internals->ChildViews;
}

void qtSelectorView::clearChildViews()
{
  Q_FOREACH (qtBaseView* childView, m_internals->ChildViews)
  {
    delete childView;
  }
  m_internals->ChildViews.clear();
}

void qtSelectorView::updateUI()
{
  this->refreshSelector();
  Q_FOREACH (qtBaseView* childView, m_internals->ChildViews)
  {
    childView->updateUI();
  }
}

void qtSelectorView::onShowCategory()
{
  this->refreshSelector();
  Q_FOREACH (qtBaseView* childView, m_internals->ChildViews)
  {
    childView->onShowCategory();
  }
  this->qtBaseAttributeView::onShowCategory();
}

void qtSelectorView::showAdvanceLevelOverlay(bool show)
{
  this->refreshSelector();
  Q_FOREACH (qtBaseView* childView, m_internals->ChildViews)
  {
    childView->showAdvanceLevelOverlay(show);
  }
  this->qtBaseAttributeView::showAdvanceLevelOverlay(show);
}

void qtSelectorView::updateModelAssociation()
{
  Q_FOREACH (qtBaseView* childView, m_internals->ChildViews)
  {
    auto* iview = dynamic_cast<qtBaseAttributeView*>(childView);
    if (iview)
    {
      iview->updateModelAssociation();
    }
  }
}
void qtSelectorView::selectionChanged()
{
  // Lets iterate over the views and set the proper visibility
  int newIndex = m_internals->m_selectorItem->discreteIndex();
  int i, n = m_internals->ChildViews.size();
  if (m_internals->m_selectorItem->isSet())
  {
    for (i = 0; i < n; i++)
    {
      m_internals->ChildViews.at(i)->widget()->setVisible(
        m_internals->m_viewEnumIdices.at(i) == newIndex);
    }
  }
  else // In this case the selector item has not been set so there is
       // nothing to be displayed.
  {
    for (i = 0; i < n; i++)
    {
      m_internals->ChildViews.at(i)->widget()->setVisible(false);
    }
  }
  Q_EMIT qtBaseView::modified();
}

bool qtSelectorView::isValid() const
{
  // If the selector itself is not set then the view
  // is not valid - else we need to test all visible views
  if (!m_internals->m_selectorAttribute->isValid())
  {
    return false;
  }
  int i, n = m_internals->ChildViews.size();
  for (i = 0; i < n; i++)
  {
    if (
      m_internals->ChildViews.at(i)->widget()->isVisible() &&
      !m_internals->ChildViews.at(i)->isValid())
    {
      return false;
    }
  }
  return true;
}
