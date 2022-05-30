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
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/operators/Signal.h"

#include "smtk/operation/Manager.h"

#include "smtk/io/Logger.h"

#include "smtk/extension/qt/qtAttributeEditorDialog.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/view/Configuration.h"

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

#include <sstream>

using namespace smtk::extension;

class qtBaseAttributeViewInternals
{
public:
  qtBaseAttributeViewInternals() = default;
  ~qtBaseAttributeViewInternals() { this->clearWidgets(); }
  void deleteWidget(QWidget* w) { delete w; }
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

bool qtBaseAttributeView::validateInformation(const smtk::view::Information& info)
{
  if (!qtBaseView::validateInformation(info) || !info.contains<qtUIManager*>())
  {
    return false;
  }
  if (info.contains<smtk::attribute::Resource::WeakPtr>())
  {
    return true;
  }
  if (info.contains<std::set<smtk::attribute::Resource::WeakPtr>>())
  {
    std::size_t num = info.get<std::set<smtk::attribute::Resource::WeakPtr>>().size();
    if (num == 0)
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(), "View Information's set of attribute Resources is empty.");
      return false;
    }
    if (num > 1)
    {
      smtkWarningMacro(
        smtk::io::Logger::instance(),
        "View Information's set of attribute Resources contains "
          << num << ", only the first will be used.");
    }
    return true;
  }
  smtkErrorMacro(
    smtk::io::Logger::instance(), "View Information's does not contain an attribute resource.");
  return false;
}

qtBaseAttributeView::qtBaseAttributeView(const smtk::view::Information& info)
  : qtBaseView(info)
{
  this->Internals = new qtBaseAttributeViewInternals;
  m_ScrollArea = nullptr;
  m_fixedLabelWidth = this->uiManager()->maxValueLabelLength();
  m_topLevelInitialized = false;
  m_ignoreCategories = this->configuration()->details().attributeAsBool("IgnoreCategories");
  // We need to be able to determine within the a Signal Operation, which View caused
  // the change in order to avoid infinite loops.  To do this, each View will have an addressString
  // set to its address.  This string is then passed to the signalAttribute function when needed.
  std::stringstream me;
  me << std::hex << (void const*)this << std::dec;
  m_addressString = me.str();
}

qtBaseAttributeView::~qtBaseAttributeView()
{
  delete this->Internals;
}

smtk::attribute::ResourcePtr qtBaseAttributeView::attributeResource() const
{
  if (m_viewInfo.contains<smtk::attribute::Resource::WeakPtr>())
  {
    return m_viewInfo.get<smtk::attribute::Resource::WeakPtr>().lock();
  }
  return m_viewInfo.get<std::set<smtk::attribute::Resource::WeakPtr>>().begin()->lock();
}

void qtBaseAttributeView::getDefinitions(
  smtk::attribute::DefinitionPtr attDef,
  QList<smtk::attribute::DefinitionPtr>& defs)
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

bool qtBaseAttributeView::displayItem(smtk::attribute::ItemPtr item) const
{
  if (!item)
  {
    return false;
  }
  auto idef = item->definition();
  return this->advanceLevelTest(item) && this->categoryTest(item);
}

bool qtBaseAttributeView::displayItemDefinition(
  const smtk::attribute::ItemDefinitionPtr& idef) const
{
  if (!idef)
  {
    return false;
  }
  if (!this->uiManager()->passAdvancedCheck(idef->advanceLevel(0)))
  {
    return false;
  }
  auto attResoure = this->attributeResource();
  return attResoure->passActiveCategoryCheck(idef->categories());
}

bool qtBaseAttributeView::categoryTest(const smtk::attribute::ItemPtr& item) const
{
  return m_ignoreCategories || item->isRelevant();
}

bool qtBaseAttributeView::isItemWriteable(const smtk::attribute::ItemPtr& item) const
{
  return this->uiManager()->passAdvancedCheck(item->advanceLevel(1));
}

bool qtBaseAttributeView::advanceLevelTest(const smtk::attribute::ItemPtr& item) const
{
  return this->uiManager()->passAdvancedCheck(item->advanceLevel(0));
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
  Q_EMIT this->modified(item);
  this->uiManager()->onViewUIModified(this, item);
}

