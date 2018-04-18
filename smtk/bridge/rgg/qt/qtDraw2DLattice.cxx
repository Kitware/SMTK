//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/bridge/rgg/qt/qtDraw2DLattice.h"

#include "vtkMath.h"
//#include "cmbNucCore.h"
//#include "cmbNucFillLattice.h"
#include "smtk/bridge/rgg/qt/rggLatticeContainer.h"
#include "smtk/bridge/rgg/qt/rggNucAssembly.h"
#include "smtk/bridge/rgg/qt/rggNucCoordinateConverter.h"

#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QDrag>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QStyleOptionGraphicsItem>

#include <set>

namespace
{

auto getRGGEntityRadius = [](const smtk::model::EntityRef& ent) -> double {
  double r(-1);
  if (ent.isValid() && ent.hasFloatProperty("max radius"))
  {
    r = ent.floatProperty("max radius")[0];
  }
  return r;
};
}

qtDraw2DLattice::qtDraw2DLattice(QWidget* p, Qt::WindowFlags f)
  : QGraphicsView(p)
  , m_currentLattice(NULL)
  , m_converter(NULL)
  , m_fullCellMode(qtLattice::HEX_FULL)
{
  m_latticeChanged = false;
  m_changed = static_cast<int>(NoChange);
  this->setScene(&this->m_canvas);
  this->setInteractive(true);
  this->setResizeAnchor(QGraphicsView::AnchorViewCenter);
  this->setWindowFlags(f);
  this->setAcceptDrops(true);
  this->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

  this->m_grid.SetDimensions(0, 0);
  init();
}

qtDraw2DLattice::~qtDraw2DLattice()
{
  this->m_canvas.clear();
  if (this->m_converter)
  {
    delete this->m_converter;
  }
}

void qtDraw2DLattice::clear()
{
  this->m_canvas.clear();
  this->m_currentLattice = NULL;
  m_latticeChanged = false;
  this->init();
}

void qtDraw2DLattice::init()
{
  this->m_grid.SetDimensions(0, 0);
  this->rebuild();
}

void qtDraw2DLattice::reset()
{
  this->m_changed = static_cast<int>(NoChange);
  m_latticeChanged = false;
  if (this->m_currentLattice)
  {
    this->m_grid = this->m_currentLattice->getLattice();
  }
  this->rebuild();
}

void qtDraw2DLattice::apply()
{
  int hasChanged = this->m_changed;
  if (this->m_currentLattice == NULL)
    return;
  if (m_changed)
  {
    this->m_currentLattice->getLattice() = this->m_grid;
    this->m_currentLattice->setUpdateUsed();
    emit(valuesChanged());
    emit(objGeometryChanged(this->m_currentLattice, hasChanged));
  }
  this->m_changed = static_cast<int>(NoChange);
  m_latticeChanged = false;
  this->rebuild();
}

int qtDraw2DLattice::layers()
{
  return static_cast<int>(this->m_grid.GetDimensions().first);
}

void qtDraw2DLattice::setLatticeXorLayers(int val)
{
  std::pair<size_t, size_t> wh = m_grid.GetDimensions();
  if (val == static_cast<int>(wh.first))
    return;
  this->m_changed |= static_cast<int>(SizeChange);
  m_grid.SetDimensions(val, static_cast<int>(wh.second));
  checkForChangeMode();
  this->rebuild();
}

void qtDraw2DLattice::setLatticeY(int val)
{
  std::pair<size_t, size_t> wh = m_grid.GetDimensions();
  if (val == static_cast<int>(wh.second))
    return;
  this->m_changed |= static_cast<int>(SizeChange);
  m_grid.SetDimensions(static_cast<int>(wh.first), val);
  checkForChangeMode();
  this->rebuild();
}

void qtDraw2DLattice::setLattice(rggLatticeContainer* l)
{
  this->m_currentLattice = l;
  this->m_changed = static_cast<int>(NoChange);
  m_latticeChanged = false;

  if (l)
  {
    l->updateLaticeFunction();
    this->m_grid = l->getLattice();
  }
  else
  {
    m_grid.SetDimensions(0, 0);
  }
  this->updateActionList();
  this->rebuild();
}

