//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtGroupView.h"

#include "smtk/extension/qt/qtCollapsibleGroupWidget.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/io/AttributeWriter.h"
#include "smtk/io/Logger.h"

#include "smtk/view/View.h"

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

using namespace smtk::attribute;
using namespace smtk::extension;

class qtGroupViewInternals
{
public:
  enum Style
  {
    TABBED,
    TILED,
    GROUP_BOX
  };
  qtGroupViewInternals()
    : m_style(TABBED)
    , m_currentTabSelected(0)
    , m_tabPosition(QTabWidget::East)
  {
  }
  QList<smtk::extension::qtBaseView*> ChildViews;
  QList<QWidget*> PageWidgets;
  QList<QIcon> PageIcons;
  QList<QLabel*> Labels;
  qtGroupViewInternals::Style m_style;
  std::vector<smtk::view::ViewPtr> m_views;
  int m_currentTabSelected;
  QTabWidget::TabPosition m_tabPosition;
};

qtBaseView* qtGroupView::createViewWidget(const ViewInfo& info)
{
  qtGroupView* view = new qtGroupView(info);
  view->buildUI();
  return view;
}

qtGroupView::qtGroupView(const ViewInfo& info)
  : qtBaseView(info)
{
  this->Internals = new qtGroupViewInternals;
}

qtGroupView::~qtGroupView()
{
  this->clearChildViews();
  delete this->Internals;
}

void qtGroupView::updateCurrentTab(int ithTab)
{
  this->Internals->m_currentTabSelected = ithTab;
  qtBaseView* currView = this->getChildView(ithTab);
  if (currView)
  {
    currView->updateUI();
  }
}

void qtGroupView::createWidget()
{
  smtk::view::ViewPtr view = this->getObject();
  if (!view)
  {
    return;
  }
  std::string val;
  if (view->details().attribute("Style", val))
  {
    std::transform(val.begin(), val.end(), val.begin(), ::tolower);
    if (val == "tiled")
    {
      this->Internals->m_style = qtGroupViewInternals::TILED;
    }
    else if (val == "tabbed")
    {
      this->Internals->m_style = qtGroupViewInternals::TABBED;
    }
    else if (val == "groupbox")
    {
      this->Internals->m_style = qtGroupViewInternals::GROUP_BOX;
    }
    else
    {
      view->details().attribute("Style", val);
      std::cerr << "Unsupported Group View Style = " << val << "\n";
    }
  }
  this->clearChildViews();
  if (this->Internals->m_style != qtGroupViewInternals::TABBED)
  {
    this->Widget = new QFrame(this->parentWidget());
  }
  else
  {
    if (view->details().attribute("TabPosition", val))
    {
      std::transform(val.begin(), val.end(), val.begin(), ::tolower);
      if (val == "north")
      {
        this->Internals->m_tabPosition = QTabWidget::North;
      }
      else if (val == "south")
      {
        this->Internals->m_tabPosition = QTabWidget::South;
      }
      else if (val == "west")
      {
        this->Internals->m_tabPosition = QTabWidget::West;
      }
      //Else leave it as the default which is East
    }
    QTabWidget* tab = new QTabWidget(this->parentWidget());
    tab->setUsesScrollButtons(true);
    this->Widget = tab;
  }

  //create the layout for the tabs area
  QVBoxLayout* layout = new QVBoxLayout(this->Widget);
  layout->setMargin(0);
  this->Widget->setLayout(layout);

  // Now process all of this view's children
  int viewsIndex;
  viewsIndex = view->details().findChild("Views");
  if (viewsIndex == -1)
  {
    // there are no children views
    return;
  }
  smtk::view::View::Component& viewsComp = view->details().child(viewsIndex);
  std::size_t i, n = viewsComp.numberOfChildren();
  smtk::view::ViewPtr v;
  smtk::attribute::ResourcePtr resource = this->uiManager()->attResource();
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
      std::cerr << "Skipping Child: " << t << " could not be found in attribute resource\n";

      continue;
    }
    // Setup the information for the new child view based off of
    // this one
    smtk::extension::ViewInfo vinfo = m_viewInfo;
    vinfo.m_view = v;
    vinfo.m_parent = this->Widget;
    qtView = this->uiManager()->createView(vinfo);
    if (qtView)
    {
      this->addChildView(qtView);
    }
  }
  if (QTabWidget* tabWidget = qobject_cast<QTabWidget*>(this->Widget))
  {
    tabWidget->setCurrentIndex(this->Internals->m_currentTabSelected);
    tabWidget->setIconSize(QSize(24, 24));
    tabWidget->setTabPosition(this->Internals->m_tabPosition);
  }
  if (this->Internals->m_style == qtGroupViewInternals::TABBED)
  {
    QObject::connect(this->Widget, SIGNAL(currentChanged(int)), this, SLOT(updateCurrentTab(int)));
  }
}

