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
  qtGroupViewInternals() = default;

  void updateChildren(qtGroupView* gview, qtBaseViewMemFn mfunc);

  // ChildViews represent all of the qtViews associated with the
  // group's children views while TabbedViews represent those
  // children who are currently being displayed as tabs
  QList<smtk::extension::qtBaseView*> m_ChildViews, m_TabbedViews;
  QList<QWidget*> m_PageWidgets;
  QList<QIcon> m_PageIcons;
  QList<QLabel*> m_Labels;
  qtGroupViewInternals::Style m_style{ TABBED };
  std::vector<smtk::view::ConfigurationPtr> m_activeViews;
  int m_currentTabSelected{ 0 };
  QTabWidget::TabPosition m_tabPosition{ QTabWidget::East };
  std::string m_savedViewName;
  QIcon m_alertIcon;
  const std::string m_activeTabViewAttNamee = "ActiveTab";
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
      auto* child = m_ChildViews.at(i);
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
    std::string lastSavedViewName;
    gview->configuration()->details().attribute(m_activeTabViewAttNamee, lastSavedViewName);
    m_TabbedViews.clear();
    m_currentTabSelected = -1;
    int i, size = m_ChildViews.size();
    for (i = 0; i < size; i++)
    {
      auto* child = m_ChildViews.at(i);
      (child->*mfunc)();
      if (child->isEmpty())
      {
        continue;
      }
      m_PageWidgets.at(i)->show();
      m_TabbedViews.append(child);
      if (child->configuration()->name() == lastSavedViewName)
      {
        m_currentTabSelected = m_TabbedViews.size() - 1;
      }
      if (m_PageIcons.at(i).isNull())
      {
        QString secTitle = child->configuration()->label().c_str();
        if (child->isValid())
        {
          tabWidget->addTab(m_PageWidgets.at(i), secTitle);
        }
        else
        {
          tabWidget->addTab(m_PageWidgets.at(i), gview->alertIcon(), secTitle);
        }
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
  else if (m_style == qtGroupViewInternals::GROUP_BOX)
  {
    int i, size = m_ChildViews.size();
    for (i = 0; i < size; i++)
    {
      auto* child = m_ChildViews.at(i);
      QString childName = child->configuration()->name().c_str();
      (child->*mfunc)();

      qtCollapsibleGroupWidget* collapsibleWidget =
        dynamic_cast<qtCollapsibleGroupWidget*>(child->widget()->parent()->parent());

      if (collapsibleWidget)
      {
        collapsibleWidget->setVisible(!child->isEmpty());
      }
    }
  }
  else
  {
    Q_FOREACH (qtBaseView* childView, m_ChildViews)
    {
      (childView->*mfunc)();
    }
  }
}

qtBaseView* qtGroupView::createViewWidget(const smtk::view::Information& info)
{
  if (qtBaseAttributeView::validateInformation(info))
  {
    auto* view = new qtGroupView(info);
    view->buildUI();
    return view;
  }
  return nullptr; // Information is not suitable for this View
}

qtGroupView::qtGroupView(const smtk::view::Information& info)
  : qtBaseAttributeView(info)
{
  m_internals = new qtGroupViewInternals;
  QPixmap image = this->uiManager()->alertPixmap();
  QTransform transform;
  std::string val;
  auto view = this->configuration();
  transform.scale(0.5, 0.5);
  if (view)
  {
    if (view->details().attribute("TabPosition", val))
    {
      std::transform(val.begin(), val.end(), val.begin(), ::tolower);
      if (val == "north")
      {
        m_internals->m_tabPosition = QTabWidget::North;
      }
      else if (val == "south")
      {
        m_internals->m_tabPosition = QTabWidget::South;
      }
      else if (val == "west")
      {
        m_internals->m_tabPosition = QTabWidget::West;
        transform.rotate(90);
      }
      else
      {
        // The default is east
        transform.rotate(-90);
      }
    }
  }
  m_internals->m_alertIcon = image.transformed(transform);
}

qtGroupView::~qtGroupView()
{
  this->clearChildViews();
  delete m_internals;
}

void qtGroupView::updateCurrentTab(int ithTab)
{
  m_internals->m_currentTabSelected = ithTab;
  if (ithTab == -1)
  {
    //Clear the active tab info since nothing is selected
    this->configuration()->details().setAttribute(m_internals->m_activeTabViewAttNamee, "");
    return;
  }
  qtBaseView* currView = this->getChildView(ithTab);
  if (currView)
  {
    this->configuration()->details().setAttribute(
      m_internals->m_activeTabViewAttNamee, currView->configuration()->name());
    currView->updateUI();
  }
}

void qtGroupView::createWidget()
{
  smtk::view::ConfigurationPtr view = this->configuration();
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
      m_internals->m_style = qtGroupViewInternals::TILED;
    }
    else if (val == "tabbed")
    {
      m_internals->m_style = qtGroupViewInternals::TABBED;
    }
    else if (val == "groupbox")
    {
      m_internals->m_style = qtGroupViewInternals::GROUP_BOX;
    }
    else
    {
      view->details().attribute("Style", val);
      std::cerr << "Unsupported Group View Style = " << val << "\n";
    }
  }
  this->clearChildViews();
  if (m_internals->m_style != qtGroupViewInternals::TABBED)
  {
    this->Widget = new QFrame(this->parentWidget());
  }
  else
  {
    QTabWidget* tab = new QTabWidget(this->parentWidget());
    // If we have previously created a widget for this view
    // lets get the name of the last selected tab View name
    this->configuration()->details().attribute(
      m_internals->m_activeTabViewAttNamee, m_internals->m_savedViewName);
    tab->setUsesScrollButtons(true);
    this->Widget = tab;
  }
  this->Widget->setObjectName(view->name().c_str());

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
  smtk::attribute::ResourcePtr resource = this->attributeResource();
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
    auto vinfo = m_viewInfo;
    vinfo.insertOrAssign<smtk::view::ConfigurationPtr>(v);
    vinfo.insertOrAssign<QWidget*>(this->Widget);
    vinfo.insertOrAssign<std::weak_ptr<smtk::attribute::Resource>>(resource);
    qtView = this->uiManager()->createView(vinfo);
    if (qtView)
    {
      this->addChildView(qtView);
    }
  }
  QTabWidget* tabWidget = qobject_cast<QTabWidget*>(this->Widget);
  if (tabWidget != nullptr)
  {
    tabWidget->setCurrentIndex(m_internals->m_currentTabSelected);
    tabWidget->setIconSize(QSize(24, 24));
    tabWidget->setTabPosition(m_internals->m_tabPosition);
    // If there is no currently saved View then lets set it to the currently selected tab
    if (m_internals->m_savedViewName.empty())
    {
      qtBaseView* currView = this->getChildView(m_internals->m_currentTabSelected);
      if (currView)
      {
        m_internals->m_savedViewName = currView->configuration()->name();
        this->configuration()->details().setAttribute(
          m_internals->m_activeTabViewAttNamee, m_internals->m_savedViewName);
      }
    }
    QObject::connect(tabWidget, &QTabWidget::currentChanged, this, &qtGroupView::updateCurrentTab);
  }
}

