//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_qt_ArcWidget_h
#define smtk_qt_ArcWidget_h

/// qtArcWidget is a qtInteractionWidget subclass that uses a
/// **smtkArcWidgetRepresentation** for enabling users to interactively add
/// points in an arc.
///
/// Developer notes: most this class is simply a modified version of
/// pqArcWidget. SMTK developers should look into cleaning up the API as needed.

#include "smtk/session/polygon/qt/qtInteractionWidget.h"
#include <QScopedPointer>

class vtkSMProxy;

class qtArcWidget : public qtInteractionWidget
{
  Q_OBJECT;
  typedef qtInteractionWidget Superclass;

public:
  qtArcWidget(QWidget* parent = nullptr);
  ~qtArcWidget() override;

  /// Changes the line color to magenta.
  void emphasize() { this->setLineColor(QColor::fromRgbF(1.0, 0.0, 1.0)); }
  void deemphasize() { this->setLineColor(QColor::fromRgbF(1.0, 1.0, 1.0)); }

  /// Set the line color
  virtual void setLineColor(const QColor& color);

  /// Update the UI to be in the Arc Editing mode.
  /// In this mode, for whole arc, the Visibility, Closed, Delete, buttonRectArc
  /// are all invisible; and for sub-arc, we only allow Modify for now.
  virtual void useArcEditingUI(bool isWholeArc);

  /// Returns the point placer proxy.
  vtkSMProxy* pointPlacer() const;
Q_SIGNALS:
  /// Signal emitted when the representation proxy's "ClosedLoop" property
  /// is modified.
  void contourLoopClosed();
  void contourDone();

public Q_SLOTS:
  void removeAllNodes();
  void checkContourLoopClosed();

  /// Close the contour loop
  void closeLoop(bool);

  /// Move to modify mode
  void ModifyMode();

  /// Check if the loop can even go to edit mode
  void checkCanBeEdited();

  /// Move to the next mode ( Drawing, Editing, Done )
  void updateMode();

  /// Finish editing the contour
  void finishContour();

  /// resets the widget.
  void reset();

  // enable/disable the apply button of the widget
  void enableApplyButton(bool);

protected Q_SLOTS:
  void deleteAllNodes();

private:
  Q_DISABLE_COPY(qtArcWidget);

  class qtInternals;
  QScopedPointer<qtInternals> Internals;
};

#endif
