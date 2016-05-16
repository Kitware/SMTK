//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "qtPolygonEdgeOperationView.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtModelOperationWidget.h"
#include "smtk/extension/qt/qtModelView.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/extension/paraview/widgets/pqArcWidget.h"
#include "smtk/bridge/polygon/qt/pqArcWidgetManager.h"
#include "smtk/bridge/polygon/qt/pqPolygonArc.h"
#include "smtk/common/View.h"
#include "smtk/model/Operator.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqRenderView.h"
#include "pqServer.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPointer>
#include <QCheckBox>
#include <QLabel>
#include <QTableWidget>
#include <QScrollArea>
#include <QMessageBox>
#include <QSpacerItem>

using namespace smtk::attribute;

//----------------------------------------------------------------------------
class qtPolygonEdgeOperationViewInternals
{
public:
  qtPolygonEdgeOperationViewInternals()
    {
    }
  ~qtPolygonEdgeOperationViewInternals()
    {
    if(ArcManager)
      delete ArcManager;
    if(CurrentAtt)
      delete CurrentAtt;
    }

  QPointer<pqArcWidgetManager> ArcManager;
  QPointer<qtAttribute> CurrentAtt;
  QPointer<QVBoxLayout> EditorLayout;
};

//----------------------------------------------------------------------------
qtBaseView *
qtPolygonEdgeOperationView::createViewWidget(const ViewInfo &info)
{
  qtPolygonEdgeOperationView *view = new qtPolygonEdgeOperationView(info);
  view->buildUI();
  return view;
}

//----------------------------------------------------------------------------
qtPolygonEdgeOperationView::
qtPolygonEdgeOperationView(const ViewInfo &info) :
  qtBaseView(info)
{
  this->Internals = new qtPolygonEdgeOperationViewInternals;
}

//----------------------------------------------------------------------------
qtPolygonEdgeOperationView::~qtPolygonEdgeOperationView()
{
  delete this->Internals;
}
//----------------------------------------------------------------------------
void qtPolygonEdgeOperationView::createWidget( )
{
  smtk::common::ViewPtr view = this->getObject();
  if (!view)
    {
    return;
    }

  QVBoxLayout* parentlayout = static_cast<QVBoxLayout*> (
    this->parentWidget()->layout());
  if(this->Widget)
    {
    if(parentlayout)
      {
      parentlayout->removeWidget(this->Widget);
      }
    delete this->Widget;
    }

  this->Widget = new QFrame(this->parentWidget());
  //create the layout for the tabs area
  QVBoxLayout* layout = new QVBoxLayout(this->Widget);
  layout->setMargin(0);
  this->Widget->setLayout( layout );

  this->Internals->EditorLayout = new QVBoxLayout;
  this->updateAttributeData();
  layout->addLayout(this->Internals->EditorLayout);
  layout->addItem(
    new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Maximum));

  QObject::disconnect(this->uiManager()->activeModelView());
  QObject::connect(this->uiManager()->activeModelView(),
    SIGNAL(operationCancelled(const smtk::model::OperatorPtr&)),
    this, SLOT(cancelOperation(const smtk::model::OperatorPtr&)));
}

inline qtAttribute* internal_createAttUI(
  smtk::attribute::AttributePtr att, QWidget* pw, qtBaseView* view)
{
  if(att && att->numberOfItems()>0)
    {
    qtAttribute* attInstance = new qtAttribute(att, pw, view);
    if(attInstance)
      {
      //Without any additional info lets use a basic layout with model associations
      // if any exists
      attInstance->createBasicLayout(true);
      attInstance->widget()->setObjectName("polygonEdgeOpEditor");
      QVBoxLayout* parentlayout = static_cast<QVBoxLayout*> (pw->layout());
      parentlayout->insertWidget(0, attInstance->widget());
      return attInstance;
      }
    }
  return NULL;
}

