//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtModelOperationWidget.h"

#include "smtk/extension/qt/qtActiveObjects.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtBaseAttributeView.h"
#include "smtk/extension/qt/qtCollapsibleGroupWidget.h"
#include "smtk/extension/qt/qtInstancedView.h"
#include "smtk/extension/qt/qtModelView.h"
#include "smtk/extension/qt/qtOperationView.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/model/Resource.h"
#include "smtk/model/Session.h"
#include "smtk/model/StringData.h"
#include "smtk/view/View.h"

#include "smtk/io/Logger.h"

#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QPointer>
#include <QSpacerItem>
#include <QSplitter>
#include <QStackedLayout>
#include <QTextEdit>
#include <QVBoxLayout>

#include <algorithm> // std::sort
#include <map>

using namespace smtk::extension;
using namespace smtk::model;

class qtModelOperationWidgetInternals
{
public:
  struct OperationInfo
  {
    smtk::operation::OperationPtr opPtr;
    QPointer<qtUIManager> opUiManager;
    QPointer<QFrame> opUiParent;
    QPointer<qtBaseView> opUiView;
  };

  smtk::model::Session::WeakPtr CurrentSession;
  smtk::model::Session::WeakPtr PreviousSession;
  std::string CurrentOpName;
  std::string PreviousOpName;
  QSplitter* LogSplitter;
  QVBoxLayout* WidgetLayout;
  QPointer<QComboBox> OperationCombo;
  QStackedLayout* OperationsLayout;
  QMap<std::string, OperationInfo> OperationMap;
  QPointer<qtModelView> ModelView;
  QPointer<qtCollapsibleGroupWidget> opLogInfo;
  QTextEdit* ResultLog;
  std::map<std::string, std::string> m_operatorLabelMap;
  std::map<std::string, std::string> m_operatorNameMap;
  std::vector<std::string> m_sortedOperationLabels;
  int findLabelPosition(const std::string& label)
  {
    return (std::find(m_sortedOperationLabels.begin(), m_sortedOperationLabels.end(), label) -
      m_sortedOperationLabels.begin());
  }
  int findNamePosition(const std::string& opName)
  {
    std::string opLabel = m_operatorNameMap[opName];
    return this->findLabelPosition(opLabel);
  }
};

qtModelOperationWidget::qtModelOperationWidget(QWidget* _p)
  : QWidget(_p)
{
  this->Internals = new qtModelOperationWidgetInternals();
  this->initWidget();
}

qtModelOperationWidget::~qtModelOperationWidget()
{
  this->setSession(smtk::model::SessionPtr());
  delete this->Internals;
}

void qtModelOperationWidget::initWidget()
{
  this->setObjectName("modelOperationWidget");
  QWidget* mainArea = new QWidget();
  mainArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
  this->Internals->LogSplitter = new QSplitter(this);
  QVBoxLayout* universe = new QVBoxLayout(this);
  universe->addWidget(this->Internals->LogSplitter);
  this->Internals->LogSplitter->setOrientation(Qt::Vertical);
  this->Internals->WidgetLayout = new QVBoxLayout(mainArea);
  this->Internals->LogSplitter->addWidget(mainArea);
  this->Internals->OperationCombo = new QComboBox(this);
  this->Internals->OperationCombo->setToolTip("Select an operator");
  this->Internals->OperationsLayout = new QStackedLayout();

  QHBoxLayout* operatorLayout = new QHBoxLayout();
  operatorLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  this->Internals->OperationCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  operatorLayout->addWidget(this->Internals->OperationCombo);
  this->Internals->WidgetLayout->addLayout(operatorLayout);
  this->Internals->WidgetLayout->addLayout(this->Internals->OperationsLayout);
  this->Internals->ResultLog = new QTextEdit();
  this->Internals->ResultLog->setReadOnly(true);
  this->Internals->ResultLog->setStyleSheet(
    "font: \"Monaco\", \"Menlo\", \"Andale Mono\", \"fixed\";");
  this->Internals->ResultLog->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

  this->Internals->opLogInfo = new qtCollapsibleGroupWidget(this->Internals->LogSplitter);
  // For now since we have the ability to show the log in the applicatiom lets hide the
  // logging widget initially
  this->Internals->opLogInfo->hide();
  this->Internals->opLogInfo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
  this->Internals->opLogInfo->setName("Show Operation Log");
  this->Internals->opLogInfo->contentsLayout()->addWidget(this->Internals->ResultLog);
  this->Internals->opLogInfo->collapse();
  this->Internals->LogSplitter->addWidget(this->Internals->opLogInfo);
  this->Internals->LogSplitter->setCollapsible(0, false);
  this->Internals->LogSplitter->setCollapsible(1, false);
  this->Internals->LogSplitter->setStretchFactor(1, 0);
  QList<int> sizes;
  sizes << 500 << 10;
  this->Internals->LogSplitter->setSizes(sizes);

  // signals/slots
  QObject::connect(this->Internals->OperationCombo, SIGNAL(currentIndexChanged(int)), this,
    SLOT(onOperationSelected()));
}

