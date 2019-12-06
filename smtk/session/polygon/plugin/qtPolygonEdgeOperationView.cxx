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
#include "smtk/extension/qt/qtActiveObjects.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtModelView.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/Resource.h"
#include "smtk/operation/Manager.h"
#include "smtk/session/polygon/qt/pqArcWidgetManager.h"
#include "smtk/session/polygon/qt/pqPolygonArc.h"
#include "smtk/session/polygon/qt/pqSplitEdgeWidget.h"
#include "smtk/view/Configuration.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqRenderView.h"
#include "pqServer.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPointer>
#include <QScrollArea>
#include <QSpacerItem>
#include <QTableWidget>
#include <QVBoxLayout>

using namespace smtk::extension;

class qtPolygonEdgeOperationViewInternals
{
public:
  qtPolygonEdgeOperationViewInternals(bool selectionManagerMode)
    : m_useSelectionManager(selectionManagerMode)
  {
  }
  ~qtPolygonEdgeOperationViewInternals()
  {
    if (ArcManager)
      delete ArcManager;
    if (CurrentAtt)
      delete CurrentAtt;
  }

  bool needArc(const smtk::operation::OperationPtr& op)
  {
    bool able2Op = op && (op->typeName() == "smtk::session::polygon::TweakEdge" ||
                           op->typeName() == "smtk::session::polygon::CreateEdge") &&
      op->ableToOperate();
    if (!able2Op)
    {
      return able2Op;
    }

    // for create-edge operation, we only handle "interactive widget" case
    if (op->typeName() == "smtk::session::polygon::CreateEdge")
    {
      smtk::attribute::IntItem::Ptr optypeItem = op->parameters()->findInt("construction method");
      able2Op = optypeItem && (optypeItem->discreteIndex(0) == 2);
    }

    return able2Op;
  }