//----------------------------------------------------------------------------
void qtPolygonEdgeOperationView::updateAttributeData()
{
  smtk::common::ViewPtr view = this->getObject();
  if (!view || !this->Widget)
    {
    return;
    }

  if(this->Internals->CurrentAtt)
    {
    delete this->Internals->CurrentAtt;
    }

  int i = view->details().findChild("AttributeTypes");
  if(i < 0)
    {
    return;
    }
  smtk::common::View::Component& comp = view->details().child(i);
  // for now, we only handle "edit edge" operator; later we could use a list
  // to show all operators (attributes), and a panel underneath to edit current
  // selected operator.
  std::string defName;
  for(int ci = 0; ci < comp.numberOfChildren(); ++ci)
    {
    smtk::common::View::Component &attComp = comp.child(ci);
    if (attComp.name() != "Att")
      {
      continue;
      }
    std::string optype;
    if(attComp.attribute("Type", optype) && optype == "edit edge")
      {
      defName = optype;
      break;
      }
    }
  if(defName.empty())
    {
    return;
    }

  smtk::model::OperatorPtr edgeOp = this->uiManager()->activeModelView()->
                       operatorsWidget()->existingOperator(defName);
  // expecting only 1 instance of the op?
  smtk::attribute::AttributePtr att = edgeOp->specification();
  this->Internals->CurrentAtt = internal_createAttUI(att, this->Widget, this);

  pqRenderView* renView = qobject_cast<pqRenderView*>(pqActiveObjects::instance().activeView());
  pqServer* server = pqApplicationCore::instance()->getActiveServer();
  if(!this->Internals->ArcManager)
    {
    this->Internals->ArcManager = new pqArcWidgetManager(server, renView);
    QObject::connect(this->Internals->ArcManager, SIGNAL(Finish()),
                     this, SLOT(operationDone()));
    QObject::connect(this->Internals->ArcManager,SIGNAL(startPicking()),
      this,SLOT(clearSelection()));
    }
  else
    {
    this->Internals->ArcManager->reset();
    this->Internals->ArcManager->updateActiveView(renView);
    this->Internals->ArcManager->updateActiveServer(server);
    }

  pqPolygonArc *objArc = new pqPolygonArc;
  objArc->setEdgeOperator(edgeOp);
  this->Internals->ArcManager->setActiveArc( objArc );
  QObject::connect(objArc, SIGNAL(operationRequested(const smtk::model::OperatorPtr&)),
       this, SLOT(requestOperation(const smtk::model::OperatorPtr&)));
  QObject::connect(objArc, SIGNAL(activateModel(const smtk::common::UUID&)),
       this->uiManager()->activeModelView()->operatorsWidget(),
       SLOT(setOperationTargetActive(const smtk::common::UUID&)));


  smtk::attribute::StringItem::Ptr optypeItem = att->findString("Operation");
  optypeItem->setToDefault();// default to "Create"

  this->valueChanged(optypeItem);
}

//----------------------------------------------------------------------------
void qtPolygonEdgeOperationView::requestOperation(const smtk::model::OperatorPtr& op)
{
  this->uiManager()->activeModelView()->requestOperation(op, false);
}

//----------------------------------------------------------------------------
void qtPolygonEdgeOperationView::cancelOperation(const smtk::model::OperatorPtr& op)
{
  if( !op || !this->Widget || !this->Internals->CurrentAtt
      || !this->Internals->ArcManager)
    return;

  this->Internals->ArcManager->cancelOperation(op);
}

//----------------------------------------------------------------------------
void qtPolygonEdgeOperationView::operationDone()
{
  if(!this->Internals->CurrentAtt || !this->Widget
     || !this->Internals->ArcManager)
    return;

  smtk::attribute::AttributePtr att =  this->Internals->CurrentAtt->attribute();
  smtk::attribute::StringItem::Ptr optypeItem = att->findString("Operation");
  std::string optype = optypeItem->value();
  // If previous op is "Create", set the operation to "Edit"
  if(optype == "Create")
    {
    optypeItem->setValue("Edit");// set to "Edit Edge"
    delete this->Internals->CurrentAtt;
    this->Internals->CurrentAtt = internal_createAttUI(att, this->Widget, this);
    }
}