void qtModelOperationWidget::showLogInfo(bool visibilityMode)
{
  this->Internals->opLogInfo->setVisible(visibilityMode);
}

QSize qtModelOperationWidget::sizeHint() const
{
  auto operatorWidget = this->Internals->OperationsLayout->currentWidget();
  if (operatorWidget)
  {
    // Compute the new width as the operator's widget width plus an offset
    // to account for the side margins (~50)
    const int operWidth = operatorWidget->width() + 50;
    QSize newSize(std::max(operWidth, this->width()), 0);
    int height = 20;
    for (int i = 0; i < this->Internals->WidgetLayout->count(); ++i)
    {
      height += this->Internals->WidgetLayout->itemAt(i)->geometry().height();
    }

    height = height + this->Internals->LogSplitter->handleWidth() +
      this->Internals->LogSplitter->widget(1)->size().height();
    newSize.setHeight(height > 500 ? height : 500);
    return newSize;
  }

  return QSize(500, 650);
}

void qtModelOperationWidget::setSession(smtk::model::SessionPtr session)
{
  // if it's current session and we've already constructed operators for it
  // TODO: this information is no longer held by the session
  // if (this->Internals->CurrentSession.lock() == session && session &&
  //   std::size_t(this->Internals->OperationCombo->count()) == session->numberOfOperations(false))
  // {
  //   return;
  // }

  // clean up current UI
  QStackedLayout* opLayout = this->Internals->OperationsLayout;
  for (int i = 0; i < opLayout->count(); ++i)
  {
    if (opLayout->widget(i))
      delete opLayout->widget(i);
  }

  this->Internals->CurrentSession = session;
  this->Internals->OperationCombo->blockSignals(true);
  this->Internals->OperationCombo->clear();
  // TODO: this information is no longer held by the session
  // if (session)
  // {
  //   this->Internals->m_operatorLabelMap = session->operatorLabelsMap(false);
  //   this->Internals->m_sortedOperationLabels.clear();
  //   this->Internals->m_operatorNameMap.clear();
  //   // Next lets get the list of labels so we can sort them
  //   for (auto imap : this->Internals->m_operatorLabelMap)
  //   {
  //     this->Internals->m_sortedOperationLabels.push_back(imap.first);
  //     this->Internals->m_operatorNameMap[imap.second] = imap.first;
  //   }
  //   std::sort(this->Internals->m_sortedOperationLabels.begin(),
  //     this->Internals->m_sortedOperationLabels.end());
  //   for (auto it : this->Internals->m_sortedOperationLabels)
  //   {
  //     this->Internals->OperationCombo->addItem(it.c_str());
  //   }
  // }
  // this->Internals->OperationCombo->blockSignals(false);
  // this->Internals->OperationMap.clear();
}

void qtModelOperationWidget::refreshOperationList()
{
  auto session = this->Internals->CurrentSession.lock();
  if (!session)
  {
    return;
  }

  auto selection = this->Internals->OperationCombo->currentText();
  this->setSession(session);
  this->Internals->OperationCombo->setCurrentText(selection);
}

void qtModelOperationWidget::setModelView(qtModelView* mv)
{
  if (this->Internals->ModelView != mv)
  {
    this->Internals->ModelView = mv;
  }
}

qtModelView* qtModelOperationWidget::modelView()
{
  return this->Internals->ModelView;
}

void qtModelOperationWidget::cancelCurrentOperation()
{
  this->cancelOperation(this->Internals->CurrentOpName);
}

void qtModelOperationWidget::cancelOperation(const std::string& opName)
{
  if (this->Internals->OperationMap.contains(opName))
  {
    // TODO: this information is no longer held by the session
    // OperationPtr brOp = this->Internals->OperationMap[opName].opPtr;
    // emit this->operationCancelled(brOp);
  }
}

