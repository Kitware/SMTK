#include "cmbNucDraw2DLattice.h"

#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QStyleOptionGraphicsItem>

#include "cmbNucAssembly.h"
#include "cmbNucCordinateConverter.h"
#include "cmbNucCore.h"
#include "cmbNucFillLattice.h"
#include "vtkMath.h"

#include <set>

cmbNucDraw2DLattice::cmbNucDraw2DLattice(QWidget* p, Qt::WindowFlags f)
  : QGraphicsView(p)
  , CurrentLattice(NULL)
  , Converter(NULL)
  , FullCellMode(Lattice::HEX_FULL)
{
  latticeChanged = false;
  changed = static_cast<int>(NoChange);
  setScene(&this->Canvas);
  setInteractive(true);
  setResizeAnchor(QGraphicsView::AnchorViewCenter);
  setWindowFlags(f);
  setAcceptDrops(true);
  setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

  this->Grid.SetDimensions(0, 0);
  init();
}

cmbNucDraw2DLattice::~cmbNucDraw2DLattice()
{
  this->Canvas.clear();
  delete Converter;
}

void cmbNucDraw2DLattice::clear()
{
  this->Canvas.clear();
  this->CurrentLattice = NULL;
  latticeChanged = false;
  this->init();
}

void cmbNucDraw2DLattice::init()
{
  this->Grid.SetDimensions(0, 0);
  this->rebuild();
}

void cmbNucDraw2DLattice::reset()
{
  this->changed = static_cast<int>(NoChange);
  latticeChanged = false;
  if (this->CurrentLattice)
  {
    this->Grid = this->CurrentLattice->getLattice();
  }
  this->rebuild();
}

void cmbNucDraw2DLattice::apply()
{
  int hasChanged = this->changed;
  if (this->CurrentLattice == NULL)
    return;
  if (changed)
  {
    this->CurrentLattice->getLattice() = this->Grid;
    this->CurrentLattice->setUpdateUsed();
    emit(valuesChanged());
    emit(objGeometryChanged(this->CurrentLattice, hasChanged));
  }
  this->changed = static_cast<int>(NoChange);
  latticeChanged = false;
  this->rebuild();
}

int cmbNucDraw2DLattice::layers()
{
  return this->Grid.GetDimensions().first;
}

void cmbNucDraw2DLattice::setLatticeXorLayers(int val)
{
  std::pair<int, int> wh = Grid.GetDimensions();
  if (val == wh.first)
    return;
  this->changed |= static_cast<int>(SizeChange);
  Grid.SetDimensions(val, wh.second);
  checkForChangeMode();
  this->rebuild();
}

void cmbNucDraw2DLattice::setLatticeY(int val)
{
  std::pair<int, int> wh = Grid.GetDimensions();
  if (val == wh.second)
    return;
  this->changed |= static_cast<int>(SizeChange);
  Grid.SetDimensions(wh.first, val);
  checkForChangeMode();
  this->rebuild();
}

void cmbNucDraw2DLattice::setLattice(LatticeContainer* l)
{
  this->CurrentLattice = l;
  this->changed = static_cast<int>(NoChange);
  latticeChanged = false;

  if (l)
  {
    l->updateLaticeFunction();
    this->Grid = l->getLattice();
  }
  else
  {
    Grid.SetDimensions(0, 0);
  }
  this->updateActionList();
  this->rebuild();
}

QColor cmbNucDraw2DLattice::getColor(QString name) const
{
  cmbNucPart* obj = this->CurrentLattice->getFromLabel(name);
  return obj ? obj->GetLegendColor() : Qt::white;
}

