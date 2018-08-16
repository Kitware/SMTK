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

  // Create the nuclide line edit autocompletion list
  QStringList nuclideLineEditAutoCompleteList;
  for (const auto& symbol : nuclideTable->symbols())
  {
    for (const auto& massNumber : nuclideTable->massNumbers(symbol))
      nuclideLineEditAutoCompleteList.append(symbol + QString::number(massNumber));
  }

  // This will sort the auto complete entries in the order we want
  nuclideLineEditAutoCompleteList.sort();

  QLineEdit* nuclideLineEdit = new QLineEdit(this);
  QCompleter* nuclideLineEditCompleter =
    new QCompleter(nuclideLineEditAutoCompleteList, nuclideLineEdit);
  nuclideLineEditCompleter->setModelSorting(QCompleter::UnsortedModel);
  nuclideLineEditCompleter->setCaseSensitivity(Qt::CaseInsensitive);
  nuclideLineEdit->setCompleter(nuclideLineEditCompleter);

  // Let's add a tooltip
  const QString& toolTip("Begin typing the symbol of a nuclide, and auto-"
                         "complete entries will appear.\nWhen a valid nuclide "
                         "symbol and mass have been entered, that nuclide will "
                         "be selected automatically.");
  nuclideLineEdit->setToolTip(toolTip);

  // Set some fixed width
  nuclideLineEdit->setFixedWidth(nuclideLineEdit->fontMetrics().width("12345") * 1.75);

  // When the nuclide line edit is modified, check to see if it is a valid
  // nuclide. If so, change it.
  auto nuclideLineEditHighlightNuclide = [nuclideTable, this](const QString& lineEditText) {
    Nuclide* nuclide = nuclideTable->nuclide(lineEditText.toLower());
    if (nuclide)
    {
      this->view()->centerOn(nuclide->pos());
      nuclide->setSelected(true);
      emit qobject_cast<NuclideTableScene*>(nuclideTable->scene())->nuclideSelected(nuclide);
    }
  };
  connect(nuclideLineEdit, &QLineEdit::textChanged, this, nuclideLineEditHighlightNuclide);

  // If a nuclide is selected elsewhere, update the line edit text
  auto nuclideLineEditUpdateNuclide = [nuclideLineEdit](Nuclide* nuclide) {
    if (nuclide)
    {
      bool prevBlockSignals = nuclideLineEdit->blockSignals(true);
      nuclideLineEdit->setText(nuclide->symbol() + QString::number(nuclide->A()));
      nuclideLineEdit->blockSignals(prevBlockSignals);
    }
  };
  connect(qobject_cast<NuclideTableScene*>(nuclideTable->scene()),
    &NuclideTableScene::nuclideSelected, this, nuclideLineEditUpdateNuclide);

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
  labelLayout->addWidget(nuclideLineEdit);
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