QColor qtDraw2DLattice::getColor(QString name) const
{
  smtk::model::EntityRef ent = this->m_currentLattice->getFromLabel(name);
  if (ent.isValid())
  {
    smtk::model::FloatList color = ent.color();
    return QColor::fromRgbF(color[0], color[1], color[2], color[3]);
  }
  else
  {
    return Qt::white;
  }
}

void qtDraw2DLattice::addCell(
  QPointF const& posF, double r, int layer, int cellIdx, qtLattice::CellDrawMode mode)
{
  qtCell lc = m_grid.GetCell(layer, cellIdx);
  if (!lc.isValid())
  {
    return;
  }
  QPolygon polygon;

  double sizeCoefficient = 1.73;
  switch (mode)
  {
    case qtLattice::HEX_FULL:
      polygon << QPoint(0, 2 * r) << QPoint(-r * sizeCoefficient, r)
              << QPoint(-r * sizeCoefficient, -r) << QPoint(0, -2 * r)
              << QPoint(r * sizeCoefficient, -r) << QPoint(r * sizeCoefficient, r);
      break;
    case qtLattice::HEX_FULL_30:
      polygon << QPoint(2 * r, 0) << QPoint(r, -r * sizeCoefficient)
              << QPoint(-r, -r * sizeCoefficient) << QPoint(-2 * r, 0)
              << QPoint(-r, r * sizeCoefficient) << QPoint(r, r * sizeCoefficient);
      break;
    case qtLattice::HEX_SIXTH_VERT_BOTTOM:
      polygon << QPoint(2 * r, 0) << QPoint(r, -r * sizeCoefficient)
              << QPoint(-r, -r * sizeCoefficient) << QPoint(-2 * r, 0);
      break;
    case qtLattice::HEX_SIXTH_VERT_CENTER:
      polygon << QPoint(2 * r, 0) << QPoint(r, -r * sizeCoefficient) << QPoint(0, 0);
      break;
    case qtLattice::HEX_SIXTH_VERT_TOP:
      polygon << QPoint(2 * r, 0) << QPoint(r, -r * sizeCoefficient)
              << QPoint(-r, r * sizeCoefficient) << QPoint(r, r * sizeCoefficient);
      break;
    case qtLattice::HEX_SIXTH_FLAT_CENTER:
      polygon << QPoint(0, 0) << QPoint(r * rggNucMathConst::cos30, -1.5 * r)
              << QPoint(r * sizeCoefficient, -r) << QPoint(r * sizeCoefficient, 0);
      break;
    case qtLattice::HEX_SIXTH_FLAT_TOP:
      polygon << QPoint(0, 2 * r)                               //keep
              << QPoint((-r * rggNucMathConst::cos30), 1.5 * r) //half
              << QPoint(r * rggNucMathConst::cos30, -1.5 * r) << QPoint(r * sizeCoefficient, -r)
              << QPoint(r * sizeCoefficient, r);
      break;
    case qtLattice::HEX_SIXTH_FLAT_BOTTOM:
    case qtLattice::HEX_TWELFTH_BOTTOM:
      polygon << QPoint(-r * sizeCoefficient, 0) << QPoint(-r * sizeCoefficient, -r)
              << QPoint(0, -2 * r) << QPoint(r * sizeCoefficient, -r)
              << QPoint(r * sizeCoefficient, 0);
      break;
    case qtLattice::HEX_TWELFTH_TOP:
      polygon << QPoint(0, 2 * r) //keep
              << QPoint(-r * sizeCoefficient, r) << QPoint(r * sizeCoefficient, -r)
              << QPoint(r * sizeCoefficient, r);
      break;
    case qtLattice::HEX_TWELFTH_CENTER:
      polygon << QPoint(0, 0) << QPoint(r * sizeCoefficient, -r) << QPoint(r * sizeCoefficient, 0);
      break;
    case qtLattice::RECT:
      polygon << QPoint(-r, -r) << QPoint(-r, r) << QPoint(r, r) << QPoint(r, -r);
      break;
  }
  qtDrawLatticeItem* cell =
    new qtDrawLatticeItem(polygon, layer, cellIdx, m_grid.getRerference(layer, cellIdx));
  m_itemLinks[layer][cellIdx] = cell;

  cell->setPos(posF);

  // update color in hex map
  scene()->addItem(cell);
}