void cmbNucDraw2DLattice::addCell(
  QPointF const& posF, double r, int layer, int cellIdx, Lattice::CellDrawMode mode)
{
  Lattice::Cell lc = Grid.GetCell(layer, cellIdx);
  if (!lc.isValid())
    return;
  QPolygon polygon;

  switch (mode)
  {
    case Lattice::HEX_FULL:
      polygon << QPoint(0, 2 * r) << QPoint(-r * 1.73, r) << QPoint(-r * 1.73, -r)
              << QPoint(0, -2 * r) << QPoint(r * 1.73, -r) << QPoint(r * 1.73, r);
      break;
    case Lattice::HEX_FULL_30:
      polygon << QPoint(2 * r, 0) << QPoint(r, -r * 1.73) << QPoint(-r, -r * 1.73)
              << QPoint(-2 * r, 0) << QPoint(-r, r * 1.73) << QPoint(r, r * 1.73);
      break;
    case Lattice::HEX_SIXTH_VERT_BOTTOM:
      polygon << QPoint(2 * r, 0) << QPoint(r, -r * 1.73) << QPoint(-r, -r * 1.73)
              << QPoint(-2 * r, 0);
      break;
    case Lattice::HEX_SIXTH_VERT_CENTER:
      polygon << QPoint(2 * r, 0) << QPoint(r, -r * 1.73) << QPoint(0, 0);
      break;
    case Lattice::HEX_SIXTH_VERT_TOP:
      polygon << QPoint(2 * r, 0) << QPoint(r, -r * 1.73) << QPoint(-r, r * 1.73)
              << QPoint(r, r * 1.73);
      break;
    case Lattice::HEX_SIXTH_FLAT_CENTER:
      polygon << QPoint(0, 0) << QPoint(r * cmbNucMathConst::cos30, -1.5 * r)
              << QPoint(r * 1.73, -r) << QPoint(r * 1.73, 0);
      break;
    case Lattice::HEX_SIXTH_FLAT_TOP:
      polygon << QPoint(0, 2 * r)                               //keep
              << QPoint((-r * cmbNucMathConst::cos30), 1.5 * r) //half
              << QPoint(r * cmbNucMathConst::cos30, -1.5 * r) << QPoint(r * 1.73, -r)
              << QPoint(r * 1.73, r);
      break;
    case Lattice::HEX_SIXTH_FLAT_BOTTOM:
    case Lattice::HEX_TWELFTH_BOTTOM:
      polygon << QPoint(-r * 1.73, 0) << QPoint(-r * 1.73, -r) << QPoint(0, -2 * r)
              << QPoint(r * 1.73, -r) << QPoint(r * 1.73, 0);
      break;
    case Lattice::HEX_TWELFTH_TOP:
      polygon << QPoint(0, 2 * r) //keep
              << QPoint(-r * 1.73, r) << QPoint(r * 1.73, -r) << QPoint(r * 1.73, r);
      break;
    case Lattice::HEX_TWELFTH_CENTER:
      polygon << QPoint(0, 0) << QPoint(r * 1.73, -r) << QPoint(r * 1.73, 0);
      break;
    case Lattice::RECT:
      polygon << QPoint(-r, -r) << QPoint(-r, r) << QPoint(r, r) << QPoint(r, -r);
      break;
  }
  DrawLatticeItem* cell =
    new DrawLatticeItem(polygon, layer, cellIdx, Grid.getRerference(layer, cellIdx));
  itemLinks[layer][cellIdx] = cell;

  cell->setPos(posF);

  // update color in hex map
  QString text(lc.getLabel());
  scene()->addItem(cell);
}

QPointF cmbNucDraw2DLattice::getLatticeLocation(int ti, int tj)
{
  double centerPos[2];
  Converter->convertToPixelXY(ti, tj, centerPos[0], centerPos[1], radius[1]);
  return QPointF(
    centerPos[0] + this->rect().center().x(), centerPos[1] + this->rect().center().y());
}

void cmbNucDraw2DLattice::rebuild()
{
  scene()->clear();
  int numLayers = this->layers();
  if (numLayers <= 0)
  {
    return;
  }

  delete Converter;
  Converter = new cmbNucCordinateConverter(this->Grid);
  Converter->computeRadius(this->width(), this->height(), radius);
  itemLinks.clear();
  itemLinks.resize(this->Grid.getSize());
  for (size_t i = 0; i < this->Grid.getSize(); ++i)
  {
    itemLinks[i].resize(this->Grid.getSize(i), NULL);
    for (size_t j = 0; j < this->Grid.getSize(i); ++j)
    {
      int ti = static_cast<int>(i);
      int tj = static_cast<int>(j);
      this->addCell(getLatticeLocation(ti, tj), radius[0], ti, tj, this->Grid.getDrawMode(tj, ti));
    }
  }
  scene()->setSceneRect(scene()->itemsBoundingRect());
  this->repaint();
}