bool qtModelOperationWidget::checkExistingOperation(const std::string& opName)
{
  // if the operator is already created, just set its UI to be current widget
  if (this->Internals->OperationMap.contains(opName))
  {
    auto iview =
      dynamic_cast<qtBaseAttributeView*>(this->Internals->OperationMap[opName].opUiView.data());
    if (iview)
    {
      iview->requestModelEntityAssociation();
    }
    this->Internals->OperationsLayout->setCurrentWidget(
      this->Internals->OperationMap[opName].opUiParent);
    return true;
  }
  return false;
}

bool qtModelOperationWidget::initOperationUI(const smtk::operation::OperationPtr& /*brOp*/)
{
  // TODO: this has all changed
  // std::string opName = brOp->name();
  // std::string opLabel = this->Internals->m_operatorNameMap[opName];
  // std::string prevOpName = this->Internals->CurrentOpName;
  // if (!prevOpName.empty() && opName != prevOpName)
  // {
  //   // we need to reset previous operator's UI
  //   this->cancelOperation(prevOpName);
  // }

  // // set the operator combobox to the new index
  // if (opLabel != this->Internals->OperationCombo->currentText().toStdString())
  // {
  //   int idx = this->Internals->findLabelPosition(opLabel);
  //   this->Internals->OperationCombo->blockSignals(true);
  //   this->Internals->OperationCombo->setCurrentIndex(idx);
  //   this->Internals->OperationCombo->blockSignals(false);
  // }

  // // if the operator is already created, just set its UI to be current widget
  // if (this->checkExistingOperation(opName))
  // {
  //   this->Internals->CurrentOpName = opName;
  //   return true;
  // }

  // if (!brOp->specification())
  // {
  //   std::cerr << "  Operation has no specification\n";
  //   return false;
  // }

  // SessionRef bs(brOp->manager(), brOp->session()->sessionId());
  // this->setSession(bs.session());
  // QFrame* opParent = new QFrame(this);

  // // Since the parent of `this` currently is a QScrollArea, it is necessary
  // // to ensure `this` and its children provide a suitable sizeHint, so a
  // // minimum is set in opParent.
  // // http://doc.qt.io/qt-5/qscrollarea.html#details
  // // The minimum in opParent is small, such that it allows certain flexibility
  // // for the top parent (QDockWidget, QDialog, etc.) to resize `this` properly.
  // opParent->setMinimumSize(50, 0);
  // opParent->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
  // QVBoxLayout* opLayout = new QVBoxLayout(opParent);
  // opLayout->setMargin(0);

  // smtk::attribute::AttributePtr att = brOp->specification();
  // // TODO: the manager is no longer accessible through the operator
  // // att->resource()->setRefModelManager(brOp->manager());

  // smtk::extension::qtUIManager* uiManager = new smtk::extension::qtUIManager(att->resource());
  // uiManager->setActiveModelView(this->Internals->ModelView);

  // // find out what view to use to construct the UI, if none is specified for this op
  // // ( meaning if there is no "AttributeTypes" specified in view components' children,
  // // or the att->type() is not included in any view "AttributeTypes" ),
  // // use "Operation" view by default
  // smtk::view::ViewPtr opView;

  // std::map<std::string, smtk::view::ViewPtr>::const_iterator it;
  // for (it = att->resource()->views().begin(); it != att->resource()->views().end(); ++it)
  // {
  //   //If  this is an Operation View we need to check its InstancedAttributes child else
  //   // we need to check AttributeTypes
  //   int i; // View Component index we need to check
  //   if (it->second->type() == "Operation")
  //   {
  //     i = it->second->details().findChild("InstancedAttributes");
  //   }
  //   else
  //   {
  //     i = it->second->details().findChild("AttributeTypes");
  //   }
  //   if (i < 0)
  //   {
  //     continue;
  //   }
  //   smtk::view::View::Component& comp = it->second->details().child(i);
  //   for (std::size_t ci = 0; ci < comp.numberOfChildren(); ++ci)
  //   {
  //     std::string optype;
  //     if (comp.child(ci).attribute("Type", optype) && optype == att->type())
  //     {
  //       opView = it->second;
  //       // If we are dealing with an Operation View - The Name attribute needs to be
  //       // set to the same of the attribute the operator is using - so in practice
  //       // the Name attribute does not have to be set in the operator's sbt info
  //       if (opView->type() == "Operation")
  //       {
  //         comp.child(ci).setAttribute("Name", att->name());
  //       }
  //       break;
  //     }
  //   }
  //   if (opView)
  //     break;
  // }

  // if (!opView || !uiManager->hasViewConstructor(opView->type()))
  // {
  //   //Lets create a default view for the operator itself
  //   opView = smtk::view::View::New("Operation", brOp->name());
  //   opView->details().setAttribute("UseSelectionManager", "true");
  //   smtk::view::View::Component& comp =
  //     opView->details().addChild("InstancedAttributes").addChild("Att");
  //   comp.setAttribute("Type", att->type()).setAttribute("Name", att->name());
  //   att->resource()->addView(opView);
  // }

  // opView->details().setAttribute("TopLevel", "true");

  // QObject::connect(uiManager, SIGNAL(fileItemCreated(smtk::extension::qtFileItem*)), this,
  //   SIGNAL(fileItemCreated(smtk::extension::qtFileItem*)));
  // QObject::connect(this, SIGNAL(operationFinished(const smtk::operation::OperationResult&)), uiManager,
  //   SLOT(onOperationFinished()));

  // qtModelOperationWidgetInternals::OperationInfo opInfo;
  // opInfo.opPtr = brOp;
  // opInfo.opUiParent = opParent;
  // opInfo.opUiManager = uiManager;
  // this->Internals->OperationMap[opName] = opInfo;

  // OperationViewInfo opViewInfo(opView, brOp, opParent, uiManager);
  // qtBaseView* theView = uiManager->setSMTKView(opViewInfo, false);
  // auto opViewWidget = dynamic_cast<qtOperationView*>(theView);
  // if (opViewWidget)
  // {
  //   QObject::connect(opViewWidget, SIGNAL(operationRequested(const smtk::operation::OperationPtr&)),
  //     this, SIGNAL(operationRequested(const smtk::operation::OperationPtr&)));
  // }

  // theView->requestModelEntityAssociation();

  // this->Internals->OperationMap[opName].opUiView = theView;

  // this->Internals->OperationsLayout->addWidget(opParent);
  // this->Internals->OperationsLayout->setCurrentWidget(opParent);
  // this->Internals->CurrentOpName = opName;
  return true;
}

