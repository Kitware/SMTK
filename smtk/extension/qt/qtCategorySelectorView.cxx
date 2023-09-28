//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtCategorySelectorView.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"
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

class qtCategorySelectorViewInternals
{
public:
  QList<smtk::extension::qtBaseView*> ChildViews;
  std::vector<smtk::view::ConfigurationPtr> m_views;
  QList<std::string> m_viewCategories;
  smtk::attribute::AttributePtr m_selectorAttribute;
  smtk::attribute::ValueItemPtr m_selectorItem;
};

qtBaseView* qtCategorySelectorView::createViewWidget(const smtk::view::Information& info)
{
  if (qtBaseAttributeView::validateInformation(info))
  {
    auto* view = new qtCategorySelectorView(info);
    view->buildUI();
    return view;
  }
  return nullptr; // Information is not suitable for this View
}

qtCategorySelectorView::qtCategorySelectorView(const smtk::view::Information& info)
  : qtBaseAttributeView(info)
{
  this->Internals = new qtCategorySelectorViewInternals;
}

qtCategorySelectorView::~qtCategorySelectorView()
{
  this->clearChildViews();
  delete this->Internals;
}

void qtCategorySelectorView::createWidget()
{
  smtk::view::ConfigurationPtr view = this->configuration();
  if (!view)
  {
    return;
  }
  this->clearChildViews();
  this->Widget = new QFrame(this->parentWidget());
  this->Widget->setObjectName(view->name().c_str());
  QVBoxLayout* layout = new QVBoxLayout(this->Widget);
  layout->setMargin(0);

  this->createChildren();
}

bool qtCategorySelectorView::createChildren()
{
  smtk::view::ConfigurationPtr view = this->configuration();
  smtk::attribute::ResourcePtr resource = this->attributeResource();

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
  std::string currentCategory = this->uiManager()->currentCategory();

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
    std::string catVal;
    if (!viewsComp.child(i).attribute("Category", catVal))
    {
      std::cerr << "View: " << t << " does not have a Category Value";
      continue;
    }
    // Setup the information for the new child view based off of
    // this one but with a different view configuration and (parent) widget
    auto vinfo = m_viewInfo;
    vinfo.insertOrAssign<smtk::view::ConfigurationPtr>(v);
    vinfo.insertOrAssign<QWidget*>(this->Widget);
    qtView = this->uiManager()->createView(vinfo);
    if (qtView)
    {
      this->addChildView(qtView, catVal);
    }
    // Should this view be visible?
    qtView->widget()->setVisible(catVal == currentCategory);
  }
  return true;
}

void qtCategorySelectorView::getChildView(const std::string& viewType, QList<qtBaseView*>& views)
{
  Q_FOREACH (qtBaseView* childView, this->Internals->ChildViews)
  {
    if (childView->configuration()->type() == viewType)
    {
      views.append(childView);
    }
    else if (childView->configuration()->type() == "Group")
    {
      qobject_cast<qtCategorySelectorView*>(childView)->getChildView(viewType, views);
    }
  }
}

qtBaseView* qtCategorySelectorView::getChildView(int pageIndex)
{
  if (pageIndex >= 0 && pageIndex < this->Internals->ChildViews.count())
  {
    return this->Internals->ChildViews.value(pageIndex);
  }
  return nullptr;
}

void qtCategorySelectorView::addChildView(qtBaseView* child, const std::string& cval)
{
  if (!this->Internals->ChildViews.contains(child))
  {
    this->Internals->ChildViews.append(child);
    this->Internals->m_viewCategories.append(cval);
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

const QList<qtBaseView*>& qtCategorySelectorView::childViews() const
{
  return this->Internals->ChildViews;
}

void qtCategorySelectorView::clearChildViews()
{
  Q_FOREACH (qtBaseView* childView, this->Internals->ChildViews)
  {
    delete childView;
  }
  this->Internals->ChildViews.clear();
}

void qtCategorySelectorView::updateUI()
{
  Q_FOREACH (qtBaseView* childView, this->Internals->ChildViews)
  {
    childView->updateUI();
  }
}

void qtCategorySelectorView::onShowCategory()
{
  // Lets iterate over the views and set the proper visibility
  // as well as updating the child view's attribute items
  int i, n = this->Internals->ChildViews.size();
  std::string currentCategory = this->uiManager()->currentCategory();
  for (i = 0; i < n; i++)
  {
    this->Internals->ChildViews.at(i)->widget()->setVisible(
      this->Internals->m_viewCategories.at(i) == currentCategory);
    this->Internals->ChildViews.at(i)->onShowCategory();
  }
  this->qtBaseAttributeView::onShowCategory();
}

void qtCategorySelectorView::showAdvanceLevelOverlay(bool show)
{
  Q_FOREACH (qtBaseView* childView, this->Internals->ChildViews)
  {
    childView->showAdvanceLevelOverlay(show);
  }
  this->qtBaseAttributeView::showAdvanceLevelOverlay(show);
}

void qtCategorySelectorView::updateModelAssociation()
{
  Q_FOREACH (qtBaseView* childView, this->Internals->ChildViews)
  {
    auto* iview = dynamic_cast<qtBaseAttributeView*>(childView);
    if (iview)
    {
      iview->updateModelAssociation();
    }
  }
}

bool qtCategorySelectorView::isValid() const
{
  Q_FOREACH (qtBaseView* childView, this->Internals->ChildViews)
  {
    if (childView->widget()->isVisible() && !childView->isValid())
    {
      return false;
    }
  }
  return true;
}