void cmbNucDraw2DLattice::showContextMenu(DrawLatticeItem* hexitem, QPoint qme)
{
  if (!hexitem)
  {
    return;
  }

  this->Grid.unselect();
  this->Grid.selectCell(hexitem->layer(), hexitem->cellIndex());
  this->refresh(hexitem);

  QMenu contextMenu(this);
  contextMenu.setObjectName("Replace_Menu");
  QMenu* replace_function = new QMenu(QString("Replace All With"), &contextMenu);
  replace_function->setObjectName(QString("Replace_All_With"));

  QAction* fillSimple = new QAction(QString("Fill"), this);
  fillSimple->setObjectName(QString("Fill_Cells"));
  fillSimple->setData(2);
  QAction* pAction = NULL;
  // available parts
  for (size_t i = 0; i < this->ActionList.size(); ++i)
  {
    QString strAct = this->ActionList[i].first;
    cmbNucPart const* part = this->ActionList[i].second;
    double r = (part) ? part->getRadius() : -1;
    pAction = new QAction(strAct, this);
    pAction->setEnabled(hexitem->checkRadiusIsOk(r));
    pAction->setData(0);
    contextMenu.addAction(pAction);
    pAction = new QAction(strAct, this);
    pAction->setData(1);
    pAction->setEnabled(hexitem->checkRadiusIsOk(r));
    replace_function->addAction(pAction);
  }
  contextMenu.addMenu(replace_function);
  contextMenu.addAction(fillSimple);

  QAction* assignAct = contextMenu.exec(qme);
  if (assignAct && assignAct->data().toInt() == 2)
  {
    cmbNucFillLattice fillFun;
    connect(&fillFun, SIGNAL(refresh(DrawLatticeItem*)), this, SLOT(refresh(DrawLatticeItem*)));
    if (fillFun.fillSimple(hexitem, &(this->Grid), this->CurrentLattice))
    {
      this->changed |= static_cast<int>(ContentChange);
      checkForChangeMode();
    }
  }
  else if (assignAct)
  {
    QString text = this->CurrentLattice->extractLabel(assignAct->text());
    cmbNucPart* newPart = this->CurrentLattice->getFromLabel(text);
    if (assignAct->data().toInt() == 0)
    {
      QString oldText = hexitem->text();
      if (this->Grid.SetCell(hexitem->layer(), hexitem->cellIndex(), newPart))
      {
        this->changed |= static_cast<int>(ContentChange);
        checkForChangeMode();
      }
    }
    else if (assignAct->data().toInt() == 1)
    {
      if (hexitem->text() != text)
      {
        this->changed |= static_cast<int>(ContentChange);
        cmbNucPart* oldPart = this->CurrentLattice->getFromLabel(hexitem->text());
        this->Grid.replacePart(oldPart, newPart);
        checkForChangeMode();
      }
    }
  }
  this->Grid.clearSelection();
  this->Grid.sendMaxRadiusToReference();
  this->refresh(hexitem);
}

DrawLatticeItem* cmbNucDraw2DLattice::getItemAt(const QPoint& pt)
{
  return dynamic_cast<DrawLatticeItem*>(this->itemAt(pt));
}

void cmbNucDraw2DLattice::dropEvent(QDropEvent* qde)
{
  DrawLatticeItem* dest = this->getItemAt(qde->pos());
  if (!dest)
    return;

  QString newLabel = qde->mimeData()->text();
  cmbNucPart* newPart = this->CurrentLattice->getFromLabel(newLabel);
  if (!dest->checkRadiusIsOk((newPart) ? newPart->getRadius() : -1))
    return;

  if (this->Grid.SetCell(dest->layer(), dest->cellIndex(), newPart))
  {
    this->changed |= (static_cast<int>(ContentChange));
    checkForChangeMode();
  }
  qde->acceptProposedAction();
  this->Grid.clearSelection();
  this->Grid.sendMaxRadiusToReference();
  this->refresh(dest);
}

