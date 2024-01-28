//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/diagram/qtConnectMode.h"

#include "smtk/extension/qt/diagram/qtBaseObjectNode.h"
#include "smtk/extension/qt/diagram/qtDiagram.h"
#include "smtk/extension/qt/diagram/qtDiagramView.h"
#include "smtk/extension/qt/diagram/qtDiagramViewConfiguration.h"
#include "smtk/extension/qt/diagram/qtPreviewArc.h"
#include "smtk/extension/qt/qtUtility.h"

#include "smtk/operation/groups/ArcCreator.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"

#include "smtk/io/Logger.h"

#include "smtk/view/icons/mode_connection_cpp.h"

#include <QColor>
#include <QComboBox>
#include <QIcon>
#include <QKeyEvent>

using namespace smtk::string::literals;

namespace smtk
{
namespace extension
{

qtConnectMode::qtConnectMode(
  qtDiagram* diagram,
  qtDiagramView* view,
  QToolBar* toolbar,
  QActionGroup* modeGroup)
  : Superclass("connect", diagram, toolbar, modeGroup)
{
  (void)view;
  if (diagram->information().contains<smtk::common::Managers::Ptr>())
  {
    auto managers = diagram->information().get<smtk::common::Managers::Ptr>();
    if (managers && managers->contains<smtk::operation::Manager::Ptr>())
    {
      m_operationManager = managers->get<smtk::operation::Manager::Ptr>();
    }
  }
  QColor background = toolbar->palette().window().color();
  QIcon connectIcon(colorAdjustedIcon(mode_connection_svg(), background));
  this->modeAction()->setIcon(connectIcon);

  // Add a combo-box for selecting the type of arc to create
  m_connectType = new QComboBox(toolbar);
  m_connectType->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  m_connectType->setFixedHeight(toolbar->height());
  m_connectType->setEditable(false);
  m_connectType->setPlaceholderText("Choose an arc type");
  m_connectType->setObjectName("arcTypeCombo");
  m_connectType->setToolTip("Choose the type of arc to create");
  QPointer<qtConnectMode> scopedSelf(this);
  m_groupObserverKey = m_operationManager->groupObservers().insert(
    [scopedSelf, this](
      const smtk::operation::Operation::Index& operationIndex,
      const std::string& groupName,
      bool adding) {
      (void)operationIndex;
      (void)adding;
      if (!scopedSelf)
      {
        return;
      }
      if (groupName == smtk::operation::ArcCreator::type_name)
      {
        this->updateArcTypes();
      }
    });
  this->updateArcTypes();
  if (m_connectType->count())
  {
    m_connectType->setCurrentIndex(0);
  }

  // Listen for changes to the combobox and update the preview arc.
  QObject::connect(
    m_connectType.data(),
    QOverload<int>::of(&QComboBox::currentIndexChanged),
    this,
    &qtConnectMode::setConnectionType);

  // Add a combo-box for arc-type to the toolbar and hide it (assumes connection-mode is inactive).
  m_connectTypeAction = toolbar->addWidget(m_connectType);
  m_connectTypeAction->setVisible(false); // We are not in connect mode by default.
  // Insert the preview arc into the scene:
  this->sceneCleared(nullptr);
}

qtConnectMode::~qtConnectMode()
{
  // Explicitly remove the preview from the scene before our destructor to
  // prevent dereferencing stale pointers.
  delete m_previewArc;
}

bool qtConnectMode::eventFilter(QObject* obj, QEvent* event)
{
  auto* diagramView = m_diagram->diagramWidget();
  if (obj == diagramView)
  {
    switch (event->type())
    {
      case QEvent::KeyPress:
      {
        auto* keyEvent = dynamic_cast<QKeyEvent*>(event);
        if (keyEvent && keyEvent->key() == Qt::Key_Escape)
        {
          this->abandonConnection();
        }
      }
      break;
      default:
        break;
    }
  }
  else if (obj == diagramView->viewport())
  {
    switch (event->type())
    {
      case QEvent::MouseMove:
      {
        auto* mouseEvent = dynamic_cast<QMouseEvent*>(event);
        if (mouseEvent)
        {
          this->hoverConnectNode(dynamic_cast<smtk::extension::qtBaseObjectNode*>(
            diagramView->itemAt(mouseEvent->pos())));
        }
      }
      break;
      case QEvent::MouseButtonPress:
      {
        auto* mouseEvent = dynamic_cast<QMouseEvent*>(event);
        if (mouseEvent)
        {
          this->clickConnectNode(dynamic_cast<smtk::extension::qtBaseObjectNode*>(
            diagramView->itemAt(mouseEvent->pos())));
        }
      }
      break;
      default:
        break;
    }
    // Allow the view to use the events as well; do not consume them.
    return false;
  }
  return this->Superclass::eventFilter(obj, event);
}

void qtConnectMode::sceneCleared(qtDiagramGenerator* generator)
{
  if (generator)
  {
    return; // Only a portion of the scene was cleared, not the entire thing.
  }
  smtk::string::Token arcType;
  std::string arcOp;
  std::string arcDestinationItemName;
  if (m_connectType)
  {
    int idx = m_connectType->currentIndex();
    if (idx >= 0)
    {
      arcType = m_connectType->itemText(idx).toStdString();
      arcOp = m_connectType->itemData(idx).toString().toStdString();
      arcDestinationItemName =
        m_connectType->itemData(idx, Qt::UserRole + 1).toString().toStdString();
    }
  }
  m_previewArc = new qtPreviewArc(
    m_diagram->diagramScene(), m_operationManager, arcType, arcOp, arcDestinationItemName);
}

void qtConnectMode::enterMode()
{
  m_connectTypeAction->setVisible(true);

  // Disable tasks so arc-preview works.
  m_diagram->enableNodeSelection(false);

  // When in connect mode, grab certain events from the view widget
  // (for key events) and its viewport (for mouse events)
  m_diagram->diagramWidget()->installEventFilter(this);
  m_diagram->diagramWidget()->viewport()->installEventFilter(this);
}

void qtConnectMode::exitMode()
{
  // When not in connect mode, do not process events from the view widget.
  m_diagram->diagramWidget()->viewport()->removeEventFilter(this);
  m_diagram->diagramWidget()->removeEventFilter(this);

  // Enable tasks. (Next mode can re-disable them as needed.)
  m_diagram->enableNodeSelection(true);

  // Force the preview to render nothing when not in connect-mode.
  if (m_previewArc)
  {
    m_previewArc->setPredecessor(nullptr);
    m_previewArc->setSuccessor(nullptr);
  }

  m_connectTypeAction->setVisible(false);
}

void qtConnectMode::updateArcTypes()
{
  if (!m_operationManager)
  {
    return;
  }

  // Clear old arc types, remembering which one was active
  auto currentArcType = m_connectType->currentText().toStdString();
  while (m_connectType->count())
  {
    m_connectType->removeItem(0);
  }

  int currentIndex = -1;
  smtk::operation::ArcCreator arcCreators(m_operationManager);
  for (const auto& entry : arcCreators.allArcCreators())
  {
    const auto& meta = m_operationManager->metadata().get<smtk::operation::IndexTag>();
    auto it = meta.find(entry.second);
    if (it == meta.end())
    {
      continue;
    }
    if (entry.first == currentArcType && currentIndex < 0)
    {
      currentIndex = m_connectType->count();
    }
    int idx = m_connectType->count();
    m_connectType->addItem(
      QString::fromStdString(entry.first), QString::fromStdString(it->typeName()));
    auto destinationItemName = arcCreators.arcDestinationItemNameForOperation(entry.second);
    m_connectType->setItemData(idx, QString::fromStdString(destinationItemName), Qt::UserRole + 1);
  }

  if (currentIndex >= 0)
  {
    m_connectType->setCurrentIndex(currentIndex);
  }
}

void qtConnectMode::setConnectionType(int arcTypeItemIndex)
{
  smtk::string::Token arcType;
  auto selectedItemText = m_connectType->itemText(arcTypeItemIndex).toStdString();
  if (!selectedItemText.empty())
  {
    arcType = selectedItemText;
  }
  auto arcOp = m_connectType->itemData(arcTypeItemIndex).toString().toStdString();
  auto arcDestinationItemName =
    m_connectType->itemData(arcTypeItemIndex, Qt::UserRole + 1).toString().toStdString();
  m_previewArc->setArcType(arcType, arcOp, arcDestinationItemName);
}

void qtConnectMode::hoverConnectNode(qtBaseObjectNode* node)
{
  if (!node)
  {
    return;
  }

  if (!m_previewArc->isPredecessorConfirmed())
  {
    m_previewArc->setPredecessor(node);
  }
  else
  {
    m_previewArc->setSuccessor(node);
  }
}

void qtConnectMode::clickConnectNode(qtBaseObjectNode* node)
{
  if (!m_previewArc->isPredecessorConfirmed())
  {
    if (node)
    {
      m_previewArc->setPredecessor(node);
    }
    m_previewArc->confirmPredecessorNode();
  }
  else
  {
    if (node)
    {
      m_previewArc->setSuccessor(node);
    }
    m_previewArc->confirmSuccessorNode();
  }
}

void qtConnectMode::abandonConnection()
{
  // If we have a predecessor, abandon that.
  // If not, leave this mode altogether.
  if (m_previewArc->isPredecessorConfirmed())
  {
    m_previewArc->setPredecessor(nullptr);
  }
  else
  {
    m_diagram->requestModeChange(m_diagram->defaultMode());
  }
}

} // namespace extension
} // namespace smtk