bool qtModelOperationWidget::setCurrentOperation(
  const std::string& /*opName*/, smtk::model::SessionPtr /*session*/)
{
  // smtk::model::Session::WeakPtr prevSession = this->Internals->CurrentSession;
  // std::string currOpName = this->Internals->CurrentOpName;
  // QMap<std::string, qtModelOperationWidgetInternals::OperationInfo>::Iterator opIt;
  // // Do not set the previous operator to be an "advanced" operator or the "Model - Save" operator:
  // std::string prevOpName =
  //   (opIt = this->Internals->OperationMap.find(currOpName)) != this->Internals->OperationMap.end() &&
  //     opIt->opPtr && opIt->opPtr->specification()->definition()->advanceLevel() < 1 &&
  //     currOpName != "save smtk model"
  //   ? currOpName
  //   : std::string();
  // this->setSession(session);
  // if (!session)
  // {
  //   return false;
  // }

  // std::string opLabel = this->Internals->m_operatorNameMap[opName];
  // int idx = this->Internals->findLabelPosition(opLabel);
  // if (this->Internals->OperationCombo->currentIndex() != idx)
  // {
  //   if (prevSession.lock() && !prevOpName.empty())
  //   {
  //     this->Internals->PreviousSession = prevSession;
  //     this->Internals->PreviousOpName = prevOpName;
  //   }
  //   this->Internals->OperationCombo->setCurrentIndex(idx);
  //   return true;
  // }
  // OperationPtr brOp = this->Internals->OperationMap.contains(opName)
  //   ? this->Internals->OperationMap[opName].opPtr
  //   : session->op(opName); // create the operator

  // if (!brOp)
  // {
  //   std::cerr << "Could not create operator: \"" << opName << "\" for session"
  //             << " \"" << session->name() << "\""
  //             << " (" << session->sessionId() << ")\n";
  //   return false;
  // }
  // emit operatorSet(brOp);
  // // We cannot rely on initOperationUI to set PreviousSession since we may have
  // // changed the session above.
  // bool didChange = this->initOperationUI(brOp);
  // if (didChange && !prevOpName.empty())
  // {
  //   this->Internals->PreviousSession = prevSession;
  //   this->Internals->PreviousOpName = prevOpName;
  // }
  // return didChange;
  return false;
}

