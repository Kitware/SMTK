//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/diagram/qtSelectMode.h"

#include "smtk/extension/qt/diagram/qtBaseObjectNode.h"
#include "smtk/extension/qt/diagram/qtDiagram.h"
#include "smtk/extension/qt/diagram/qtDiagramScene.h"
#include "smtk/extension/qt/diagram/qtDiagramView.h"
#include "smtk/extension/qt/diagram/qtDiagramViewConfiguration.h"
#include "smtk/extension/qt/qtUtility.h"

#include "smtk/view/Selection.h"

#include "smtk/Regex.h"

#include "smtk/view/icons/mode_selection_cpp.h"

#include <QAction>
#include <QActionGroup>
#include <QColor>
#include <QIcon>
#include <QKeyEvent>

using namespace smtk::string::literals;

namespace smtk
{
namespace extension
{

qtSelectMode::qtSelectMode(
  qtDiagram* diagram,
  qtDiagramView* view,
  QToolBar* toolbar,
  QActionGroup* modeGroup)
  : Superclass("select", diagram, toolbar, modeGroup)
{
  (void)view;
  QColor background = toolbar->palette().window().color();
  QIcon selectionIcon(colorAdjustedIcon(mode_selection_svg(), background));
  this->modeAction()->setIcon(selectionIcon);
}

qtSelectMode::~qtSelectMode() = default;

void qtSelectMode::enableSelectionSensitiveActions()
{
  auto nodes = m_diagram->diagramScene()->nodesOfSelection();
  this->changeSelectionSensitiveActions(nodes.size() > 1);
}

bool qtSelectMode::eventFilter(QObject* obj, QEvent* event)
{
  if (obj == m_diagram->diagramWidget())
  {
    if (this->isModeActive())
    {
      if (event->type() == QEvent::KeyPress)
      {
        auto* keyEvent = dynamic_cast<QKeyEvent*>(event);
        if (keyEvent)
        {
          switch (keyEvent->key())
          {
            case Qt::Key_Shift:
              m_diagram->diagramWidget()->addModeSnapback(Qt::Key_Shift, "pan"_token);
              break;
            case Qt::Key_Escape:
              m_diagram->requestModeChange(m_diagram->defaultMode());
              break;
            case Qt::Key_Backspace:
            case Qt::Key_Delete:
              this->removeSelectedObjects();
              break;
          }
        }
      }
    }
    return false;
  }
  return this->Superclass::eventFilter(obj, event);
}

void qtSelectMode::enterMode()
{
  m_diagram->diagramWidget()->setDragMode(QGraphicsView::RubberBandDrag);

  // When in connect mode, grab certain events from the view widget.
  m_diagram->diagramWidget()->installEventFilter(this);

  // Add align/distribute/autolayout buttons to toolbar and show them.
  this->addModeButtons();
  this->showModeButtons(true);
}

void qtSelectMode::exitMode()
{
  // Hide toolbar buttons specific to this mode:
  this->showModeButtons(false);

  // When not in connect mode, do not process events from the view widget.
  m_diagram->diagramWidget()->removeEventFilter(this);

  m_diagram->diagramWidget()->setDragMode(QGraphicsView::ScrollHandDrag);
}

void qtSelectMode::alignLeft()
{
  m_diagram->diagramScene()->alignHorizontal(m_diagram->diagramScene()->nodesOfSelection(), -1);
}

void qtSelectMode::alignHCenter()
{
  m_diagram->diagramScene()->alignHorizontal(m_diagram->diagramScene()->nodesOfSelection(), 0);
}

void qtSelectMode::alignRight()
{
  m_diagram->diagramScene()->alignHorizontal(m_diagram->diagramScene()->nodesOfSelection(), +1);
}

void qtSelectMode::alignTop()
{
  m_diagram->diagramScene()->alignVertical(m_diagram->diagramScene()->nodesOfSelection(), -1);
}

void qtSelectMode::alignVCenter()
{
  m_diagram->diagramScene()->alignVertical(m_diagram->diagramScene()->nodesOfSelection(), 0);
}

void qtSelectMode::alignBottom()
{
  m_diagram->diagramScene()->alignVertical(m_diagram->diagramScene()->nodesOfSelection(), +1);
}

void qtSelectMode::distributeHCenters()
{
  m_diagram->diagramScene()->distributeHorizontal(
    m_diagram->diagramScene()->nodesOfSelection(), "centers"_token);
}

void qtSelectMode::distributeHGaps()
{
  m_diagram->diagramScene()->distributeHorizontal(
    m_diagram->diagramScene()->nodesOfSelection(), "gaps"_token);
}

void qtSelectMode::distributeVCenters()
{
  m_diagram->diagramScene()->distributeVertical(
    m_diagram->diagramScene()->nodesOfSelection(), "centers"_token);
}

void qtSelectMode::distributeVGaps()
{
  m_diagram->diagramScene()->distributeVertical(
    m_diagram->diagramScene()->nodesOfSelection(), "gaps"_token);
}

void qtSelectMode::layoutSelectedNodesWithTheirArcs()
{
  std::unordered_set<qtBaseNode*> nodesToLayOut = m_diagram->diagramScene()->nodesOfSelection();
  std::unordered_set<qtBaseArc*> arcsToLayOut;
  for (const auto& node : nodesToLayOut)
  {
    auto arcs = m_diagram->arcsOfNode(node);
    for (const auto& arc : arcs)
    {
      if (
        nodesToLayOut.find(arc->predecessor()) != nodesToLayOut.end() &&
        nodesToLayOut.find(arc->successor()) != nodesToLayOut.end())
      {
        arcsToLayOut.insert(arc);
      }
    }
  }
  m_diagram->diagramScene()->computeLayout(nodesToLayOut, arcsToLayOut);
}

void qtSelectMode::addModeButtons()
{
  if (m_alignLeft)
  {
    return;
  }

  m_alignLeft = new QAction("Align left edges");
  m_alignLeft->setObjectName("AlignLf");
  m_alignLeft->setIcon(QIcon(":/icons/diagram/align_left.svg"));
  m_diagram->tools()->addAction(m_alignLeft);
  QObject::connect(m_alignLeft, &QAction::triggered, this, &qtSelectMode::alignLeft);

  m_alignHCenter = new QAction("Align horizontal centers");
  m_alignHCenter->setObjectName("AlignHC");
  m_alignHCenter->setIcon(QIcon(":/icons/diagram/align_hcenter.svg"));
  m_diagram->tools()->addAction(m_alignHCenter);
  QObject::connect(m_alignHCenter, &QAction::triggered, this, &qtSelectMode::alignHCenter);

  m_alignRight = new QAction("Align right edges");
  m_alignRight->setObjectName("AlignRt");
  m_alignRight->setIcon(QIcon(":/icons/diagram/align_right.svg"));
  m_diagram->tools()->addAction(m_alignRight);
  QObject::connect(m_alignRight, &QAction::triggered, this, &qtSelectMode::alignRight);

  auto* spacer = new QFrame;
  spacer->setObjectName("DiagramAlignHVSpacer");
  spacer->setFrameShape(QFrame::NoFrame);
  spacer->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
  spacer->setMinimumSize(15, 1);
  m_diagram->tools()->addWidget(spacer);

  m_alignTop = new QAction("Align top edges");
  m_alignTop->setObjectName("AlignTp");
  m_alignTop->setIcon(QIcon(":/icons/diagram/align_top.svg"));
  m_diagram->tools()->addAction(m_alignTop);
  QObject::connect(m_alignTop, &QAction::triggered, this, &qtSelectMode::alignTop);

  m_alignVCenter = new QAction("Align vertical centers");
  m_alignVCenter->setObjectName("AlignVC");
  m_alignVCenter->setIcon(QIcon(":/icons/diagram/align_vcenter.svg"));
  m_diagram->tools()->addAction(m_alignVCenter);
  QObject::connect(m_alignVCenter, &QAction::triggered, this, &qtSelectMode::alignVCenter);

  m_alignBottom = new QAction("Align bottom edges");
  m_alignBottom->setObjectName("AlignBt");
  m_alignBottom->setIcon(QIcon(":/icons/diagram/align_bottom.svg"));
  m_diagram->tools()->addAction(m_alignBottom);
  QObject::connect(m_alignBottom, &QAction::triggered, this, &qtSelectMode::alignBottom);

  spacer = new QFrame;
  spacer->setObjectName("DiagramDistributeVSpacer");
  spacer->setFrameShape(QFrame::NoFrame);
  spacer->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
  spacer->setMinimumSize(15, 1);
  m_diagram->tools()->addWidget(spacer);

  m_distributeHGaps = new QAction("Distribute horizontally with eqal gaps");
  m_distributeHGaps->setObjectName("DistributeHG");
  m_distributeHGaps->setIcon(QIcon(":/icons/diagram/distribute_hgaps.svg"));
  m_diagram->tools()->addAction(m_distributeHGaps);
  QObject::connect(m_distributeHGaps, &QAction::triggered, this, &qtSelectMode::distributeHGaps);

  m_distributeHCenters = new QAction("Distribute centers horizontally");
  m_distributeHCenters->setObjectName("DistributeHC");
  m_distributeHCenters->setIcon(QIcon(":/icons/diagram/distribute_hcenters.svg"));
  m_diagram->tools()->addAction(m_distributeHCenters);
  QObject::connect(
    m_distributeHCenters, &QAction::triggered, this, &qtSelectMode::distributeHCenters);

  spacer = new QFrame;
  spacer->setObjectName("DiagramDistributeVSpacer");
  spacer->setFrameShape(QFrame::NoFrame);
  spacer->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
  spacer->setMinimumSize(15, 1);
  m_diagram->tools()->addWidget(spacer);

  m_distributeVGaps = new QAction("Distribute vertically with equal gaps");
  m_distributeVGaps->setObjectName("DistributeVG");
  m_distributeVGaps->setIcon(QIcon(":/icons/diagram/distribute_vgaps.svg"));
  m_diagram->tools()->addAction(m_distributeVGaps);
  QObject::connect(m_distributeVGaps, &QAction::triggered, this, &qtSelectMode::distributeVGaps);

  m_distributeVCenters = new QAction("Distribute vertical centers");
  m_distributeVCenters->setObjectName("DistributeVC");
  m_distributeVCenters->setIcon(QIcon(":/icons/diagram/distribute_vcenters.svg"));
  m_diagram->tools()->addAction(m_distributeVCenters);
  QObject::connect(
    m_distributeVCenters, &QAction::triggered, this, &qtSelectMode::distributeVCenters);

  spacer = new QFrame;
  spacer->setObjectName("DiagramReLayOutVSpacer");
  spacer->setFrameShape(QFrame::NoFrame);
  spacer->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
  spacer->setMinimumSize(15, 1);
  m_diagram->tools()->addWidget(spacer);

  m_relayoutNodes = new QAction("Lay out nodes using their arcs");
  m_relayoutNodes->setObjectName("ReLayOut");
  m_relayoutNodes->setIcon(QIcon(":/icons/diagram/relayout.svg"));
  m_diagram->tools()->addAction(m_relayoutNodes);
  QObject::connect(
    m_relayoutNodes, &QAction::triggered, this, &qtSelectMode::layoutSelectedNodesWithTheirArcs);

  QObject::connect(
    m_diagram->diagramScene(),
    &QGraphicsScene::selectionChanged,
    this,
    &qtSelectMode::enableSelectionSensitiveActions);

  // Force button state to match current selection:
  this->enableSelectionSensitiveActions();
}

void qtSelectMode::showModeButtons(bool show)
{
  if (!m_alignLeft)
  {
    return;
  }

  m_alignLeft->setVisible(show);
  m_alignHCenter->setVisible(show);
  m_alignRight->setVisible(show);

  m_alignTop->setVisible(show);
  m_alignVCenter->setVisible(show);
  m_alignBottom->setVisible(show);

  m_distributeHCenters->setVisible(show);
  m_distributeHGaps->setVisible(show);

  m_distributeVCenters->setVisible(show);
  m_distributeVGaps->setVisible(show);

  m_relayoutNodes->setVisible(show);
}

void qtSelectMode::changeSelectionSensitiveActions(bool enable)
{
  if (!m_alignLeft)
  {
    return;
  }

  m_alignLeft->setEnabled(enable);
  m_alignHCenter->setEnabled(enable);
  m_alignRight->setEnabled(enable);

  m_alignTop->setEnabled(enable);
  m_alignVCenter->setEnabled(enable);
  m_alignBottom->setEnabled(enable);

  m_distributeHCenters->setEnabled(enable);
  m_distributeHGaps->setEnabled(enable);

  m_distributeVCenters->setEnabled(enable);
  m_distributeVGaps->setEnabled(enable);

  m_relayoutNodes->setEnabled(enable);
}

} // namespace extension
} // namespace smtk