namespace
{

void signalAttribute(
  smtk::extension::qtUIManager* uiManager,
  const smtk::attribute::AttributePtr& attr,
  const char* itemName,
  std::vector<std::string> items,
  const std::string& source)
{
  if (attr && uiManager && itemName && itemName[0])
  {
    // create a Signal operation that will let Observers know that an
    // attribute was created, modified, or removed.
    auto opManager = uiManager->operationManager();
    if (opManager)
    {
      auto signalOp = opManager->create<smtk::attribute::Signal>();
      if (signalOp)
      {
        signalOp->parameters()->findString("source")->setValue(source);
        signalOp->parameters()->findComponent(itemName)->appendValue(attr);
        signalOp->parameters()->findString("items")->setValues(items.begin(), items.end());
        opManager->launchers()(signalOp);
      }
      else
      {
        smtkWarningMacro(
          smtk::io::Logger::instance(), "Could not create Signal Operation for attribute event.");
      }
    }
  }
}
} // namespace

void qtBaseAttributeView::attributeCreated(const smtk::attribute::AttributePtr& attr)
{
  if (attr == nullptr)
  {
    return;
  }

  qtBaseAttributeView* topView = dynamic_cast<qtBaseAttributeView*>(this->uiManager()->topView());
  // If the toplevel view is using analysis configuration we need
  // to check to see if the attribute is an analysis configuration
  if (topView && topView->Internals->m_configurationCombo != nullptr)
  {
    smtk::attribute::DefinitionPtr def = topView->m_topLevelConfigurationDef.lock();
    if (attr->definition()->isA(def))
    {
      // if the combobox allows creation of configurations, the number
      // of existing configurations is 1 less.
      int n = topView->Internals->m_configurationCombo->count();
      if (topView->m_topLevelCanCreateConfigurations)
      {
        --n;
      }
      // If there are no configurations in the combobox just prep it
      if (!n)
      {
        topView->prepConfigurationComboBox(attr->name());
      }
      else
      {
        // Lets insert it where it belongs
        bool needToInsert = true;
        for (int i = 0; (i < n) && needToInsert; i++)
        {
          if (topView->Internals->m_configurationCombo->itemText(i).toStdString() > attr->name())
          {
            topView->Internals->m_configurationCombo->insertItem(i, attr->name().c_str());
            needToInsert = false;
          }
        }
        if (needToInsert)
        {
          topView->Internals->m_configurationCombo->insertItem(n, attr->name().c_str());
        }
      }
    }
  }
  //Let observers know the attribute was created and who created it
  signalAttribute(this->uiManager(), attr, "created", std::vector<std::string>(), m_addressString);
}

void qtBaseAttributeView::attributeChanged(
  const smtk::attribute::AttributePtr& attr,
  std::vector<std::string> items)
{
  if (attr == nullptr)
  {
    return;
  }

  qtBaseAttributeView* topView = dynamic_cast<qtBaseAttributeView*>(this->uiManager()->topView());
  // If the toplevel view is using analysis configuration we need
  // to check to see if the attribute is an analysis configuration
  if (topView && topView->Internals->m_configurationCombo != nullptr)
  {
    smtk::attribute::DefinitionPtr def = topView->m_topLevelConfigurationDef.lock();
    if (attr->definition()->isA(def))
    {
      // We only need to refresh the combobox
      topView->prepConfigurationComboBox("");
    }
  }
  //Let observers know the attribute was modified and which view  modified it
  signalAttribute(this->uiManager(), attr, "modified", items, m_addressString);
}

