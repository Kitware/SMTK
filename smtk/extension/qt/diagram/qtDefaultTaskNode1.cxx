//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/diagram/qtDefaultTaskNode1.h"

#include "smtk/extension/qt/diagram/qtDiagram.h"
#include "smtk/extension/qt/diagram/qtDiagramGenerator.h"
#include "smtk/extension/qt/diagram/qtDiagramScene.h"
#include "smtk/extension/qt/diagram/qtDiagramViewConfiguration.h"
#include "smtk/extension/qt/diagram/qtTaskEditor.h"
#include "smtk/extension/qt/qtBaseView.h"

#include "smtk/task/Active.h"
#include "smtk/task/Manager.h"
#include "smtk/task/Task.h"

#include <QAction>
#include <QApplication>
#include <QColor>
#include <QEvent>
#include <QGraphicsProxyWidget>
#include <QGraphicsTextItem>
#include <QIcon>
#include <QLabel>
#include <QLayout>
#include <QMenu>
#include <QPainter>

#include "task/ui_DefaultTaskNode1.h"

class QAbstractItemModel;
class QItemSelection;
class QTreeView;

namespace
{

template<typename F>
/**
 * Intercept all events from a particular QObject and process them using the
 * given @c functor. This is usually used with the QObjects::installEventFilter()
 * function.
 */
class Interceptor final : public QObject
{
public:
  /**
   * Create an Interceptor that process all events of @c parent using @c functor.
   */
  Interceptor(QObject* parent, F fn)
    : QObject(parent)
    , functor(fn)
  {
  }
  ~Interceptor() override = default;

protected:
  /**
   * Filters events if this object has been installed as an event filter for the watched object.
   */
  bool eventFilter(QObject* object, QEvent* event) override { return this->functor(object, event); }

  F functor;
};

/**
 * Create a new Interceptor instance.
 */
template<typename F>
Interceptor<F>* createInterceptor(QObject* parent, F functor)
{
  return new Interceptor<F>(parent, functor);
};

} // namespace

