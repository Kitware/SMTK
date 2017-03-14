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
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtModelOperationWidget.h"
#include "smtk/extension/qt/qtModelView.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/bridge/polygon/qt/pqArcWidgetManager.h"
#include "smtk/bridge/polygon/qt/pqPolygonArc.h"
#include "smtk/bridge/polygon/qt/pqSplitEdgeWidget.h"
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

using namespace smtk::extension;

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

  bool needArc(const smtk::model::OperatorPtr& op)
  {
    bool able2Op = op
                   && (op->name() == "tweak edge"
                      || op->name() == "create edge")
                   && op->ensureSpecification()
                   ;
    if(!able2Op)
      {
      return able2Op;
      }

    // for create-edge operation, we only handle "interactive widget" case
    if(op->name() == "create edge")
      {
      smtk::attribute::IntItem::Ptr optypeItem =
        op->specification()->findInt("construction method");
      able2Op = optypeItem && (optypeItem->discreteIndex(0) == 2);
      }

    return able2Op;
  }

  qtAttribute* createAttUI(
    smtk::attribute::AttributePtr att, QWidget* pw, qtBaseView* view)
  {
    if(att && att->numberOfItems()>0)
      {
      qtAttribute* attInstance = new qtAttribute(att, pw, view);
      if(attInstance && attInstance->widget())
        {
        //Without any additional info lets use a basic layout with model associations
        // if any exists
        attInstance->createBasicLayout(true);
        attInstance->widget()->setObjectName("polygonEdgeOpEditor");
        QVBoxLayout* parentlayout = static_cast<QVBoxLayout*> (pw->layout());
        parentlayout->insertWidget(0, attInstance->widget());
        }
      return attInstance;
      }
    return NULL;
  }

  QPointer<pqArcWidgetManager> ArcManager;
  QPointer<qtAttribute> CurrentAtt;
  QPointer<QVBoxLayout> EditorLayout;

  QPointer<pqSplitEdgeWidget> SplitEdgeWidget;
  smtk::weak_ptr<smtk::model::Operator> CurrentOp;
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
void qtPolygonEdgeOperationView::attributeModified()
{
  this->Internals->ArcManager->
    enableApplyButton(this->Internals->CurrentAtt->attribute()->isValid());
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
  this->Widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

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
  // for now, we only handle "tweak edge" operator; later we could use a list
  // to show all operators (attributes), and a panel underneath to edit current
  // selected operator.
  std::string defName;
  for(std::size_t ci = 0; ci < comp.numberOfChildren(); ++ci)
    {
    smtk::common::View::Component &attComp = comp.child(ci);
    if (attComp.name() != "Att")
      {
      continue;
      }
    std::string optype;
    if(attComp.attribute("Type", optype) &&
      (optype == "tweak edge" || optype == "create edge" || "split edge"))
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
  this->Internals->CurrentOp = edgeOp;
  // expecting only 1 instance of the op?
  smtk::attribute::AttributePtr att = edgeOp->specification();
  this->Internals->CurrentAtt = this->Internals->createAttUI(att, this->Widget, this);
  // The arc widget interaction is only needed for:
  // * create edge op : with "construction method" set to "interactive widget"
  // * tweak edge
  if(this->Internals->needArc(edgeOp))
    {
    pqRenderView* renView = qobject_cast<pqRenderView*>(pqActiveObjects::instance().activeView());
    pqServer* server = pqApplicationCore::instance()->getActiveServer();
    if(!this->Internals->ArcManager)
      {
      this->Internals->ArcManager = new pqArcWidgetManager(server, renView);
      QObject::connect(this->Internals->ArcManager, SIGNAL(operationDone()),
                       this, SLOT(arcOperationDone()));
      QObject::connect(this->Internals->ArcManager,SIGNAL(startPicking()),
        this,SLOT(clearSelection()));
      }
    else
      {
      this->Internals->ArcManager->reset();
      this->Internals->ArcManager->updateActiveView(renView);
      this->Internals->ArcManager->updateActiveServer(server);
      }

  if (this->Internals->CurrentAtt)
    {
      QObject::connect(this->Internals->CurrentAtt,SIGNAL(modified()),
        this,SLOT(attributeModified()));
      this->Internals->ArcManager->enableApplyButton(att->isValid());
    }

    pqPolygonArc *objArc = new pqPolygonArc;
    objArc->setEdgeOperator(edgeOp);
    this->Internals->ArcManager->setActiveArc( objArc );
    QObject::connect(objArc, SIGNAL(operationRequested(const smtk::model::OperatorPtr&)),
         this, SLOT(requestOperation(const smtk::model::OperatorPtr&)));
    QObject::connect(objArc, SIGNAL(activateModel(const smtk::common::UUID&)),
         this->uiManager()->activeModelView()->operatorsWidget(),
         SLOT(setOperationTargetActive(const smtk::common::UUID&)));
    }

  this->operationSelected(edgeOp);
}

//----------------------------------------------------------------------------
void qtPolygonEdgeOperationView::requestOperation(const smtk::model::OperatorPtr& op)
{
  if(!op || !op->specification())
    {
    return;
    }
  this->uiManager()->activeModelView()->requestOperation(op, false);
}

//----------------------------------------------------------------------------
void qtPolygonEdgeOperationView::cancelOperation(const smtk::model::OperatorPtr& op)
{
  if( !op || !this->Widget || !this->Internals->CurrentAtt )
    return;
  if(this->Internals->ArcManager)
    {
    this->Internals->ArcManager->cancelOperation(op);
    }
  if(this->Internals->SplitEdgeWidget && this->Internals->SplitEdgeWidget->isActive())
    {
    this->Internals->SplitEdgeWidget->resetWidget();
    }
}
//----------------------------------------------------------------------------
void qtPolygonEdgeOperationView::valueChanged(smtk::attribute::ItemPtr valitem)
{
  // "create edge" op "construction method" changed.
  if(!this->Internals->CurrentAtt || !this->Widget
     || !this->Internals->CurrentOp.lock()
     || this->Internals->CurrentOp.lock()->name() != "create edge")
    {
    return;
    }

  smtk::attribute::IntItem::Ptr optypeItem =
    smtk::dynamic_pointer_cast<smtk::attribute::IntItem>(valitem);
  if(!optypeItem || optypeItem->name() != "construction method")
    {
    return;
    }
  this->operationSelected(this->Internals->CurrentOp.lock());
}

//----------------------------------------------------------------------------
void qtPolygonEdgeOperationView::arcOperationDone()
{
  if(!this->Internals->CurrentAtt || !this->Widget ||
     !this->Internals->CurrentOp.lock())
    {
    return;
    }

  this->operationSelected(this->Internals->CurrentOp.lock());
}

//----------------------------------------------------------------------------
void qtPolygonEdgeOperationView::clearSelection()
{
  this->uiManager()->activeModelView()->clearSelection();
}

//----------------------------------------------------------------------------
void qtPolygonEdgeOperationView::operationSelected(const smtk::model::OperatorPtr& op)
{
  if(!this->Internals->CurrentAtt || !this->Widget)
    return;

  // Based on which type of operations, we update UI panel
  if(this->Internals->needArc(op) && this->Internals->ArcManager)
    {
    if(this->Internals->SplitEdgeWidget)
      {
      this->Internals->SplitEdgeWidget->setVisible(false);
      }

    // This handles ui panel update when we need an arc-edit widget
    QWidget* prevUiWidget = this->Internals->ArcManager->getActiveWidget();
    if(op->name() == "create edge")
      {
      this->Internals->ArcManager->create();
      }
    else if(op->name() == "tweak edge")
      {
      this->Internals->ArcManager->edit();
      }

    QWidget* pWidget = this->Widget;
    QWidget* selUiWidget = this->Internals->ArcManager->getActiveWidget();
    if(!selUiWidget)
      {
      return;
      }
    QString widgetName = selUiWidget->objectName();

    QList<QWidget *> userUiWidgets =
        pWidget->findChildren<QWidget *>(widgetName);
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
      if (prevUiWidget && prevUiWidget != selUiWidget) {
        prevUiWidget->setVisible(false);
        prevUiWidget->setEnabled(false);
        prevUiWidget->hide();
      }

      if (!selUiWidget) {
        selUiWidget->setVisible(true);
        selUiWidget->setEnabled(true);
        selUiWidget->show();
      }
    }
  else
    {
    // This handles ui panel update when we don't need an arc-edit widget
    // first, hide arc widget related panels if they exist
    if(this->Internals->ArcManager)
      {
      QWidget* prevUiWidget = this->Internals->ArcManager->getActiveWidget();
      // turn off previous active widgets if they are not the active one anymore
      if (prevUiWidget) {
        prevUiWidget->setVisible(false);
        prevUiWidget->setEnabled(false);
        prevUiWidget->hide();
        }
      }

    // for edge operations that do not need arc-edit widget
    // * create edge op : "points" and "vertex ids" // no custom ui needed
    // * split edge: // this need a special
    if(op->name() == "split edge") // otherwise, only handle split edge custom ui
      {
      // If this is the same operator the SplitEdgeWidget is actively working with,
      // then nothing to do here
      if(this->Internals->SplitEdgeWidget &&
         this->Internals->SplitEdgeWidget->isActive() &&
         this->Internals->SplitEdgeWidget->edgeOperator() == op)
        {
        return;
        }

      if(!this->Internals->SplitEdgeWidget)
        {
        this->Internals->SplitEdgeWidget = new pqSplitEdgeWidget(this->Widget);
        this->Internals->EditorLayout->addWidget(this->Internals->SplitEdgeWidget);
        QObject::connect(this->Internals->SplitEdgeWidget,
          SIGNAL(operationRequested(const smtk::model::OperatorPtr&)),
          this, SLOT(requestOperation(const smtk::model::OperatorPtr&)));
        }

      pqRenderView* renView = qobject_cast<pqRenderView*>(pqActiveObjects::instance().activeView());
      this->Internals->SplitEdgeWidget->resetWidget();
      this->Internals->SplitEdgeWidget->setView(renView);
      this->Internals->SplitEdgeWidget->setEdgeOperator(op);
      this->Internals->SplitEdgeWidget->setVisible(true);
      }
    }
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