QPointF qtDraw2DLattice::getLatticeLocation(int ti, int tj)
{
  double centerPos[2];
  this->m_converter->convertToPixelXY(ti, tj, centerPos[0], centerPos[1], m_radius[1]);
  return QPointF(
    centerPos[0] + this->rect().center().x(), centerPos[1] + this->rect().center().y());
}

void qtDraw2DLattice::rebuild()
{
  scene()->clear();
  int numLayers = this->layers();
  if (numLayers <= 0)
  {
    return;
  }

  if (this->m_converter)
  {
    delete this->m_converter;
  }

  this->m_converter = new rggNucCoordinateConverter(this->m_grid);
  this->m_converter->computeRadius(this->width(), this->height(), m_radius);
  m_itemLinks.clear();
  m_itemLinks.resize(this->m_grid.getSize());
  for (size_t i = 0; i < this->m_grid.getSize(); ++i)
  {
    m_itemLinks[i].resize(this->m_grid.getSize(i), NULL);
    for (size_t j = 0; j < this->m_grid.getSize(i); ++j)
    {
      int ti = static_cast<int>(i);
      int tj = static_cast<int>(j);
      // Why getDrawMode has a different index system
      this->addCell(
        this->getLatticeLocation(ti, tj), m_radius[0], ti, tj, this->m_grid.getDrawMode(tj, ti));
    }
  }
  scene()->setSceneRect(scene()->itemsBoundingRect());
  this->repaint();
}

void qtDraw2DLattice::showContextMenu(qtDrawLatticeItem* hexitem, QPoint qme)
{
  if (!hexitem)
  {
    return;
  }

  this->m_grid.unselect();
  this->m_grid.selectCell(hexitem->layer(), hexitem->cellIndex());
  this->refresh(hexitem);

  QMenu contextMenu(this);
  contextMenu.setObjectName("Replace_Menu");
  QMenu* replace_function = new QMenu(QString("Replace All With"), &contextMenu);
  replace_function->setObjectName(QString("Replace_All_With"));

  QAction* fillSimple = new QAction(QString("Fill(WIP)"), this);
  fillSimple->setObjectName(QString("Fill_Cells"));
  fillSimple->setData(2);
  fillSimple->setEnabled(false); // FIXME

  QAction* pAction = nullptr;
  // Fill single part mode
  this->updateActionList(); // User might have changed pin label
  for (size_t i = 0; i < this->m_actionList.size(); ++i)
  {
    QString strAct = this->m_actionList[i].first;
    smtk::model::EntityRef part = this->m_actionList[i].second;

    pAction = new QAction(strAct, this);
    //FIXME: Always enable for now
    //pAction->setEnabled(hexitem->checkRadiusIsOk(r));
    pAction->setData(replaceMode::Single);
    contextMenu.addAction(pAction);
    pAction = new QAction(strAct, this);
    pAction->setData(replaceMode::All);
    //FIXME: Always enable for now
    //pAction->setEnabled(hexitem->checkRadiusIsOk(r));
    replace_function->addAction(pAction);
  }
  // Replace all mode
  contextMenu.addMenu(replace_function);
  // Fill function mode(WIP)
  contextMenu.addAction(fillSimple);

  QAction* assignAct = contextMenu.exec(qme);
  if (assignAct && assignAct->data().toInt() == replaceMode::Fill)
  {
    //    cmbNucFillLattice fillFun;
    //    connect(&fillFun, SIGNAL(refresh(DrawLatticeItem*)), this, SLOT(refresh(DrawLatticeItem*)));
    //    if(fillFun.fillSimple(hexitem, &(this->m_grid), this->m_currentLattice))
    //    {
    //     this->m_changed |= static_cast<int>(ContentChange);
    //     checkForChangeMode();
    //    }
  }
  else if (assignAct)
  {
    QString label = this->m_currentLattice->extractLabel(assignAct->text());
    smtk::model::EntityRef newPart = this->m_currentLattice->getFromLabel(label);
    if (assignAct->data().toInt() == replaceMode::Single)
    {
      if (this->m_grid.SetCell(hexitem->layer(), hexitem->cellIndex(), newPart))
      {
        this->m_changed |= static_cast<int>(ContentChange);
        checkForChangeMode();
      }
    }
    else if (assignAct->data().toInt() == replaceMode::All)
    {
      if (hexitem->text() != label)
      {
        this->m_changed |= static_cast<int>(ContentChange);
        smtk::model::EntityRef oldPart = this->m_currentLattice->getFromLabel(hexitem->text());
        this->m_grid.replacePart(oldPart, newPart);
        checkForChangeMode();
      }
    }
  }
  this->m_grid.clearSelection();
  this->m_grid.sendMaxRadiusToReference();
  this->refresh(hexitem);
}

