//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/diagram/qtDiagramLegend.h"

#include "smtk/extension/qt/diagram/qtDiagram.h"
#include "smtk/extension/qt/diagram/qtDiagramLegendEntry.h"
#include "smtk/extension/qt/diagram/qtLegendDelegate.h"

#include <QHeaderView>
#include <QMouseEvent>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTableView>

namespace smtk
{
namespace extension
{

qtDiagramLegend::qtDiagramLegend(const QString& title, qtDiagram* parent)
  : Superclass(title, nullptr)
  , m_diagram(parent)
  , m_keys(new QStandardItemModel(0, 3, this))
  , m_sortModel(new QSortFilterProxyModel(this))
  , m_view(new QTableView(this))
{
  this->setObjectName("Legend");
  this->setFlat(true);
  m_view->setObjectName("LegendView");
  m_keys->setObjectName("LegendModel");
  m_keys->setHeaderData(Column::Group, Qt::Horizontal, tr("type"));
  m_keys->setHeaderData(Column::Symbol, Qt::Horizontal, tr("symbol"));
  m_keys->setHeaderData(Column::Description, Qt::Horizontal, tr("description"));
  m_sortModel->setSourceModel(m_keys);
  m_view->setModel(m_sortModel);
  m_view->horizontalHeader()->setStretchLastSection(true);
  m_view->horizontalHeader()->setVisible(false);
  m_view->verticalHeader()->setVisible(false);
  m_view->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
  m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_view->setShowGrid(false);
  // Hide the group column; it is the same for many rows and thus visual clutter:
  m_view->setColumnHidden(Column::Group, true);

  m_view->setSortingEnabled(true);
  // Word wrap legend entries.
  // Just setting wordWrap to true (which is the default, anyway) does *not*
  // actually cause row heights to change. Instead, you must
  // 1. Force off text elision.
  // 2. When the user changes column widths, then the view must resize row heights.
  // 3. When the model changes (rows inserted or descriptions changed), then
  //    the view must be told to resize the rows.
  m_view->setWordWrap(true);
  m_view->setTextElideMode(Qt::ElideNone);
  QObject::connect(
    m_view->horizontalHeader(),
    &QHeaderView::sectionResized,
    this,
    &qtDiagramLegend::legendUpdated);
  QObject::connect(
    m_sortModel, &QAbstractItemModel::dataChanged, this, &qtDiagramLegend::legendUpdated);
  QObject::connect(
    m_sortModel, &QAbstractItemModel::rowsInserted, this, &qtDiagramLegend::legendUpdated);

  // If we set the delegate for the entire table, then our
  // m_view->setItemDelegate(new qtLegendDelegate(this));
  m_symbolDelegate = new qtLegendDelegate(this);
  m_view->setItemDelegateForColumn(Column::Symbol, m_symbolDelegate);

  QSizePolicy policy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
  policy.setVerticalStretch(1);
  m_view->setSizePolicy(policy);
  this->setLayout(new QVBoxLayout);
  this->layout()->setObjectName("LegendLayout");
  this->layout()->addWidget(m_view);
  this->layout()->setMargin(0);
}

qtDiagramLegend::~qtDiagramLegend() = default;

const qtDiagramLegend::EntriesByType& qtDiagramLegend::typesInGroup(smtk::string::Token group) const
{
  auto git = m_index.find(group);
  if (git == m_index.end())
  {
    static EntriesByType dummy;
    return dummy;
  }
  return git->second;
}

bool qtDiagramLegend::addEntry(qtDiagramLegendEntry* entry)
{
  if (!entry)
  {
    return false;
  }

  // Check whether the entry already exists.
  auto git = m_index.find(entry->group());
  if (git != m_index.end())
  {
    auto nit = git->second.find(entry->type());
    if (nit != git->second.end())
    {
      return false;
    }
  }

  QVariant entryRef(QMetaType::QObjectStar, &entry);
  QList<QStandardItem*> rowValues;
  // Insert a value for Column::Group, Column::Symbol, and Column::Description:
  rowValues.push_back(new QStandardItem);
  rowValues[Column::Group]->setData(entryRef);

  rowValues.push_back(new QStandardItem);
  rowValues[Column::Symbol]->setData(entryRef);

  rowValues.push_back(new QStandardItem(QString::fromStdString(entry->label().data())));
  rowValues[Column::Description]->setEditable(false); // Do not let users rename arc types (yet).

  m_keys->appendRow(rowValues);
  m_index[entry->group()][entry->type()] = entry;
  return true;
}

bool qtDiagramLegend::removeEntry(qtDiagramLegendEntry* entry)
{
  if (!entry)
  {
    return false;
  }

  // Check whether the entry already exists.
  bool found = false;
  int rr;
  for (rr = 0; rr < m_keys->rowCount(); ++rr)
  {
    if (auto* qo = qvariant_cast<QObject*>(m_keys->item(rr, 0)->data()))
    {
      if (qo == entry)
      {
        // Entry already exists.
        // Erase the row.
        this->eraseIndex(entry);
        m_keys->removeRows(rr, 1);
        found = true;
        // Back up one row (since we just deleted rr)
        --rr;
      }
      else if (auto* re = qobject_cast<qtDiagramLegendEntry*>(qo))
      {
        if (*re == *entry)
        {
          // A matching entry (not the same object) already exists.
          // Erase the row.
          this->eraseIndex(entry);
          m_keys->removeRows(rr, 1);
          found = true;
          // Back up one row (since we just deleted rr)
          --rr;
        }
      }
    }
  }
  return found;
}

qtDiagram* qtDiagramLegend::diagram() const
{
  return m_diagram;
}

void qtDiagramLegend::legendUpdated()
{
  m_view->resizeRowsToContents();
  m_view->sortByColumn(Column::Description, Qt::AscendingOrder);
  m_view->sortByColumn(Column::Group, Qt::AscendingOrder);
}

void qtDiagramLegend::eraseIndex(qtDiagramLegendEntry* entry)
{
  auto git = m_index.find(entry->group());
  if (git != m_index.end())
  {
    auto nit = git->second.find(entry->type());
    if (nit != git->second.end())
    {
      git->second.erase(nit);
      if (git->second.empty())
      {
        m_index.erase(git);
      }
    }
  }
}

} // namespace extension
} // namespace smtk
