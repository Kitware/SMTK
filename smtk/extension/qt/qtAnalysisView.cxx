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
#include "smtk/attribute/StringItem.h"
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

qtBaseView* qtAnalysisView::createViewWidget(const ViewInfo& info)
{
  qtAnalysisView* view = new qtAnalysisView(info);
  view->buildUI();
  return view;
}

qtAnalysisView::qtAnalysisView(const ViewInfo& info)
  : qtBaseAttributeView(info)
  , m_qtAnalysisAttribute(nullptr)
{
  // Analysis Views don't use categories
  this->setIgnoreCategories(true);
}

qtAnalysisView::~qtAnalysisView()
{
  delete m_qtAnalysisAttribute;
}

void qtAnalysisView::createWidget()
{
  // If there is a previous qt analysis attribute delete it
  delete m_qtAnalysisAttribute;

  smtk::view::ConfigurationPtr view = this->getObject();
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
    attDef = attRes->analyses().buildAnalysesDefinition(attRes, defName);
  }
  m_analysisAttribute = attRes->findAttribute(attName);
  if (!m_analysisAttribute)
  {
    m_analysisAttribute = attRes->createAttribute(attName, attDef);
  }

  // OK Now lets create a qtAttribute for the Analysis Attribute
  int labelWidth =
    this->uiManager()->getWidthOfAttributeMaxLabel(attDef, this->uiManager()->advancedFont());

  this->setFixedLabelWidth(labelWidth);
  smtk::view::Configuration::Component comp; // Right now not being used
  m_qtAnalysisAttribute = new qtAttribute(m_analysisAttribute, comp, this->widget(), this);
  m_qtAnalysisAttribute->createBasicLayout(true);
  layout->addWidget(m_qtAnalysisAttribute->widget());
  QObject::connect(m_qtAnalysisAttribute, SIGNAL(modified()), this, SLOT(analysisChanged()));
  // OK - lets apply the initial state.
  this->analysisChanged();
}

void qtAnalysisView::analysisChanged()
{
  // Lets iterate over the items in the analysis attribute and set
  // the categories accordingly
  std::set<std::string> cats;
  auto attRes = this->uiManager()->attResource();
  if (attRes == nullptr)
  {
    return; // There is nothing we can do
  }
  attRes->analyses().getAnalysisAttributeCategories(m_analysisAttribute, cats);
  this->uiManager()->setTopLevelCategories(cats);
}

bool qtAnalysisView::categoryTest(smtk::attribute::ItemPtr /*unused*/)
{
  // Analysis View contents ignores category filtering
  return true;
}
