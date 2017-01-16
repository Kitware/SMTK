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
#include "ui_qtTimeZoneSelectWidget.h"
#include "qtTimeZoneRegionModel.h"

#include <QDebug>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QSortFilterProxyModel>
#include <QtGlobal>
#include <QVariant>

namespace {

// Use internal proxy model to enable sorting and override header text
// for region view
// -----------------------------------------------------------------------------
  class TimeZoneRegionProxyModel : public QSortFilterProxyModel
  {
  public:
    TimeZoneRegionProxyModel(QObject *parent = NULL)
      : QSortFilterProxyModel(parent) {}
    virtual QVariant headerData(
      int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  };

  QVariant TimeZoneRegionProxyModel::headerData(
    int section, Qt::Orientation orientation, int role) const
  {
    const char *regionHeaders[] = {"Region", "Offset", "Abbrev."};
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
      {
      if (section < sizeof(regionHeaders)/sizeof(const char *))
        {
        return regionHeaders[section];
        }
      }
    // (else)
    return QSortFilterProxyModel::headerData(section, orientation, role);
  }


}  // namespace

namespace smtk {
  namespace extension {

//-----------------------------------------------------------------------------
class qtTimeZoneSelectWidget::qtTimeZoneSelectWidgetInternal
{
 public:
  TimeZoneRegionProxyModel *RegionProxyModel;
};

//-----------------------------------------------------------------------------
qtTimeZoneSelectWidget::qtTimeZoneSelectWidget(QWidget* parent)
  : QWidget(parent)
{
  this->UI = new Ui_qtTimeZoneSelectWidget;
  this->UI->setupUi(this);

  qtTimeZoneRegionModel *model = new qtTimeZoneRegionModel(this);
  model->initialize();

  this->Internal = new qtTimeZoneSelectWidgetInternal;
  this->Internal->RegionProxyModel = new TimeZoneRegionProxyModel(this);
  this->Internal->RegionProxyModel->setSourceModel(model);

  this->UI->ContinentView->setModel(model);
  QModelIndex rootIndex = model->index(0, 0);
  this->UI->ContinentView->setRootIndex(rootIndex);

  QItemSelectionModel *selectionModel = this->UI->ContinentView->selectionModel();
  QObject::connect(
    selectionModel,
    SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
    this,
    SLOT(onContinentChanged(const QItemSelection&, const QItemSelection&)));

  // Set all stretch to RegionView. and other cosmetic stuff
  this->UI->Splitter->setStretchFactor(0, 0);
  this->UI->Splitter->setStretchFactor(1, 1);
  this->UI->RegionView->horizontalHeader()->setStretchLastSection(true);
  this->UI->RegionView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
}

//-----------------------------------------------------------------------------
qtTimeZoneSelectWidget::~qtTimeZoneSelectWidget()
{
  delete this->Internal;
}

// -----------------------------------------------------------------------------
void qtTimeZoneSelectWidget::setContinent(const QModelIndex index)
{
  // Initialize regoin-proxy model if needed
  if (!this->UI->RegionView->model())
    {
    this->UI->RegionView->setModel(this->Internal->RegionProxyModel);
    QItemSelectionModel *selectionModel = this->UI->RegionView->selectionModel();
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
  QModelIndex regionRootIndex = this->Internal->RegionProxyModel->index(
    row, 0, proxyRootIndex);
  this->UI->RegionView->setRootIndex(regionRootIndex);
}

// -----------------------------------------------------------------------------
void qtTimeZoneSelectWidget::onContinentChanged(
  const QItemSelection& selected, const QItemSelection& deselected)
{
  if (selected.size() != 1)
    {
    qWarning() << "Unexpected selection size:" << selected.size();
    return;
    }

  QModelIndex index = selected[0].topLeft();
  this->setContinent(index);
}

// -----------------------------------------------------------------------------
void qtTimeZoneSelectWidget::onRegionChanged(
  const QItemSelection& selected, const QItemSelection& deselected)
{
  if (selected.size() != 1)
    {
    qWarning() << "Unexpected selection size:" << selected.size();
    return;
    }

  QModelIndex index = selected[0].topLeft();
  qDebug() << "Selected region" << index.data();
}

// -----------------------------------------------------------------------------

  }  // namespace extension
}  // namespace smtk
