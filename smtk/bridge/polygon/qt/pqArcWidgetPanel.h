//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_polygon_pq_ArcWidgetPanel_h
#define __smtk_polygon_pq_ArcWidgetPanel_h

#include "smtk/bridge/polygon/qt/Exports.h"
#include "smtk/common/UUID.h"
#include "vtkType.h"
#include <QAction> //needed for ArcPicker
#include <QPointer>
#include <QWidget>

class pqOutputPort;
class pqRenderView;
class pqRenderViewSelectionReaction;
class pqPolygonArc;
class qtArcWidget;
class pqArcWidgetManager;
class pqPipelineSource;
class vtkSelectionNode;
class vtkPVSelectionInformation;

namespace Ui
{

struct PickInfo
{
  pqOutputPort* port;
  smtk::common::UUID EdgeId;
  vtkIdType BlockIndex;

  PickInfo()
    : port(NULL)
    , EdgeId(smtk::common::UUID::null())
    , BlockIndex(-1)
  {
  }
};

class ArcPicker : public QAction
{
  Q_OBJECT

public:
  ArcPicker(QObject* parent);
  virtual ~ArcPicker();
signals:
  //called by the selector when a valid selection is finished.
  void pickFinished();
  //emitted to allow selection to happen
  void triggered(bool);

public slots:
  void doPick(pqRenderView* view, pqPolygonArc* arc, PickInfo& info);

protected slots:
  //saves the information returned from the selection.
  void selectedInfo(pqOutputPort* port);
  // picking arc end point finished
  void onPickingFinished();
  vtkSelectionNode* gatherSelectionNode(
    pqPipelineSource* source, vtkPVSelectionInformation* selInfo);

private:
  PickInfo* Info;
  pqPolygonArc* Arc;
  pqRenderView* View;
  pqRenderViewSelectionReaction* Selecter;
  bool m_isActive;
};
}

class SMTKPOLYGONQTEXT_EXPORT pqArcWidgetPanel : public QWidget
{
  Q_OBJECT

public:
  explicit pqArcWidgetPanel(QWidget* parent = 0);
  virtual ~pqArcWidgetPanel();

  virtual void setView(pqRenderView* view) { this->View = view; }
  virtual void setArc(pqPolygonArc* arc);
  virtual void setArcManager(pqArcWidgetManager* arcManager) { this->ArcManager = arcManager; }

signals:
  void arcModified(qtArcWidget*, const smtk::common::UUID& edgeid);
  void arcModificationfinished();
  void arcModificationCacelled();
  void startArcEditing();
  void startArcPicking();

  friend class pqArcWidgetManager;

protected slots:
  //shows the edit widget and hides the pick widget
  void showEditWidget();

  //shows the pick widget and hides the edit widget
  void showPickWidget();
  //called when arc editing is done
  void arcEditingFinished();

  //marks that that we don't want to save the modifications
  //to the arc
  void cancelEdit();

  // save the modified arc to original edge. This will
  // replace all the points in the arc with the points on the arc widget.
  void saveEdit();

  // start modifying the selected edge.
  void modifyArc();

  // hide the arc editing widget
  void hideArcWidget();

  // pick the whole arc for operations
  void pickWholeArc();

  void arcPicked();

private:
  //resets the widget to what it would be like if it was just created
  void resetWidget();
  // this will create a representation for the specified sub arc
  void updateWidgetRepresentation();

  class pqInternals;
  pqInternals* Internals;

  Ui::PickInfo ArcInfo;
  Ui::ArcPicker Picker;
  pqRenderView* View;
  pqPolygonArc* Arc;
  QPointer<qtArcWidget> ArcWidget;
  pqArcWidgetManager* ArcManager;
};

#endif // __smtk_polygon_pq_ArcWidgetPanel_h
