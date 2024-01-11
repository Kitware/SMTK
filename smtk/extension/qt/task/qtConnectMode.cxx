//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/task/qtConnectMode.h"

#include "smtk/extension/qt/qtUtility.h"
#include "smtk/extension/qt/task/qtBaseTaskNode.h"
#include "smtk/extension/qt/task/qtPreviewArc.h"
#include "smtk/extension/qt/task/qtTaskEditor.h"
#include "smtk/extension/qt/task/qtTaskView.h"
#include "smtk/extension/qt/task/qtTaskViewConfiguration.h"

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
  const std::shared_ptr<smtk::operation::Manager>& operationManager,
  qtTaskEditor* editor,
  qtTaskView* view,
  QToolBar* toolbar,
  QActionGroup* modeGroup)
  : Superclass("connect", editor, toolbar, modeGroup)
  , m_operationManager(operationManager)
{
  (void)view;
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
}

qtConnectMode::~qtConnectMode()
{
  // Explicitly remove the preview from the scene before our destructor to
  // prevent dereferencing stale pointers.
  delete m_previewArc;
}

bool qtConnectMode::eventFilter(QObject* obj, QEvent* event)
{
  auto* taskView = m_editor->taskWidget();
  if (obj == taskView)
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
  else if (obj == taskView->viewport())
  {
    switch (event->type())
    {
      case QEvent::MouseMove:
      {
        auto* mouseEvent = dynamic_cast<QMouseEvent*>(event);
        if (mouseEvent)
        {
          this->hoverConnectNode(
            dynamic_cast<smtk::extension::qtBaseTaskNode*>(taskView->itemAt(mouseEvent->pos())));
        }
      }
      break;
      case QEvent::MouseButtonPress:
      {
        auto* mouseEvent = dynamic_cast<QMouseEvent*>(event);
        if (mouseEvent)
        {
          this->clickConnectNode(
            dynamic_cast<smtk::extension::qtBaseTaskNode*>(taskView->itemAt(mouseEvent->pos())));
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

void qtConnectMode::sceneCleared()
{
  smtk::string::Token arcType;
  std::string arcOp;
  if (m_connectType)
  {
    int idx = m_connectType->currentIndex();
    if (idx >= 0)
    {
      arcType = m_connectType->itemText(idx).toStdString();
      arcOp = m_connectType->itemData(idx).toString().toStdString();
    }
  }
  m_previewArc = new qtPreviewArc(m_editor->taskScene(), m_operationManager, arcType, arcOp);
}

void qtConnectMode::enterMode()
{
  m_connectTypeAction->setVisible(true);

  // Disable tasks so arc-preview works.
  m_editor->enableTasks(false);

  // When in connect mode, grab certain events from the view widget
  // (for key events) and its viewport (for mouse events)
  m_editor->taskWidget()->installEventFilter(this);
  m_editor->taskWidget()->viewport()->installEventFilter(this);
}

void qtConnectMode::exitMode()
{
  // When not in connect mode, do not process events from the view widget.
  m_editor->taskWidget()->viewport()->removeEventFilter(this);
  m_editor->taskWidget()->removeEventFilter(this);

  // Enable tasks. (Next mode can re-disable them as needed.)
  m_editor->enableTasks(true);

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
    m_connectType->addItem(
      QString::fromStdString(entry.first), QString::fromStdString(it->typeName()));
  }

  if (currentIndex >= 0)
  {
    m_connectType->setCurrentIndex(currentIndex);
  }
}

void qtConnectMode::setConnectionType(int arcTypeItemIndex)
{
  smtk::string::Token arcType = m_connectType->itemText(arcTypeItemIndex).toStdString();
  auto arcOp = m_connectType->itemData(arcTypeItemIndex).toString().toStdString();
  // std::cout << "Arc type now " << arcType.data() << ", op " << arcOp << "\n";
  m_previewArc->setArcType(arcType, arcOp);
}

void qtConnectMode::hoverConnectNode(qtBaseTaskNode* node)
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

void qtConnectMode::clickConnectNode(qtBaseTaskNode* node)
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
    m_editor->requestModeChange(m_editor->defaultMode());
  }
}

} // namespace extension
} // namespace smtk
