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

#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtInstancedView.h"
#include "smtk/extension/qt/qtModelEntityItem.h"
#include "smtk/extension/qt/qtModelView.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/System.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/model/Session.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Operator.h"
#include "smtk/model/StringData.h"
#include "smtk/common/View.h"

#include <QStackedLayout>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QComboBox>
#include <QLabel>
#include <QPointer>
#include <QSpacerItem>

#include <map>
#include <algorithm>    // std::sort

using namespace smtk::extension;
using namespace smtk::model;

//----------------------------------------------------------------------------
class qtModelOperationWidgetInternals
{
public:
  struct OperatorInfo
  {
  smtk::model::OperatorPtr opPtr;
  QPointer<qtUIManager> opUiManager;
  QPointer<QFrame> opUiParent;
  QPointer<qtBaseView> opUiView;
  };

  smtk::model::Session::WeakPtr CurrentSession;
  std::string CurrrentOpName;
  QVBoxLayout* WidgetLayout;
  QPointer<QComboBox> OperationCombo;
  QStackedLayout* OperationsLayout;
  QPointer<QPushButton> OperateButton;
  // <operator-name, <opPtr, opUI-parent> >
  QMap<std::string, OperatorInfo > OperatorMap;
  QPointer<qtModelView> ModelView;
};

//----------------------------------------------------------------------------
qtModelOperationWidget::qtModelOperationWidget(QWidget* _p): QWidget(_p)
{
  this->Internals = new qtModelOperationWidgetInternals();
  this->initWidget();
}

//----------------------------------------------------------------------------
qtModelOperationWidget::~qtModelOperationWidget()
{
  this->setSession(smtk::model::SessionPtr());
  delete this->Internals;
}

//----------------------------------------------------------------------------
void qtModelOperationWidget::initWidget( )
{
  this->setObjectName("modelOperationWidget");
  this->Internals->WidgetLayout = new QVBoxLayout(this);
  this->Internals->OperationCombo = new QComboBox(this);
  this->Internals->OperationCombo->setToolTip("Select an operator");
  this->Internals->OperationsLayout = new QStackedLayout();
  this->Internals->OperateButton = new QPushButton(this);
  this->Internals->OperateButton->setText("Apply");
  QPalette applyPalette = this->Internals->OperateButton->palette();
  applyPalette.setColor(QPalette::Active, QPalette::Button, QColor(161, 213, 135));
  applyPalette.setColor(QPalette::Inactive, QPalette::Button, QColor(161, 213, 135));
  this->Internals->OperateButton->setPalette(applyPalette);
  this->Internals->OperateButton->setDefault(true);

  QHBoxLayout* operatorLayout = new QHBoxLayout();
  operatorLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  this->Internals->OperationCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  this->Internals->OperateButton->setMinimumHeight(32);
  this->Internals->OperateButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  operatorLayout->addWidget(this->Internals->OperateButton);
  operatorLayout->addWidget(this->Internals->OperationCombo);
  this->Internals->WidgetLayout->addLayout(operatorLayout);
  this->Internals->WidgetLayout->addLayout(this->Internals->OperationsLayout);

  // signals/slots
  QObject::connect(this->Internals->OperationCombo,
    SIGNAL(currentIndexChanged(int)), this, SLOT(onOperationSelected()));
  QObject::connect(this->Internals->OperateButton,
    SIGNAL(clicked()), this, SLOT(onOperate()));

}

//-----------------------------------------------------------------------------
QSize qtModelOperationWidget::sizeHint() const
{
  if(QWidget* opW = this->Internals->OperationsLayout->currentWidget())
    {
//    std::cout << "Use current op widget for size hint \n"
//    << "width: " << opW->width() << "height: " << opW->height() << "\n";
    return QSize(opW->width(),
                 opW->height() + this->Internals->OperationCombo->height() + 20);
    }
  return QSize(450, 250);
}

//----------------------------------------------------------------------------
void qtModelOperationWidget::setSession(smtk::model::SessionPtr session)
{
  if(this->Internals->CurrentSession.lock() == session)
    return;

  // clean up current UI
  QStackedLayout* opLayout = this->Internals->OperationsLayout;
  for(int i=0; i< opLayout->count(); ++i)
    {
    if(opLayout->widget(i))
      delete opLayout->widget(i);
    }

  this->Internals->CurrentSession = session;
  this->Internals->OperationCombo->blockSignals(true);
  this->Internals->OperationCombo->clear();
  if(session)
    {
    StringList opNames = session->operatorNames(false);
    std::sort(opNames.begin(), opNames.end());
    for(StringList::const_iterator it = opNames.begin();
        it != opNames.end(); ++it)
      {
      this->Internals->OperationCombo->addItem((*it).c_str());
      }
    }
  this->Internals->OperationCombo->blockSignals(false);
  this->Internals->OperatorMap.clear();
}
//----------------------------------------------------------------------------
void qtModelOperationWidget::setModelView(qtModelView* mv)
{
  if (this->Internals->ModelView != mv)
    {
    this->Internals->ModelView = mv;
    }
}
//----------------------------------------------------------------------------
qtModelView *qtModelOperationWidget::modelView()
{
  return this->Internals->ModelView;
}

