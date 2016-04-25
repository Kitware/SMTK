//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqArcWidgetManager
// .SECTION Description
//  Create and controls the arc editing singelton widget
// .SECTION Caveats


#ifndef __smtk_polygon_pq_ArcWidgetManager_h
#define __smtk_polygon_pq_ArcWidgetManager_h

#include "smtk/bridge/polygon/qt/Exports.h"

#include <QList>
#include <QObject>
#include <QPointer>
#include <QWidget>
#include "vtkType.h"

class pqArcWidget;
class pqArcWidgetPanel;
class pqPolygonArc;
class pqRenderView;
class pqServer;
class vtkDoubleArray;
class vtkIdTypeArray;

class SMTKPOLYGONQTEXT_EXPORT pqArcWidgetManager : public QObject
{
  Q_OBJECT

public:
  pqArcWidgetManager(pqServer *server, pqRenderView *view);
  virtual ~pqArcWidgetManager();

  int create();
  int edit();
  void reset();

  pqArcWidget* createDefaultContourWidget(int& normal, double& pos);

  QWidget* getActiveWidget() { return ActiveWidget; }
  pqPolygonArc* getActiveArc();
  void setActiveArc(pqPolygonArc*);

signals:
  void Busy();
  void Ready();
  void Finish();

  //This signal is emitted when an arc is split into multiple new arcs
  void ArcSplit(QList<vtkIdType>);
  void ArcSplit2(pqPolygonArc*, QList<vtkIdType>);

  //this signal is emitted whenever an arc is modified, including when
  //split
  void ArcModified();
  void ArcModified2(pqPolygonArc*);

  void editingStarted();

public slots:
  // slot for operations on an arc or sub-arc
  void straightenArc(vtkIdType startIdx, vtkIdType endIdx);
  void collapseSubArc(vtkIdType startIdx, vtkIdType endIdx);
  void makeArc(vtkIdType startIdx, vtkIdType endIdx);

  void updateActiveView( pqRenderView *view ){ View=view;}
  void updateActiveServer( pqServer *server ){ Server=server;}

protected slots:
  // called when a whole arc is done creating or modifying.
  void createEdge();
  // called when a sub arc modification is done
  void updateModifiedArc(
    pqArcWidget* subArcWidget, vtkIdType startPID, vtkIdType endPID);
  // called when the edit widget is closed
  void editingFinished();

protected:
  void getDefaultArcPlane(int& normal, double& pos);
  void resetArcPlane(int normal, double pos);
  pqArcWidget* createContourWidget( int normal, double position );
  void modifyArc(vtkIdType startIdx, vtkIdType endIdx, int opType);
  pqPolygonArc* createLegacyV1Contour(
    const int &normal,const double &position,const int &closedLoop,
    vtkDoubleArray* nodePositions, vtkIdTypeArray* SelIndices);

  QPointer<pqArcWidget> ArcWidget;
  QPointer<pqArcWidgetPanel> EditWidget;
  QPointer<pqPolygonArc> Arc;

  pqRenderView *View;
  pqServer *Server;
  QPointer<QWidget> ActiveWidget;
};

#endif /* __smtk_polygon_pq_ArcWidgetManager_h */