void qtGroupView::getChildView(const std::string& viewType, QList<qtBaseView*>& views)
{
  foreach (qtBaseView* childView, this->Internals->ChildViews)
  {
    if (childView->getObject()->type() == viewType)
    {
      views.append(childView);
    }
    else if (childView->getObject()->type() == "Group")
    {
      qobject_cast<qtGroupView*>(childView)->getChildView(viewType, views);
    }
  }
}

qtBaseView* qtGroupView::getChildView(int pageIndex)
{
  if (pageIndex >= 0 && pageIndex < this->Internals->ChildViews.count())
  {
    return this->Internals->ChildViews.value(pageIndex);
  }
  return NULL;
}

void qtGroupView::addChildView(qtBaseView* child)
{
  if (!this->Internals->ChildViews.contains(child))
  {
    this->Internals->ChildViews.append(child);
    if (this->Internals->m_style == qtGroupViewInternals::TILED)
    {
      this->addTileEntry(child);
    }
    else if (this->Internals->m_style == qtGroupViewInternals::GROUP_BOX)
    {
      this->addGroupBoxEntry(child);
    }
    else
    {
      this->addTabEntry(child);
    }
  }
}

const QList<qtBaseView*>& qtGroupView::childViews() const
{
  return this->Internals->ChildViews;
}

void qtGroupView::clearChildViews()
{
  foreach (qtBaseView* childView, this->Internals->ChildViews)
  {
    delete childView;
  }
  this->Internals->ChildViews.clear();
  this->Internals->Labels.clear();
  this->Internals->PageWidgets.clear();
  this->Internals->PageIcons.clear();
}

void qtGroupView::updateUI()
{
  // In the case of tiling we don't want to show the
  // label for empty views
  if (this->Internals->m_style == qtGroupViewInternals::TILED)
  {
    int i, size = this->Internals->ChildViews.size();
    for (i = 0; i < size; i++)
    {
      auto child = this->Internals->ChildViews.at(i);
      child->updateUI();
      if (child->isEmpty())
      {
        child->widget()->hide();
        this->Internals->Labels.at(i)->hide();
      }
      else
      {
        child->widget()->show();
        this->Internals->Labels.at(i)->show();
      }
    }
  }
  else if (this->Internals->m_style == qtGroupViewInternals::TABBED)
  {
    QTabWidget* tabWidget = dynamic_cast<QTabWidget*>(this->Widget);
    if (!tabWidget)
    {
      return;
    }
    tabWidget->clear();
    int i, size = this->Internals->ChildViews.size();
    for (i = 0; i < size; i++)
    {
      auto child = this->Internals->ChildViews.at(i);
      child->updateUI();
      if (child->isEmpty())
      {
        continue;
      }
      this->Internals->PageWidgets.at(i)->show();
      if (this->Internals->PageIcons.at(i).isNull())
      {
        QString secTitle = child->getObject()->label().c_str();
        tabWidget->addTab(this->Internals->PageWidgets.at(i), secTitle);
      }
      else
      {
        tabWidget->addTab(this->Internals->PageWidgets.at(i), this->Internals->PageIcons.at(i), "");
      }
    }
  }
  else
  {
    foreach (qtBaseView* childView, this->Internals->ChildViews)
    {
      childView->updateUI();
    }
  }
}

void qtGroupView::onShowCategory()
{
  // In the case of tiling we don't want to show the
  // label for empty views
  if (this->Internals->m_style == qtGroupViewInternals::TILED)
  {
    int i, size = this->Internals->ChildViews.size();
    for (i = 0; i < size; i++)
    {
      auto child = this->Internals->ChildViews.at(i);
      child->onShowCategory();
      if (child->isEmpty())
      {
        child->widget()->hide();
        this->Internals->Labels.at(i)->hide();
      }
      else
      {
        child->widget()->show();
        this->Internals->Labels.at(i)->show();
      }
    }
  }
  else if (this->Internals->m_style == qtGroupViewInternals::TABBED)
  {
    QTabWidget* tabWidget = dynamic_cast<QTabWidget*>(this->Widget);
    if (!tabWidget)
    {
      return;
    }
    tabWidget->clear();
    int i, size = this->Internals->ChildViews.size();
    for (i = 0; i < size; i++)
    {
      auto child = this->Internals->ChildViews.at(i);
      child->onShowCategory();
      if (child->isEmpty())
      {
        continue;
      }
      this->Internals->PageWidgets.at(i)->show();
      if (this->Internals->PageIcons.at(i).isNull())
      {
        QString secTitle = child->getObject()->label().c_str();
        tabWidget->addTab(this->Internals->PageWidgets.at(i), secTitle);
      }
      else
      {
        tabWidget->addTab(this->Internals->PageWidgets.at(i), this->Internals->PageIcons.at(i), "");
      }
    }
  }
  else
  {
    foreach (qtBaseView* childView, this->Internals->ChildViews)
    {
      childView->onShowCategory();
    }
  }
  this->qtBaseView::onShowCategory();
}

