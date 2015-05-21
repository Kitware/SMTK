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

using namespace smtk::attribute;
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
  QPointer<qtInstancedView> opUiView;
  };

  smtk::model::Session::WeakPtr CurrentSession;
  QVBoxLayout* WidgetLayout;
  QPointer<QComboBox> OperationCombo;
  QStackedLayout* OperationsLayout;
  QPointer<QPushButton> OperateButton;
  // <operator-name, <opPtr, opUI-parent> >
  QMap<std::string, OperatorInfo > OperatorMap;
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
//  QLabel* opLabel = new QLabel("Operator:");
//  opLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  this->Internals->OperateButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
//  operatorLayout->addWidget(opLabel);
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
bool qtModelOperationWidget::setCurrentOperation(
  const smtk::model::OperatorPtr& brOp)
{
  if(!brOp->specification()->isValid())
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

  //Lets see if we have root view for the operations
  smtk::common::ViewPtr rootView = att->system()->findView("Operators");
  if (!rootView)
    {
    // Lets create one
    rootView = smtk::common::View::New("Root", "Operators");
    att->system()->addView(rootView);
    rootView->details().addChild("Views");
    }
  
  //Lets create a view for the operator itself
  smtk::common::ViewPtr instanced = smtk::common::View::New(brOp->name(), "Instanced");
  smtk::common::View::Component &comp =
    instanced->details().addChild("InstancedAttributes").addChild("Att");
  comp.setAttribute("Type", att->type());
  comp.setContents(att->name());
  int compIndex = rootView->details().findChild("Views");
  if (compIndex < 0)
    {
    // There is no component called "Views" - better add one
    rootView->details().addChild("Views");
    compIndex = rootView->details().findChild("Views");
    }
  
  rootView->details().child(compIndex).addChild("View")
    .setAttribute("Title",instanced->title());
  
  att->system()->addView(instanced);

  smtk::attribute::qtUIManager* uiManager =
    new smtk::attribute::qtUIManager(*(att->system()), "Operators");


  QObject::connect(uiManager, SIGNAL(fileItemCreated(smtk::attribute::qtFileItem*)),
    this, SIGNAL(fileItemCreated(smtk::attribute::qtFileItem*)));
  QObject::connect(uiManager, SIGNAL(modelEntityItemCreated(smtk::attribute::qtModelEntityItem*)),
    this, SIGNAL(modelEntityItemCreated(smtk::attribute::qtModelEntityItem*)));
  QObject::connect(uiManager, SIGNAL(meshSelectionItemCreated(smtk::attribute::qtMeshSelectionItem*)),
    this, SLOT(onMeshSelectionItemCreated(smtk::attribute::qtMeshSelectionItem*)));

  qtInstancedView* theView = qobject_cast<qtInstancedView*>(
    uiManager->initializeView(opParent, instanced, false));
  theView->requestModelEntityAssociation();
  qtModelOperationWidgetInternals::OperatorInfo opInfo;
  opInfo.opPtr = brOp;
  opInfo.opUiParent = opParent;
  opInfo.opUiManager = uiManager;
  opInfo.opUiView = theView;

  this->Internals->OperatorMap[brOp->name()] = opInfo;
  this->Internals->OperationsLayout->addWidget(opParent);
  this->Internals->OperationsLayout->setCurrentWidget(opParent);

  if(brOp->name() != this->Internals->OperationCombo->currentText().toStdString())
    {
    StringList opNames = brOp->session()->operatorNames(false);
    std::sort(opNames.begin(), opNames.end());
    int idx = std::find(opNames.begin(), opNames.end(), brOp->name()) - opNames.begin();
    this->Internals->OperationCombo->blockSignals(true);
    this->Internals->OperationCombo->setCurrentIndex(idx);
    this->Internals->OperationCombo->blockSignals(false);
    }
  return true;
}

//----------------------------------------------------------------------------
bool qtModelOperationWidget::setCurrentOperation(
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

  if(this->Internals->OperatorMap.contains(opName))
    {
    // check if the operation has an item "refetchfromserver", if yes
    // we have to recreate the UI since the spec may have been changed.
    smtk::attribute::VoidItemPtr refetchItem =
      smtk::dynamic_pointer_cast<smtk::attribute::VoidItem>(
      this->Internals->OperatorMap[opName].opPtr->specification()->find("refetchfromserver"));
    if(!refetchItem || !refetchItem->isEnabled())
      {
      this->Internals->OperatorMap[opName].opUiView->requestModelEntityAssociation();
      this->Internals->OperationsLayout->setCurrentWidget(
        this->Internals->OperatorMap[opName].opUiParent);
      return true;
      }
    else
      this->Internals->OperatorMap.erase(this->Internals->OperatorMap.find(opName));
    }

  // create the operator
  OperatorPtr brOp = session->op(opName);
  if (!brOp)
    {
    std::cerr
      << "Could not create operator: \"" << opName << "\" for session"
      << " \"" << session->name() << "\""
      << " (" << session->sessionId() << ")\n";
    return false;
    }
  return this->setCurrentOperation(brOp);
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
  this->setCurrentOperation(
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
  smtk::attribute::qtMeshSelectionItem* meshItem)
{
  std::string opName = this->Internals->OperationCombo->currentText().toStdString();
  if(this->Internals->CurrentSession.lock())
    {
    smtk::common::UUID sessId =
      this->Internals->CurrentSession.lock()->sessionId();
    emit this->meshSelectionItemCreated(meshItem, opName, sessId);
    }
}