//----------------------------------------------------------------------------
void qtPolygonEdgeOperationView::clearSelection()
{  
  this->uiManager()->activeModelView()->clearSelection();
}

//----------------------------------------------------------------------------
void qtPolygonEdgeOperationView::valueChanged(smtk::attribute::ItemPtr valitem)
{
  if(!this->Internals->CurrentAtt || !this->Widget
     || !this->Internals->ArcManager)
    return;

  // Based on which type of operations, we update UI panel
  // default to create arc mode
  smtk::attribute::StringItem::Ptr optypeItem =
    smtk::dynamic_pointer_cast<smtk::attribute::StringItem>(valitem);
  if(!optypeItem || optypeItem->name() != "Operation")
    return;

  QWidget* prevUiWidget = this->Internals->ArcManager->getActiveWidget();
  pq3DWidget* prev3dWidget = qobject_cast<pq3DWidget*>(prevUiWidget);

  std::string optype = optypeItem->value();
  if(optype == "Create")
    {
    this->Internals->ArcManager->create();
    }
  else if(optype == "Edit")
    {
    this->Internals->ArcManager->edit();
    }

  QWidget* pWidget = this->Widget;
  QWidget* selUiWidget = this->Internals->ArcManager->getActiveWidget();
  if(!selUiWidget)
    {
    return;
    }
  pq3DWidget* sel3dWidget = qobject_cast<pq3DWidget*>(selUiWidget);
  QString widgetName = selUiWidget->objectName();
 
  // we need to make invisible all 3d widget UI panels
  QList< pq3DWidget* > user3dWidgets = pWidget->findChildren<pq3DWidget*>();
  QList< QWidget* > userUiWidgets;
  // if this is not a 3d widget
  if(!sel3dWidget)
    {
    userUiWidgets = pWidget->findChildren<QWidget*>(widgetName);
    }
  userUiWidgets.append(reinterpret_cast< QList<QWidget*>& >(user3dWidgets));
  bool found = false;
  for(int i=0; i<userUiWidgets.count(); i++)
    {
    if(!found && userUiWidgets.value(i) == selUiWidget)
      {
      found = true;
      }
    else
      {
      userUiWidgets.value(i)->setVisible(0);
      }
    }
  if(!found)
    {
    selUiWidget->setParent(pWidget);
    this->Internals->EditorLayout->addWidget(selUiWidget);
    }

  // turn off previous active widgets if they are not the active one anymore
  if(prev3dWidget && prev3dWidget != sel3dWidget)
    {
    prev3dWidget->deselect();
    prev3dWidget->setVisible(false);
    prev3dWidget->setEnabled(false);
    }
  else if(prevUiWidget && prevUiWidget != selUiWidget)
    {
    prevUiWidget->setVisible(false);
    prevUiWidget->setEnabled(false);
    prevUiWidget->hide();
    }


  if(sel3dWidget && sel3dWidget->widgetVisible())
    {
    sel3dWidget->select();
    sel3dWidget->setVisible(true);
    sel3dWidget->setEnabled(true);
    }
  else if(!sel3dWidget)
    {
    selUiWidget->setVisible(true);
    selUiWidget->setEnabled(true);
    selUiWidget->show();
    }
/*
  else // turn off the active widget
    {
    if(sel3dWidget && sel3dWidget->widgetVisible())
      {
      sel3dWidget->deselect();
      sel3dWidget->setVisible(false);
      sel3dWidget->setEnabled(false);
      }
    else if(!sel3dWidget)
      {
      selUiWidget->setVisible(false);
      selUiWidget->setEnabled(false);
      selUiWidget->hide();
      }
    }
*/
}

//----------------------------------------------------------------------------
void qtPolygonEdgeOperationView::showAdvanceLevelOverlay(bool show)
{
  this->qtBaseView::showAdvanceLevelOverlay(show);
}

//----------------------------------------------------------------------------
void qtPolygonEdgeOperationView::requestModelEntityAssociation()
{
  this->updateAttributeData();
}
