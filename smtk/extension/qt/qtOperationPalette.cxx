//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtOperationPalette.h"

#include "smtk/extension/qt/qtOperationTypeModel.h"
#include "smtk/extension/qt/qtOperationTypeView.h"

#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/operators/Signal.h"

#include "smtk/operation/Manager.h"

#include "smtk/common/Managers.h"

#include "smtk/io/Logger.h"

#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/view/Configuration.h"

#include <QApplication>
#include <QCheckBox>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QWidget>

using namespace smtk::extension;

qtOperationPalette::qtOperationPalette(const smtk::view::Information& info)
  : qtBaseView(info)
  , m_subset(new QSortFilterProxyModel(this))
  , m_filter(new QSortFilterProxyModel(this))
{
  if (info.contains<std::shared_ptr<smtk::common::Managers>>())
  {
    const auto& managers = info.get<std::shared_ptr<smtk::common::Managers>>();
    if (managers->contains<QSharedPointer<qtOperationTypeModel>>())
    {
      m_model = managers->get<QSharedPointer<qtOperationTypeModel>>().get();
    }
  }
  if (!m_model)
  {
    m_model = new qtOperationTypeModel(info, this);
  }
  std::string subsetFilterRegex("[4-9]");
  std::string subsetSortColumn("TypeName");
  std::string filterSortColumn("Associability");
  const auto& config = this->configuration();
  if (config)
  {
    config->details().attribute("SubsetAssociability", subsetFilterRegex);
    config->details().attribute("SubsetSort", subsetSortColumn);
    config->details().attribute("FilterSort", filterSortColumn);
  }
  auto subsetSort = static_cast<int>(qtOperationTypeModel::columnEnumFromName(subsetSortColumn));
  auto filterSort = static_cast<int>(qtOperationTypeModel::columnEnumFromName(filterSortColumn));
  m_subset->setSourceModel(m_model);
  m_subset->sort(subsetSort);
  m_subset->setFilterKeyColumn(static_cast<int>(qtOperationTypeModel::Column::Associability));
  m_subset->setFilterRegularExpression(QString::fromStdString(subsetFilterRegex));

  m_filter->setSourceModel(m_subset);
  m_filter->sort(filterSort, Qt::DescendingOrder);
  m_filter->setFilterCaseSensitivity(Qt::CaseInsensitive);
  m_filter->setFilterKeyColumn(static_cast<int>(qtOperationTypeModel::Column::Label));
  m_filter->setSortCaseSensitivity(Qt::CaseInsensitive);
}

qtOperationPalette::~qtOperationPalette() = default;

bool qtOperationPalette::isEmpty() const
{
  // If the user is (1) not searching and (2) not limiting by selection and
  // (3) there are no operations available, then view is empty.
  return (!m_search || m_search->displayText().isEmpty()) && m_filter->rowCount(QModelIndex()) == 0;
}

bool qtOperationPalette::validateInformation(const smtk::view::Information& info)
{
  return Superclass::validateInformation(info) && info.contains<smtk::operation::Manager::Ptr>() &&
    info.contains<smtk::view::Manager::Ptr>();
}

QSortFilterProxyModel* qtOperationPalette::model() const
{
  return m_filter;
}

qtOperationTypeModel* qtOperationPalette::operationModel() const
{
  return m_model;
}

qtOperationTypeView* qtOperationPalette::operationView() const
{
  return m_list;
}

QLineEdit* qtOperationPalette::searchTextWidget() const
{
  return m_search;
}

void qtOperationPalette::onInfo()
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

void qtOperationPalette::editTopOperation()
{
  bool ok = false;
  auto operationId = m_filter->index(0, static_cast<int>(qtOperationTypeModel::Column::TypeIndex))
                       .data()
                       .toULongLong(&ok);
  if (ok)
  {
    m_model->editOperationParameters(operationId);
  }
}

void qtOperationPalette::toggleFiltering(int filterState)
{
  if (!m_decorator)
  {
    // Remember the decorator if it is present so we can reset it
    // when the checkbox is toggled again.
    m_decorator = m_model->decorator();
  }
  switch (filterState)
  {
    case Qt::Checked:
      m_subset->setFilterRegularExpression("");
      m_model->setDecorator(nullptr);
      break;
    default:
    case Qt::Unchecked:
    {
      std::string subsetFilterRegex("[4-9]");
      const auto& config = this->configuration();
      if (config)
      {
        config->details().attribute("SubsetAssociability", subsetFilterRegex);
      }
      m_subset->setFilterRegularExpression(QString::fromStdString(subsetFilterRegex));
      m_model->setDecorator(m_decorator);
    }
    break;
  }
}

void qtOperationPalette::buildUI()
{
  this->createWidget();

  if (!(this->Widget && this->parentWidget()))
  {
    // Should say some kind of error
    return;
  }

  QVBoxLayout* parentLayout = static_cast<QVBoxLayout*>(this->parentWidget()->layout());

  if (!parentLayout)
  {
    // Should say some kind of error or maybe create one?
    return;
  }
  parentLayout->addWidget(this->Widget);
}

void qtOperationPalette::createWidget()
{
  // Remove any pre-existing widget from parent.
  QVBoxLayout* parentLayout = static_cast<QVBoxLayout*>(this->parentWidget()->layout());
  if (this->Widget)
  {
    if (parentLayout)
    {
      parentLayout->removeWidget(this->Widget);
    }
    delete this->Widget;
  }

  this->Widget = new QWidget;
  this->Widget->setObjectName("OperationPalette");
  m_list = new qtOperationTypeView;
  m_list->setFocusPolicy(Qt::NoFocus); // Children accept focus, but not frame itself.
  m_list->setObjectName("Operations");
  m_list->setModel(m_filter);
  m_list->setFrameShape(QFrame::NoFrame);
  m_list->setFrameShadow(QFrame::Plain);
  m_layout = new QVBoxLayout;
  m_layout->setObjectName("Layout");
  this->Widget->setLayout(m_layout);
  auto* controlsLayout = new QHBoxLayout;
  controlsLayout->setObjectName("Controls");
  bool haveControls = false;
  const auto& conf = m_viewInfo.configuration()->details();
  if (conf.attributeAsBool("SearchBar"))
  {
    m_search = new QLineEdit();
    m_search->setObjectName("Search");
    m_search->setPlaceholderText("Search");
    m_search->setToolTip("Search for an operation by its name.");
    controlsLayout->addWidget(m_search);
    haveControls = true;
    QObject::connect(
      m_search, &QLineEdit::textChanged, m_filter, &QSortFilterProxyModel::setFilterWildcard);
    QObject::connect(
      m_search, &QLineEdit::returnPressed, this, &qtOperationPalette::editTopOperation);
  }
  if (!conf.attributeAsBool("AlwaysLimit"))
  {
    auto* alwaysLimit = new QCheckBox();
    alwaysLimit->setObjectName("AlwaysLimit");
    alwaysLimit->setText("All");
    alwaysLimit->setToolTip("Click to show all available operations.");
    alwaysLimit->setCheckState(Qt::Unchecked);
    controlsLayout->addWidget(alwaysLimit);
    haveControls = true;
    m_decorator = m_model->decorator();
    QObject::connect(
      alwaysLimit, &QCheckBox::stateChanged, this, &qtOperationPalette::toggleFiltering);
  }
  if (haveControls)
  {
    m_layout->addLayout(controlsLayout);
  }
  else
  {
    delete controlsLayout;
  }
  m_layout->addWidget(m_list);
}

void qtOperationPalette::setInfoToBeDisplayed()
{
  m_infoDialog->displayInfo(this->configuration());
}
