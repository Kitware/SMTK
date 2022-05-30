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
#include "smtk/attribute/Item.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/io/AttributeWriter.h"
#include "smtk/io/Logger.h"
#include "smtk/simulation/UserData.h"

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
#include <vector>

using namespace smtk::attribute;
using namespace smtk::extension;

qtBaseView* qtAnalysisView::createViewWidget(const smtk::view::Information& info)
{
  if (qtBaseAttributeView::validateInformation(info))
  {
    auto* view = new qtAnalysisView(info);
    view->buildUI();
    return view;
  }
  return nullptr; // Information is not suitable for this View
}

qtAnalysisView::qtAnalysisView(const smtk::view::Information& info)
  : qtBaseAttributeView(info)
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

  smtk::view::ConfigurationPtr view = this->configuration();
  if (!view)
  {
    return;
  }
  this->Widget = new QFrame(this->parentWidget());
  this->Widget->setObjectName(view->name().c_str());
  //create the layout for the frame area
  QVBoxLayout* layout = new QVBoxLayout(this->Widget);
  layout->setMargin(0);
  this->Widget->setLayout(layout);

  auto attRes = this->attributeResource();
  std::string attName, defName;
  view->details().attribute("AnalysisAttributeName", attName);
  view->details().attribute("AnalysisAttributeType", defName);

  // Lets see if we need to create the analysis
  // attribute or its definition?
  bool attChanged = false;
  auto attDef = attRes->findDefinition(defName);
  if (!attDef)
  {
    attDef = attRes->analyses().buildAnalysesDefinition(attRes, defName);
  }
  m_analysisAttribute = attRes->findAttribute(attName);
  if (!m_analysisAttribute)
  {
    m_analysisAttribute = attRes->createAttribute(attName, attDef);
    attChanged = true;
  }

#if !defined(__APPLE__)
  // This is a workaround for an unwanted scrolling behavoir that has been observed
  // on Windows and linux builds. More details can be found at
  // https://gitlab.kitware.com/cmb/smtk/-/issues/442
  // The following code marks all void items in the analyis attribute with a UserData
  // instance that is read by qtVoidItem. This is only done for analysis views.
  std::vector<smtk::attribute::Item::Ptr> items;
  auto filter = [](smtk::attribute::Item::Ptr item) {
    return item->type() == smtk::attribute::Item::VoidType;
  };
  m_analysisAttribute->filterItems(items, filter, false);
  auto uData = smtk::simulation::UserDataInt::New();
  for (auto& item : items)
  {
    item->setUserData("smtk.extensions.void_item.no_focus", uData);
  }
#endif

  // OK Now lets create a qtAttribute for the Analysis Attribute
  int labelWidth =
    this->uiManager()->getWidthOfAttributeMaxLabel(attDef, this->uiManager()->advancedFont());

  this->setFixedLabelWidth(labelWidth);
  smtk::view::Configuration::Component comp; // Right now not being used
  m_qtAnalysisAttribute = new qtAttribute(m_analysisAttribute, comp, this->widget(), this);
  m_qtAnalysisAttribute->createBasicLayout(true);
  layout->addWidget(m_qtAnalysisAttribute->widget());
  QObject::connect(
    m_qtAnalysisAttribute, &qtAttribute::modified, this, &qtAnalysisView::analysisAttributeChanged);
  // OK - lets apply the initial state.
  this->analysisChanged(attChanged);
}

void qtAnalysisView::analysisAttributeChanged()
{
  this->analysisChanged(true);
}

void qtAnalysisView::analysisChanged(bool attChanged)
{
  // Lets iterate over the items in the analysis attribute and set
  // the categories accordingly
  std::set<std::string> cats;
  auto attRes = this->attributeResource();
  if (attRes == nullptr)
  {
    return; // There is nothing we can do
  }
  attRes->analyses().getAnalysisAttributeCategories(m_analysisAttribute, cats);
  attRes->setActiveCategories(cats);
  attRes->setActiveCategoriesEnabled(true);
  // If we have a top level View - tell it to refresh
  if (this->uiManager()->topView())
  {
    this->uiManager()->topView()->onShowCategory();
  }

  if (attChanged)
  {
    // let the outside world know the analysis attribute has changed
    this->attributeChanged(m_analysisAttribute);
  }
  // We shouldn't need to modified since this should have caused all the Qt views to
  // update
}

bool qtAnalysisView::categoryTest(const smtk::attribute::ItemPtr& /*unused*/) const
{
  // Analysis View contents ignores category filtering
  return true;
}

bool qtAnalysisView::isValid() const
{
  return m_analysisAttribute->isValid();
}