namespace smtk
{
namespace extension
{

class DefaultTaskNodeWidget1
  : public QWidget
  , public Ui::DefaultTaskNode1
{
public:
  DefaultTaskNodeWidget1(qtDefaultTaskNode1* node, QWidget* parent = nullptr)
    : QWidget(parent)
    , m_node(node)
  {
    this->setupUi(this);
    m_nodeMenu = new QMenu(m_headlineButton);
    m_nodeMenu->setObjectName("contextMenu");
    m_activateTask = new QAction("Work on this");
    m_activateTask->setObjectName("activateAction");
    m_expandTask = new QAction("Show controls");
    m_expandTask->setObjectName("expandAction");
    m_toggleCompletion = new QAction("Mark completed");
    m_toggleCompletion->setObjectName("toggleCompletionAction");
    m_nodeMenu->addAction(m_activateTask);
    m_nodeMenu->addAction(m_expandTask);
    m_nodeMenu->addAction(m_toggleCompletion);
    m_toggleCompletion->setEnabled(false);
    m_headlineButton->setMenu(m_nodeMenu);
    QObject::connect(
      m_activateTask, &QAction::triggered, this, &DefaultTaskNodeWidget1::activateTask);
    QObject::connect(
      m_toggleCompletion, &QAction::triggered, this, &DefaultTaskNodeWidget1::markCompleted);
    QObject::connect(
      m_expandTask, &QAction::triggered, this, &DefaultTaskNodeWidget1::toggleControls);
    m_taskObserver = m_node->task()->observers().insert(
      [this](smtk::task::Task&, smtk::task::State prev, smtk::task::State next) {
        // Sometimes the application invokes this observer after the GUI
        // has been shut down. Calling setEnabled on widgets generates a
        // log message and attempting to construct a QPixmap throws exceptions;
        // so, check that qApp exists before going further.
        if (qApp)
        {
          this->updateTaskState(prev, next, m_node->isActive());
        }
      },
      "DefaultTaskNodeWidget1 observer");
    this->updateTaskState(m_node->m_task->state(), m_node->m_task->state(), m_node->isActive());
  }

  void updateTaskState(smtk::task::State prev, smtk::task::State next, bool active)
  {
    // The node's superclass updates the tooltip for us.
    m_node->updateTaskState(prev, next, active);

    auto& cfg = *m_node->scene()->configuration();
    QPalette p = this->palette();
    QColor newStateColor;
    newStateColor = cfg.colorForState(next);
    m_toggleCompletion->setText("Mark completed");

    p.setColor(QPalette::Window, newStateColor);
    this->setPalette(p);
    switch (next)
    {
      case smtk::task::State::Irrelevant:
        m_headlineButton->setEnabled(false);
        break;
      case smtk::task::State::Unavailable:
        m_headlineButton->setEnabled(false);
        m_activateTask->setEnabled(false);
        break;
      case smtk::task::State::Incomplete:
        m_headlineButton->setEnabled(true);
        m_activateTask->setEnabled(!m_node->isActive());
        m_toggleCompletion->setEnabled(false);
        break;
      case smtk::task::State::Completable:
        m_headlineButton->setEnabled(true);
        m_activateTask->setEnabled(!m_node->isActive());
        m_toggleCompletion->setEnabled(true);
        break;
      case smtk::task::State::Completed:
        m_headlineButton->setEnabled(true);
        m_activateTask->setEnabled(false);
        m_toggleCompletion->setEnabled(true);
        m_toggleCompletion->setText("Undo completion");
        break;
    }
    m_headlineButton->setToolTip(QString::fromStdString("Status: " + smtk::task::stateName(next)));
    m_headlineButton->setIcon(this->renderStatusIcon(next, m_headlineButton->height() / 2));
  }

  void activateTask()
  {
    auto* taskManager = m_node->m_task->manager();
    if (taskManager)
    {
      auto* previouslyActive = taskManager->active().task();
      if (previouslyActive != m_node->m_task)
      {
        if (taskManager->active().switchTo(m_node->m_task))
        {
          m_activateTask->setEnabled(false); // Can't activate task since it is now active.
          auto* prevNode = (m_node->scene() && m_node->scene()->diagram())
            ? dynamic_cast<qtBaseTaskNode*>(
                m_node->scene()->diagram()->findNode(previouslyActive->id()))
            : nullptr;
          if (prevNode)
          {
            smtk::task::State prevState = previouslyActive->state();
            prevNode->updateTaskState(prevState, prevState, false);
          }
        }
        // TODO: Provide feedback if no action taken (e.g., flash red)
      }
    }
  }

  void markCompleted()
  {
    m_node->m_task->markCompleted(m_node->m_task->state() == smtk::task::State::Completable);
    // TODO: Provide feedback if no action taken (e.g., flash red)
  }

  QIcon renderStatusIcon(smtk::task::State state, int radius)
  {
    if (radius < 10)
    {
      radius = 10;
    }
    QPixmap pix(radius, radius);
    pix.fill(QColor(0, 0, 0, 0));

    auto& cfg = *m_node->scene()->configuration();
    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush(QBrush(cfg.colorForState(state)));
    painter.drawEllipse(1, 1, radius - 2, radius - 2);
    painter.end();
    return QIcon(pix);
  }

  void toggleControls()
  {
    bool shouldShow = !m_controls->isVisible();
    m_controls->setVisible(shouldShow);
    m_expandTask->setText(shouldShow ? "Hide controls" : "Show controls");
  }

  QMenu* m_nodeMenu;
  QAction* m_activateTask;
  QAction* m_expandTask;
  QAction* m_toggleCompletion;
  qtDefaultTaskNode1* m_node;
  smtk::task::Task::Observers::Key m_taskObserver;
};

qtDefaultTaskNode1::qtDefaultTaskNode1(
  qtDiagramGenerator* generator,
  smtk::task::Task* task,
  QGraphicsItem* parent)
  : Superclass(generator, task, parent)
  , m_container(new DefaultTaskNodeWidget1(this))
{
  qtDiagramViewConfiguration& cfg(*this->scene()->configuration());
  // Create a container to hold node contents
  {
    m_container->setObjectName("nodeContainer");
    m_container->setMinimumWidth(cfg.nodeWidth());

    // install resize event filter
    m_container->installEventFilter(
      createInterceptor(m_container, [this](QObject* /*object*/, QEvent* event) {
        if (event->type() == QEvent::LayoutRequest)
        {
          this->updateSize();
        }
        return false;
      }));

    auto* graphicsProxyWidget = new QGraphicsProxyWidget(this);
    graphicsProxyWidget->setObjectName("graphicsProxyWidget");
    graphicsProxyWidget->setWidget(m_container);
    graphicsProxyWidget->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    graphicsProxyWidget->setPos(QPointF(0, 0));

    m_container->m_headlineButton->setText(QString::fromStdString(m_task->name()));
    m_container->m_controls->hide();
  }

  this->addToScene();

  // === Task-specific constructor ===
  this->setZValue(cfg.nodeLayer());
}

void qtDefaultTaskNode1::setContentStyle(ContentStyle cs)
{
  m_contentStyle = cs;
  switch (cs)
  {
    case ContentStyle::Minimal:
      m_container->hide();
      break;
    case ContentStyle::Summary:
    case ContentStyle::Details:
      m_container->show();
      break;
    default:
      break;
  }
}

void qtDefaultTaskNode1::setOutlineStyle(OutlineStyle os)
{
  qtDiagramViewConfiguration& cfg(*this->scene()->configuration());
  m_outlineStyle = os;
  this->setZValue(cfg.nodeLayer() - (os == OutlineStyle::Normal ? 0 : 1));
  this->update(this->boundingRect());
}

QRectF qtDefaultTaskNode1::boundingRect() const
{
  qtDiagramViewConfiguration& cfg(*this->scene()->configuration());
  const auto border = 4 * cfg.nodeBorderThickness();
  const double height = m_container->height();
  // was = m_headlineHeight + (m_container->isVisible() ? m_container->height() : 0.0);
  return QRectF(0, 0, m_container->width(), height).adjusted(-border, -border, border, border);
}

void qtDefaultTaskNode1::paint(
  QPainter* painter,
  const QStyleOptionGraphicsItem* option,
  QWidget* widget)
{
  (void)option;
  (void)widget;
  qtDiagramViewConfiguration& cfg(*this->scene()->configuration());
  QPainterPath path;
  // Make sure the whole node is redrawn to avoid artifacts:
  const double borderOffset = cfg.nodeBorderThickness();
  const QRectF br =
    this->boundingRect().adjusted(borderOffset, borderOffset, -borderOffset, -borderOffset);
  path.addRoundedRect(br, 2 * cfg.nodeBorderThickness(), 2 * cfg.nodeBorderThickness());

  const QColor baseColor = QApplication::palette().window().color();
  const QColor highlightColor = QApplication::palette().highlight().color();
  const QColor contrastColor = QColor::fromHslF(
    baseColor.hueF(),
    baseColor.saturationF(),
    baseColor.lightnessF() > 0.5 ? baseColor.lightnessF() - 0.5 : baseColor.lightnessF() + 0.5);
  // const QColor greenBaseColor = QColor::fromHslF(0.361, 0.666, baseColor.lightnessF() * 0.4 + 0.2);

  QPen pen;
  pen.setWidth(cfg.nodeBorderThickness() * 1.1);
  switch (m_outlineStyle)
  {
    case OutlineStyle::Normal:
      pen.setBrush(contrastColor);
      break;
    case OutlineStyle::Active:
      pen.setBrush(highlightColor);
      break;
    default:
      break;
  }

  painter->setPen(pen);
  painter->fillPath(path, cfg.colorForState(m_task->state()));
  painter->drawPath(path);

  if (this->isSelected())
  {
    QPen spen;
    spen.setWidth(cfg.nodeBorderThickness());
    QPainterPath selPath;
    const QColor contrastColor2 = QColor::fromHslF(
      pen.brush().color().hueF(),
      pen.brush().color().saturationF(),
      qBound(
        0.,
        baseColor.lightnessF() > 0.5 ? pen.brush().color().lightnessF() + 0.25
                                     : pen.brush().color().lightnessF() - 0.25,
        1.));
    spen.setBrush(contrastColor2);
    painter->setPen(spen);
    const QRectF selRect = br.adjusted(borderOffset, borderOffset, -borderOffset, -borderOffset);
    selPath.addRoundedRect(selRect, cfg.nodeBorderThickness(), cfg.nodeBorderThickness());
    painter->drawPath(selPath);
  }
}

int qtDefaultTaskNode1::updateSize()
{
  this->prepareGeometryChange();

  m_container->resize(m_container->layout()->sizeHint());
  Q_EMIT this->nodeResized();

  return 1;
}

void qtDefaultTaskNode1::updateTaskState(
  smtk::task::State prev,
  smtk::task::State next,
  bool active)
{
  m_container->updateTaskState(prev, next, active);
}

void qtDefaultTaskNode1::dataUpdated()
{
  m_container->m_headlineButton->setText(QString::fromStdString(m_task->name()));
}

} // namespace extension
} // namespace smtk
