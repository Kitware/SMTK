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

#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/io/AttributeWriter.h"
#include "smtk/io/Logger.h"

#include "smtk/common/View.h"

#include <QScrollArea>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSize>
#include <QTabWidget>
#include <QFile>
#include <QApplication>
#include <QVariant>
#include <QFrame>
#include <QLabel>
#include <QFont>

using namespace smtk::attribute;

//----------------------------------------------------------------------------
class qtGroupViewInternals
{
public:
  enum Style
  {
    TABBED,
    TILED
  };
  qtGroupViewInternals() : m_style(TABBED), m_currentTabSelected(0) {}
  QList<smtk::attribute::qtBaseView*> ChildViews;
  qtGroupViewInternals::Style m_style;
  std::vector<smtk::common::ViewPtr> m_views;
  int m_currentTabSelected;
};

//----------------------------------------------------------------------------
qtBaseView *
qtGroupView::createViewWidget(smtk::common::ViewPtr dataObj,
                                  QWidget* p, qtUIManager* uiman)
{
  qtGroupView *view = new qtGroupView(dataObj, p, uiman);
  view->buildUI();
  return view;
}

//----------------------------------------------------------------------------
qtGroupView::
qtGroupView(smtk::common::ViewPtr dataObj, QWidget* p,
  qtUIManager* uiman) : qtBaseView(dataObj, p, uiman)
{
  this->Internals = new qtGroupViewInternals;
}

//----------------------------------------------------------------------------
qtGroupView::~qtGroupView()
{
  this->clearChildViews();
  delete this->Internals;
}
//----------------------------------------------------------------------------
void qtGroupView::updateCurrentTab(int ithTab)
{
  this->Internals->m_currentTabSelected = ithTab;
}

//----------------------------------------------------------------------------
void qtGroupView::createWidget( )
{
  smtk::common::ViewPtr view = this->getObject();
  if (!view)
    {
    return;
    }
  std::string val;
  if (view->details().attribute("Style", val))
    {
    std::transform(val.begin(), val.end(), val.begin(), ::tolower);
    this->Internals->m_style = (val == "tiled" ? qtGroupViewInternals::TILED :
      qtGroupViewInternals::TABBED);
    }

  this->clearChildViews();
  if(this->Internals->m_style == qtGroupViewInternals::TILED)
    {
    this->Widget = new QFrame(this->parentWidget());
    }
  else
    {
    QTabWidget *tab = new QTabWidget(this->parentWidget());
    tab->setUsesScrollButtons( true );
    this->Widget = tab;
    }

  //create the layout for the tabs area
  QVBoxLayout* layout = new QVBoxLayout(this->Widget);
  layout->setMargin(0);
  this->Widget->setLayout( layout );

  // Now process all of this view's children
  int viewsIndex;
  viewsIndex = view->details().findChild("Views");
  if (viewsIndex == -1)
    {
    // there are no children views
    return;
    }
  smtk::common::View::Component &viewsComp = view->details().child(viewsIndex);
  std::size_t i, n = viewsComp.numberOfChildren();
  smtk::common::ViewPtr v;
  smtk::attribute::System *sys = this->uiManager()->attSystem();
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
    qtView = this->uiManager()->createView(v, this->Widget);
    if (qtView)
      {
      this->addChildView(qtView);
      }
    }
    if(QTabWidget* tabWidget = qobject_cast<QTabWidget*>(this->Widget))
      {
      tabWidget->setCurrentIndex(this->Internals->m_currentTabSelected);
      tabWidget->setIconSize( QSize(24,24) );
      tabWidget->setTabPosition(QTabWidget::East);
      }
    if(this->Internals->m_style == qtGroupViewInternals::TABBED)
    {
    QObject::connect(this->Widget, SIGNAL(currentChanged(int)),
                     this, SLOT(updateCurrentTab(int)));
    }
}
//----------------------------------------------------------------------------
void qtGroupView::getChildView(const std::string &viewType, QList<qtBaseView*>& views)
{
  foreach(qtBaseView* childView, this->Internals->ChildViews)
    {
    if(childView->getObject()->type() == viewType)
      {
      views.append(childView);
      }
    else if(childView->getObject()->type() == "Group")
      {
      qobject_cast<qtGroupView*>(childView)->getChildView(
        viewType, views);
      }
    }
}
//----------------------------------------------------------------------------
qtBaseView* qtGroupView::getChildView(int pageIndex)
{
  if(pageIndex >= 0 && pageIndex < this->Internals->ChildViews.count())
    {
    return this->Internals->ChildViews.value(pageIndex);
    }
  return NULL;
}
//----------------------------------------------------------------------------
void qtGroupView::addChildView(qtBaseView* child)
{
  if(!this->Internals->ChildViews.contains(child))
    {
    this->Internals->ChildViews.append(child);
    if(this->Internals->m_style == qtGroupViewInternals::TILED)
      {
      this->addTileEntry(child);
      }
    else
      {
      this->addTabEntry(child);
      }
    }
}
//----------------------------------------------------------------------------
const QList<qtBaseView*>& qtGroupView::childViews() const
{
  return this->Internals->ChildViews;
}
//----------------------------------------------------------------------------
void qtGroupView::clearChildViews()
{
  foreach(qtBaseView* childView, this->Internals->ChildViews)
    {
    delete childView;
    }
  this->Internals->ChildViews.clear();
}

