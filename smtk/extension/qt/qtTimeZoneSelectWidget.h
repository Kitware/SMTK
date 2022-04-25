//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtTimeZoneSelectWidget - ui panel for selecting timezone
// .SECTION Description
// .SECTION Caveats

#ifndef smtk_extension_qtTimeZoneSelectWidget_h
#define smtk_extension_qtTimeZoneSelectWidget_h

#include "smtk/extension/qt/Exports.h"
#include <QItemSelection>
#include <QModelIndex>
#include <QString>
#include <QWidget>

class Ui_qtTimeZoneSelectWidget;

namespace smtk
{
namespace extension
{

class SMTKQTEXT_EXPORT qtTimeZoneSelectWidget : public QWidget
{
  Q_OBJECT

public:
  qtTimeZoneSelectWidget(QWidget* parent = nullptr);
  ~qtTimeZoneSelectWidget() override;

  // Used to initialize model
  void setRegion(const QString& index);

  // Returns continent/region or empty string
  QString selectedRegion() const;

public Q_SLOTS:

Q_SIGNALS:
  void regionSelected(QString id);

protected Q_SLOTS:
  void onContinentChanged(const QItemSelection& selected, const QItemSelection& deselected);
  void onRegionChanged(const QItemSelection& selected, const QItemSelection& deselected);

protected:
  Ui_qtTimeZoneSelectWidget* UI;

  void setContinent(QModelIndex index);

private:
  class qtTimeZoneSelectWidgetInternal;
  qtTimeZoneSelectWidgetInternal* Internal;
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtTimeZoneSelectWidget_h
