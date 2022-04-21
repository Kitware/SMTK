//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/session/polygon/qt/pqArcWidgetPanel.h"
#include "ui_qtArcWidgetPanel.h"

#include "pqApplicationCore.h"
#include "pqDataRepresentation.h"
#include "pqObjectBuilder.h"
#include "pqOutputPort.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "pqRenderViewSelectionReaction.h"

#include "vtkClientServerStream.h"
#include "vtkDoubleArray.h"
#include "vtkNew.h"
#include "vtkPVSelectionInformation.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyManager.h"
#include "vtkSMProxyProperty.h"
#include "vtkSMRepresentationProxy.h"
#include "vtkSMSession.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMVectorProperty.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkUnsignedIntArray.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/extension/vtk/widgets/vtkSMTKArcRepresentation.h"
#include "smtk/model/Edge.h"
#include "smtk/model/Resource.h"
#include "smtk/operation/Operation.h"
#include "smtk/session/polygon/qt/pqArcWidgetManager.h"
#include "smtk/session/polygon/qt/pqPolygonArc.h"
#include "smtk/session/polygon/qt/qtArcWidget.h"
#include "smtk/session/polygon/qt/vtkPolygonArcInfo.h"
#include <QtDebug>

namespace Ui
{
ArcPicker::ArcPicker(QObject* parent)
  : QAction(parent)
  , Info(nullptr)
  , Arc(nullptr)
  , View(nullptr)
  , Selecter(nullptr)
  , m_isActive(false)
{
  this->setCheckable(true);
}

ArcPicker::~ArcPicker()
{
  if (this->Selecter)
  {
    this->Selecter->disconnect();
    delete this->Selecter;
  }
}

void ArcPicker::doPick(pqRenderView* view, pqPolygonArc* arc, PickInfo& arcinfo)
{
  delete this->Selecter;
  m_isActive = false;
  this->View = nullptr; //clear the view each time
  if (view)
  {
    this->Selecter = new pqRenderViewSelectionReaction(
      this, view, pqRenderViewSelectionReaction::SELECT_SURFACE_CELLS);

    // we only want selection on one representation.
    view->setUseMultipleRepresentationSelection(false);
    // things are selected
    QObject::connect(
      view,
      SIGNAL(selected(pqOutputPort*)),
      this,
      SLOT(selectedInfo(pqOutputPort*)),
      Qt::UniqueConnection);
    // selection is done
    QObject::connect(
      view,
      SIGNAL(selectionModeChanged(bool)),
      this,
      SLOT(onPickingFinished()),
      Qt::UniqueConnection);

    this->Arc = arc;
    this->View = view;
    this->Info = &arcinfo;
    m_isActive = true;
    // make sure we are using the correct source for picking
    if (arc)
      arc->resetOperationSource();

    Q_EMIT triggered(true);
  }
}

vtkSelectionNode* ArcPicker::gatherSelectionNode(
  pqPipelineSource* source,
  vtkPVSelectionInformation* selInfo)
{
  vtkSMSourceProxy* smSource = vtkSMSourceProxy::SafeDownCast(source->getProxy());
  vtkSMSourceProxy* selSource = smSource->GetSelectionInput(0);
  selSource->UpdatePipeline();

  selSource->GatherInformation(selInfo);
  if (selInfo->GetSelection() && selInfo->GetSelection()->GetNumberOfNodes())
  {
    return selInfo->GetSelection()->GetNode(0);
  }
  return nullptr;
}

void ArcPicker::selectedInfo(pqOutputPort* port)
{
  // ignore selections triggered from elsewhere
  if (!m_isActive)
    return;

  //always update the port
  this->Info->EdgeId = smtk::common::UUID::null();
  this->Info->port = nullptr;

  if (port && this->Arc->edgeOperation())
  {
    // This "IDs" only have three components [composite_index, processId, Index]
    // where composit_index is blockId and index is cellId
    // Arc source is a block in a Composite Dataset
    vtkSMSourceProxy* selSource = port->getSelectionInput();
    vtkSMPropertyHelper selIDs(selSource, "IDs");
    std::vector<vtkIdType> ids = selIDs.GetArray<vtkIdType>();

    if (ids.size() > 2 && (ids.size() % 3 == 0)) // a valid selection
    {
      // get first selected edge
      vtkNew<vtkPolygonArcInfo> arcInfo;
      //collect the information from the server model source
      vtkSMProxy* proxy = port->getSource()->getProxy();
      smtk::model::Edge edge;
      vtkIdType flatIdx(-1);
      smtk::common::UUID edgeId;

      // get the first selected edge's blockId
      for (size_t index = 0; index <= ids.size() / 3; index++)
      {
        // we only care about blockId
        flatIdx = ids[index * 3];
        //TODO: why minus 1 here?
        arcInfo->SetBlockIndex(flatIdx - 1);
        proxy->GatherInformation(arcInfo.GetPointer());
        if (arcInfo->GetModelEntityID())
        {
          edgeId = smtk::common::UUID(arcInfo->GetModelEntityID());
          // TODO: cannot access manager through operator directly
          // edge = smtk::model::Edge(this->Arc->edgeOperation()->manager(), edgeId);
          // if (edge.isValid())
          // {
          //   break;
          // }
        }
      }

      if (flatIdx != -1)
      {
        this->Info->EdgeId = edgeId;
        this->Info->BlockIndex = flatIdx - 1;
        this->Arc->setSource(port->getSource());
        // once find the edge needed, turn off the selection for model source
        port->setSelectionInput(nullptr, 0);
        this->View->render();
      }
    }
  }
}

void ArcPicker::onPickingFinished()
{
  //we want the connection to happen once the view goes away so
  //remove the connection
  if (this->Selecter)
  {
    this->Selecter->disconnect();
    delete this->Selecter;
    this->Selecter = nullptr;
  }
  if (this->View)
  {
    this->View->forceRender();
    // reset multiple selection to true
    this->View->setUseMultipleRepresentationSelection(false);
  }
  m_isActive = false;
  Q_EMIT this->pickFinished();
}
} // namespace Ui

class pqArcWidgetPanel::pqInternals : public Ui::qtArcWidgetPanel
{
public:
  // QPointer<pqPipelineSource> ArcSource;
};

pqArcWidgetPanel::pqArcWidgetPanel(QWidget* parent)
  : QWidget(parent)
  , Internals(new pqArcWidgetPanel::pqInternals)
  , Picker(parent)
  , ArcWidget(nullptr)
{
  Internals->setupUi(this);
  this->setObjectName("pqArcWidgetPanel");

  //connect up the pick buttons
  QObject::connect(
    this->Internals->SelectArcButton, SIGNAL(toggled(bool)), this, SLOT(pickWholeArc()));

  //connect up the edit buttons
  QObject::connect(this->Internals->cancelButton, SIGNAL(clicked()), this, SLOT(cancelEdit()));

  //connect the picker up so we know when it is done.
  QObject::connect(
    &this->Picker, SIGNAL(pickFinished()), this, SLOT(arcPicked()), Qt::QueuedConnection);
}

pqArcWidgetPanel::~pqArcWidgetPanel()
{
  delete this->ArcWidget;
  delete Internals;
}

void pqArcWidgetPanel::setArc(pqPolygonArc* arc)
{
  if (this->Arc != arc)
  {
    this->Arc = arc;
    if (this->Arc)
    {
      this->resetWidget();
    }
  }
}

void pqArcWidgetPanel::showEditWidget()
{
  //hide the pointWidget from the layout to reclaim the space
  this->Internals->ArcEditWidgtet->show();
  this->Internals->SelectArcButton->hide();

  //update the layout
  this->updateGeometry();

  this->modifyArc();

  // clear selection on the selected port when contourWidget is shown
  if (this->ArcInfo.port)
  {
    this->ArcInfo.port->setSelectionInput(nullptr, 0);
  }

  Q_EMIT this->startArcEditing();
}

void pqArcWidgetPanel::showPickWidget()
{
  //shows the pick widget and hides the edit widget
  this->Internals->SelectArcButton->show();
  this->Internals->ArcEditWidgtet->hide();

  //update the layout
  this->updateGeometry();
}

void pqArcWidgetPanel::pickWholeArc()
{
  if (this->Internals->SelectArcButton->isChecked())
  {
    // The edge operator will tell which edge to use for editing
    if (!this->Arc)
    {
      return;
    }
    this->Arc->resetOperationSource();

    this->Picker.doPick(this->View, this->Arc, this->ArcInfo);
    // clear all selections
    Q_EMIT this->startArcPicking();
  }
  else
  {
    // update the UI for whole arc
    this->showPickWidget();
  }
}

void pqArcWidgetPanel::arcPicked()
{
  if (this->Arc && this->Arc->edgeOperation())
  {
    // TODO: cannot access the manager directly through the operator
    // smtk::model::Edge edge(this->Arc->edgeOperation()->manager(), this->ArcInfo.EdgeId);
    // if (edge.isValid())
    // {
    //   smtk::attribute::AttributePtr opSpec = this->Arc->edgeOperation()->specification();
    //   edge.setIntegerProperty("block_index", this->ArcInfo.BlockIndex);
    //   if (!opSpec->isEntityAssociated(edge))
    //   {
    //     opSpec->removeAllAssociations();
    //     opSpec->associateEntity(edge);
    //   }
    //   this->showEditWidget();
    //   return;
    // }
  }

  this->resetWidget();
}

void pqArcWidgetPanel::hideArcWidget()
{
  if (this->ArcWidget)
  {
    this->ArcWidget->setEnableInteractivity(false);
    this->ArcWidget->setVisible(false);
    this->ArcWidget->setView(nullptr);
    this->ArcWidget->hide();
  }
}

void pqArcWidgetPanel::modifyArc()
{
  if (!this->ArcManager)
  {
    qCritical() << "There was not valid sub-arc to modify yet.\n";
    return;
  }
  pqPolygonArc* arcObj = this->ArcManager->activeArc();
  if (!arcObj)
  {
    return;
  }
  bool created = false;
  int normal;
  double planePos;
  if (!this->ArcWidget)
  {
    this->ArcWidget = this->ArcManager->createDefaultContourWidget(normal, planePos);
    this->ArcWidget->setParent(this->Internals->ArcEditWidgtet);
    this->Internals->ContourLayout->addWidget(this->ArcWidget);

    QObject::connect(this->ArcWidget, SIGNAL(contourDone()), this, SLOT(arcEditingFinished()));
    created = true;
  }
  if (!created)
  {
    this->ArcWidget->setView(this->View);
    if (arcObj)
    {
      if (vtkSMProxy* pointplacer = this->ArcWidget->pointPlacer())
      {
        vtkSMPropertyHelper(pointplacer, "ProjectionNormal")
          .Set(arcObj->getPlaneProjectionNormal());
        vtkSMPropertyHelper(pointplacer, "ProjectionPosition")
          .Set(arcObj->getPlaneProjectionPosition());
        // pointplacer->MarkModified(pointplacer);
        pointplacer->UpdateVTKObjects();
      }
    }
  }

  this->ArcWidget->useArcEditingUI(true);
  this->ArcWidget->setEnableInteractivity(true);
  this->ArcWidget->show();
  if (arcObj)
  {
    if (!created)
    {
      this->ArcWidget->reset(); // FIXME:why even bother reusing? Simply create
                                // a new widget everytime.
    }

    //pass the info from the arc into the widget proxy
    this->updateWidgetRepresentation();

    vtkSMPropertyHelper(this->ArcWidget->widgetProxy(), "CanEdit").Set(0);
    this->ArcWidget->widgetProxy()->UpdateVTKObjects();

    this->ArcWidget->checkContourLoopClosed();
    this->ArcWidget->ModifyMode();
    this->ArcWidget->checkCanBeEdited();
  }

  this->View->forceRender();
}

void pqArcWidgetPanel::resetWidget()
{
  //resets the widget to what it would be like if it was just created
  this->Internals->SelectArcButton->blockSignals(true);
  this->Internals->SelectArcButton->setChecked(false);
  this->Internals->SelectArcButton->blockSignals(false);

  this->showPickWidget();
}

void pqArcWidgetPanel::updateWidgetRepresentation()
{
  if (!this->Arc || !this->Arc->getSource())
  {
    return;
  }

  //create an arc provider for this arc
  vtkSMProxy* smArcSource =
    vtkSMProxyManager::GetProxyManager()->NewProxy("polygon_operators", "PolygonArcProvider");
  vtkSMPropertyHelper(smArcSource, "Input").Set(this->Arc->getSource()->getProxy(), 0);

  //tell the provider the arc id it needs to be connected too
  vtkSMSourceProxy* sourceProxy = vtkSMSourceProxy::SafeDownCast(smArcSource);
  vtkSMPropertyHelper(sourceProxy, "BlockIndex").Set(this->Arc->getAssignedEdgeBlock());
  sourceProxy->UpdateVTKObjects();
  sourceProxy->UpdatePipeline();

  vtkSMNewWidgetRepresentationProxy* widgetProxy = this->ArcWidget->widgetProxy();
  vtkSMProxy* repProxy = widgetProxy->GetRepresentationProxy();
  vtkClientServerStream stream;
  stream << vtkClientServerStream::Invoke << VTKOBJECT(sourceProxy) << "GetOutput"
         << vtkClientServerStream::End << vtkClientServerStream::Invoke << VTKOBJECT(repProxy)
         << "InitializeContour" << vtkClientServerStream::LastResult << 0
         << vtkClientServerStream::End;
  repProxy->GetSession()->ExecuteStream(repProxy->GetLocation(), stream);

  smArcSource->FastDelete();
}

void pqArcWidgetPanel::arcEditingFinished()
{
  this->saveEdit();
  //marks that we are finished editing this arc
  //update the server and close the widget
  this->resetWidget();
  Q_EMIT this->arcModificationfinished();
}

void pqArcWidgetPanel::cancelEdit()
{
  //marks that that we don't want to save the modifications
  //to the arc
  this->hideArcWidget();

  //now show the pick widget
  this->resetWidget();
  Q_EMIT this->arcModificationCacelled();
}

void pqArcWidgetPanel::saveEdit()
{
  // modify/replace the edge with the arc from the arc widget
  Q_EMIT this->arcModified(this->ArcWidget, this->ArcInfo.EdgeId);

  //hide the arc widget
  this->hideArcWidget();

  //now show the pick widget
  this->resetWidget();
}
