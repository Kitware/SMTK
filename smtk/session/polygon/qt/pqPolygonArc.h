//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqPolygonArc - represents an arc object for polygon session.
// .SECTION Description
// .SECTION Caveats

#ifndef smtk_polygon_pq_Arc_h
#define smtk_polygon_pq_Arc_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/common/UUID.h"
#include "smtk/session/polygon/qt/Exports.h"
#include "vtkType.h"
#include <QObject>
#include <QPointer>

class pqRenderView;
class pqServer;
class vtkSMOutputPort;
class pqPipelineSource;
class pqDataRepresentation;
class vtkSMNewWidgetRepresentationProxy;
class vtkSMSourceProxy;
class vtkPolygonArcInfo;
class vtkIdTypeArray;
class vtkSMProxy;

class SMTKPOLYGONQTEXT_EXPORT pqPolygonArc : public QObject
{
  Q_OBJECT

public:
  //Description: Default constructor that than
  //needs createArc called on once its input for arc shape is created
  pqPolygonArc(QObject* parent = nullptr);

  ~pqPolygonArc() override;

  //Description:
  //Creates the server side arc from the widget poly data that is passed in
  //this method can only be called once per polygonArc.
  //returns true if it created the arc
  virtual bool createEdge(vtkSMNewWidgetRepresentationProxy* widget);

  //Description:
  //Edit this arc representation with the widget proxy passed in
  //this will update the arc state when it is done
  virtual bool editEdge(
    vtkSMNewWidgetRepresentationProxy* widget,
    const smtk::common::UUID& edgeid);

  //Description:
  //Update the server side arc with the new widget proxy shape
  //returns true if it could update the arc
  //filles the passed in vtkIdTypeArray with arcs that need to be created
  virtual bool updateArc(
    vtkSMNewWidgetRepresentationProxy* widget,
    vtkIdTypeArray* newlyCreatedArcIds);

  //Description:
  //Update the server with a connection between this arc
  //and the arc that is being passed in
  //Returns the id of the created arc. Will return -1 if we can't connect
  //the arcs
  virtual vtkIdType autoConnect(const vtkIdType& secondArcId);

  virtual bool isDefaultConstrained() const { return true; }
  vtkPolygonArcInfo* getArcInfo(int block_index);
  pqPipelineSource* getSource();
  void setSource(pqPipelineSource*);

  smtk::shared_ptr<smtk::operation::Operation> edgeOperation();
  void setEdgeOperation(smtk::operation::OperationPtr edgeOp);

  bool isClosedLoop();
  int getClosedLoop();

  void inheritPolygonRelationships(pqPolygonArc* parent);

  int getPlaneProjectionNormal() { return PlaneProjectionNormal; }
  double getPlaneProjectionPosition() { return PlaneProjectionPosition; }

  void setPlaneProjectionNormal(const int& norm) { PlaneProjectionNormal = norm; }
  void setPlaneProjectionPosition(const double& pos) { PlaneProjectionPosition = pos; }

  // Overwrite the databounds
  virtual void getDataBounds(double bounds[6]) const
  {
    (void)bounds; /*this->getBounds(bounds)*/
    ;
  }

  virtual void setSelectionInput(vtkSMSourceProxy* selectionInput);
  virtual void select();
  virtual void deselect();

  virtual void getColor(double color[4]) const;
  virtual void setColor(double color[4], bool updateRep = true);

  virtual void setMarkedForDeletion();
  virtual void unsetMarkedForDeletion();

  void arcIsModified();
  int getAssignedEdgeBlock() const;

  void resetOperationSource();

Q_SIGNALS:
  void operationRequested(const smtk::operation::OperationPtr& brOp);
  void activateModel(const smtk::common::UUID& modelid);

protected:
  vtkPolygonArcInfo* ArcInfo;

  virtual void setRepresentation(pqDataRepresentation* rep);
  void updatePlaneProjectionInfo(vtkSMNewWidgetRepresentationProxy* widget);
  vtkSMProxy* prepareOperation(vtkSMNewWidgetRepresentationProxy* widget);

  // Indicates the projection normal as lying along the
  // XAxis, YAxis, ZAxis, or Oblique. For X, Y, and Z axes,
  // the projection normal is assumed to be anchored at
  // (0,0,0)
  int PlaneProjectionNormal; // for the bounded plane

  // Indicates a distance from the origin of the projection
  // normal where the project plane will be placed
  double PlaneProjectionPosition;

  double origColor[4];
  double selColor[4];

  smtk::common::UUID m_currentModelId;
  QPointer<pqPipelineSource> Source;
  smtk::weak_ptr<smtk::operation::Operation> m_edgeOp;
};
#endif
