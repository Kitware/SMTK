//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_qtLineWidget_h
#define __smtk_qtLineWidget_h

#include "smtk/extension/paraview/widgets/qtInteractionWidget.h"
#include <QScopedPointer>

/// qtLineWidget is a qtInteractionWidget subclass that uses
/// **LineWidgetRepresentation** for enabling suers to interactively set the end
/// points for a line.
class SMTKPQWIDGETSEXT_EXPORT qtLineWidget : public qtInteractionWidget
{
  Q_OBJECT;
  typedef qtInteractionWidget Superclass;

public:
  qtLineWidget(QWidget* parent = nullptr);
  virtual ~qtLineWidget();

  /// Changes the line color to magenta.
  void emphasize() { this->setLineColor(QColor::fromRgbF(1.0, 0.0, 1.0)); }
  void deemphasize() { this->setLineColor(QColor::fromRgbF(1.0, 1.0, 1.0)); }

  /// Set the line color
  void setLineColor(const QColor& color);
  void setLineColor(const double rgb[3])
  {
    this->setLineColor(QColor::fromRgbF(rgb[0], rgb[1], rgb[2]));
  }
  QColor color() const;

  /// set the point locations.
  void setPoints(const double p1[3], const double p2[3]);

  /// get the point locations.
  void points(double p1[3], double p2[3]) const;

public slots:
  /// Make the line aligned with x-axis.
  void xAxis();

  /// Make the line aligned with y-axis.
  void yAxis();

  /// Make the line aligned with z-axis.
  void zAxis();

private:
  Q_DISABLE_COPY(qtLineWidget);

  class qtInternals;
  QScopedPointer<qtInternals> Internals;
};

#endif
