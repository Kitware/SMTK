//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtDraw2DLattice - A QGraphicsView that serves as a schema planner for
// the user. It can be used for cores and assemblies to specify which and where
// assembly/pin should be placed and do a validation check.
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_bridge_rgg_qt_qtDraw2DLattice_h
#define __smtk_bridge_rgg_qt_qtDraw2DLattice_h

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPointF>

#include "smtk/bridge/rgg/qt/qtDrawLatticeItem.h"
#include "smtk/bridge/rgg/qt/qtLattice.h"
#include "smtk/bridge/rgg/qt/rggNucPartDefinition.h"
#include "smtk/model/EntityRef.h"

#include <map>
#include <vector>

class QMouseEvent;
class rggLatticeContainer;
class QPoint;
class rggNucCoordinateConverter;

class qtDraw2DLattice : public QGraphicsView
{
  Q_OBJECT
  typedef QGraphicsView Superclass;

public:
  enum changeMode
  {
    NoChange = 0,
    SizeChange = 1,
    ContentChange = 2
  };
  enum replaceMode
  {
    Single = 0,
    All = 1,
    Fill = 2
  };
  qtDraw2DLattice(QWidget* parent = 0, Qt::WindowFlags f = 0);
  ~qtDraw2DLattice();

  int layers();
  void rebuild();
  void showContextMenu(qtDrawLatticeItem* hexitem, QPoint loc);

signals:
  // TODO: Add validation check
  // void sendMode(cmbNucWidgetChangeChecker::mode);
  void valuesChanged();
  // Who listens to it?
  void objGeometryChanged(rggLatticeContainer* selObj, int changeType);

public slots:
  void clear();
  void createImage(QString name);
  void setLattice(rggLatticeContainer* l);
  void setLatticeXorLayers(int v);
  void setLatticeY(int v);
  void updatePitch(double x, double y);
  void apply();
  void reset();
  void updateActionList();

protected:
  virtual void mousePressEvent(QMouseEvent* event);
  virtual void dropEvent(QDropEvent* event);
  virtual void resizeEvent(QResizeEvent* event);

private slots:
  void init();

  void addCell(
    QPointF const& pos, double radius, int layer, int cellIdx, qtLattice::CellDrawMode mode);

  void refresh(qtDrawLatticeItem* hexitem = NULL);

private:
  rggLatticeContainer* m_currentLattice;
  rggNucCoordinateConverter* m_converter;
  double m_radius[2];
  QGraphicsScene m_canvas;
  qtLattice m_grid;
  int m_changed;
  bool m_latticeChanged;

  qtLattice::CellDrawMode m_fullCellMode;

  std::vector<std::pair<QString, smtk::model::EntityRef> > m_actionList;

  QColor getColor(QString name) const;

  void checkForChangeMode();

  qtDrawLatticeItem* getItemAt(const QPoint& pt);
  QPointF getLatticeLocation(int layer, int cellIdx);
  std::vector<std::vector<qtDrawLatticeItem*> > m_itemLinks;
};

#endif
