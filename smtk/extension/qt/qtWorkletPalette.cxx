//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtWorkletPalette.h"

#include "smtk/extension/qt/qtWorkletModel.h"

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
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QWidget>

using namespace smtk::extension;

qtWorkletPalette::qtWorkletPalette(const smtk::view::Information& info)
  : qtBaseView(info)
  , m_filter(new QSortFilterProxyModel(this))
{
  if (info.contains<std::shared_ptr<smtk::common::Managers>>())
  {
    const auto& managers = info.get<std::shared_ptr<smtk::common::Managers>>();
    if (managers->contains<QSharedPointer<qtWorkletModel>>())
    {
      m_model = managers->get<QSharedPointer<qtWorkletModel>>().get();
    }
  }
  if (!m_model)
  {
    m_model = new qtWorkletModel(info, this);
  }
  std::string filterSortColumn("Label");
  const auto& config = this->configuration();
  if (config)
  {
    config->details().attribute("FilterSort", filterSortColumn);
  }
  auto filterSort = static_cast<int>(qtWorkletModel::columnEnumFromName(filterSortColumn));

  m_filter->setSourceModel(m_model);
  m_filter->sort(filterSort, Qt::AscendingOrder);
  m_filter->setFilterCaseSensitivity(Qt::CaseInsensitive);
  m_filter->setFilterKeyColumn(static_cast<int>(qtWorkletModel::Column::Label));
  m_filter->setSortCaseSensitivity(Qt::CaseInsensitive);
}

qtWorkletPalette::~qtWorkletPalette() = default;

bool qtWorkletPalette::isEmpty() const
{
  // If the user is (1) not searching and (2) not limiting by selection and
  // (3) there are no worklets available, then view is empty.
  return (!m_search || m_search->displayText().isEmpty()) && m_filter->rowCount(QModelIndex()) == 0;
}

bool qtWorkletPalette::validateInformation(const smtk::view::Information& info)
{
  return Superclass::validateInformation(info) && info.contains<smtk::operation::Manager::Ptr>() &&
    info.contains<smtk::view::Manager::Ptr>();
}

QSortFilterProxyModel* qtWorkletPalette::model() const
{
  return m_filter;
}

qtWorkletModel* qtWorkletPalette::workletModel() const
{
  return m_model;
}

QTableView* qtWorkletPalette::workletView() const
{
  return m_list;
}

QLineEdit* qtWorkletPalette::searchTextWidget() const
{
  return m_search;
}

void qtWorkletPalette::onInfo()
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

void qtWorkletPalette::instantiateTopWorklet()
{
  auto worklet =
    m_filter->index(0, static_cast<int>(qtWorkletModel::Column::Label)).data().toString();
  if (!worklet.isEmpty())
  {
    std::array<double, 2> location{ { std::numeric_limits<double>::quiet_NaN(),
                                      std::numeric_limits<double>::quiet_NaN() } };
    Q_EMIT emplaceWorklet(worklet.toStdString(), location);
  }
}

void qtWorkletPalette::toggleFiltering(int filterState)
{
  (void)filterState;
  // switch (filterState) { case Qt::Checked: case Qt::Unchecked: default: break; }
}

void qtWorkletPalette::buildUI()
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

void qtWorkletPalette::createWidget()
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
  this->Widget->setObjectName("WorkletPalette");
  m_list = new QTableView;
  m_list->horizontalHeader()->setVisible(true);
  m_list->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
  m_list->horizontalHeader()->setStretchLastSection(true);
  m_list->setSortingEnabled(true);
  // m_list->setFocusPolicy(Qt::NoFocus); // Children accept focus, but not frame itself.
  m_list->setObjectName("Worklets");
  m_list->setModel(m_filter);
  m_list->setFrameShape(QFrame::NoFrame);
  m_list->setFrameShadow(QFrame::Plain);
  m_list->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
  m_list->setAlternatingRowColors(true);

  // Worklet descriptions may be long, but the table prevents us from reading them.
  // Just setting wordWrap to true (which is the default, anyway) does *not*
  // actually cause row heights to change. Instead, you must
  // 1. Force off text elision.
  // 2. When the user changes column widths, then the view must resize row heights.
  // 3. When the model changes (rows inserted or descriptions changed), then
  //    the view must be told to resize the rows.
  m_list->setWordWrap(true);
  m_list->setTextElideMode(Qt::ElideNone);
  QObject::connect(
    m_list->horizontalHeader(),
    &QHeaderView::sectionResized,
    m_list,
    &QTableView::resizeRowsToContents);
  QObject::connect(
    m_filter, &QAbstractItemModel::dataChanged, m_list, &QTableView::resizeRowsToContents);
  QObject::connect(
    m_filter, &QAbstractItemModel::rowsInserted, m_list, &QTableView::resizeRowsToContents);

  m_list->setDragEnabled(true);
  m_list->setDragDropMode(QAbstractItemView::DragOnly);
  m_list->hideColumn(static_cast<int>(qtWorkletModel::Column::WorkletOp));
  // m_list->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

  m_layout = new QVBoxLayout;
  m_layout->setObjectName("Layout");
  m_layout->setMargin(0);
  this->Widget->setLayout(m_layout);
  const auto& conf = m_viewInfo.configuration()->details();
  if (conf.attributeAsBool("ShowTitle"))
  {
    std::string title("Worklets");
    conf.attribute("Title", title);
    auto* titlebar = new QLabel(title.c_str());
    m_layout->addWidget(titlebar);
  }
  auto* controlsLayout = new QHBoxLayout;
  controlsLayout->setObjectName("Controls");
  bool haveControls = false;
  if (conf.attributeAsBool("SearchBar"))
  {
    m_search = new QLineEdit();
    m_search->setObjectName("Search");
    m_search->setPlaceholderText("Search");
    m_search->setToolTip("Search for a worklet by its name.");
    controlsLayout->addWidget(m_search);
    haveControls = true;
    QObject::connect(
      m_search, &QLineEdit::textChanged, m_filter, &QSortFilterProxyModel::setFilterWildcard);
    QObject::connect(
      m_search, &QLineEdit::returnPressed, this, &qtWorkletPalette::instantiateTopWorklet);
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
  if (parentLayout)
  {
    parentLayout->addWidget(this->Widget);
  }
  else if (auto* pw = this->parentWidget())
  {
    this->Widget->setParent(pw);
  }
}

void qtWorkletPalette::setInfoToBeDisplayed()
{
  m_infoDialog->displayInfo(this->configuration());
}