void cmbNucDraw2DLattice::resizeEvent(QResizeEvent* qre)
{
  QGraphicsView::resizeEvent(qre);
  this->rebuild();
}

void cmbNucDraw2DLattice::mousePressEvent(QMouseEvent* qme)
{
  DrawLatticeItem* hitem = this->getItemAt(qme->pos());
  if (!hitem)
  {
    return;
  }

  // Context menu on right click
  if (qme->button() == Qt::RightButton)
  {
    this->showContextMenu(hitem, qme->globalPos());
  }
  // Drag and drop on left click
  else if (qme->button() == Qt::LeftButton)
  {
    QMimeData* mimeData = new QMimeData;
    mimeData->setText(hitem->text());
    cmbNucPart* part = hitem->getPart();
    this->Grid.setPotentialConflictCells((part) ? part->getRadius() : -1);
    this->refresh(hitem);

    QSize tmpsize = hitem->boundingRect().size().toSize();
    QPixmap pixmap(tmpsize.width() + 1, tmpsize.height() + 1);
    pixmap.fill(QColor(255, 255, 255, 0)); //Transparent background
    QPainter imagePainter(&pixmap);
    imagePainter.translate(-hitem->boundingRect().topLeft());

    QStyleOptionGraphicsItem gstyle;

    hitem->paint(&imagePainter, &gstyle, NULL);

    QDrag* drag = new QDrag(this);
    drag->setMimeData(mimeData);
#ifdef _WIN32
    drag->setPixmap(pixmap.scaledToHeight(20, Qt::SmoothTransformation));
#else
    drag->setPixmap(pixmap.scaledToHeight(40, Qt::SmoothTransformation));
#endif
    drag->exec(Qt::CopyAction);
    this->Grid.clearSelection();
    this->refresh(hitem);
    imagePainter.end();
  }
}

void cmbNucDraw2DLattice::checkForChangeMode()
{
  Lattice::changeMode cm = CurrentLattice->getLattice().compair(Grid);
  if (cm == Lattice::Same && latticeChanged)
  {
    emit sendMode(cmbNucWidgetChangeChecker::reverted_to_orginal);
    latticeChanged = true;
  }
  else if (cm == Lattice::ContentDiff && !latticeChanged)
  {
    latticeChanged = true;
    emit sendMode(cmbNucWidgetChangeChecker::newly_changed);
  }
}

void cmbNucDraw2DLattice::createImage(QString fname)
{
  QGraphicsView* view = new QGraphicsView(&Canvas, this);
  view->setSceneRect(Canvas.itemsBoundingRect());
  view->setMinimumHeight(600);
  view->setMinimumWidth(600);
  QPixmap pixMap = QPixmap::grabWidget(view, 0, 0, 600, 600);
  pixMap.toImage().convertToFormat(QImage::Format_RGB32).save(fname);
  delete view;
}

void cmbNucDraw2DLattice::refresh(DrawLatticeItem* hexitem)
{
  if (hexitem == NULL)
  {
    if (Canvas.items().isEmpty())
    {
      this->repaint();
      return;
    }
    hexitem = dynamic_cast<DrawLatticeItem*>(Canvas.items()[0]);
  }
  scene()->removeItem(hexitem);
  scene()->addItem(hexitem);
  this->repaint();
}

void cmbNucDraw2DLattice::updateActionList()
{
  this->ActionList.clear();
  if (this->CurrentLattice != NULL)
  {
    this->CurrentLattice->fillList(this->ActionList);
  }
}

void cmbNucDraw2DLattice::updatePitch(double xin, double yin)
{
  //get the width and height radius, if -1, do nothing, otherwise redraw
  Grid.updatePitchForMaxRadius(xin, yin);
  Grid.sendMaxRadiusToReference();
  this->refresh();
}