//----------------------------------------------------------------------------
void qtGroupView::updateUI()
{
  foreach(qtBaseView* childView, this->Internals->ChildViews)
    {
    childView->updateUI();
    }
}

//----------------------------------------------------------------------------
void qtGroupView::showAdvanceLevelOverlay(bool show)
{
  foreach(qtBaseView* childView, this->Internals->ChildViews)
    {
    childView->showAdvanceLevelOverlay(show);
    }
  this->qtBaseView::showAdvanceLevelOverlay(show);
}

//----------------------------------------------------------------------------
void qtGroupView::addTabEntry(qtBaseView* child)
{
  QTabWidget* tabWidget = static_cast<QTabWidget*>(this->Widget);
  if(!tabWidget || !child || !child->getObject())
    {
    return;
    }
  QWidget* tabPage = new QWidget();
  tabPage->setSizePolicy(QSizePolicy::Preferred,
    QSizePolicy::Expanding);

  QScrollArea* s = new QScrollArea(tabWidget);
  s->setWidgetResizable(true);
  s->setFrameShape(QFrame::NoFrame);
  QString secTitle = child->getObject()->title().c_str();
  QString name = "tab" + QString(secTitle);
  s->setObjectName(name);
  s->setWidget(tabPage);

  QVBoxLayout* vLayout = new QVBoxLayout(tabPage);
  vLayout->setMargin(0);
  vLayout->addWidget(child->widget());

  int index = tabWidget->addTab(s, secTitle);
  tabWidget->setTabToolTip( index, secTitle);

  //using the ui label name find if we have an icon resource
  QString resourceName = QApplication::applicationDirPath().append("/../Resources/Icons/");
  //QString resourceName = ":/SimBuilder/Icons/";
  resourceName.append(child->getObject()->iconName().c_str());
  resourceName.append(".png");

  // If user specified icons are not found, use default ones
  if ( !QFile::exists( resourceName ) )
    {
    resourceName = QString(":/SimBuilder/Icons/").
      append(secTitle).append(".png");
    }

  if ( QFile::exists( resourceName ) )
    {
    QPixmap image(resourceName);
    QIcon icon;
    QMatrix transform;
    if ( tabWidget->tabPosition() == QTabWidget::East )
      {
      transform.rotate(-90);
      }
    else if( tabWidget->tabPosition() == QTabWidget::West )
      {
      transform.rotate(90);
      }
    icon = image.transformed( transform );
    tabWidget->setTabIcon( index, icon );
    tabWidget->setTabText( index, "");
    }

  vLayout->setAlignment(Qt::AlignTop);
}

//----------------------------------------------------------------------------
void qtGroupView::addTileEntry(qtBaseView* child)
{
  QFrame* frame = static_cast<QFrame*>(this->Widget);
  if(!frame || !child || !child->getObject())
    {
    return;
    }
  QLabel* label = new QLabel(child->getObject()->title().c_str(),
    this->Widget);
  QFont titleFont;
  titleFont.setBold(true);
  titleFont.setItalic(true);
  label->setFont(titleFont);

  QLayout* vLayout = this->Widget->layout();
  vLayout->setMargin(0);
  vLayout->addWidget(label);
  vLayout->addWidget(child->widget());
  vLayout->setAlignment(Qt::AlignTop);
}
//----------------------------------------------------------------------------
void qtGroupView::updateModelAssociation()
{
  foreach(qtBaseView* childView, this->Internals->ChildViews)
    {
    childView->updateModelAssociation();
    }
}