void qtBaseAttributeView::attributeRemoved(const smtk::attribute::AttributePtr& attr)
{
  if (attr == nullptr)
  {
    return;
  }

  qtBaseAttributeView* topView = dynamic_cast<qtBaseAttributeView*>(this->uiManager()->topView());
  // If the toplevel view is using analysis configuration we need
  // to check to see if the attribute is an analysis configuration
  if (topView && topView->Internals->m_configurationCombo != nullptr)
  {
    smtk::attribute::DefinitionPtr def = topView->m_topLevelConfigurationDef.lock();
    if (attr->definition()->isA(def))
    {
      // See if we can find the attribute
      int n = topView->Internals->m_configurationCombo->findText(attr->name().c_str());

      if (n != -1)
      {
        topView->Internals->m_configurationCombo->blockSignals(true);
        int currentIndex = topView->Internals->m_configurationCombo->currentIndex();
        // If the attribute being removed the selected one?  If it is then select the
        // select another configuration first
        if (n == currentIndex)
        {
          // By default, we will select the previous configuration if it esists
          int newSelection = n - 1;
          if (newSelection < 0)
          {
            // Else we will select the first one in the list if one
            // exists.
            int count = topView->Internals->m_configurationCombo->count();
            if (topView->m_topLevelCanCreateConfigurations)
            {
              --count;
            }
            newSelection = (count > 1) ? 0 : -1;
          }
          topView->Internals->m_configurationCombo->removeItem(n);
          topView->Internals->m_configurationCombo->setCurrentIndex(newSelection);
        }
        else
        {
          // This is the case where the attribute is not currently selected.
          topView->Internals->m_configurationCombo->removeItem(n);
        }
        topView->Internals->m_configurationCombo->blockSignals(false);
      }
    }
  }
  // A deleted attribute can effect categories (if it is an active
  // Analysis Configuration) or  other attributes' validity (as in
  // the case of an attribute being referenced by a Reference Item
  // - See SMTK Issue 415)
  // so we need to update the UI.
  if (topView)
  {
    topView->updateUI();
  }
  //Let observers know the attribute was removed and which view removed it
  signalAttribute(this->uiManager(), attr, "expunged", std::vector<std::string>(), m_addressString);
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

  if (m_ScrollArea)
  {
    m_ScrollArea->setWidget(this->Widget);
  }
  else
  {
    parentlayout->addWidget(this->Widget);
  }
}

void qtBaseAttributeView::setInitialCategory()
{
  if (
    this->isTopLevel() && (this->Internals->ShowCategoryCombo != nullptr) &&
    this->Internals->ShowCategoryCombo->isEnabled())
  {
    this->onShowCategory();
  }
}

void qtBaseAttributeView::topLevelPrepAdvanceLevels(const smtk::view::ConfigurationPtr& view)
{
  bool flag;
  // Do we need to provide advance level filtering? - this is on by default
  if ((!view->details().attributeAsBool("FilterByAdvanceLevel", flag)) || flag)
  {
    this->Internals->AdvLevelCombo = new QComboBox(this->parentWidget());
    this->Internals->AdvLevelCombo->setObjectName("advanceLevel");
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
    editButton->setObjectName("editAccessLevel");
    connect(editButton, SIGNAL(toggled(bool)), this, SLOT(showAdvanceLevelOverlay(bool)));
    this->Internals->AdvLevelEditButton = editButton;
  }
}