//----------------------------------------------------------------------------
void qtModelOperationWidget::cancelCurrentOperator()
{
  this->cancelOperator(this->Internals->CurrrentOpName);
}

//----------------------------------------------------------------------------
void qtModelOperationWidget::cancelOperator(const std::string& opName)
{
  if(this->Internals->OperatorMap.contains(opName))
    {
    OperatorPtr brOp = this->Internals->OperatorMap[opName].opPtr;
    emit this->operationCancelled(brOp);
    }
}

//----------------------------------------------------------------------------
bool qtModelOperationWidget::checkExistingOperator(const std::string& opName)
{
  // if the operator is already created, just set its UI to be current widget
  if(this->Internals->OperatorMap.contains(opName))
    {
    this->Internals->OperatorMap[opName].opUiView->requestModelEntityAssociation();
    this->Internals->OperationsLayout->setCurrentWidget(
      this->Internals->OperatorMap[opName].opUiParent);
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
bool qtModelOperationWidget::initOperatorUI(
  const smtk::model::OperatorPtr& brOp)
{
  std::string opName = brOp->name();
  std::string prevOpName = this->Internals->CurrrentOpName;
  if(!prevOpName.empty() && opName != prevOpName)
    {
    // we need to reset previous operator's UI
    this->cancelOperator(prevOpName);
    }

  // set the operator combobox to the corrent index
  if(opName != this->Internals->OperationCombo->currentText().toStdString())
    {
    StringList opNames = brOp->session()->operatorNames(false);
    std::sort(opNames.begin(), opNames.end());
    int idx = std::find(opNames.begin(), opNames.end(), opName) - opNames.begin();
    this->Internals->OperationCombo->blockSignals(true);
    this->Internals->OperationCombo->setCurrentIndex(idx);
    this->Internals->OperationCombo->blockSignals(false);
    }

  // if the operator is already created, just set its UI to be current widget
  if(this->checkExistingOperator(opName))
    {
    this->Internals->CurrrentOpName = opName;
    return true;
    }

  if(!brOp->specification())
    {
    return false;
    }

  SessionRef bs(brOp->manager(), brOp->session()->sessionId());
  this->setSession(bs.session());
  QFrame* opParent = new QFrame(this);
  QVBoxLayout* opLayout = new QVBoxLayout(opParent);
  opLayout->setMargin(0);

  smtk::attribute::AttributePtr att = brOp->specification();
  att->system()->setRefModelManager(brOp->manager());

  smtk::extension::qtUIManager* uiManager =
    new smtk::extension::qtUIManager(*(att->system()));
  uiManager->setActiveModelView(this->Internals->ModelView);

  // find out what view to use to construct the UI, if none is specified for this op
  // ( meaning if there is no "AttributeTypes" specified in view components' children,
  // or the att->type() is not included in any view "AttributeTypes" ),
  // use "Instanced" view by default
  smtk::common::ViewPtr opView;

  std::map<std::string, smtk::common::ViewPtr>::const_iterator it;
  for(it = att->system()->views().begin(); it != att->system()->views().end(); ++it)
    {
    int i = it->second->details().findChild("AttributeTypes");
    if(i < 0)
      {
      continue;
      }
    smtk::common::View::Component& comp = it->second->details().child(i);
    for(int ci = 0; ci < comp.numberOfChildren(); ++ci)
      {
      std::string optype;
      if(comp.child(ci).attribute("Type", optype) && optype == att->type())
        {
        opView = it->second;
        break;
        }
      }
    if(opView)
      break;
    }

  if(!opView || !uiManager->hasViewConstructor(opView->type()))
    {
    //Lets create a default view for the operator itself
    opView = smtk::common::View::New("Instanced", brOp->name());
    
    smtk::common::View::Component &comp =
      opView->details().addChild("InstancedAttributes").addChild("Att");
    comp.setAttribute("Type", att->type()).setAttribute("Name", att->name());  
    att->system()->addView(opView);
    }

  opView->details().setAttribute("TopLevel", "true");

  QObject::connect(uiManager, SIGNAL(fileItemCreated(smtk::extension::qtFileItem*)),
    this, SIGNAL(fileItemCreated(smtk::extension::qtFileItem*)));
  QObject::connect(uiManager, SIGNAL(modelEntityItemCreated(smtk::extension::qtModelEntityItem*)),
    this, SIGNAL(modelEntityItemCreated(smtk::extension::qtModelEntityItem*)));
  QObject::connect(uiManager, SIGNAL(meshSelectionItemCreated(smtk::extension::qtMeshSelectionItem*)),
    this, SLOT(onMeshSelectionItemCreated(smtk::extension::qtMeshSelectionItem*)));
  QObject::connect(uiManager, SIGNAL(entitiesSelected(const smtk::common::UUIDs&)),
    this, SIGNAL(entitiesSelected(const smtk::common::UUIDs&)));

  qtModelOperationWidgetInternals::OperatorInfo opInfo;
  opInfo.opPtr = brOp;
  opInfo.opUiParent = opParent;
  opInfo.opUiManager = uiManager;
  this->Internals->OperatorMap[opName] = opInfo;

  qtBaseView* theView = uiManager->setSMTKView(opView, opParent, false);
  theView->requestModelEntityAssociation();

  this->Internals->OperatorMap[opName].opUiView = theView;

  this->Internals->OperationsLayout->addWidget(opParent);
  this->Internals->OperationsLayout->setCurrentWidget(opParent);
  this->Internals->CurrrentOpName = opName;
  return true;
}

//----------------------------------------------------------------------------
bool qtModelOperationWidget::setCurrentOperator(
  const std::string& opName, smtk::model::SessionPtr session)
{
  this->setSession(session);
  if(!session)
    return false;

  StringList opNames = session->operatorNames(false);
  std::sort(opNames.begin(), opNames.end());
  int idx = std::find(opNames.begin(), opNames.end(), opName) - opNames.begin();
  if(this->Internals->OperationCombo->currentIndex() != idx)
    {
    this->Internals->OperationCombo->setCurrentIndex(idx);
    return true;
    }

  OperatorPtr brOp = this->Internals->OperatorMap.contains(opName) ?
    this->Internals->OperatorMap[opName].opPtr :
    session->op(opName); // create the operator

  if (!brOp)
    {
    std::cerr
      << "Could not create operator: \"" << opName << "\" for session"
      << " \"" << session->name() << "\""
      << " (" << session->sessionId() << ")\n";
    return false;
    }
  return this->initOperatorUI(brOp);
}
//----------------------------------------------------------------------------
smtk::model::OperatorPtr qtModelOperationWidget::existingOperator(
  const std::string& opName)
{
  OperatorPtr brOp;
  if(this->Internals->OperatorMap.contains(opName))
    {
    brOp = this->Internals->OperatorMap[opName].opPtr;
    }

  return brOp;
}
//----------------------------------------------------------------------------
void qtModelOperationWidget::expungeEntities(
        const smtk::model::EntityRefs& expungedEnts)
{
  QMapIterator<std::string, qtModelOperationWidgetInternals::OperatorInfo > it(
    this->Internals->OperatorMap);
  while(it.hasNext())
    {
    it.next();
    if(it.value().opPtr && it.value().opPtr->specification()->isValid())
      {
      for (EntityRefs::const_iterator bit = expungedEnts.begin();
        bit != expungedEnts.end(); ++bit)
        {
        //std::cout << "expunge from op " << bit->flagSummary(0) << " " << bit->entity() << "\n";
        it.value().opPtr->specification()->disassociateEntity(*bit);
        }
      }
    it.value().opUiView->updateUI();
    }
}

//----------------------------------------------------------------------------
void qtModelOperationWidget::onOperationSelected()
{
  this->setCurrentOperator(
    this->Internals->OperationCombo->currentText().toStdString(),
    this->Internals->CurrentSession.lock());
}
//----------------------------------------------------------------------------
void qtModelOperationWidget::onOperate()
{
  std::string opName = this->Internals->OperationCombo->currentText().toStdString();
  if(this->Internals->OperatorMap.contains(opName))
    {
    OperatorPtr brOp = this->Internals->OperatorMap[opName].opPtr;
    emit this->operationRequested(brOp);
    }
}

//----------------------------------------------------------------------------
void qtModelOperationWidget::onMeshSelectionItemCreated(
  smtk::extension::qtMeshSelectionItem* meshItem)
{
  if(this->Internals->CurrentSession.lock())
    {
    std::string opName = this->Internals->OperationCombo->currentText().toStdString();
    smtk::common::UUID sessId =
      this->Internals->CurrentSession.lock()->sessionId();
    emit this->meshSelectionItemCreated(meshItem, opName, sessId);
    }
}
