//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtBaseAttributeView.h"

#include "smtk/attribute/Analyses.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/operators/Signal.h"

#include "smtk/operation/Manager.h"

#include "smtk/io/Logger.h"

#include "smtk/extension/qt/qtAttributeEditorDialog.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/view/View.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QPointer>
#include <QScrollArea>
#include <QTableWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

using namespace smtk::extension;

class qtBaseAttributeViewInternals
{
public:
  qtBaseAttributeViewInternals() {}
  ~qtBaseAttributeViewInternals() { this->clearWidgets(); }
  void deleteWidget(QWidget* w)
  {
    if (w)
    {
      delete w;
    }
  }
  void clearWidgets()
  {
    this->deleteWidget(this->AdvLevelCombo);
    this->deleteWidget(this->ShowCategoryCombo);
    this->deleteWidget(this->FilterByCategory);
    this->deleteWidget(this->AdvLevelEditButton);
    this->deleteWidget(this->AdvLevelLabel);
    this->deleteWidget(this->m_configurationCombo);
    if (this->TopLevelLayout)
    {
      delete this->TopLevelLayout;
    }
  }

  QPointer<QComboBox> AdvLevelCombo;
  QPointer<QCheckBox> FilterByCategory;
  QPointer<QLabel> FilterByCategoryLabel;
  QPointer<QComboBox> ShowCategoryCombo;
  QPointer<QLabel> AdvLevelLabel;
  QPointer<QToolButton> AdvLevelEditButton;
  QPointer<QHBoxLayout> TopLevelLayout;
  QPointer<QComboBox> m_configurationCombo;
  QPointer<QLabel> m_configurationLabel;
};

qtBaseAttributeView::qtBaseAttributeView(const ViewInfo& info)
  : qtBaseView(info)
  , m_topLevelCanCreateConfigurations(false)
{
  this->Internals = new qtBaseAttributeViewInternals;
  m_ScrollArea = nullptr;
  m_fixedLabelWidth = m_viewInfo.m_UIManager->maxValueLabelLength();
  m_topLevelInitialized = false;
  m_ignoreCategories = m_viewInfo.m_view->details().attributeAsBool("IgnoreCategories");
}

qtBaseAttributeView::~qtBaseAttributeView()
{
  if (this->Internals)
  {
    delete this->Internals;
  }
}

void qtBaseAttributeView::getDefinitions(
  smtk::attribute::DefinitionPtr attDef, QList<smtk::attribute::DefinitionPtr>& defs)
{
  std::vector<smtk::attribute::DefinitionPtr> newdefs;
  attribute::ResourcePtr attResource = attDef->resource();
  attResource->findAllDerivedDefinitions(attDef, true, newdefs);
  if (!attDef->isAbstract() && !defs.contains(attDef))
  {
    defs.push_back(attDef);
  }
  std::vector<smtk::attribute::DefinitionPtr>::iterator itDef;
  for (itDef = newdefs.begin(); itDef != newdefs.end(); ++itDef)
  {
    if (!(*itDef)->isAbstract() && !defs.contains(*itDef))
    {
      defs.push_back(*itDef);
    }
  }
}

bool qtBaseAttributeView::displayItem(smtk::attribute::ItemPtr item)
{
  if (!item)
  {
    return false;
  }
  return this->advanceLevelTest(item) && this->categoryTest(item);
}

bool qtBaseAttributeView::categoryTest(smtk::attribute::ItemPtr item)
{
  return m_ignoreCategories || this->uiManager()->passItemCategoryCheck(item->definition());
}

bool qtBaseAttributeView::advanceLevelTest(smtk::attribute::ItemPtr item)
{
  return this->uiManager()->passAdvancedCheck(item->advanceLevel());
}

void qtBaseAttributeView::setIgnoreCategories(bool mode)
{
  m_ignoreCategories = mode;
}

void qtBaseAttributeView::checkConfigurations(smtk::attribute::ItemPtr& item)
{
  // Could this be a configuration?
  if (this->Internals->m_configurationCombo != nullptr)
  {
    auto att = item->attribute();
    if (att != nullptr)
    {
      auto def = att->definition();
      if ((def != nullptr) && (def == m_topLevelConfigurationDef.lock()))
      {
        this->prepConfigurationComboBox("");
      }
    }
  }
}

void qtBaseAttributeView::valueChanged(smtk::attribute::ItemPtr item)
{
  auto* topView = dynamic_cast<qtBaseAttributeView*>(this->uiManager()->topView());
  if (topView != nullptr)
  {
    topView->checkConfigurations(item);
  }
  emit this->modified(item);
  this->uiManager()->onViewUIModified(this, item);
}

