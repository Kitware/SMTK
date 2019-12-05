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

using namespace smtk::attribute;
using namespace smtk::extension;

typedef void (qtBaseView::*qtBaseViewMemFn)();

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

  void updateChildren(qtGroupView* gview, qtBaseViewMemFn mfunc);

  // ChildViews represent all of the qtViews associated with the
  // group's children views while TabbedViews represent those
  // children who are currently being displayed as tabs
  QList<smtk::extension::qtBaseView *> m_ChildViews, m_TabbedViews;
  QList<QWidget*> m_PageWidgets;
  QList<QIcon> m_PageIcons;
  QList<QLabel*> m_Labels;
  qtGroupViewInternals::Style m_style;
  std::vector<smtk::view::ConfigurationPtr> m_activeViews;
  int m_currentTabSelected;
  QTabWidget::TabPosition m_tabPosition;
  std::string m_savedViewName;
};

void qtGroupViewInternals::updateChildren(qtGroupView* gview, qtBaseViewMemFn mfunc)
{
  // In the case of tiling we don't want to show the
  // label for empty views
  if (m_style == qtGroupViewInternals::TILED)
  {
    int i, size = m_ChildViews.size();
    for (i = 0; i < size; i++)
    {
      auto child = m_ChildViews.at(i);
      (child->*mfunc)();
      if (child->isEmpty())
      {
        child->widget()->hide();
        m_Labels.at(i)->hide();
      }
      else
      {
        child->widget()->show();
        m_Labels.at(i)->show();
      }
    }
  }
  else if (m_style == qtGroupViewInternals::TABBED)
  {
    QTabWidget* tabWidget = dynamic_cast<QTabWidget*>(gview->widget());

    if (!tabWidget)
    {
      return;
    }
    tabWidget->blockSignals(true);
    tabWidget->clear();
    std::string lastSavedViewName = gview->uiManager()->activeTabInfo(gview->getObject()->name());
    m_TabbedViews.clear();
    m_currentTabSelected = -1;
    int i, size = m_ChildViews.size();
    for (i = 0; i < size; i++)
    {
      auto child = m_ChildViews.at(i);
      (child->*mfunc)();
      if (child->isEmpty())
      {
        continue;
      }
      m_PageWidgets.at(i)->show();
      m_TabbedViews.append(child);
      if (child->getObject()->name() == lastSavedViewName)
      {
        m_currentTabSelected = m_TabbedViews.size() - 1;
      }
      if (m_PageIcons.at(i).isNull())
      {
        QString secTitle = child->getObject()->label().c_str();
        tabWidget->addTab(m_PageWidgets.at(i), secTitle);
      }
      else
      {
        tabWidget->addTab(m_PageWidgets.at(i), m_PageIcons.at(i), "");
      }
    }
    // If we could not find the saved tab and there are tabs in the widget
    // set the curren to the first
    if ((m_currentTabSelected == -1) && !m_TabbedViews.empty())
    {
      m_currentTabSelected = 0;
    }
    tabWidget->blockSignals(false);
    tabWidget->setCurrentIndex(m_currentTabSelected);
  }
  else
  {
    foreach (qtBaseView* childView, m_ChildViews)
    {
      (childView->*mfunc)();
    }
  }
}

qtBaseView* qtGroupView::createViewWidget(const ViewInfo& info)
{
  qtGroupView* view = new qtGroupView(info);
  view->buildUI();
  return view;
}

qtGroupView::qtGroupView(const ViewInfo& info)
  : qtBaseAttributeView(info)
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
  if (ithTab == -1)
  {
    //Clear the active tab info since nothing is selected
    this->uiManager()->setActiveTabInfo(this->getObject()->name(), "");
    return;
  }
  qtBaseView* currView = this->getChildView(ithTab);
  if (currView)
  {
    this->uiManager()->setActiveTabInfo(this->getObject()->name(), currView->getObject()->name());
    currView->updateUI();
  }
}

void qtGroupView::createWidget()
{
  smtk::view::ConfigurationPtr view = this->getObject();
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
    // If we have previously created a widget for this view
    // lets get the name of the last selected tab View name
    this->Internals->m_savedViewName = this->uiManager()->activeTabInfo(this->getObject()->name());
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
  smtk::view::Configuration::Component& viewsComp = view->details().child(viewsIndex);
  std::size_t i, n = viewsComp.numberOfChildren();
  smtk::view::ConfigurationPtr v;
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
    // If there is no currently saved View then lets set it to the currently selected tab
    if (this->Internals->m_savedViewName.empty())
    {
      qtBaseView* currView = this->getChildView(this->Internals->m_currentTabSelected);
      if (currView)
      {
        this->uiManager()->setActiveTabInfo(
          this->getObject()->name(), currView->getObject()->name());
        this->Internals->m_savedViewName = currView->getObject()->name();
      }
    }
  }
  if (this->Internals->m_style == qtGroupViewInternals::TABBED)
  {
    QObject::connect(this->Widget, SIGNAL(currentChanged(int)), this, SLOT(updateCurrentTab(int)));
  }
}

qtBaseView* qtGroupView::getChildView(int pageIndex)
{
  if (pageIndex >= 0 && pageIndex < this->Internals->m_TabbedViews.count())
  {
    return this->Internals->m_TabbedViews.value(pageIndex);
  }
  return NULL;
}

void qtGroupView::addChildView(qtBaseView* child)
{
  if (!this->Internals->m_ChildViews.contains(child))
  {
    this->Internals->m_ChildViews.append(child);
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
  return this->Internals->m_ChildViews;
}

void qtGroupView::clearChildViews()
{
  foreach (qtBaseView* childView, this->Internals->m_ChildViews)
  {
    delete childView;
  }
  this->Internals->m_ChildViews.clear();
  this->Internals->m_Labels.clear();
  this->Internals->m_PageWidgets.clear();
  this->Internals->m_PageIcons.clear();
  this->Internals->m_TabbedViews.clear();
}

void qtGroupView::updateUI()
{
  this->Internals->updateChildren(this, &qtBaseView::updateUI);
}

void qtGroupView::onShowCategory()
{
  this->Internals->updateChildren(this, &qtBaseView::onShowCategory);
  this->qtBaseAttributeView::onShowCategory();
}

void qtGroupView::showAdvanceLevelOverlay(bool show)
{
  foreach (qtBaseView* childView, this->Internals->m_ChildViews)
  {
    childView->showAdvanceLevelOverlay(show);
  }
  this->qtBaseAttributeView::showAdvanceLevelOverlay(show);
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
  this->Internals->m_PageWidgets.push_back(tabPage);
  this->Internals->m_PageIcons.push_back(icon);

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

    this->Internals->m_TabbedViews.append(child);
    tabWidget->setTabToolTip(index, secTitle);
    if (child->getObject()->name() == this->Internals->m_savedViewName)
    {
      this->Internals->m_currentTabSelected = index;
    }
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
  this->Internals->m_Labels.append(label);
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
  foreach (qtBaseView* childView, this->Internals->m_ChildViews)
  {
    auto iview = dynamic_cast<qtBaseAttributeView*>(childView);
    if (iview)
    {
      iview->updateModelAssociation();
    }
  }
}

bool qtGroupView::isEmpty() const
{
  foreach (qtBaseView* childView, this->Internals->m_ChildViews)
  {
    if (!childView->isEmpty())
    {
      return false;
    }
  }
  return true;
}
