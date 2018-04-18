//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/bridge/rgg/qt/nuclides/NuclideTableView.h"

#include "smtk/bridge/rgg/qt/nuclides/NuclideTable.h"

#include <QtWidgets>
#include <qmath.h>

#include <set>

namespace smtk
{
namespace bridge
{
namespace rgg
{

#if QT_CONFIG(wheelevent)
void NuclideTableGraphicsView::wheelEvent(QWheelEvent* e)
{
  if (e->modifiers() & Qt::ControlModifier)
  {
    if (e->delta() > 0)
      view->zoomIn(6);
    else
      view->zoomOut(6);
    e->accept();
  }
  else
  {
    QGraphicsView::wheelEvent(e);
  }
}
#endif

NuclideTableView::NuclideTableView(const QString& name, QWidget* parent)
  : QFrame(parent)
{
  Q_UNUSED(name);
  NuclideTable* nuclideTable = dynamic_cast<NuclideTable*>(parent);
  if (nuclideTable == nullptr)
    return;

  setFrameStyle(Sunken | StyledPanel);
  graphicsView = new NuclideTableGraphicsView(this);
  graphicsView->setDragMode(QGraphicsView::NoDrag);
  graphicsView->setOptimizationFlags(QGraphicsView::DontSavePainterState);
  graphicsView->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
  graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

  int size = style()->pixelMetric(QStyle::PM_ToolBarIconSize);
  QSize iconSize(size, size);

  QToolButton* zoomInIcon = new QToolButton;
  zoomInIcon->setAutoRepeat(true);
  zoomInIcon->setAutoRepeatInterval(33);
  zoomInIcon->setAutoRepeatDelay(0);
  zoomInIcon->setIcon(QPixmap(":/rgg/qt/nuclide/zoomin.png"));
  zoomInIcon->setIconSize(iconSize);
  QToolButton* zoomOutIcon = new QToolButton;
  zoomOutIcon->setAutoRepeat(true);
  zoomOutIcon->setAutoRepeatInterval(33);
  zoomOutIcon->setAutoRepeatDelay(0);
  zoomOutIcon->setIcon(QPixmap(":/rgg/qt/nuclide/zoomout.png"));
  zoomOutIcon->setIconSize(iconSize);
  zoomSlider = new QSlider;
  zoomSlider->setMinimum(75);
  zoomSlider->setMaximum(300);
  zoomSlider->setValue(150);
  zoomSlider->setTickPosition(QSlider::TicksRight);

  // Zoom slider layout
  QVBoxLayout* zoomSliderLayout = new QVBoxLayout;
  zoomSliderLayout->addWidget(zoomInIcon);
  zoomSliderLayout->addWidget(zoomSlider);
  zoomSliderLayout->addWidget(zoomOutIcon);

  // Nuclide selector layout
  QHBoxLayout* labelLayout = new QHBoxLayout;
  label = new QLabel(tr("Nuclide:"));
  symbol = new QComboBox();
  massNumber = new QComboBox();

  // When the available symbols are retrieved, they are unsorted. We put them in
  // a QList to sort them before inserting them in the combo box.
  {
    auto symbols = nuclideTable->symbols();
    QList<QString> symbolList;
    for (auto& smbl : symbols)
    {
      symbolList.append(smbl);
    }
    symbolList.sort();
    for (auto smbl = symbolList.begin(); smbl != symbolList.end(); ++smbl)
    {
      symbol->addItem(*smbl);
    }
  }

  // When a nuclide is selected from the visual display, update the combo box to
  // reflect the selection.
  auto setSymbol = [=](Nuclide* nuclide) {
    int filterId = symbol->findText(nuclide->symbol());
    if (filterId != -1)
    {
      // We must first block the signals of the mass number combo box.
      // Otherwise, the change of symbol triggers a reset of the available
      // mass numbers. Since the mass number combo box is also listening to this
      // signal, it doesn't need to update as a result of the symbol change.
      bool oldState = massNumber->blockSignals(true);
      symbol->setCurrentIndex(filterId);
      massNumber->blockSignals(oldState);
    }
  };
  connect(static_cast<NuclideTableScene*>(nuclideTable->scene()),
    &NuclideTableScene::nuclideSelected, setSymbol);

  // When a nuclide symbol is selected from the combo box, update the mass
  // number combo box to show the available mass numbers for this isotope
  // family.
  auto setMassNumberList = [=](const QString& smbl) {
    massNumber->clear();
    auto massNumbers = nuclideTable->massNumbers(smbl);
    std::set<unsigned int> massNumbersSet;
    for (auto& mn : massNumbers)
    {
      massNumbersSet.insert(mn);
    }
    for (auto& mn : massNumbersSet)
    {
      massNumber->addItem(QString::number(mn));
    }
  };
  connect(symbol, (void (QComboBox::*)(const QString&)) & QComboBox::currentIndexChanged,
    setMassNumberList);

  // When a nuclide is selected from the visual display, update the combo box to
  // reflect the selection.
  auto setMassNumber = [=](Nuclide* nuclide) {
    int filterId = massNumber->findText(QString::number(nuclide->A()));
    if (filterId != -1)
    {
      bool oldState = massNumber->blockSignals(true);
      massNumber->setCurrentIndex(filterId);
      massNumber->blockSignals(oldState);
    }
  };
  connect(static_cast<NuclideTableScene*>(nuclideTable->scene()),
    &NuclideTableScene::nuclideSelected, setMassNumber);

  // When a nuclide is selected from the combo box, update the visual display to
  // reflect the selection.
  auto highlightNuclide = [=](const QString& massNumberValue) {
    Nuclide* nuclide = nuclideTable->nuclide(symbol->currentText().toLower() + massNumberValue);
    if (nuclide)
    {
      this->view()->centerOn(nuclide->pos());
      nuclide->setSelected(true);
      emit static_cast<NuclideTableScene*>(nuclideTable->scene())->nuclideSelected(nuclide);
    }
  };

  connect(massNumber, (void (QComboBox::*)(const QString&)) & QComboBox::currentIndexChanged,
    highlightNuclide);

  int filterId = symbol->findText("H");
  if (filterId != -1)
  {
    symbol->setCurrentIndex(filterId);
  }

  label2 = new QLabel(tr("Pointer Mode"));
  selectModeButton = new QToolButton;
  selectModeButton->setText(tr("Select"));
  selectModeButton->setCheckable(true);
  selectModeButton->setChecked(true);
  dragModeButton = new QToolButton;
  dragModeButton->setText(tr("Drag"));
  dragModeButton->setCheckable(true);
  dragModeButton->setChecked(false);

  QButtonGroup* pointerModeGroup = new QButtonGroup(this);
  pointerModeGroup->setExclusive(true);
  pointerModeGroup->addButton(selectModeButton);
  pointerModeGroup->addButton(dragModeButton);

  labelLayout->addWidget(label);
  labelLayout->addWidget(symbol);
  labelLayout->addWidget(massNumber);
  labelLayout->addStretch();
  labelLayout->addWidget(label2);
  labelLayout->addWidget(selectModeButton);
  labelLayout->addWidget(dragModeButton);

  QGridLayout* topLayout = new QGridLayout;
  topLayout->addLayout(labelLayout, 0, 0);
  topLayout->addWidget(graphicsView, 1, 0);
  topLayout->addLayout(zoomSliderLayout, 1, 1);
  setLayout(topLayout);

  connect(zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(setupMatrix()));
  connect(selectModeButton, SIGNAL(toggled(bool)), this, SLOT(togglePointerMode()));
  connect(dragModeButton, SIGNAL(toggled(bool)), this, SLOT(togglePointerMode()));
  connect(zoomInIcon, SIGNAL(clicked()), this, SLOT(zoomIn()));
  connect(zoomOutIcon, SIGNAL(clicked()), this, SLOT(zoomOut()));

  setupMatrix();
}

QGraphicsView* NuclideTableView::view() const
{
  return static_cast<QGraphicsView*>(graphicsView);
}

void NuclideTableView::setupMatrix()
{
  qreal scale = qPow(qreal(2), (zoomSlider->value() - 250) / qreal(50));

  QMatrix matrix;
  matrix.scale(scale, scale);

  graphicsView->setMatrix(matrix);
}

void NuclideTableView::togglePointerMode()
{
  graphicsView->setDragMode(
    selectModeButton->isChecked() ? QGraphicsView::NoDrag : QGraphicsView::ScrollHandDrag);
  graphicsView->setInteractive(selectModeButton->isChecked());
}

void NuclideTableView::zoomIn(int level)
{
  zoomSlider->setValue(zoomSlider->value() + level);
}

void NuclideTableView::zoomOut(int level)
{
  zoomSlider->setValue(zoomSlider->value() - level);
}
}
}
}
