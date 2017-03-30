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
#include "smtk/bridge/polygon/qt/Exports.h"
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
  virtual ~EdgePointPicker();

  void doPick(pqRenderView* view);
  void donePicking(pqRenderView* view);

  pqRenderViewSelectionReaction* Selecter;
  QToolButton* InteractiveSelectButton;
  bool m_isActive;

signals:
  //emitted to allow selection to happen
  void triggered(bool);
};
}

class SMTKPOLYGONQTEXT_EXPORT pqSplitEdgeWidget : public QWidget
{
  Q_OBJECT

public:
  explicit pqSplitEdgeWidget(QWidget* parent = 0);
  virtual ~pqSplitEdgeWidget();

  virtual void setView(pqRenderView* view);
  void setEdgeOperator(smtk::model::OperatorPtr edgeOp);
  smtk::shared_ptr<smtk::model::Operator> edgeOperator();
  bool isActive();

signals:
  void operationRequested(const smtk::model::OperatorPtr& brOp);
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
  smtk::weak_ptr<smtk::model::Operator> m_edgeOp;
};

#endif // __smtk_polygon_pq_SplitEdgeWidget_h