qtDrawLatticeItem* qtDraw2DLattice::getItemAt(const QPoint& pt)
{
  return dynamic_cast<qtDrawLatticeItem*>(this->itemAt(pt));
}

void qtDraw2DLattice::dropEvent(QDropEvent* qde)
{
  qtDrawLatticeItem* dest = this->getItemAt(qde->pos());
  if (!dest)
    return;

  QString newLabel = qde->mimeData()->text();
  smtk::model::EntityRef newPart = this->m_currentLattice->getFromLabel(newLabel);

  double r = getRGGEntityRadius(newPart);
  if (!dest->checkRadiusIsOk(r))
  {
    return;
  }

  if (this->m_grid.SetCell(dest->layer(), dest->cellIndex(), newPart))
  {
    this->m_changed |= (static_cast<int>(ContentChange));
    checkForChangeMode();
  }
  qde->acceptProposedAction();
  this->m_grid.clearSelection();
  this->m_grid.sendMaxRadiusToReference();
  this->refresh(dest);
}

void qtDraw2DLattice::resizeEvent(QResizeEvent* qre)
{
  QGraphicsView::resizeEvent(qre);
  this->rebuild();
}

void qtDraw2DLattice::mousePressEvent(QMouseEvent* qme)
{
  qtDrawLatticeItem* hitem = this->getItemAt(qme->pos());
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
    smtk::model::EntityRef part = hitem->getPart();
    this->m_grid.setPotentialConflictCells(getRGGEntityRadius(part));
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
    this->m_grid.clearSelection();
    this->refresh(hitem);
    imagePainter.end();
  }
}

void qtDraw2DLattice::checkForChangeMode()
{
  qtLattice::ChangeMode cm = m_currentLattice->getLattice().compair(m_grid);
  if (cm == qtLattice::Same && m_latticeChanged)
  {
    //emit sendMode(cmbNucWidgetChangeChecker::reverted_to_orginal);
    m_latticeChanged = true;
  }
  else if (cm == qtLattice::ContentDiff && !m_latticeChanged)
  {
    m_latticeChanged = true;
    //emit sendMode(cmbNucWidgetChangeChecker::newly_changed);
  }
}

void qtDraw2DLattice::createImage(QString fname)
{
  QGraphicsView* view = new QGraphicsView(&m_canvas, this);
  view->setSceneRect(m_canvas.itemsBoundingRect());
  view->setMinimumHeight(600);
  view->setMinimumWidth(600);
  QPixmap pixMap = QPixmap::grabWidget(view, 0, 0, 600, 600);
  pixMap.toImage().convertToFormat(QImage::Format_RGB32).save(fname);
  delete view;
}

void qtDraw2DLattice::refresh(qtDrawLatticeItem* hexitem)
{
  if (hexitem == NULL)
  {
    if (m_canvas.items().isEmpty())
    {
      this->repaint();
      return;
    }
    hexitem = dynamic_cast<qtDrawLatticeItem*>(m_canvas.items()[0]);
  }
  scene()->removeItem(hexitem);
  scene()->addItem(hexitem);
  this->repaint();
}

void qtDraw2DLattice::updateActionList()
{
  this->m_actionList.clear();
  if (this->m_currentLattice != NULL)
  {
    this->m_currentLattice->fillList(this->m_actionList);
  }
}

void qtDraw2DLattice::updatePitch(double xin, double yin)
{
  //get the width and height radius, if -1, do nothing, otherwise redraw
  m_grid.updatePitchForMaxRadius(xin, yin);
  m_grid.sendMaxRadiusToReference();
  this->refresh();
}