void qtBaseAttributeView::topLevelPrepCategories(
  const smtk::view::ConfigurationPtr& view,
  const smtk::attribute::ResourcePtr& attResource)
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
    this->Internals->ShowCategoryCombo->setObjectName("categoryCombo");
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
  const smtk::view::ConfigurationPtr& view,
  const smtk::attribute::ResourcePtr& attResource)
{
  bool flag;
  // Do we need to provide category filtering - this is off by default
  if (!(view->details().attributeAsBool("UseConfigurations", flag) && flag))
  {
    return;
  }

  // First lets see if the definition information was set
  std::string configDefType;
  if ((!view->details().attribute("ConfigurationType", configDefType)) || configDefType.empty())
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
    if (
      (!view->details().attribute("ConfigurationTypeLabel", configDefLabel)) ||
      configDefLabel.empty())
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

  smtk::view::ConfigurationPtr view = this->configuration();

  this->Internals->clearWidgets();
  const attribute::ResourcePtr attResource = this->attributeResource();

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

    QObject::connect(
      this->Internals->m_configurationCombo,
      SIGNAL(currentIndexChanged(int)),
      this,
      SLOT(onConfigurationChanged(int)));
  }
  else if (this->Internals->ShowCategoryCombo)
  {
    if (this->Internals->FilterByCategory)
    {
      this->Internals->TopLevelLayout->addWidget(this->Internals->FilterByCategory);
      QObject::connect(
        this->Internals->FilterByCategory,
        SIGNAL(stateChanged(int)),
        this,
        SLOT(enableShowBy(int)));
    }
    else
    {
      this->Internals->TopLevelLayout->addWidget(this->Internals->FilterByCategoryLabel);
    }
    this->Internals->TopLevelLayout->addWidget(this->Internals->ShowCategoryCombo);

    QObject::connect(
      this->Internals->ShowCategoryCombo,
      SIGNAL(currentIndexChanged(int)),
      this,
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
    QObject::connect(
      this->Internals->AdvLevelCombo,
      SIGNAL(currentIndexChanged(int)),
      this,
      SLOT(onAdvanceLevelChanged(int)));
  }
  QVBoxLayout* parentlayout = static_cast<QVBoxLayout*>(this->parentWidget()->layout());
  parentlayout->setAlignment(Qt::AlignTop);
  parentlayout->addLayout(this->Internals->TopLevelLayout);

  if (view->details().attributeAsBool("UseScrollingContainer"))
  {
    m_ScrollArea = new QScrollArea(this->parentWidget());
    m_ScrollArea->setWidgetResizable(true);
    m_ScrollArea->setAlignment(Qt::AlignHCenter);
    m_ScrollArea->setFrameShape(QFrame::NoFrame);
    m_ScrollArea->setObjectName("topLevelScrollArea");
    parentlayout->addWidget(m_ScrollArea);
  }
}

void qtBaseAttributeView::showAdvanceLevel(int level)
{
  // If this is not a toplevel widget don't do anything
  if (!m_isTopLevel)
  {
    return;
  }

  smtk::view::ConfigurationPtr view = this->configuration();
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
  this->updateUI();
}

void qtBaseAttributeView::enableShowBy(int enable)
{
  this->Internals->ShowCategoryCombo->setEnabled(enable != 0);
  this->onShowCategory();
}

std::string qtBaseAttributeView::currentCategory() const
{
  return this->categoryEnabled() ? this->Internals->ShowCategoryCombo->currentText().toStdString()
                                 : "";
}

bool qtBaseAttributeView::categoryEnabled() const
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

int qtBaseAttributeView::advanceLevel() const
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

void qtBaseAttributeView::onConfigurationChanged(int index)
{
  std::set<std::string> cats;
  smtk::attribute::ResourcePtr attRes = this->attributeResource();
  smtk::attribute::AttributePtr att;
  std::string attName;

  if ((this->Internals->m_configurationCombo == nullptr) || (attRes == nullptr))
  {
    return; // there is nothing to do
  }

  if (index == -1)
  {
    // Nothing is selected so lets disable category filtering
    attRes->setActiveCategoriesEnabled(false);
    return;
  }

  // Are we dealing with the ability to create new configurations and did the
  // user pick the create option?
  if (
    m_topLevelCanCreateConfigurations &&
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
    // Tell the Resource we don't want to filter on active categories
    bool origEnableActiveCategories = attRes->activeCategoriesEnabled();
    attRes->setActiveCategoriesEnabled(false);
    auto* editor =
      new smtk::extension::qtAttributeEditorDialog(att, this->uiManager(), this->widget());
    auto status = editor->exec();
    attRes->setActiveCategoriesEnabled(origEnableActiveCategories);
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
  smtk::attribute::ResourcePtr attRes = this->attributeResource();
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

    if (
      (att->properties().get<long>().contains("_selectedConfiguration")) &&
      (att->properties().get<long>()["_selectedConfiguration"] == 1))
    {
      // If the current config name is not set then record this as the
      // current configuration
      if (currentConfig.empty())
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
      //Let observers know the attribute was modified
      signalAttribute(
        this->uiManager(), att, "modified", std::vector<std::string>(), m_addressString);
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
  attRes->setActiveCategories(cats);
  attRes->setActiveCategoriesEnabled(true);
  this->Internals->m_configurationCombo->blockSignals(false);
  this->updateUI();
}