void qtGroupView::showAdvanceLevelOverlay(bool show)
{
  foreach (qtBaseView* childView, this->Internals->ChildViews)
  {
    childView->showAdvanceLevelOverlay(show);
  }
  this->qtBaseView::showAdvanceLevelOverlay(show);
}

void qtGroupView::addTabEntry(qtBaseView* child)
{
  QTabWidget* tabWidget = dynamic_cast<QTabWidget*>(this->Widget);
  if (!tabWidget || !child || !child->getObject())
  {
    return;
  }
  QWidget* content = new QWidget();
  content->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

  QScrollArea* tabPage = new QScrollArea(tabWidget);
  tabPage->setWidgetResizable(true);
  tabPage->setFrameShape(QFrame::NoFrame);
  QString secTitle = child->getObject()->label().c_str();
  QString name = "tab" + QString(secTitle);
  tabPage->setObjectName(name);
  tabPage->setWidget(content);

  QVBoxLayout* vLayout = new QVBoxLayout(content);
  vLayout->setMargin(0);
  vLayout->addWidget(child->widget());

  QIcon icon;
  //using the ui label name find if we have an icon resource
  QString resourceName = QApplication::applicationDirPath().append("/../Resources/Icons/");
  //QString resourceName = ":/SimBuilder/Icons/";
  resourceName.append(child->getObject()->iconName().c_str());
  resourceName.append(".png");

  // If user specified icons are not found, use default ones
  if (!QFile::exists(resourceName))
  {
    resourceName = QString(":/SimBuilder/Icons/").append(secTitle).append(".png");
  }

  if (QFile::exists(resourceName))
  {
    QPixmap image(resourceName);
    QMatrix transform;
    if (tabWidget->tabPosition() == QTabWidget::East)
    {
      transform.rotate(-90);
    }
    else if (tabWidget->tabPosition() == QTabWidget::West)
    {
      transform.rotate(90);
    }
    icon = image.transformed(transform);
  }

  // Save the page widget and icon
  this->Internals->PageWidgets.push_back(tabPage);
  this->Internals->PageIcons.push_back(icon);

  if (!child->isEmpty())
  {
    int index;
    if (icon.isNull())
    {
      index = tabWidget->addTab(tabPage, secTitle);
    }
    else
    {
      index = tabWidget->addTab(tabPage, icon, "");
    }

    tabWidget->setTabToolTip(index, secTitle);
  }
  else
  {
    tabPage->hide();
  }

  vLayout->setAlignment(Qt::AlignTop);
}

void qtGroupView::addGroupBoxEntry(qtBaseView* child)
{
  QFrame* frame = dynamic_cast<QFrame*>(this->Widget);
  if (!frame || !child || !child->getObject())
  {
    return;
  }
  smtk::extension::qtCollapsibleGroupWidget* gw = new qtCollapsibleGroupWidget(frame);
  this->Widget->layout()->addWidget(gw);
  gw->setName(child->getObject()->label().c_str());
  gw->contentsLayout()->addWidget(child->widget());
  gw->collapse();
}

void qtGroupView::addTileEntry(qtBaseView* child)
{
  QFrame* frame = dynamic_cast<QFrame*>(this->Widget);
  if (!frame || !child || !child->getObject())
  {
    return;
  }
  QLabel* label = new QLabel(child->getObject()->label().c_str(), this->Widget);
  this->Internals->Labels.append(label);
  QFont titleFont;
  titleFont.setBold(true);
  titleFont.setItalic(true);
  label->setFont(titleFont);

  QLayout* vLayout = this->Widget->layout();
  vLayout->setMargin(0);
  vLayout->addWidget(label);
  vLayout->addWidget(child->widget());
  vLayout->setAlignment(Qt::AlignTop);
  if (child->isEmpty())
  {
    label->hide();
    child->widget()->hide();
  }
}

void qtGroupView::updateModelAssociation()
{
  foreach (qtBaseView* childView, this->Internals->ChildViews)
  {
    childView->updateModelAssociation();
  }
}

bool qtGroupView::isEmpty() const
{
  foreach (qtBaseView* childView, this->Internals->ChildViews)
  {
    if (!childView->isEmpty())
    {
      return false;
    }
  }
  return true;
}
