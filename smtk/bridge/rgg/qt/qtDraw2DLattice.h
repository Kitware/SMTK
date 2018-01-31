#ifndef __cmbNucDraw2DLattice_h
#define __cmbNucDraw2DLattice_h

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPointF>

#include "DrawLatticeItem.h"
#include "cmbNucLattice.h"
#include "cmbNucPartDefinition.h"
#include "cmbNucWidgetChangeChecker.h"

#include <map>
#include <vector>

class QMouseEvent;
class LatticeContainer;
class cmbNucDraw2DLattice;
class pqXMLEventObserver;
class XMLEventSource;
class QPoint;
class cmbNucCordinateConverter;

class cmbNucDraw2DLattice : public QGraphicsView
{
  Q_OBJECT
  typedef QGraphicsView Superclass;

public:
  friend class pqXMLEventObserver;
  friend class XMLEventSource;
  enum changeMode
  {
    NoChange = 0,
    SizeChange = 1,
    ContentChange = 2
  };
  cmbNucDraw2DLattice(QWidget* parent = 0, Qt::WindowFlags f = 0);
  ~cmbNucDraw2DLattice();

  int layers();
  void rebuild();
  void showContextMenu(DrawLatticeItem* hexitem, QPoint loc);

signals:
  void sendMode(cmbNucWidgetChangeChecker::mode);
  void valuesChanged();
  void objGeometryChanged(cmbNucPart* selObj, int changeType);

public slots:
  void clear();
  void createImage(QString name);
  void setLattice(LatticeContainer* l);
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
    QPointF const& pos, double radius, int layer, int cellIdx, Lattice::CellDrawMode mode);

  void refresh(DrawLatticeItem* hexitem = NULL);

private:
  LatticeContainer* CurrentLattice;
  cmbNucCordinateConverter* Converter;
  double radius[2];
  QGraphicsScene Canvas;
  Lattice Grid;
  int changed;
  bool latticeChanged;

  Lattice::CellDrawMode FullCellMode;

  std::vector<std::pair<QString, cmbNucPart*> > ActionList;

  QColor getColor(QString name) const;

  void checkForChangeMode();

  DrawLatticeItem* getItemAt(const QPoint& pt);
  QPointF getLatticeLocation(int layer, int cellIdx);
  std::vector<std::vector<DrawLatticeItem*> > itemLinks;
};

#endif