qtBaseView* qtGroupView::getChildView(int pageIndex)
{
  if (pageIndex >= 0 && pageIndex < m_internals->m_TabbedViews.count())
  {
    return m_internals->m_TabbedViews.value(pageIndex);
  }
  return nullptr;
}

bool qtGroupView::isValid() const
{
  // If the view is tabbed lets look at the tabbed views which
  // is a subset of the children views which are visible tabs
  // else we need to examine the children views
  if (m_internals->m_style == qtGroupViewInternals::TABBED)
  {
    Q_FOREACH (qtBaseView* childView, m_internals->m_TabbedViews)
    {
      if (!childView->isValid())
      {
        return false;
      }
    }
    return true;
  }
  Q_FOREACH (qtBaseView* childView, m_internals->m_ChildViews)
  {
    if (!childView->isValid())
    {
      return false;
    }
  }
  return true;
}

void qtGroupView::addChildView(qtBaseView* child)
{
  if (!m_internals->m_ChildViews.contains(child))
  {
    m_internals->m_ChildViews.append(child);
    if (m_internals->m_style == qtGroupViewInternals::TILED)
    {
      this->addTileEntry(child);
    }
    else if (m_internals->m_style == qtGroupViewInternals::GROUP_BOX)
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
  return m_internals->m_ChildViews;
}

void qtGroupView::clearChildViews()
{
  Q_FOREACH (qtBaseView* childView, m_internals->m_ChildViews)
  {
    delete childView;
  }
  m_internals->m_ChildViews.clear();
  m_internals->m_Labels.clear();
  m_internals->m_PageWidgets.clear();
  m_internals->m_PageIcons.clear();
  m_internals->m_TabbedViews.clear();
}

void qtGroupView::updateUI()
{
  m_internals->updateChildren(this, &qtBaseView::updateUI);
}

void qtGroupView::onShowCategory()
{
  m_internals->updateChildren(this, &qtBaseView::onShowCategory);
  this->qtBaseAttributeView::onShowCategory();
}

void qtGroupView::showAdvanceLevelOverlay(bool show)
{
  Q_FOREACH (qtBaseView* childView, m_internals->m_ChildViews)
  {
    childView->showAdvanceLevelOverlay(show);
  }
  this->qtBaseAttributeView::showAdvanceLevelOverlay(show);
}

void qtGroupView::addTabEntry(qtBaseView* child)
{
  QTabWidget* tabWidget = dynamic_cast<QTabWidget*>(this->Widget);
  if (!tabWidget || !child || !child->configuration())
  {
    return;
  }
  QWidget* content = new QWidget();
  content->setObjectName("tabContent");
  content->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

  QScrollArea* tabPage = new QScrollArea(tabWidget);
  tabPage->setWidgetResizable(true);
  tabPage->setFrameShape(QFrame::NoFrame);
  QString secTitle = child->configuration()->label().c_str();
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
  resourceName.append(child->configuration()->iconName().c_str());
  resourceName.append(".png");

  // If user specified icons are not found, use default ones
  if (!QFile::exists(resourceName))
  {
    resourceName = QString(":/SimBuilder/Icons/").append(secTitle).append(".png");
  }

  if (QFile::exists(resourceName))
  {
    QPixmap image(resourceName);
    QTransform transform;
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
  m_internals->m_PageWidgets.push_back(tabPage);
  m_internals->m_PageIcons.push_back(icon);
  QObject::connect(child, &qtBaseView::modified, this, &qtGroupView::childModified);

  if (!child->isEmpty())
  {
    int index;
    if (icon.isNull())
    {
      if (child->isValid())
      {
        index = tabWidget->addTab(tabPage, secTitle);
      }
      else
      {
        index = tabWidget->addTab(tabPage, m_internals->m_alertIcon, secTitle);
      }
    }
    else
    {
      index = tabWidget->addTab(tabPage, icon, "");
    }

    m_internals->m_TabbedViews.append(child);
    tabWidget->setTabToolTip(index, secTitle);
    if (child->configuration()->name() == m_internals->m_savedViewName)
    {
      m_internals->m_currentTabSelected = index;
    }
  }
  else
  {
    tabPage->hide();
  }

  vLayout->setAlignment(Qt::AlignTop);
}

void qtGroupView::childModified()
{
  auto* child = dynamic_cast<qtBaseView*>(this->sender());
  if (child == nullptr)
  {
    return;
  }
  // Note that even though Child View A was modified, Child View B
  // could have its validity changed - for example it could have reference item
  // whose value is dependent on the contents of View A - so we should check all
  // of the tabs not just the one modified explicitly.
  QTabWidget* tabWidget = dynamic_cast<QTabWidget*>(this->Widget);
  if (tabWidget != nullptr)
  {
    int i, n = m_internals->m_TabbedViews.count();
    for (i = 0; i < n; i++)
    {
      if (m_internals->m_TabbedViews.value(i)->isValid())
      {
        tabWidget->setTabIcon(i, QIcon());
      }
      else
      {
        tabWidget->setTabIcon(i, m_internals->m_alertIcon);
      }
    }
  }
  Q_EMIT qtBaseView::modified();
}
void qtGroupView::addGroupBoxEntry(qtBaseView* child)
{
  QFrame* frame = dynamic_cast<QFrame*>(this->Widget);
  if (!frame || !child || !child->configuration())
  {
    return;
  }
  QObject::connect(child, &qtBaseView::modified, this, &qtGroupView::childModified);
  smtk::extension::qtCollapsibleGroupWidget* gw =
    new qtCollapsibleGroupWidget(frame, child->configuration());
  this->Widget->layout()->addWidget(gw);
  gw->setName(child->configuration()->label().c_str());
  gw->contentsLayout()->addWidget(child->widget());
  bool open = false; // incase the Att was not set
  child->configuration()->details().attributeAsBool("Open", open);

  if (open)
  {
    gw->open();
  }
  else
  {
    gw->collapse();
  }
  gw->setVisible(!child->isEmpty());
}

void qtGroupView::addTileEntry(qtBaseView* child)
{
  QFrame* frame = dynamic_cast<QFrame*>(this->Widget);
  if (!frame || !child || !child->configuration())
  {
    return;
  }
  QObject::connect(child, &qtBaseView::modified, this, &qtGroupView::childModified);
  QLabel* label = new QLabel(child->configuration()->label().c_str(), this->Widget);
  m_internals->m_Labels.append(label);
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
  Q_FOREACH (qtBaseView* childView, m_internals->m_ChildViews)
  {
    auto* iview = dynamic_cast<qtBaseAttributeView*>(childView);
    if (iview)
    {
      iview->updateModelAssociation();
    }
  }
}

bool qtGroupView::isEmpty() const
{
  Q_FOREACH (qtBaseView* childView, m_internals->m_ChildViews)
  {
    if (!childView->isEmpty())
    {
      return false;
    }
  }
  return true;
}

const QIcon& qtGroupView::alertIcon() const
{
  return m_internals->m_alertIcon;
}