  qtAttribute* createAttUI(smtk::attribute::AttributePtr att, QWidget* pw, qtBaseView* view)
  {
    if (att && att->numberOfItems() > 0)
    {
      smtk::view::Configuration::Component comp; // currently not used
      qtAttribute* attInstance = new qtAttribute(att, comp, pw, view);
      //attInstance->setUseSelectionManager(m_useSelectionManager);
      if (attInstance && attInstance->widget())
      {
        //Without any additional info lets use a basic layout with model associations
        // if any exists
        attInstance->createBasicLayout(true);
        attInstance->widget()->setObjectName("polygonEdgeOpEditor");
        QVBoxLayout* parentlayout = static_cast<QVBoxLayout*>(pw->layout());
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
  smtk::weak_ptr<smtk::operation::Operation> CurrentOp;
  std::map<smtk::common::UUID, int> EntitiesToVisibility;
  bool m_useSelectionManager;
};

qtBaseView* qtPolygonEdgeOperationView::createViewWidget(const ViewInfo& info)
{
  qtPolygonEdgeOperationView* view = new qtPolygonEdgeOperationView(info);
  view->buildUI();
  return view;
}

void qtPolygonEdgeOperationView::attributeModified()
{
  this->Internals->ArcManager->enableApplyButton(
    this->Internals->CurrentAtt->attribute()->isValid());
}

qtPolygonEdgeOperationView::qtPolygonEdgeOperationView(const ViewInfo& info)
  : qtBaseAttributeView(info)
{
  this->Internals = new qtPolygonEdgeOperationViewInternals(m_useSelectionManager);
}

qtPolygonEdgeOperationView::~qtPolygonEdgeOperationView()
{
  delete this->Internals;
}

void qtPolygonEdgeOperationView::createWidget()
{
  smtk::view::ConfigurationPtr view = this->getObject();
  if (!view)
  {
    return;
  }

  QVBoxLayout* parentlayout = static_cast<QVBoxLayout*>(this->parentWidget()->layout());
  if (this->Widget)
  {
    if (parentlayout)
    {
      parentlayout->removeWidget(this->Widget);
    }
    delete this->Widget;
  }

  this->Widget = new QFrame(this->parentWidget());
  //create the layout for the tabs area
  QVBoxLayout* layout = new QVBoxLayout(this->Widget);
  layout->setMargin(0);
  this->Widget->setLayout(layout);
  this->Widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

  this->Internals->EditorLayout = new QVBoxLayout;
  this->updateUI();
  layout->addLayout(this->Internals->EditorLayout);
  layout->addItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Maximum));

  QObject::disconnect(this->uiManager()->activeModelView());
  QObject::connect(this->uiManager()->activeModelView(),
    SIGNAL(operationCancelled(const smtk::operation::OperationPtr&)), this,
    SLOT(cancelOperation(const smtk::operation::OperationPtr&)));
}

void qtPolygonEdgeOperationView::onShowCategory()
{
  this->updateUI();
}

void qtPolygonEdgeOperationView::updateUI()
{
  smtk::view::ConfigurationPtr view = this->getObject();
  if (!view || !this->Widget)
  {
    return;
  }

  if (this->Internals->CurrentAtt)
  {
    delete this->Internals->CurrentAtt;
  }

  int i = view->details().findChild("AttributeTypes");
  if (i < 0)
  {
    return;
  }
  smtk::view::Configuration::Component& comp = view->details().child(i);
  // for now, we only handle "smtk::session::polygon::TweakEdge" operator; later we could use a list
  // to show all operators (attributes), and a panel underneath to edit current
  // selected operator.
  std::string defName;
  for (std::size_t ci = 0; ci < comp.numberOfChildren(); ++ci)
  {
    smtk::view::Configuration::Component& attComp = comp.child(ci);
    if (attComp.name() != "Att")
    {
      continue;
    }
    std::string optype;
    if (attComp.attribute("Type", optype) &&
      (optype == "smtk::session::polygon::TweakEdge" ||
          optype == "smtk::session::polygon::CreateEdge" || "smtk::session::polygon::SplitEdge"))
    {
      defName = optype;
      break;
    }
  }
  if (defName.empty())
  {
    return;
  }

  // FIXME: This used to fetch a pre-existing operation, which assumed there was only one.
  smtk::operation::OperationPtr edgeOp = this->uiManager()->operationManager()->create(defName);
  this->Internals->CurrentOp = edgeOp;
  // expecting only 1 instance of the op?
  smtk::attribute::AttributePtr att = edgeOp->parameters();
  this->Internals->CurrentAtt = this->Internals->createAttUI(att, this->Widget, this);
  // The arc widget interaction is only needed for:
  // * smtk::session::polygon::CreateEdge op : with "construction method" set to "interactive widget"
  // * smtk::session::polygon::TweakEdge
  if (this->Internals->needArc(edgeOp))
  {
    pqRenderView* renView = qobject_cast<pqRenderView*>(pqActiveObjects::instance().activeView());
    pqServer* server = pqApplicationCore::instance()->getActiveServer();
    if (!this->Internals->ArcManager)
    {
      this->Internals->ArcManager = new pqArcWidgetManager(server, renView);
      QObject::connect(
        this->Internals->ArcManager, SIGNAL(operationDone()), this, SLOT(arcOperationDone()));
      QObject::connect(
        this->Internals->ArcManager, SIGNAL(startPicking()), this, SLOT(clearSelection()));
    }
    else
    {
      this->Internals->ArcManager->reset();
      this->Internals->ArcManager->updateActiveView(renView);
      this->Internals->ArcManager->updateActiveServer(server);
    }

    if (this->Internals->CurrentAtt)
    {
      QObject::connect(
        this->Internals->CurrentAtt, SIGNAL(modified()), this, SLOT(attributeModified()));
      this->Internals->ArcManager->enableApplyButton(att->isValid());
    }

    pqPolygonArc* objArc = new pqPolygonArc;
    objArc->setEdgeOperation(edgeOp);
    this->Internals->ArcManager->setActiveArc(objArc);
    QObject::connect(objArc, SIGNAL(operationRequested(const smtk::operation::OperationPtr&)), this,
      SLOT(requestOperation(const smtk::operation::OperationPtr&)));
    /* FIXME: When the arc editor is active, keep UI synced.
    QObject::connect(objArc, SIGNAL(activateModel(const smtk::common::UUID&)),
      this->uiManager()->activeModelView()->operatorsWidget(),
      SLOT(setOperationTargetActive(const smtk::common::UUID&)));
      */
  }

  this->operationSelected(edgeOp);
}

void qtPolygonEdgeOperationView::requestOperation(const smtk::operation::OperationPtr& op)
{
  if (!op || !op->parameters())
  {
    return;
  }
  this->uiManager()->activeModelView()->requestOperation(op, false);
}

void qtPolygonEdgeOperationView::cancelOperation(const smtk::operation::OperationPtr& op)
{
  if (!op || !this->Widget || !this->Internals->CurrentAtt)
    return;
  if (this->Internals->ArcManager)
  {
    this->Internals->ArcManager->cancelOperation(op);
  }
  if (this->Internals->SplitEdgeWidget && this->Internals->SplitEdgeWidget->isActive())
  {
    this->Internals->SplitEdgeWidget->resetWidget();
  }
}

void qtPolygonEdgeOperationView::valueChanged(smtk::attribute::ItemPtr valitem)
{
  // "smtk::session::polygon::CreateEdge" op "construction method" changed.
  if (!this->Internals->CurrentAtt || !this->Widget || !this->Internals->CurrentOp.lock() ||
    this->Internals->CurrentOp.lock()->typeName() != "smtk::session::polygon::CreateEdge")
  {
    return;
  }

  smtk::attribute::IntItem::Ptr optypeItem =
    smtk::dynamic_pointer_cast<smtk::attribute::IntItem>(valitem);
  if (!optypeItem || optypeItem->name() != "construction method")
  {
    return;
  }
  this->operationSelected(this->Internals->CurrentOp.lock());
}

void qtPolygonEdgeOperationView::onHideAllFaces(bool status)
{
  // get all faces
  smtk::model::EntityRefs faces;
  // for polygon model, cells should be enough to cover all faces
  smtk::model::CellEntities facesInCell = qtActiveObjects::instance().activeModel().cells();
  for (const auto& face : facesInCell)
  {
    if (face.isFace())
    {
      faces.insert(face.as<smtk::model::EntityRef>());
    }
  }

  smtk::model::SessionRef activeSession = qtActiveObjects::instance().activeModel().session();
  smtk::operation::OperationPtr setPropertyOp; // = activeSession.op("set property");

  if (setPropertyOp && setPropertyOp->parameters())
  {
    setPropertyOp->parameters()->attributeResource()->associate(activeSession.resource());
    if (status)
    { // cache faces' visiblity and set them all to invisible
      for (const auto& face : faces)
      {
        int visible = face.visible();
        this->Internals->EntitiesToVisibility[face.entity()] = visible;
      }
      this->uiManager()->activeModelView()->setEntityVisibility(
        faces, smtk::mesh::MeshSets(), false, setPropertyOp);
    }
    else
    { // use the cached visibility to restore them
      smtk::model::EntityRefs visibleFaces, invisibleFaces;
      for (const auto& face : faces)
      {
        if (this->Internals->EntitiesToVisibility.find(face.entity()) !=
          this->Internals->EntitiesToVisibility.end())
        {
          if (this->Internals->EntitiesToVisibility[face.entity()])
          {
            visibleFaces.insert(face);
          }
          else
          {
            invisibleFaces.insert(face);
          }
        }
      }
      this->uiManager()->activeModelView()->setEntityVisibility(
        visibleFaces, smtk::mesh::MeshSets(), true, setPropertyOp);
      this->uiManager()->activeModelView()->setEntityVisibility(
        invisibleFaces, smtk::mesh::MeshSets(), false, setPropertyOp);
    }
  }
}

void qtPolygonEdgeOperationView::arcOperationDone()
{
  if (!this->Internals->CurrentAtt || !this->Widget || !this->Internals->CurrentOp.lock())
  {
    return;
  }

  this->operationSelected(this->Internals->CurrentOp.lock());
}

void qtPolygonEdgeOperationView::clearSelection()
{
  this->uiManager()->activeModelView()->clearSelection();
}

void qtPolygonEdgeOperationView::operationSelected(const smtk::operation::OperationPtr& op)
{
  if (!this->Internals->CurrentAtt || !this->Widget)
    return;

  // Based on which type of operations, we update UI panel
  if (this->Internals->needArc(op) && this->Internals->ArcManager)
  {
    if (this->Internals->SplitEdgeWidget)
    {
      this->Internals->SplitEdgeWidget->setVisible(false);
    }

    // This handles ui panel update when we need an arc-edit widget
    QWidget* prevUiWidget = this->Internals->ArcManager->getActiveWidget();
    if (op->typeName() == "smtk::session::polygon::CreateEdge")
    {
      this->Internals->ArcManager->create();
    }
    else if (op->typeName() == "smtk::session::polygon::TweakEdge")
    {
      this->Internals->ArcManager->edit();
    }

    QWidget* pWidget = this->Widget;
    QWidget* selUiWidget = this->Internals->ArcManager->getActiveWidget();
    if (!selUiWidget)
    {
      return;
    }
    QString widgetName = selUiWidget->objectName();

    QList<QWidget*> userUiWidgets = pWidget->findChildren<QWidget*>(widgetName);
    bool found = false;
    for (int i = 0; i < userUiWidgets.count(); i++)
    {
      if (!found && userUiWidgets.value(i) == selUiWidget)
      {
        found = true;
      }
      else
      {
        userUiWidgets.value(i)->setVisible(0);
      }
    }
    if (!found)
    {
      selUiWidget->setParent(pWidget);
      this->Internals->EditorLayout->addWidget(selUiWidget);
    }

    // turn off previous active widgets if they are not the active one anymore
    if (prevUiWidget && prevUiWidget != selUiWidget)
    {
      prevUiWidget->setVisible(false);
      prevUiWidget->setEnabled(false);
      prevUiWidget->hide();
    }

    if (!selUiWidget)
    {
      selUiWidget->setVisible(true);
      selUiWidget->setEnabled(true);
      selUiWidget->show();
    }
  }
  else
  {
    // This handles ui panel update when we don't need an arc-edit widget
    // first, hide arc widget related panels if they exist
    if (this->Internals->ArcManager)
    {
      QWidget* prevUiWidget = this->Internals->ArcManager->getActiveWidget();
      // turn off previous active widgets if they are not the active one anymore
      if (prevUiWidget)
      {
        prevUiWidget->setVisible(false);
        prevUiWidget->setEnabled(false);
        prevUiWidget->hide();
      }
    }

    // for edge operations that do not need arc-edit widget
    // * smtk::session::polygon::CreateEdge op : "points" and "vertex ids" // no custom ui needed
    // * smtk::session::polygon::SplitEdge: // this need a special
    if (op->typeName() ==
      "smtk::session::polygon::SplitEdge") // otherwise, only handle smtk::session::polygon::SplitEdge custom ui
    {
      // If this is the same operator the SplitEdgeWidget is actively working with,
      // then nothing to do here
      if (this->Internals->SplitEdgeWidget && this->Internals->SplitEdgeWidget->isActive() &&
        this->Internals->SplitEdgeWidget->edgeOperation() == op)
      {
        return;
      }

      if (!this->Internals->SplitEdgeWidget)
      {
        this->Internals->SplitEdgeWidget = new pqSplitEdgeWidget(this->Widget);
        this->Internals->EditorLayout->addWidget(this->Internals->SplitEdgeWidget);
        QObject::connect(this->Internals->SplitEdgeWidget,
          SIGNAL(operationRequested(const smtk::operation::OperationPtr&)), this,
          SLOT(requestOperation(const smtk::operation::OperationPtr&)));
        QObject::connect(this->Internals->SplitEdgeWidget, SIGNAL(hideAllFaces(bool)), this,
          SLOT(onHideAllFaces(bool)));
      }

      pqRenderView* renView = qobject_cast<pqRenderView*>(pqActiveObjects::instance().activeView());
      this->Internals->SplitEdgeWidget->resetWidget();
      this->Internals->SplitEdgeWidget->setView(renView);
      this->Internals->SplitEdgeWidget->setEdgeOperation(op);
      this->Internals->SplitEdgeWidget->setVisible(true);
    }
  }
}

void qtPolygonEdgeOperationView::requestModelEntityAssociation()
{
  this->updateUI();
}
