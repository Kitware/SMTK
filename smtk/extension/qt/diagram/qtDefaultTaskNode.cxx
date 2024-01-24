//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/diagram/qtDefaultTaskNode.h"

#include "smtk/extension/qt/diagram/qtDiagram.h"
#include "smtk/extension/qt/diagram/qtDiagramScene.h"
#include "smtk/extension/qt/diagram/qtDiagramView.h"
#include "smtk/extension/qt/diagram/qtDiagramViewConfiguration.h"
#include "smtk/extension/qt/diagram/qtTaskEditor.h"
#include "smtk/extension/qt/qtBaseView.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/operation/Manager.h"
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
#include <QInputDialog>
#include <QLabel>
#include <QLayout>
#include <QMenu>
#include <QPainter>

#include "task/ui_DefaultTaskNode.h"

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

class DefaultTaskNodeWidget
  : public QWidget
  , public Ui::DefaultTaskNode
{
public:
  DefaultTaskNodeWidget(qtDefaultTaskNode* node, QWidget* parent = nullptr)
    : QWidget(parent)
    , m_node(node)
  {
    this->setupUi(this);
    m_title->setCheckable(true);
    QObject::connect(m_title, &QPushButton::toggled, this, &DefaultTaskNodeWidget::activateTask);
    QObject::connect(m_completed, &QCheckBox::toggled, this, &DefaultTaskNodeWidget::markCompleted);
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
      "DefaultTaskNodeWidget observer");
    // Create the context menu for the Task Button
    QPointer<DefaultTaskNodeWidget> self(this);
    m_title->setContextMenuPolicy(Qt::CustomContextMenu);
    // We are capturing the this pointer to deal with a warning on Windows - we should not
    // need to do this
    QObject::connect(
      m_title, &QPushButton::customContextMenuRequested, this, [self, this](const QPoint& pt) {
        if (!self)
        {
          return;
        }
        // Create a name for the context menu
        QString menuName(self->m_node->task()->name().c_str());
        menuName.append("_contextMenu");

        QMenu* menu = new QMenu();
        menu->setObjectName(menuName);
        auto* renameAction = new QAction("Rename task…");
        renameAction->setObjectName("RenameTaskAction");
        QObject::connect(
          renameAction, &QAction::triggered, self.data(), &DefaultTaskNodeWidget::renameTask);
        menu->addAction(renameAction);
        // Figure out where to place the context menu.
        // Simply calling self->mapToGlobal(pt) doesn't work; instead we must
        // translate from the proxy-widget's coordinates to the parent node's coordinates,
        // then to the scene coordinates, then to the viewport coordinates and finally
        // to the global (screen/device) coordinates.
        auto* view = self->m_node->diagram()->diagramWidget();
        QPoint xpt = self->m_title->mapToParent(pt);
        QPoint tpt = view->mapToGlobal(view->mapFromScene(self->m_node->mapToScene(xpt)));
        menu->exec(tpt);
        delete menu;
      });
    this->updateTaskState(m_node->m_task->state(), m_node->m_task->state(), m_node->isActive());
  }

  // This method runs the rename task operation
  bool renameTask()
  {
    bool ok;
    auto* task = m_node->task();
    if (!task)
    {
      return false;
    }
    // Request the new name from the user
    QString origName(task->name().c_str());
    QString newName = QInputDialog::getText(
      nullptr, tr("Renaming Task"), tr("New Task Name: "), QLineEdit::Normal, origName, &ok);
    // If the name is the same or the user canceled the dialog just return
    if (!ok || (origName == newName))
    {
      return false;
    }

    // Prep and run the operation (if possible)
    auto opMgr = task->manager()->managers()->get<smtk::operation::Manager::Ptr>();
    auto op = opMgr->create("smtk::task::RenameTask");
    if (!op)
    {
      return false;
    }
    if (!op->parameters()->associate(task->shared_from_this()))
    {
      return false;
    }
    if (!op->parameters()->findAs<smtk::attribute::StringItem>("name")->setValue(
          newName.toStdString()))
    {
      return false;
    }
    if (!op->ableToOperate())
    {
      return false;
    }
    opMgr->launchers()(op);
    return true;
  }

  void flashWarning()
  {
    // TODO: pulse the widget background.
  }

  void updateTaskState(smtk::task::State prev, smtk::task::State next, bool active)
  {
    (void)prev;
    m_completed->setEnabled(m_node->m_task->editableCompletion());

    // Update the checkbox widget without infinite recursion:
    m_completed->blockSignals(true);
    m_completed->setChecked(next == smtk::task::State::Completed);
    m_completed->blockSignals(false);

    switch (next)
    {
      case smtk::task::State::Irrelevant:
      case smtk::task::State::Unavailable:
        m_title->setEnabled(false);
        break;
      case smtk::task::State::Incomplete:
      case smtk::task::State::Completable:
      case smtk::task::State::Completed:
        m_title->setEnabled(true);
        break;
    }
    m_completed->setToolTip(
      next == smtk::task::State::Completed ? "Undo completion" : "Mark completed");
    // Force the title button to match the active/inactive state of the task
    // without infinite recursion.
    m_title->blockSignals(true);
    m_title->setChecked(active);
    m_title->blockSignals(false);
    m_title->setToolTip(QString::fromStdString("Status: " + smtk::task::stateName(next)));
    m_title->setIcon(this->renderStatusIcon(next, m_title->height() / 2));
  }

  void activateTask()
  {
    auto* taskManager = m_node->m_task->manager();
    if (taskManager)
    {
      auto* previouslyActive = taskManager->active().task();
      if (previouslyActive != m_node->m_task)
      {
        if (!taskManager->active().switchTo(m_node->m_task))
        {
          this->flashWarning();
        }
      }
      else
      {
        // Deactivate this task without making any other active.
        m_title->setChecked(false);
        if (!taskManager->active().switchTo(nullptr))
        {
          this->flashWarning();
        }
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

  qtDefaultTaskNode* m_node;
  smtk::task::Task::Observers::Key m_taskObserver;
};

qtDefaultTaskNode::qtDefaultTaskNode(
  qtDiagramGenerator* generator,
  smtk::task::Task* task,
  QGraphicsItem* parent)
  : Superclass(generator, task, parent)
  , m_container(new DefaultTaskNodeWidget(this))
{
  qtDiagramViewConfiguration& cfg(*this->scene()->configuration());
  // Create a container to hold node contents
  {
    m_container->setObjectName(QString::fromStdString("nodeContainer_" + task->id().toString()));
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
    // Some older versions of Qt may need the setWindowFlags() below to work around
    // this bug: https://bugreports.qt.io/browse/QTBUG-10683 .
    // graphicsProxyWidget->setWindowFlags(Qt::BypassGraphicsProxyWidget);

    m_container->m_title->setText(QString::fromStdString(m_task->title()));
  }

  this->addToScene();

  // === Task-specific constructor ===
  this->setZValue(cfg.nodeLayer());
}

void qtDefaultTaskNode::setContentStyle(ContentStyle cs)
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

void qtDefaultTaskNode::setOutlineStyle(OutlineStyle os)
{
  qtDiagramViewConfiguration& cfg(*this->scene()->configuration());
  m_outlineStyle = os;
  this->setZValue(cfg.nodeLayer() - (os == OutlineStyle::Normal ? 0 : 1));
  this->update(this->boundingRect());
}

QRectF qtDefaultTaskNode::boundingRect() const
{
  qtDiagramViewConfiguration& cfg(*this->scene()->configuration());
  const auto border = 4 * cfg.nodeBorderThickness();
  const double height = m_container->height();
  return QRectF(0, 0, m_container->width(), height).adjusted(-border, -border, border, border);
}

void qtDefaultTaskNode::paint(
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
  painter->fillPath(path, baseColor);
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

int qtDefaultTaskNode::updateSize()
{
  this->prepareGeometryChange();

  m_container->resize(m_container->layout()->sizeHint());
  Q_EMIT this->nodeResized();

  return 1;
}

void qtDefaultTaskNode::updateTaskState(smtk::task::State prev, smtk::task::State next, bool active)
{
  m_container->updateTaskState(prev, next, active);
}

void qtDefaultTaskNode::dataUpdated()
{
  m_container->m_title->setText(QString::fromStdString(m_task->name()));
}

} // namespace extension
} // namespace smtk
