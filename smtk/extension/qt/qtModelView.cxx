//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtModelView.h"
#include "smtk/extension/qt/qtActiveObjects.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/mesh/core/MeshSet.h"
#include "smtk/mesh/core/Resource.h"

#include "smtk/model/Entity.h"
#include "smtk/model/EntityIterator.h"
#include "smtk/model/FloatData.h"
#include "smtk/model/Group.h"
#include "smtk/model/IntegerData.h"
#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"
#include "smtk/model/SessionRef.h"
#include "smtk/model/StringData.h"

#include "smtk/model/SessionRef.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtModelOperationWidget.h"
#include "smtk/extension/qt/qtModelPanel.h"
#include "smtk/extension/qt/qtOperationDockWidget.h"
#include "smtk/extension/qt/qtSMTKUtilities.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/mesh/core/Resource.h"

#include <QAction>
#include <QApplication>
#include <QColorDialog>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QHeaderView>
#include <QMenu>
#include <QPointer>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QVariant>

#include <algorithm> // std::sort
#include <iomanip>

using namespace smtk::model;

namespace smtk
{
namespace extension
{

static const std::string pEntityGroupOpName("entity group");

qtModelView::qtModelView(QWidget* p)
  : QTreeView(p)
{
  this->setSelectionBehavior(QAbstractItemView::SelectRows);
  this->setSelectionMode(QAbstractItemView::ExtendedSelection);
  this->setSortingEnabled(true);

  QSizePolicy expandPolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  this->setSizePolicy(expandPolicy);
  this->setDragDropMode(QAbstractItemView::DragDrop);
  this->setDropIndicatorShown(true);
  this->setDragEnabled(true);
  this->setAcceptDrops(true);
  this->setContextMenuPolicy(Qt::CustomContextMenu);
  //  QObject::connect(this,
  //                   SIGNAL(customContextMenuRequested(const QPoint &)),
  //                   this, SLOT(showContextMenu(const QPoint &)));
  m_ContextMenu = NULL;
  m_OperationsDock = NULL;
  m_OperationsWidget = NULL;

  this->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

  std::ostringstream receiverSource;
  receiverSource << "qtModelView_" << this;
  m_selectionSourceName = receiverSource.str();
}

qtModelView::~qtModelView()
{
  // Explicitly delete dock widget if not parented
  if (m_OperationsDock && !m_OperationsDock->parent())
  {
    delete m_OperationsDock;
  }
}

void qtModelView::keyPressEvent(QKeyEvent* keyEvent)
{
  this->QTreeView::keyPressEvent(keyEvent);
}

void qtModelView::mouseReleaseEvent(QMouseEvent* mouseEvent)
{
  QPoint evtpos = mouseEvent->pos();
  if (mouseEvent->button() & Qt::RightButton)
  {
    this->showContextMenu(evtpos);
  }

  QTreeView::mouseReleaseEvent(mouseEvent);
}

Qt::DropActions qtModelView::supportedDropActions() const
{
  // returns what actions are supported when dropping
  return Qt::CopyAction;
}

void qtModelView::startDrag(Qt::DropActions supportedActions)
{
  //  emit this->dragStarted(this);
  this->QTreeView::startDrag(supportedActions);
}

void qtModelView::dragEnterEvent(QDragEnterEvent* eevent)
{
  this->QTreeView::dragEnterEvent(eevent);
}

void qtModelView::dragMoveEvent(QDragMoveEvent* mevent)
{
  if (mevent->proposedAction() & this->supportedDropActions())
  {
    mevent->accept();
  }
}

void qtModelView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  QTreeView::selectionChanged(selected, deselected);
}

// when the dataChanged is emitted from the model, we want to scroll to
// that index so that the changes are visible in the tree view.
void qtModelView::newIndexAdded(const QModelIndex& newidx)
{
  this->scrollTo(newidx);
}

void qtModelView::updateActiveModelByModelIndex()
{
}

bool qtModelView::removeSession(const smtk::model::SessionRef&)
{
  return false;
}

void qtModelView::showContextMenu(const QModelIndex& idx, const QPoint& p)
{
  if (m_ContextMenu)
  {
    m_ContextMenu->clear();
  }
  else
  {
    m_ContextMenu = new QMenu(this);
    m_ContextMenu->setTitle("Operations Menu");
  }

  smtk::model::SessionRef brSession;

  // get the current model and compare with active model
  smtk::model::Model currentModel;

  if (!brSession.isValid())
  {
    // Nothing to do the session is not valid;
    return;
  }
  std::string sessionString = brSession.entity().toString();
  // Have we already processed this session previously? Have the number of
  // operators changed?
  // auto sinfoIt = m_sessionInfo.find(sessionString);
  // TODO: Sessions on longer have information about operators
  // if (sinfoIt == m_sessionInfo.end() ||
  //   (*sinfoIt).first.size() != brSession.session()->numberOfOperations(false))
  // {
  //   // This is the first time seeing this session
  //   // First we need to get the mapping between operator labels and their names
  //   std::map<std::string, std::string> opLabelsMap = brSession.session()->operatorLabelsMap(false);
  //   // Next lets get the list of labels so we can sort them
  //   std::vector<std::string> keyList;
  //   for (auto imap : opLabelsMap)
  //   {
  //     keyList.push_back(imap.first);
  //   }
  //   std::sort(keyList.begin(), keyList.end());
  //   m_sessionInfo[sessionString] =
  //     std::pair<std::vector<std::string>, std::map<std::string, std::string> >(
  //       keyList, opLabelsMap);
  // }
  auto sinfo = m_sessionInfo[sessionString];
  // Compare the current model with active model. If true, show related
  // operators. If not, only show `set as active model`.
  // Currently all sessions would show their operators
  smtk::model::Model currentActiveModel = qtActiveObjects::instance().activeModel();

  if ((brSession.isValid() && !currentModel.isValid()) /* click on active session*/
    || (currentModel.isValid() && (currentModel.entity() == currentActiveModel.entity())))
  {
    for (StringList::const_iterator it = sinfo.first.begin(); it != sinfo.first.end(); ++it)
    {
      QAction* act = m_ContextMenu->addAction((*it).c_str());
      QVariant vdata = qtSMTKUtilities::UUIDToQVariant(brSession.entity());
      act->setData(vdata);
      QObject::connect(act, SIGNAL(triggered()), this, SLOT(operatorInvoked()));
    }
  }
  else if (currentModel.isValid()) // if invalid, then it's sessionRef
  {                                // set active model
    std::string setAsActiveModel("set as active model");
    QAction* act = m_ContextMenu->addAction(setAsActiveModel.c_str());
    QVariant vdata = qtSMTKUtilities::UUIDToQVariant(brSession.entity());
    act->setData(vdata);
    QObject::connect(act, SIGNAL(triggered()), this, SLOT(updateActiveModelByModelIndex()));
  }

  // store current contextMenuIndex to set active model
  m_contextMenuIndex = idx;

  QPoint popP = p;
  if (popP.isNull())
  {
    QRect idxRec = this->visualRect(idx);
    popP.setX(idxRec.right() / 2);
    popP.setY(idxRec.top());
  }
  m_ContextMenu->popup(this->mapToGlobal(popP));
}

void qtModelView::showContextMenu(const QPoint& p)
{
  QModelIndex idx = this->indexAt(p);
  this->showContextMenu(idx, p);
}

void qtModelView::operatorInvoked()
{
}

qtOperationDockWidget* qtModelView::operatorsDock()
{
  if (m_OperationsDock && m_OperationsWidget)
  {
    return m_OperationsDock;
  }

  qtModelOperationWidget* opWidget = new qtModelOperationWidget();
  opWidget->setModelView(this);
  QObject::connect(opWidget, SIGNAL(operationRequested(const smtk::operation::OperationPtr&)), this,
    SIGNAL(operationRequested(const smtk::operation::OperationPtr&)));
  QObject::connect(opWidget, SIGNAL(operationCancelled(const smtk::operation::OperationPtr&)), this,
    SIGNAL(operationCancelled(const smtk::operation::OperationPtr&)));
  QObject::connect(opWidget, SIGNAL(fileItemCreated(smtk::extension::qtFileItem*)), this,
    SIGNAL(fileItemCreated(smtk::extension::qtFileItem*)));
  QObject::connect(this, SIGNAL(operationFinished(const smtk::operation::OperationResult&)),
    opWidget, SIGNAL(operationFinished(const smtk::operation::OperationResult&)));

  QWidget* dockP = NULL;
  foreach (QWidget* widget, QApplication::topLevelWidgets())
  {
    if (widget->inherits("QMainWindow"))
    {
      dockP = widget;
      break;
    }
  }

  qtOperationDockWidget* dw = new qtOperationDockWidget(dockP);
  QScrollArea* s = new QScrollArea(dw);
  s->setWidgetResizable(true);
  s->setFrameShape(QFrame::NoFrame);
  s->setObjectName("scrollArea");

  opWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
  s->setWidget(opWidget);
  dw->setWidget(s);

  QObject::connect(dw, SIGNAL(closing()), this, SLOT(onOperationPanelClosing()));

  m_OperationsWidget = opWidget;
  m_OperationsDock = dw;
  QObject::connect(&qtActiveObjects::instance(), SIGNAL(activeModelChanged()), m_OperationsDock,
    SLOT(reset()), Qt::UniqueConnection);
  //  m_OperationsDock->hide();
  return dw;
}

qtModelOperationWidget* qtModelView::operatorsWidget()
{
  if (!m_OperationsWidget)
  {
    this->operatorsDock();
  }
  return m_OperationsWidget;
}

void qtModelView::initOperationsDock(const std::string& opName, smtk::model::SessionPtr session)
{
  // make sure the operator widget is created.
  this->operatorsDock()->raise();
  this->operatorsDock()->show();
  SessionRef bs(session->resource(), session->sessionId());

  m_OperationsWidget->setCurrentOperation(opName, session);
  m_OperationsDock->setWindowTitle(bs.flagSummary().c_str());

  if (QScrollArea* scrollArea = qobject_cast<QScrollArea*>(m_OperationsDock->widget()))
  {
    scrollArea->ensureWidgetVisible(m_OperationsWidget);
  }
  // sizeHint() alone doesn't work, so force resize
  m_OperationsDock->resize(m_OperationsWidget->sizeHint());
  m_OperationsDock->updateGeometry();
}

bool qtModelView::requestOperation(const smtk::operation::OperationPtr& brOp, bool launchUI)
{
  if (!brOp)
  {
    return false;
  }

  if (!launchUI)
  {
    emit this->operationRequested(brOp);
  }
  else // launch the m_OperationsDock
  {
    this->operatorsDock()->show();
    // TODO: operators no longer have access to this information
    // SessionRef bs(brOp->manager(), brOp->session()->sessionId());

    m_OperationsWidget->initOperationUI(brOp);
    // TODO: operators no longer have access to this information
    // m_OperationsDock->setWindowTitle(bs.flagSummary().c_str());

    if (QScrollArea* scrollArea = qobject_cast<QScrollArea*>(m_OperationsDock->widget()))
    {
      scrollArea->ensureWidgetVisible(m_OperationsWidget);
    }
    // sizeHint() alone doesn't work, so force resize
    m_OperationsDock->resize(m_OperationsWidget->sizeHint());
    m_OperationsDock->updateGeometry();
  }
  return true;
  //  cJSON* json = cJSON_CreateObject();
  //  SaveJSON::forOperation(brOp, json);
  //  std::cout << "Found operator " << cJSON_Print(json) << ")\n";
  //  OperationResult result = brOp->operate();
  //  json = cJSON_CreateObject();
  //  SaveJSON::forOperationResult(result, json);
  //  std::cout << "Result " << cJSON_Print(json) << "\n";

  //  emit this->operationRequested(uid, action->text());
  //  emit this->operationFinished(result);
}

bool qtModelView::requestOperation(const std::string&, const smtk::common::UUID&, bool)
{
  return false;
}

// bool qtModelView::hasSessionOp(const QModelIndex& idx, const std::string& opname)
// {
//   smtk::model::SessionRef sref = this->owningEntityAs<smtk::model::SessionRef>(idx);
//   return this->hasSessionOp(sref, opname);
// }

// bool qtModelView::hasSessionOp(const smtk::model::SessionRef& brSession, const std::string& opname)
// {
//   if (brSession.isValid())
//   {
//     StringList opNames = brSession.operatorNames();
//     return std::find(opNames.begin(), opNames.end(), opname) != opNames.end();
//   }
//   return false;
// }

// OperationPtr qtModelView::getOp(const QModelIndex& idx, const std::string& opname)
// {
//   smtk::model::SessionRef sref = this->owningEntityAs<smtk::model::SessionRef>(idx);
//   if (!sref.isValid())
//   {
//     std::cerr << "Could not find session!\n";
//     return OperationPtr();
//   }

//   if (!this->hasSessionOp(sref, opname))
//   {
//     std::cout << "The requested operator: \"" << opname << "\" for session"
//               << " \"" << (sref.session() ? sref.session()->name() : "(invalid)") << "\""
//               << " is not part of session operators.\n";
//     return OperationPtr();
//   }

//   smtk::model::SessionPtr session = sref.session();
//   return this->getOp(session, opname);
// }

// OperationPtr qtModelView::getOp(const smtk::model::SessionPtr& brSession, const std::string& opname)
// {
//   OperationPtr brOp;
//   if (!brSession || !(brOp = brSession->op(opname)))
//   {
//     std::cerr << "Could not create operator: \"" << opname << "\" for session"
//               << " \"" << (brSession ? brSession->name() : "(null)") << "\""
//               << " (" << (brSession ? brSession->sessionId().toString() : "--") << ")\n";
//     return OperationPtr();
//   }

//   smtk::attribute::AttributePtr attrib = brOp->specification();
//   if (!attrib)
//   {
//     std::cerr << "Invalid spec for the op: " << brOp->name() << "\n";
//     return OperationPtr();
//   }

//   attrib->resource()->setRefModelResource(brSession->resource());

//   return brOp;
// }

void qtModelView::toggleEntityVisibility(const QModelIndex& /*idx*/)
{
}

bool qtModelView::setEntityVisibility(const smtk::model::EntityRefs& /*selentityrefs*/,
  const smtk::mesh::MeshSets& /*selmeshes*/, int /*vis*/, smtk::operation::OperationPtr /*brOp*/)
{
  return false;
}

QColor internal_convertColor(const FloatList& rgba)
{
  int ncomp = static_cast<int>(rgba.size());
  float alpha = ncomp != 4 ? 1. : std::max(0., std::min(rgba[3], 1.0));
  // alpha can't be zero
  alpha = alpha == 0. ? 1.0 : alpha;
  return ncomp >= 3 ? QColor::fromRgbF(rgba[0], rgba[1], rgba[2], alpha) : QColor();
}

void qtModelView::onEntitiesExpunged(const smtk::model::EntityRefs& expungedEnts)
{
  if (!m_OperationsWidget)
    return;
  m_OperationsWidget->expungeEntities(expungedEnts);
}

std::string qtModelView::determineAction(const QPoint&) const
{
  return "";
}

void qtModelView::onOperationPanelClosing()
{
  // If the operation panel is closing, cancel current operation
  if (m_OperationsWidget)
  {
    m_OperationsWidget->cancelCurrentOperation();
  }
}

bool qtModelView::showPreviousOpOrHide(bool alwaysHide)
{
  if (m_OperationsWidget)
  {
    if (m_OperationsWidget->showPreviousOp() && !alwaysHide)
    {
      return true;
    }
    if (m_OperationsDock)
    {
      m_OperationsDock->hide();
    }
  }
  return false; // false implies the widget is hidden
}

} // namespace model
} // namespace smtk