namespace
{

static void signalAttribute(smtk::extension::qtUIManager* uiManager,
  const smtk::attribute::AttributePtr& attr, const char* itemName)
{
  if (attr && uiManager && itemName && itemName[0])
  {
    bool didNotify = false;
    // create a "dummy" operation that will mark the attribute resource
    // as modified so that applications know when a "save" is required.
    auto opManager = uiManager->operationManager();
    if (opManager)
    {
      auto markModified = opManager->create<smtk::attribute::Signal>();
      if (markModified)
      {
        didNotify = markModified->parameters()->findComponent(itemName)->appendObjectValue(attr);
        auto result = markModified->operate();
        didNotify &= result->findInt("outcome")->value() ==
          static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED);
      }
    }
    if (!didNotify)
    {
      static bool once = true;
      if (once)
      {
        once = false;
        smtkWarningMacro(
          smtk::io::Logger::instance(), "Could not notify operation observers of attribute event.");
      }
    }
  }
}
}

void qtBaseAttributeView::attributeCreated(const smtk::attribute::AttributePtr& attr)
{
  signalAttribute(this->uiManager(), attr, "created");
}

void qtBaseAttributeView::attributeChanged(const smtk::attribute::AttributePtr& attr)
{
  signalAttribute(this->uiManager(), attr, "modified");
}

void qtBaseAttributeView::attributeRemoved(const smtk::attribute::AttributePtr& attr)
{
  signalAttribute(this->uiManager(), attr, "expunged");
}

bool qtBaseAttributeView::setFixedLabelWidth(int w)
{
  w = std::min(w, this->uiManager()->maxValueLabelLength());
  w = std::max(w, this->uiManager()->minValueLabelLength());
  m_fixedLabelWidth = w;
  return false;
}

void qtBaseAttributeView::buildUI()
{
  if (m_isTopLevel && (!m_topLevelInitialized))
  {
    // Process the aspects associated with top level views
    this->makeTopLevel();
  }

  // Build the View's Main Widget
  this->createWidget();

  if (!(this->Widget && this->parentWidget()))
  {
    // Should say some kind of error
    return;
  }

  QVBoxLayout* parentlayout = static_cast<QVBoxLayout*>(this->parentWidget()->layout());

  if (!parentlayout)
  {
    // Should say some kind of error or maybe create one?
    return;
  }

  if (!this->isTopLevel())
  {
    parentlayout->setAlignment(Qt::AlignTop);
    parentlayout->addWidget(this->Widget);
    return;
  }

  if (!m_ScrollArea)
  {
    // This should be an error!
    return;
  }

  m_ScrollArea->setWidget(this->Widget);
}

void qtBaseAttributeView::setInitialCategory()
{
  if (this->isTopLevel() && (this->Internals->ShowCategoryCombo != nullptr) &&
    this->Internals->ShowCategoryCombo->isEnabled())
  {
    this->onShowCategory();
  }
}

void qtBaseAttributeView::topLevelPrepAdvanceLevels(const smtk::view::ViewPtr& view)
{
  bool flag;
  // Do we need to provide advance level filtering? - this is on by default
  if ((!view->details().attributeAsBool("FilterByAdvanceLevel", flag)) || flag)
  {
    this->Internals->AdvLevelCombo = new QComboBox(this->parentWidget());
    this->uiManager()->initAdvanceLevels(this->Internals->AdvLevelCombo);

    this->Internals->AdvLevelLabel = new QLabel("Show Level:");
    this->Internals->AdvLevelLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  }

  // Do we need to show the advance level for each item?
  if (view->details().attributeAsBool("DisplayItemAccessLevel"))
  {
    QToolButton* editButton = new QToolButton(this->parentWidget());
    editButton->setCheckable(true);
    QString resourceName(":/icons/attribute/lock.png");
    editButton->setFixedSize(QSize(20, 20));
    editButton->setIcon(QIcon(resourceName));
    editButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    editButton->setToolTip("Edit access level");
    connect(editButton, SIGNAL(toggled(bool)), this, SLOT(showAdvanceLevelOverlay(bool)));
    this->Internals->AdvLevelEditButton = editButton;
  }
}

