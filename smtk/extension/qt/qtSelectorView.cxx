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

#include "smtk/common/View.h"

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
  QList<smtk::extension::qtBaseView*> ChildViews;
  std::vector<smtk::common::ViewPtr> m_views;
  QList<int> m_viewEnumIdices;
  smtk::attribute::AttributePtr m_selectorAttribute;
  smtk::attribute::ValueItemPtr m_selectorItem;
};

qtBaseView* qtSelectorView::createViewWidget(const ViewInfo& info)
{
  qtSelectorView* view = new qtSelectorView(info);
  view->buildUI();
  return view;
}

qtSelectorView::qtSelectorView(const ViewInfo& info)
  : qtBaseView(info)
{
  this->Internals = new qtSelectorViewInternals;
}

qtSelectorView::~qtSelectorView()
{
  this->clearChildViews();
  delete this->Internals;
}

void qtSelectorView::createWidget()
{
  smtk::common::ViewPtr view = this->getObject();
  if (!view)
  {
    return;
  }
  this->clearChildViews();
  this->Widget = new QFrame(this->parentWidget());

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
  smtk::common::ViewPtr view = this->getObject();
  QVBoxLayout* layout = new QVBoxLayout(this->Widget);
  layout->setMargin(0);
  this->Widget->setLayout(layout);

  // Get the Selector Attribute
  smtk::attribute::SystemPtr sys = this->uiManager()->attSystem();
  std::string attName, defName;
  view->details().attribute("SelectorName", attName);
  view->details().attribute("SelectorType", defName);

  smtk::attribute::DefinitionPtr attDef;
  this->Internals->m_selectorAttribute = sys->findAttribute(attName);
  if (!this->Internals->m_selectorAttribute)
  {
    attDef = sys->findDefinition(defName);
    if (!attDef)
    {
      std::cerr << "Selector Type: " << defName << " could not be found!\n";
      return false;
    }
    this->Internals->m_selectorAttribute = sys->createAttribute(attName, attDef);
  }
  else
  {
    attDef = this->Internals->m_selectorAttribute->definition();
  }

  // Is the first item discrete?
  if (!this->Internals->m_selectorAttribute->numberOfItems())
  {
    std::cerr << "Selector Attribute: " << attName << " has no items!\n";
    return false;
  }
  this->Internals->m_selectorItem =
    dynamic_pointer_cast<ValueItem>(this->Internals->m_selectorAttribute->item(0));
  if ((this->Internals->m_selectorItem == nullptr) ||
    (!this->Internals->m_selectorItem->isDiscrete()))
  {
    std::cerr << "Selector Attribute: " << attName << " does not have a discrete item!\n";
    return false;
  }

  // OK Now lets create a qtAttribute for the selector
  int labelWidth =
    this->uiManager()->getWidthOfAttributeMaxLabel(attDef, this->uiManager()->advancedFont());

  this->setFixedLabelWidth(labelWidth);
  qtAttribute* attInstance =
    new qtAttribute(this->Internals->m_selectorAttribute, this->widget(), this);
  attInstance->createBasicLayout(true);
  layout->addWidget(attInstance->widget());
  QObject::connect(attInstance, SIGNAL(modified()), this, SLOT(selectionChanged()));
  return true;
}

bool qtSelectorView::createChildren()
{
  smtk::common::ViewPtr view = this->getObject();
  smtk::attribute::SystemPtr sys = this->uiManager()->attSystem();

  // We need the selector item's definition in order to get the enumeration info
  auto selItemDef =
    dynamic_pointer_cast<const ValueItemDefinition>(this->Internals->m_selectorItem->definition());

  int viewsIndex;
  viewsIndex = view->details().findChild("Views");
  if (viewsIndex == -1)
  {
    // there are no children views
    return false;
  }
  smtk::common::View::Component& viewsComp = view->details().child(viewsIndex);
  std::size_t i, n = viewsComp.numberOfChildren();
  smtk::common::ViewPtr v;
  bool hasDefaultIndex = selItemDef->hasDefault();
  int defaultIndex = (hasDefaultIndex ? selItemDef->defaultDiscreteIndex() : -1);
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
    v = sys->findView(t);
    if (!v)
    {
      // No such View by that name in attribute system
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
    smtk::extension::ViewInfo vinfo = this->m_viewInfo;
    vinfo.m_view = v;
    vinfo.m_parent = this->Widget;
    qtView = this->uiManager()->createView(vinfo);
    if (qtView)
    {
      this->addChildView(qtView, static_cast<int>(enumIndex));
    }
    // Should this view be visible?
    qtView->widget()->setVisible(hasDefaultIndex && (defaultIndex == static_cast<int>(enumIndex)));
  }
  return true;
}

void qtSelectorView::getChildView(const std::string& viewType, QList<qtBaseView*>& views)
{
  foreach (qtBaseView* childView, this->Internals->ChildViews)
  {
    if (childView->getObject()->type() == viewType)
    {
      views.append(childView);
    }
    else if (childView->getObject()->type() == "Group")
    {
      qobject_cast<qtSelectorView*>(childView)->getChildView(viewType, views);
    }
  }
}

qtBaseView* qtSelectorView::getChildView(int pageIndex)
{
  if (pageIndex >= 0 && pageIndex < this->Internals->ChildViews.count())
  {
    return this->Internals->ChildViews.value(pageIndex);
  }
  return NULL;
}

void qtSelectorView::addChildView(qtBaseView* child, int viewEnumIndex)
{
  if (!this->Internals->ChildViews.contains(child))
  {
    this->Internals->ChildViews.append(child);
    this->Internals->m_viewEnumIdices.append(viewEnumIndex);
    QFrame* frame = dynamic_cast<QFrame*>(this->Widget);
    if (!frame || !child || !child->getObject())
    {
      return;
    }

    QLayout* vLayout = this->Widget->layout();
    vLayout->setMargin(0);
    vLayout->addWidget(child->widget());
    vLayout->setAlignment(Qt::AlignTop);
  }
}

const QList<qtBaseView*>& qtSelectorView::childViews() const
{
  return this->Internals->ChildViews;
}

void qtSelectorView::clearChildViews()
{
  foreach (qtBaseView* childView, this->Internals->ChildViews)
  {
    delete childView;
  }
  this->Internals->ChildViews.clear();
}

void qtSelectorView::updateUI()
{
  foreach (qtBaseView* childView, this->Internals->ChildViews)
  {
    childView->updateUI();
  }
}

void qtSelectorView::onShowCategory()
{
  foreach (qtBaseView* childView, this->Internals->ChildViews)
  {
    childView->onShowCategory();
  }
  this->qtBaseView::onShowCategory();
}

void qtSelectorView::showAdvanceLevelOverlay(bool show)
{
  foreach (qtBaseView* childView, this->Internals->ChildViews)
  {
    childView->showAdvanceLevelOverlay(show);
  }
  this->qtBaseView::showAdvanceLevelOverlay(show);
}

void qtSelectorView::updateModelAssociation()
{
  foreach (qtBaseView* childView, this->Internals->ChildViews)
  {
    childView->updateModelAssociation();
  }
}
void qtSelectorView::selectionChanged()
{
  // Lets iterate over the views and set the proper visibility
  int newIndex = this->Internals->m_selectorItem->discreteIndex();
  int i, n = this->Internals->ChildViews.size();
  for (i = 0; i < n; i++)
  {
    this->Internals->ChildViews.at(i)->widget()->setVisible(
      this->Internals->m_viewEnumIdices.at(i) == newIndex);
  }
}
