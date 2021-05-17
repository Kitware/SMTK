//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_polygon_pq_SplitEdgeWidget_h
#define __smtk_polygon_pq_SplitEdgeWidget_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/session/polygon/qt/Exports.h"
#include <QAction>
#include <QWidget>

class pqRenderView;
class pqOutputPort;
class pqRenderViewSelectionReaction;
class QToolButton;

namespace pqSplitEdgeWidgetInternals
{
class EdgePointPicker : public QAction
{
  Q_OBJECT

public:
  EdgePointPicker(QObject* p);
  ~EdgePointPicker() override;

  void doPick(pqRenderView* view);
  void donePicking(pqRenderView* view);

  pqRenderViewSelectionReaction* Selecter;
  QToolButton* InteractiveSelectButton;
  bool m_isActive;

signals:
  //emitted to allow selection to happen
  void triggered(bool);
};
} // namespace pqSplitEdgeWidgetInternals

class SMTKPOLYGONQTEXT_EXPORT pqSplitEdgeWidget : public QWidget
{
  Q_OBJECT

public:
  explicit pqSplitEdgeWidget(QWidget* parent = nullptr);
  ~pqSplitEdgeWidget() override;

  virtual void setView(pqRenderView* view);
  void setEdgeOperation(smtk::operation::OperationPtr edgeOp);
  smtk::shared_ptr<smtk::operation::Operation> edgeOperation();
  bool isActive();

signals:
  void operationRequested(const smtk::operation::OperationPtr& brOp);
  /// update face visbility before picking points
  /// hide all faces when picking then restore the visibility after picking
  void hideAllFaces(bool status);

public slots:
  void resetWidget();
  void onSelectionModeChanged();

protected slots:
  void arcPointPicked(pqOutputPort*);
  void splitEdgeOperation(bool start);

private:
  class pqInternals;
  pqInternals* Internals;

  pqSplitEdgeWidgetInternals::EdgePointPicker* m_edgePointPicker;

  pqRenderView* View;
  smtk::weak_ptr<smtk::operation::Operation> m_edgeOp;
};

#endif // __smtk_polygon_pq_SplitEdgeWidget_h