void qtBaseAttributeView::topLevelPrepCategories(
  const smtk::view::ViewPtr& view, const attribute::ResourcePtr& attResource)
{
  bool flag;
  // Do we need to provide category filtering - this is on by default
  if ((!view->details().attributeAsBool("FilterByCategory", flag)) || flag)
  {
    std::string fbcm;
    std::string catLabel("Show by Category: ");
    view->details().attribute("FilterByCategoryLabel", catLabel);
    view->details().attribute("FilterByCategoryMode", fbcm);
    // is category filtering always suppose to be on?
    if (fbcm == "alwaysOn")
    {
      this->Internals->FilterByCategoryLabel = new QLabel(this->parentWidget());
      this->Internals->FilterByCategoryLabel->setText(catLabel.c_str());
      this->Internals->FilterByCategoryLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    }
    else
    {
      this->Internals->FilterByCategory = new QCheckBox(this->parentWidget());
      this->Internals->FilterByCategory->setText(catLabel.c_str());
      this->Internals->FilterByCategory->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    }
    this->Internals->ShowCategoryCombo = new QComboBox(this->parentWidget());
    std::set<std::string>::const_iterator it;
    const std::set<std::string>& cats = attResource->categories();
    for (it = cats.begin(); it != cats.end(); it++)
    {
      this->Internals->ShowCategoryCombo->addItem(it->c_str());
    }
    this->Internals->ShowCategoryCombo->setEnabled(fbcm == "alwaysOn");
  }
}

