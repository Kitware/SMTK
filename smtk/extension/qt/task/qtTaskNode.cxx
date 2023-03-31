//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/task/qtTaskNode.h"

#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/extension/qt/task/qtTaskScene.h"
#include "smtk/extension/qt/task/qtTaskViewConfiguration.h"

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
#include <QTimer>

#include "task/ui_TaskNode.h"

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

class TaskNodeWidget
  : public QWidget
  , public Ui::TaskNode
{
public:
  TaskNodeWidget(qtTaskNode* node, QWidget* parent = nullptr)
    : QWidget(parent)
    , m_node(node)
  {
    this->setupUi(this);
    m_nodeMenu = new QMenu(m_headlineButton);
    m_activateTask = new QAction("Work on this");
    m_expandTask = new QAction("Show controls");
    m_markCompleted = new QAction("Mark completed");
    m_nodeMenu->addAction(m_activateTask);
    m_nodeMenu->addAction(m_expandTask);
    m_nodeMenu->addAction(m_markCompleted);
    m_markCompleted->setEnabled(false);
    m_headlineButton->setMenu(m_nodeMenu);
    QObject::connect(m_activateTask, &QAction::triggered, this, &TaskNodeWidget::activateTask);
    QObject::connect(m_markCompleted, &QAction::triggered, this, &TaskNodeWidget::markCompleted);
    QObject::connect(m_expandTask, &QAction::triggered, this, &TaskNodeWidget::toggleControls);
    m_taskObserver = m_node->task()->observers().insert(
      [this](smtk::task::Task&, smtk::task::State prev, smtk::task::State next) {
        // Sometimes the application invokes this observer after the GUI
        // has been shut down. Calling setEnabled on widgets generates a
        // log message and attempting to construct a QPixmap throws exceptions;
        // so, check that qApp exists before going further.
        if (qApp)
        {
          this->updateTaskState(prev, next);
        }
      },
      "TaskNodeWidget observer");
    this->updateTaskState(m_node->m_task->state(), m_node->m_task->state());
  }

  void updateTaskState(smtk::task::State prev, smtk::task::State next)
  {
    (void)prev;
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
        m_activateTask->setEnabled(true);
        m_markCompleted->setEnabled(false);
        break;
      case smtk::task::State::Completable:
        m_headlineButton->setEnabled(true);
        m_activateTask->setEnabled(true);
        m_markCompleted->setEnabled(true);
        break;
      case smtk::task::State::Completed:
        m_headlineButton->setEnabled(true);
        m_activateTask->setEnabled(false);
        m_markCompleted->setEnabled(true);
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
      taskManager->active().switchTo(m_node->m_task);
      // TODO: Provide feedback if no action taken (e.g., flash red)
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

    auto& cfg = *m_node->m_scene->configuration();
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
  QAction* m_markCompleted;
  qtTaskNode* m_node;
  smtk::task::Task::Observers::Key m_taskObserver;
};

qtTaskNode::qtTaskNode(qtTaskScene* scene, smtk::task::Task* task, QGraphicsItem* parent)
  : Superclass(parent)
  , m_scene(scene)
  , m_task(task)
  , m_container(new TaskNodeWidget(this))
{
  qtTaskViewConfiguration& cfg(*m_scene->configuration());
  // === Base constructor ===
  this->setFlag(GraphicsItemFlag::ItemIsMovable);
  this->setFlag(GraphicsItemFlag::ItemSendsGeometryChanges);
  this->setCacheMode(CacheMode::DeviceCoordinateCache);
  this->setCursor(Qt::ArrowCursor);
  this->setObjectName(QString("node") + QString::fromStdString(m_task->title()));

  // Create a container to hold node contents
  {
    m_container->setObjectName("nodeContainer");
    m_container->setMinimumWidth(cfg.nodeWidth());
    // m_container->setMaximumWidth(cfg.nodeWidth());

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

    m_container->m_headlineButton->setText(QString::fromStdString(m_task->title()));
    m_container->m_controls->hide();

    // Configure timer to rate-limit nodeMoved signal
    m_moveSignalTimer = new QTimer(this);
    m_moveSignalTimer->setSingleShot(true);
    m_moveSignalTimer->setInterval(100);
    QObject::connect(m_moveSignalTimer, &QTimer::timeout, this, &qtTaskNode::nodeMoved);
  }

  this->updateSize();
  m_scene->addItem(this);

  // === Task-specific constructor ===
  this->setZValue(cfg.nodeLayer());
}

qtTaskNode::~qtTaskNode()
{
  m_scene->removeItem(this);
}

void qtTaskNode::setContentStyle(ContentStyle cs)
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

void qtTaskNode::setOutlineStyle(OutlineStyle os)
{
  qtTaskViewConfiguration& cfg(*m_scene->configuration());
  m_outlineStyle = os;
  this->setZValue(cfg.nodeLayer() - (os == OutlineStyle::Normal ? 0 : 1));
  this->update(this->boundingRect());
}

QRectF qtTaskNode::boundingRect() const
{
  qtTaskViewConfiguration& cfg(*m_scene->configuration());
  const auto& border = cfg.nodeBorderThickness();
  const double height = m_container->height();
  // was = m_headlineHeight + (m_container->isVisible() ? m_container->height() : 0.0);
  return QRectF(0, 0, m_container->width(), height).adjusted(-border, -border, border, border);
}

QVariant qtTaskNode::itemChange(GraphicsItemChange change, const QVariant& value)
{
  if (change == GraphicsItemChange::ItemPositionHasChanged)
  {
    m_moveSignalTimer->start();
    Q_EMIT this->nodeMovedImmediate();
  }

  return QGraphicsItem::itemChange(change, value);
}

void qtTaskNode::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
  (void)option;
  (void)widget;
  qtTaskViewConfiguration& cfg(*m_scene->configuration());
  QPainterPath path;
  // Make sure the whole node is redrawn to avoid artifacts:
  const double borderOffset = 0.5 * cfg.nodeBorderThickness();
  const QRectF br =
    this->boundingRect().adjusted(borderOffset, borderOffset, -borderOffset, -borderOffset);
  path.addRoundedRect(br, cfg.nodeBorderThickness(), cfg.nodeBorderThickness());

  const QColor baseColor = QApplication::palette().window().color();
  const QColor highlightColor = QApplication::palette().highlight().color();
  const QColor contrastColor = QColor::fromHslF(
    baseColor.hueF(),
    baseColor.saturationF(),
    baseColor.lightnessF() > 0.5 ? baseColor.lightnessF() - 0.5 : baseColor.lightnessF() + 0.5);
  // const QColor greenBaseColor = QColor::fromHslF(0.361, 0.666, baseColor.lightnessF() * 0.4 + 0.2);

  QPen pen;
  pen.setWidth(cfg.nodeBorderThickness());
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
  painter->fillPath(path, baseColor);
  painter->drawPath(path);
}

int qtTaskNode::updateSize()
{
  this->prepareGeometryChange();

  m_container->resize(m_container->layout()->sizeHint());
  Q_EMIT this->nodeResized();

  return 1;
}

} // namespace extension
} // namespace smtk