smtk::operation::OperationPtr qtModelOperationWidget::existingOperation(
  const std::string& /*opName*/)
{
  // TODO: this information is no longer held by the session
  // OperationPtr brOp;
  // if (this->Internals->OperationMap.contains(opName))
  // {
  //   brOp = this->Internals->OperationMap[opName].opPtr;
  // }

  // return brOp;
  return nullptr;
}

qtBaseView* qtModelOperationWidget::existingOperationView(const std::string& opName)
{
  qtBaseView* opview = nullptr;
  if (this->Internals->OperationMap.contains(opName))
  {
    opview = this->Internals->OperationMap[opName].opUiView;
  }

  return opview;
}

void qtModelOperationWidget::expungeEntities(const smtk::model::EntityRefs& expungedEnts)
{
  QMapIterator<std::string, qtModelOperationWidgetInternals::OperationInfo> it(
    this->Internals->OperationMap);
  while (it.hasNext())
  {
    it.next();
    bool associationChanged = false;
    if (it.value().opPtr && it.value().opPtr->parameters())
    {
      // update operator's parameters
      associationChanged = it.value().opPtr->parameters()->removeExpungedEntities(expungedEnts);
      if (!it.value().opPtr->parameters()->associations())
      { // Operation's modelEntityItem has been expunged. Update in qtModelEntityItem
        emit broadcastExpungeEntities(expungedEnts);
      }
    }
    if (associationChanged)
      it.value().opUiView->updateUI();
  }
}

void qtModelOperationWidget::onOperationSelected()
{
  std::string opName =
    this->Internals
      ->m_operatorLabelMap[this->Internals->OperationCombo->currentText().toStdString()];
  this->setCurrentOperation(opName, this->Internals->CurrentSession.lock());
}

void qtModelOperationWidget::onOperate()
{
  // TODO: this information is no longer held by the session
  // std::string opLabel = this->Internals->OperationCombo->currentText().toStdString();
  // std::string opName = this->Internals->m_operatorLabelMap[opLabel];
  // if (this->Internals->OperationMap.contains(opName))
  // {
  //   OperationPtr brOp = this->Internals->OperationMap[opName].opPtr;
  //   emit this->operationRequested(brOp);
  // }
}

/// Display messages summarizing the result of an operation to the user.
void qtModelOperationWidget::displayResult(const smtk::io::Logger& log)
{
  QString txt(log.convertToString(false).c_str());
  this->Internals->ResultLog->setText(txt);
}

void qtModelOperationWidget::resetUI()
{
  smtk::model::SessionRef activeSession = qtActiveObjects::instance().activeModel().owningSession();
  // Clean up current UI only when switching to a new session or closing all data
  // then model tree would update properly
  if (this->Internals &&
    (!activeSession.isValid() || activeSession.session() != this->Internals->CurrentSession.lock()))
  {
    QStackedLayout* opLayout = this->Internals->OperationsLayout;
    for (int i = 0; i < opLayout->count(); ++i)
    {
      if (opLayout->widget(i))
      {
        delete opLayout->widget(i);
      }
    }
    this->Internals->PreviousSession = smtk::model::Session::WeakPtr();
    this->Internals->PreviousOpName.clear();
    this->Internals->CurrentSession = activeSession.session();
    this->Internals->OperationCombo->blockSignals(true);
    this->Internals->OperationCombo->clear();
    this->Internals->OperationCombo->blockSignals(false);
    foreach (auto& opInfo, this->Internals->OperationMap)
    {
      if (!opInfo.opUiManager.isNull())
      {
        // The frame and base view would be taken care of by Qt
        delete opInfo.opUiManager;
      }
    }
    this->Internals->OperationMap.clear();
  }
}

bool qtModelOperationWidget::showPreviousOp()
{
  smtk::model::Session::Ptr prevSess = this->Internals->PreviousSession.lock();
  if (!prevSess || this->Internals->PreviousOpName.empty())
  {
    return false;
  }
  return this->setCurrentOperation(this->Internals->PreviousOpName, prevSess);
}
