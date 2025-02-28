//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtTimeZoneSelectWidget.h"
#include "qtTimeZoneRegionModel.h"
#include "ui_qtTimeZoneSelectWidget.h"

#include <QDebug>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QSortFilterProxyModel>
#include <QVariant>
#include <QtGlobal>

namespace
{
// Use internal proxy model to enable sorting and override header text
// for region view
class TimeZoneRegionProxyModel : public QSortFilterProxyModel
{
public:
  TimeZoneRegionProxyModel(QObject* parent = nullptr)
    : QSortFilterProxyModel(parent)
  {
  }
  [[nodiscard]] QVariant
  headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
};

QVariant TimeZoneRegionProxyModel::headerData(int section, Qt::Orientation orientation, int role)
  const
{
  const char* regionHeaders[] = { "TimeZone", "Offset/DST", "Abbrev." };
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
  {
    std::size_t numRegionHeaders = sizeof(regionHeaders) / sizeof(const char*);
    if (section < static_cast<int>(numRegionHeaders))
    {
      return regionHeaders[section];
    }
  }
  // (else)
  return QSortFilterProxyModel::headerData(section, orientation, role);
}
} // namespace

namespace smtk
{
namespace extension
{

class qtTimeZoneSelectWidget::qtTimeZoneSelectWidgetInternal
{
public:
  qtTimeZoneRegionModel* TimeZoneRegionModel;
  TimeZoneRegionProxyModel* RegionProxyModel;
};

qtTimeZoneSelectWidget::qtTimeZoneSelectWidget(QWidget* parent)
  : QWidget(parent)
{
  this->UI = new Ui_qtTimeZoneSelectWidget;
  this->UI->setupUi(this);
  this->Internal = new qtTimeZoneSelectWidgetInternal;

  this->Internal->TimeZoneRegionModel = new qtTimeZoneRegionModel(this);
  this->Internal->TimeZoneRegionModel->initialize();

  this->UI->ContinentView->setModel(this->Internal->TimeZoneRegionModel);
  QModelIndex rootIndex = this->Internal->TimeZoneRegionModel->index(0, 0);
  this->UI->ContinentView->setRootIndex(rootIndex);

  QItemSelectionModel* selectionModel = this->UI->ContinentView->selectionModel();
  QObject::connect(
    selectionModel,
    SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
    this,
    SLOT(onContinentChanged(const QItemSelection&, const QItemSelection&)));

  // Set all stretch to RegionView. and other cosmetic stuff
  this->UI->Splitter->setStretchFactor(0, 0);
  this->UI->Splitter->setStretchFactor(1, 1);
  this->UI->RegionView->horizontalHeader()->setStretchLastSection(true);
  this->UI->RegionView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

qtTimeZoneSelectWidget::~qtTimeZoneSelectWidget()
{
  delete this->Internal;
}

void qtTimeZoneSelectWidget::setRegion(const QString& region)
{
  if (region.isEmpty())
  {
    // Empty string means unselect
    this->UI->ContinentView->selectionModel()->clear();
    this->UI->RegionView->selectionModel()->clear();
    return;
  }

  // Update ContinentView
  QModelIndex index = this->Internal->TimeZoneRegionModel->findModelIndex(region);
  this->UI->ContinentView->selectionModel()->select(index.parent(), QItemSelectionModel::Select);
  this->UI->ContinentView->scrollTo(index.parent());

  // Update RegionView (which uses proxy model)
  QModelIndex proxyIndexLeft = this->Internal->RegionProxyModel->mapFromSource(index);
  // Select the full row for region
  int row = proxyIndexLeft.row();
  int column = this->Internal->TimeZoneRegionModel->columnCount(index.parent()) - 1;
  QModelIndex proxyIndexRight = proxyIndexLeft.sibling(row, column);
  QItemSelection proxyRange(proxyIndexLeft, proxyIndexRight);
  this->UI->RegionView->selectionModel()->select(proxyRange, QItemSelectionModel::Select);
  this->UI->RegionView->scrollTo(proxyIndexLeft, QAbstractItemView::PositionAtCenter);
  // Call scrollTo() twice to force repaint, see:
  // http://www.qtcentre.org/threads/60873-QTableView-scrollTo()-not-work
  // https://forum.qt.io/topic/27337/unlogical-running-of-scrollto-in-qtableview
  this->UI->RegionView->scrollTo(proxyIndexLeft, QAbstractItemView::PositionAtCenter);
}

QString qtTimeZoneSelectWidget::selectedRegion() const
{
  QString selected;

  QItemSelectionModel* selectionModel = this->UI->RegionView->selectionModel();
  QModelIndexList selectedRows = selectionModel->selectedRows();
  if (selectedRows.size() == 1)
  {
    QModelIndex proxyIndex = selectedRows[0];
    QModelIndex sourceIndex = this->Internal->RegionProxyModel->mapToSource(proxyIndex);
    selected = this->Internal->TimeZoneRegionModel->regionId(sourceIndex);
  }

  return selected;
}

void qtTimeZoneSelectWidget::onContinentChanged(
  const QItemSelection& selected,
  const QItemSelection& deselected)
{
  (void)deselected;

  //qDebug() << "onContinentChanged";
  if (selected.empty())
  {
    return; // deselected case
  }

  if (selected.size() > 1)
  {
    qWarning() << "Unexpected selection size:" << selected.size();
    return;
  }

  // First unselect current region
  if (this->UI->RegionView->selectionModel())
  {
    this->UI->RegionView->selectionModel()->clear();
    //this->UI->RegionView->scrollToTop();
  }

  // Then select the continent
  QModelIndex index = selected[0].topLeft();
  this->setContinent(index);
}

void qtTimeZoneSelectWidget::onRegionChanged(
  const QItemSelection& selected,
  const QItemSelection& deselected)
{
  (void)deselected;

  if (selected.size() != 1)
  {
    qWarning() << "Unexpected selection size:" << selected.size();
    return;
  }

  QModelIndex index = selected[0].topLeft();
  qDebug() << "Selected region" << index.data();
  Q_EMIT this->regionSelected(index.data().toString());
}

void qtTimeZoneSelectWidget::setContinent(const QModelIndex index)
{
  qDebug() << "setContinent";
  // Initialize regoin-proxy model if needed
  if (!this->UI->RegionView->model())
  {
    this->Internal->RegionProxyModel = new TimeZoneRegionProxyModel(this);
    this->Internal->RegionProxyModel->setSourceModel(

      this->Internal->TimeZoneRegionModel);
    this->UI->RegionView->setModel(this->Internal->RegionProxyModel);
    QItemSelectionModel* selectionModel = this->UI->RegionView->selectionModel();
    QObject::connect(
      selectionModel,
      SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
      this,
      SLOT(onRegionChanged(const QItemSelection&, const QItemSelection&)));
  }

  // Update region view
  QModelIndex proxyIndex = this->Internal->RegionProxyModel->mapFromSource(index);
  int row = proxyIndex.row();
  QModelIndex proxyRootIndex = this->Internal->RegionProxyModel->index(0, 0);
  QModelIndex regionRootIndex = this->Internal->RegionProxyModel->index(row, 0, proxyRootIndex);
  this->UI->RegionView->setRootIndex(regionRootIndex);
}

} // namespace extension
} // namespace smtk