void qtBaseAttributeView::topLevelPrepConfigurations(
  const smtk::view::ViewPtr& view, const attribute::ResourcePtr& attResource)
{
  bool flag;
  // Do we need to provide category filtering - this is off by default
  if (!(view->details().attributeAsBool("UseConfigurations", flag) && flag))
  {
    return;
  }

  // First lets see if the definition information was set
  std::string configDefType;
  if ((!view->details().attribute("ConfigurationType", configDefType)) || (configDefType == ""))
  {
    std::cerr << "Could not find ConfigurationType: " << configDefType << "\n";
    return;
  }

  // Either find or create the definition
  smtk::attribute::DefinitionPtr configDef = attResource->findDefinition(configDefType);
  if (configDef == nullptr)
  {
    smtk::attribute::Analyses& analyses = attResource->analyses();
    // Was a definition label specified?
    std::string configDefLabel;
    if ((!view->details().attribute("ConfigurationTypeLabel", configDefLabel)) ||
      (configDefLabel == ""))
    {
      configDef = analyses.buildAnalysesDefinition(attResource, configDefType);
    }
    else
    {
      configDef = analyses.buildAnalysesDefinition(attResource, configDefType, configDefLabel);
    }
  }
  m_topLevelConfigurationDef = configDef;
  // Prep the combobox
  this->Internals->m_configurationCombo = new QComboBox(this->parentWidget());
  view->details().attributeAsBool("CreateConfigurations", m_topLevelCanCreateConfigurations);
  // Build the combobox without any prefered configuration.  If there is an attribute
  // marked as the current configuration then use it.
  this->prepConfigurationComboBox("");
  std::string configLabel("Configuration: ");
  view->details().attribute("ConfigurationLabel", configLabel);
  this->Internals->m_configurationLabel = new QLabel(this->parentWidget());
  this->Internals->m_configurationLabel->setText(configLabel.c_str());
  this->Internals->m_configurationLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void qtBaseAttributeView::makeTopLevel()
{
  // Use the parent implementation to set up the UI manager from the view
  this->qtBaseView::makeTopLevel();
  m_topLevelInitialized = true;

  smtk::view::ViewPtr view = this->getObject();

  this->Internals->clearWidgets();
  const attribute::ResourcePtr attResource = this->uiManager()->attResource();

  this->topLevelPrepAdvanceLevels(view);
  this->topLevelPrepConfigurations(view, attResource);
  if (this->Internals->m_configurationCombo == nullptr)
  {
    this->topLevelPrepCategories(view, attResource);
  }

  this->Internals->TopLevelLayout = new QHBoxLayout();
  if (this->Internals->m_configurationCombo)
  {
    this->Internals->TopLevelLayout->addWidget(this->Internals->m_configurationLabel);
    this->Internals->TopLevelLayout->addWidget(this->Internals->m_configurationCombo);

    QObject::connect(this->Internals->m_configurationCombo, SIGNAL(currentIndexChanged(int)), this,
      SLOT(onConfigurationChanged(int)));
  }
  else if (this->Internals->ShowCategoryCombo)
  {
    if (this->Internals->FilterByCategory)
    {
      this->Internals->TopLevelLayout->addWidget(this->Internals->FilterByCategory);
      QObject::connect(this->Internals->FilterByCategory, SIGNAL(stateChanged(int)), this,
        SLOT(enableShowBy(int)));
    }
    else
    {
      this->Internals->TopLevelLayout->addWidget(this->Internals->FilterByCategoryLabel);
    }
    this->Internals->TopLevelLayout->addWidget(this->Internals->ShowCategoryCombo);

    QObject::connect(this->Internals->ShowCategoryCombo, SIGNAL(currentIndexChanged(int)), this,
      SLOT(onShowCategory()));
  }

  if (this->Internals->AdvLevelEditButton)
  {
    this->Internals->TopLevelLayout->addWidget(this->Internals->AdvLevelEditButton);
  }
  if (this->Internals->AdvLevelLabel)
  {
    this->Internals->TopLevelLayout->addWidget(this->Internals->AdvLevelLabel);
    this->Internals->TopLevelLayout->addWidget(this->Internals->AdvLevelCombo);
    QObject::connect(this->Internals->AdvLevelCombo, SIGNAL(currentIndexChanged(int)), this,
      SLOT(onAdvanceLevelChanged(int)));
  }
  QVBoxLayout* parentlayout = static_cast<QVBoxLayout*>(this->parentWidget()->layout());
  parentlayout->setAlignment(Qt::AlignTop);
  parentlayout->addLayout(this->Internals->TopLevelLayout);

  m_ScrollArea = new QScrollArea(this->parentWidget());
  m_ScrollArea->setWidgetResizable(true);
  m_ScrollArea->setAlignment(Qt::AlignHCenter);
  m_ScrollArea->setFrameShape(QFrame::NoFrame);
  m_ScrollArea->setObjectName("topLevelScrollArea");
  parentlayout->addWidget(m_ScrollArea);
}

void qtBaseAttributeView::showAdvanceLevel(int level)
{
  // If this is not a toplevel widget don't do anything
  if (!m_isTopLevel)
  {
    return;
  }

  smtk::view::ViewPtr view = this->getObject();
  if (!view)
  {
    return;
  }

  // Are we filtering on advance level info?
  if (this->Internals->AdvLevelCombo)
  {
    this->Internals->AdvLevelCombo->blockSignals(true);
    for (int i = 0; i < this->Internals->AdvLevelCombo->count(); i++)
    {
      int l = this->Internals->AdvLevelCombo->itemData(i).toInt();
      if (level == l)
      {
        this->Internals->AdvLevelCombo->setCurrentIndex(i);
        break;
      }
    }
    this->Internals->AdvLevelCombo->blockSignals(false);
  }

  this->uiManager()->setAdvanceLevel(level);
  if (this->advanceLevelVisible())
  {
    this->showAdvanceLevelOverlay(true);
  }
  if (this->Widget)
  {
    delete this->Widget;
    this->Widget = NULL;
  }
  this->buildUI();
}

void qtBaseAttributeView::enableShowBy(int enable)
{
  this->Internals->ShowCategoryCombo->setEnabled(enable ? true : false);
  this->onShowCategory();
}

std::string qtBaseAttributeView::currentCategory()
{
  return this->categoryEnabled() ? this->Internals->ShowCategoryCombo->currentText().toStdString()
                                 : "";
}

bool qtBaseAttributeView::categoryEnabled()
{
  return this->Internals->ShowCategoryCombo && this->Internals->ShowCategoryCombo->isEnabled();
}

void qtBaseAttributeView::onAdvanceLevelChanged(int levelIdx)
{
  // If this is not a toplevel widget don't do anything
  if (!m_isTopLevel)
  {
    return;
  }

  int level = this->Internals->AdvLevelCombo->itemData(levelIdx).toInt();
  this->showAdvanceLevel(level);
}

int qtBaseAttributeView::advanceLevel()
{
  return this->Internals->AdvLevelCombo->currentIndex();
}

void qtBaseAttributeView::onInfo()
{
  if (!m_infoDialog)
  {
    // Try to get the dialog to be displayed on top - note that in the
    // case of dock widgets this can be an issue.  In that case to at least get the dialog
    // not to be completely hidden by the operator widget when it is undocked
    // we need to parent the dialog on something else
    QWidgetList l = QApplication::topLevelWidgets();
    m_infoDialog = new qtViewInfoDialog(l.value(0));
  }
  this->setInfoToBeDisplayed();
  m_infoDialog->show();
  m_infoDialog->raise();
  m_infoDialog->activateWindow();
}

bool qtBaseAttributeView::isEmpty() const
{
  return false;
}

void qtBaseAttributeView::setTopLevelCategories(const std::set<std::string>& categories)
{
  if ((!m_isTopLevel) || (this->Internals->ShowCategoryCombo == nullptr))
  {
    this->onShowCategory();
    return;
  }
  auto current = this->Internals->ShowCategoryCombo->currentText();
  this->Internals->ShowCategoryCombo->blockSignals(true);
  this->Internals->ShowCategoryCombo->clear();
  for (auto cat : categories)
  {
    this->Internals->ShowCategoryCombo->addItem(cat.c_str());
  }
  this->Internals->ShowCategoryCombo->blockSignals(false);
  int pos = this->Internals->ShowCategoryCombo->findText(current);
  if (pos > -1)
  {
    this->Internals->ShowCategoryCombo->setCurrentIndex(pos);
  }
  else
  {
    this->Internals->ShowCategoryCombo->setCurrentIndex(0);
  }
  this->onShowCategory();
}

void qtBaseAttributeView::onConfigurationChanged(int index)
{
  std::set<std::string> cats;
  smtk::attribute::ResourcePtr attRes = this->uiManager()->attResource();
  smtk::attribute::AttributePtr att;
  std::string attName;

  if ((this->Internals->m_configurationCombo == nullptr) || (attRes == nullptr))
  {
    return; // there is nothing to do
  }

  if (index == -1)
  {
    // Nothing is selected so clear it
    this->uiManager()->setTopLevelCategories(cats);
    return;
  }

  // Are we dealing with the ability to create new configurations and did the
  // user pick the create option?
  if (m_topLevelCanCreateConfigurations &&
    (index == (this->Internals->m_configurationCombo->count() - 1)))
  {
    smtk::attribute::DefinitionPtr def = m_topLevelConfigurationDef.lock();
    if (def == nullptr)
    {
      std::cerr << "qtBaseAttributeView::onConfigurationChanged - "
                << "Could not retrieve Analysis Definition\n";
      return;
    }
    att = attRes->createAttribute(def);
    // Tell the uiManager we don't want to filter on categories
    this->uiManager()->disableCategoryChecks();
    auto editor =
      new smtk::extension::qtAttributeEditorDialog(att, this->uiManager(), this->widget());
    auto status = editor->exec();
    this->uiManager()->enableCategoryChecks();
    if (status == QDialog::Rejected)
    {
      attRes->removeAttribute(att);
      // Reset the combo box to its original configuration
      this->prepConfigurationComboBox("");
    }
    else
    {
      // Set the combobox to the new configuration
      this->prepConfigurationComboBox(att->name());
    }
    return;
  }
  else
  {
    attName = this->Internals->m_configurationCombo->currentText().toStdString();
    this->prepConfigurationComboBox(attName);
  }
}

void qtBaseAttributeView::prepConfigurationComboBox(const std::string& newConfigurationName)
{
  std::set<std::string> cats;
  smtk::attribute::ResourcePtr attRes = this->uiManager()->attResource();
  smtk::attribute::DefinitionPtr def = m_topLevelConfigurationDef.lock();
  if (def == nullptr)
  {
    std::cerr
      << "qtBaseAttributeView::prepConfigurationComboBox - configuration definition is null!\n";
    return;
  }

  std::vector<smtk::attribute::AttributePtr> atts;
  attRes->findAttributes(def, atts);
  QStringList attNames;
  std::string currentConfig = newConfigurationName;
  for (auto& att : atts)
  {
    attNames.push_back(att->name().c_str());

    if ((att->properties().get<long>().contains("_selectedConfiguration")) &&
      (att->properties().get<long>()["_selectedConfiguration"] == 1))
    {
      // If the current config name is not set then record this as the
      // current configuration
      if (currentConfig == "")
      {
        currentConfig = att->name();
        attRes->analyses().getAnalysisAttributeCategories(att, cats);
      }
      else if (att->name() != currentConfig)
      {
        // This attribute was a previous configuration
        att->properties().get<long>().erase("_selectedConfiguration");
      }
      else
      {
        // The current configuration is also the previously selected one
        attRes->analyses().getAnalysisAttributeCategories(att, cats);
      }
    }
    else if (att->name() == currentConfig)
    {
      // Set this as the new current configuration
      att->properties().get<long>()["_selectedConfiguration"] = 1;
      attRes->analyses().getAnalysisAttributeCategories(att, cats);
    }
  }

  attNames.sort();
  if (m_topLevelCanCreateConfigurations)
  {
    attNames.push_back("Create...");
  }
  this->Internals->m_configurationCombo->blockSignals(true);
  this->Internals->m_configurationCombo->clear();
  this->Internals->m_configurationCombo->addItems(attNames);
  int index;
  index = this->Internals->m_configurationCombo->findText(currentConfig.c_str());
  this->Internals->m_configurationCombo->setCurrentIndex(index);
  this->uiManager()->setTopLevelCategories(cats);
  this->Internals->m_configurationCombo->blockSignals(false);
}
