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
#include "smtk/extension/qt/qtModelEntityItem.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/System.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"

#include "smtk/model/Bridge.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Operator.h"
#include "smtk/model/StringData.h"
#include "smtk/view/Root.h"
#include "smtk/view/Instanced.h"

#include <QStackedLayout>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QComboBox>
#include <QPointer>
#include <QSpacerItem>

#include <map>

using namespace smtk::attribute;
using namespace smtk::model;

//----------------------------------------------------------------------------
class qtModelOperationWidgetInternals
{
public:

  smtk::model::WeakOperatorPtr CurrentOp;
  smtk::model::Bridge::WeakPtr CurrentBridge;
  QVBoxLayout* WidgetLayout;
  QPointer<QComboBox> OperationCombo;
  QStackedLayout* OperationsLayout;
  QPointer<QPushButton> OperateButton;
  // <operator-name, <opPtr, opUI-parent
  QMap<std::string, QPair<smtk::model::OperatorPtr, QFrame*> > OperatorMap;
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
  this->setBridge(smtk::model::BridgePtr());
  delete this->Internals;
}

//----------------------------------------------------------------------------
void qtModelOperationWidget::initWidget( )
{
  this->Internals->WidgetLayout = new QVBoxLayout(this);
  this->Internals->OperationCombo = new QComboBox(this);
  this->Internals->OperationsLayout = new QStackedLayout();
  this->Internals->OperateButton = new QPushButton(this);
  this->Internals->OperateButton->setText("Operate");

  this->Internals->WidgetLayout->addWidget(this->Internals->OperationCombo);
  this->Internals->WidgetLayout->addLayout(this->Internals->OperationsLayout);
  this->Internals->WidgetLayout->addWidget(this->Internals->OperateButton);
  this->Internals->WidgetLayout->addItem(
    new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Maximum));

  // signals/slots
  QObject::connect(this->Internals->OperationCombo,
    SIGNAL(currentIndexChanged(int)), this, SLOT(onOperationSelected()));
  QObject::connect(this->Internals->OperateButton,
    SIGNAL(clicked()), this, SLOT(onOperate()));

}

//----------------------------------------------------------------------------
void qtModelOperationWidget::setBridge(smtk::model::BridgePtr bridge)
{
  if(this->Internals->CurrentBridge.lock() == bridge)
    return;

  // clean up current UI
  QStackedLayout* opLayout = this->Internals->OperationsLayout;
  for(int i=0; i< opLayout->count(); ++i)
    {
    if(opLayout->widget(i))
      delete opLayout->widget(i);
    }

  this->Internals->CurrentBridge = bridge;
  this->Internals->OperationCombo->blockSignals(true);
  this->Internals->OperationCombo->clear();
  if(bridge)
    {
    StringList opNames = bridge->operatorNames();
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
  const std::string& opName, smtk::model::BridgePtr bridge)
{
  this->setBridge(bridge);
  if(!bridge)
    return false;

  StringList opNames = bridge->operatorNames();
  int pos = std::find(opNames.begin(), opNames.end(), opName) - opNames.begin();
  if(this->Internals->OperationCombo->currentIndex() != pos)
    {
    this->Internals->OperationCombo->setCurrentIndex(pos);
    return true;
    }

  if(this->Internals->OperatorMap.contains(opName))
    {
    this->Internals->OperationsLayout->setCurrentWidget(
      this->Internals->OperatorMap[opName].second);
    return true;
    }

  // not yet existed
  OperatorPtr brOp = bridge->op(opName);
  if (!brOp)
    {
    std::cerr
      << "Could not create operator: \"" << opName << "\" for bridge"
      << " \"" << bridge->name() << "\""
      << " (" << bridge->sessionId() << ")\n";
    return false;
    }

  if(!brOp->specification()->isValid())
    {
    return false;
    }
  QFrame* opParent = new QFrame(this);
  QVBoxLayout* opLayout = new QVBoxLayout(opParent);
  opLayout->setMargin(0);

  smtk::attribute::AttributePtr att = brOp->specification();
  attribute::DefinitionPtr attDef = att->definition();
  att->system()->setRefModelManager(bridge->manager());
  smtk::attribute::qtUIManager uiManager(*(att->system()));
  smtk::view::RootPtr rootView = uiManager.attSystem()->rootView();
  smtk::view::InstancedPtr instanced = smtk::view::Instanced::New(brOp->name());
  instanced->addInstance(att);
  rootView->addSubView(instanced);
  QObject::connect(&uiManager, SIGNAL(fileItemCreated(smtk::attribute::qtFileItem*)),
    this, SIGNAL(fileItemCreated(smtk::attribute::qtFileItem*)));

  uiManager.initializeView(opParent, instanced, false);
  QPair<smtk::model::OperatorPtr, QFrame*> opPair;
  opPair.first = brOp;
  opPair.second = opParent;
  this->Internals->OperatorMap[opName] = opPair;
  this->Internals->OperationsLayout->addWidget(opParent);
  this->Internals->OperationsLayout->setCurrentWidget(opParent);

  return true;
}

//----------------------------------------------------------------------------
void qtModelOperationWidget::onOperationSelected()
{
  this->setCurrentOperation(
    this->Internals->OperationCombo->currentText().toStdString(),
    this->Internals->CurrentBridge.lock());
}
//----------------------------------------------------------------------------
void qtModelOperationWidget::onOperate()
{
  std::string opName = this->Internals->OperationCombo->currentText().toStdString();
  if(this->Internals->OperatorMap.contains(opName))
    {
    OperatorPtr brOp = this->Internals->OperatorMap[opName].first;
    emit this->operationRequested(brOp);
    }
}
