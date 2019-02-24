//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtAnalysisView.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/extension/qt/qtAttribute.h"
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

#include <algorithm>

using namespace smtk::attribute;
using namespace smtk::extension;

qtBaseView* qtAnalysisView::createViewWidget(const ViewInfo& info)
{
  qtAnalysisView* view = new qtAnalysisView(info);
  view->buildUI();
  return view;
}

qtAnalysisView::qtAnalysisView(const ViewInfo& info)
  : qtBaseView(info)
{
}

qtAnalysisView::~qtAnalysisView()
{
}

void qtAnalysisView::createWidget()
{
  smtk::view::ViewPtr view = this->getObject();
  if (!view)
  {
    return;
  }
  this->Widget = new QFrame(this->parentWidget());
  //create the layout for the frame area
  QVBoxLayout* layout = new QVBoxLayout(this->Widget);
  layout->setMargin(0);
  this->Widget->setLayout(layout);

  auto attRes = this->uiManager()->attResource();
  std::string attName, defName;
  view->details().attribute("AnalysisAttributeName", attName);
  view->details().attribute("AnalysisAttributeType", defName);

  // Lets see if we need to create the analysis
  // attribute or its definition?
  auto attDef = attRes->findDefinition(defName);
  if (!attDef)
  {
    attDef = attRes->buildAnalysesDefinition(defName);
  }
  m_analysisAttribute = attRes->findAttribute(attName);
  if (!m_analysisAttribute)
  {
    m_analysisAttribute = attRes->createAttribute(attName, attDef);
  }

  // OK Now lets create a qtAttribute for the Analysis Attribute
  // Tell the UI Manager not to filter
  this->uiManager()->disableCategoryChecks();
  int labelWidth =
    this->uiManager()->getWidthOfAttributeMaxLabel(attDef, this->uiManager()->advancedFont());

  this->setFixedLabelWidth(labelWidth);
  smtk::view::View::Component comp; // Right now not being used
  qtAttribute* attInstance = new qtAttribute(m_analysisAttribute, comp, this->widget(), this);
  attInstance->createBasicLayout(true);
  this->uiManager()->enableCategoryChecks();
  layout->addWidget(attInstance->widget());
  QObject::connect(attInstance, SIGNAL(modified()), this, SLOT(analysisChanged()));
  // OK - lets apply the initial state.
  this->analysisChanged();
}

void qtAnalysisView::processAnalysisItem(
  smtk::attribute::ConstItemPtr item, std::set<std::string>& cats)
{
  // If the item is not active there is nothing to do
  if (!item->isEnabled())
  {
    return;
  }
  auto attRes = this->uiManager()->attResource();
  auto myCats = attRes->analysisCategories(item->name());
  cats.insert(myCats.begin(), myCats.end());
  auto gitem = std::dynamic_pointer_cast<const GroupItem>(item);
  if (gitem == nullptr)
  {
    return;
  }

  std::size_t i, n = gitem->numberOfItemsPerGroup();
  for (i = 0; i < n; i++)
  {
    this->processAnalysisItem(gitem->item(i), cats);
  }
}

void qtAnalysisView::analysisChanged()
{
  // Lets iterate over the items in the analysis attribute and set
  // the categories accordingly
  std::set<std::string> cats;
  std::size_t i, n = m_analysisAttribute->numberOfItems();
  for (i = 0; i < n; i++)
  {
    this->processAnalysisItem(m_analysisAttribute->item(i), cats);
  }
  this->uiManager()->setToLevelCategories(cats);
}
