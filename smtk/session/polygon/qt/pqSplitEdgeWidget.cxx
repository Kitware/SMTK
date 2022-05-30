//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/session/polygon/qt/pqSplitEdgeWidget.h"

#include "pqApplicationCore.h"
#include "pqDataRepresentation.h"
#include "pqObjectBuilder.h"
#include "pqOutputPort.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "pqRenderViewSelectionReaction.h"

#include "vtkClientServerStream.h"
#include "vtkDoubleArray.h"
#include "vtkMemberFunctionCommand.h"
#include "vtkNew.h"
#include "vtkPVRenderView.h"
#include "vtkPVSelectionInformation.h"
#include "vtkPolygonArcInfo.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkSMInteractiveSelectionPipeline.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyManager.h"
#include "vtkSMProxyProperty.h"
#include "vtkSMRenderViewProxy.h"
#include "vtkSMRepresentationProxy.h"
#include "vtkSMSession.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMVectorProperty.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkSmartPointer.h"
#include "vtkUnsignedIntArray.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/model/Edge.h"
#include "smtk/model/Resource.h"
#include "smtk/operation/Operation.h"

#include <QToolButton>
#include <QVBoxLayout>
#include <QtDebug>

namespace pqSplitEdgeWidgetInternals
{
EdgePointPicker::EdgePointPicker(QObject* p)
  : QAction(p)
{
  this->setCheckable(true);
}

EdgePointPicker::~EdgePointPicker()
{
  if (this->Selecter)
  {
    this->Selecter->disconnect();
    delete this->Selecter;
  }
}

void EdgePointPicker::doPick(pqRenderView* view)
{
  if (this->Selecter)
  {
    this->Selecter->disconnect();
    delete this->Selecter;
  }
  this->Selecter = new pqRenderViewSelectionReaction(
    this, view, pqRenderViewSelectionReaction::SELECT_SURFACE_POINTS_INTERACTIVELY);

  // we only want selection on one representation.
  view->setUseMultipleRepresentationSelection(false);

  m_isActive = true;
  Q_EMIT triggered(true);
}

void EdgePointPicker::donePicking(pqRenderView* view)
{
  //resets the widget to what it would be like if it was just created
  this->InteractiveSelectButton->blockSignals(true);
  this->InteractiveSelectButton->setChecked(false);
  this->InteractiveSelectButton->blockSignals(false);
  m_isActive = false;
  Q_EMIT triggered(false);
  //we want the connection to stop so remove the connection
  if (this->Selecter)
  {
    vtkSMInteractiveSelectionPipeline::GetInstance()->Hide(
      vtkSMRenderViewProxy::SafeDownCast(view->getViewProxy()));
    this->Selecter->disconnect();
    delete this->Selecter;
    this->Selecter = nullptr;
  }
  if (view)
  {
    // reset multiple selection to true
    view->setUseMultipleRepresentationSelection(true);
  }
}
} // namespace pqSplitEdgeWidgetInternals

class pqSplitEdgeWidget::pqInternals
{
public:
  vtkSmartPointer<vtkCommand> InteractionModePropertyObserver;
};

pqSplitEdgeWidget::pqSplitEdgeWidget(QWidget* prent)
  : QWidget(prent)
  , Internals(new pqSplitEdgeWidget::pqInternals)
  , m_edgePointPicker(new pqSplitEdgeWidgetInternals::EdgePointPicker(this))
{
  this->setObjectName("pqSplitEdgeWidget");
  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  QToolButton* splitButton = new QToolButton(this);
  splitButton->setText("Split Edge");
  splitButton->setToolTip("Split an edge by clicking on an edge point");
  splitButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  QString iconName(":/icons/attribute/edgesplit.png");
  splitButton->setIcon(QIcon(iconName));
  splitButton->setIconSize(QSize(32, 32));
  splitButton->setSizePolicy(sizeFixedPolicy);
  splitButton->setCheckable(true);

  m_edgePointPicker->InteractiveSelectButton = splitButton;
  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setMargin(0);
  layout->addWidget(splitButton);

  this->Internals->InteractionModePropertyObserver.TakeReference(
    vtkMakeMemberFunctionCommand(*this, &pqSplitEdgeWidget::onSelectionModeChanged));

  //connect up the split buttons
  QObject::connect(splitButton, SIGNAL(toggled(bool)), this, SLOT(splitEdgeOperation(bool)));
}

pqSplitEdgeWidget::~pqSplitEdgeWidget()
{
  this->setView(nullptr);
  delete this->Internals;
  delete m_edgePointPicker;
}

void pqSplitEdgeWidget::setView(pqRenderView* view)
{
  if (this->View != view)
  {
    vtkSMProperty* controlledProperty;
    vtkSMRenderViewProxy* rmp;
    if (this->View)
    {
      rmp = this->View->getRenderViewProxy();
      controlledProperty = rmp->GetProperty("InteractionMode");
      controlledProperty->RemoveObserver(this->Internals->InteractionModePropertyObserver);
    }
    this->View = view;
    if (this->View)
    {
      rmp = this->View->getRenderViewProxy();
      controlledProperty = rmp->GetProperty("InteractionMode");
      controlledProperty->AddObserver(
        vtkCommand::ModifiedEvent, this->Internals->InteractionModePropertyObserver);
    }
  }
}

void pqSplitEdgeWidget::setEdgeOperation(smtk::operation::OperationPtr edgeOp)
{
  if (edgeOp && edgeOp->typeName() == "smtk::session::polygon::operators::SplitEdge")
    m_edgeOp = edgeOp;
  else
    m_edgeOp = smtk::operation::Operation::Ptr();
}
smtk::shared_ptr<smtk::operation::Operation> pqSplitEdgeWidget::edgeOperation()
{
  return m_edgeOp.lock();
}

void pqSplitEdgeWidget::splitEdgeOperation(bool start)
{
  if (this->View && m_edgeOp.lock() && start)
  {
    // Clear the selection first. This used qtActiveObjects in the past.

    int curSelMode = 0;
    vtkSMPropertyHelper(this->View->getRenderViewProxy(), "InteractionMode").Get(&curSelMode);
    if (curSelMode == vtkPVRenderView::INTERACTION_MODE_SELECTION)
    {
      qCritical()
        << "The render view is in use with another selection. Stop that selection first.\n"
        << "You can do a rubber band selection in the render window to reset selection mode";
      m_edgePointPicker->InteractiveSelectButton->blockSignals(true);
      m_edgePointPicker->InteractiveSelectButton->setChecked(false);
      m_edgePointPicker->InteractiveSelectButton->blockSignals(false);
      return;
    }

    // things are selected
    QObject::connect(
      this->View,
      SIGNAL(selected(pqOutputPort*)),
      this,
      SLOT(arcPointPicked(pqOutputPort*)),
      Qt::UniqueConnection);
    // selection is done
    QObject::connect(
      this->View,
      SIGNAL(selectionModeChanged(bool)),
      this,
      SLOT(resetWidget()),
      Qt::UniqueConnection);

    Q_EMIT hideAllFaces(true); // hide faces before selection
    m_edgePointPicker->doPick(this->View);
  }
  else
  {
    Q_EMIT hideAllFaces(false); // restore faces visibility
    this->resetWidget();
  }
}

void pqSplitEdgeWidget::arcPointPicked(pqOutputPort* port)
{
  if (!m_edgePointPicker->m_isActive)
    return;

  if (port && this->View && m_edgeOp.lock())
  {
    // get the selected point id
    // This "IDs" only have three components [composite_index, processId, Index]
    // because the arc source is a block in a Composite Dataset
    vtkSMSourceProxy* selSource = port->getSelectionInput();
    // [composite_index, process_id, index]
    vtkSMPropertyHelper selIDs(selSource, "IDs");
    std::vector<vtkIdType> ids = selIDs.GetArray<vtkIdType>();
    bool readytoOp = false;
    vtkIdType flatIdx;
    vtkIdType ptid;
    vtkNew<vtkPolygonArcInfo> arcInfo;

    //collect the information from the server model source
    vtkSMProxy* proxy = port->getSource()->getProxy();
    smtk::attribute::AttributePtr opSpec = m_edgeOp.lock()->parameters();

    // find the first proper point to start spliting
    if (ids.size() > 2 && (ids.size() % 3 == 0)) // A valid selection
    {
      for (size_t index = 0; index < ids.size() / 3; index++)
      {
        flatIdx = ids[3 * index];
        ptid = ids[3 * index + 2];
        arcInfo->SetBlockIndex(flatIdx - 1);
        arcInfo->SetSelectedPointId(ptid);
        proxy->GatherInformation(arcInfo.GetPointer());
        if (arcInfo->GetModelEntityID())
        {
          smtk::common::UUID edgeId(arcInfo->GetModelEntityID());
          // TODO: manager cannot be accessed through the operator directly
          // smtk::model::Edge edge(m_edgeOp.lock()->manager(), edgeId);
          // if (edge.isValid())
          // {
          //   opSpec->removeAllAssociations();
          //   opSpec->associateEntity(edge);
          //   double ptcoords[3];
          //   arcInfo->GetSelectedPointCoordinates(ptcoords);
          //   opSpec->findDouble("point")->setValue(0, ptcoords[0]);
          //   opSpec->findDouble("point")->setValue(1, ptcoords[1]);
          //   opSpec->findInt("point id")->setValue(ptid);
          //   // now request the operation
          //   readytoOp = true;
          //   break;
          // }
        }
      }
    }

    // once find the edge needed, turn off the selection for model source
    port->setSelectionInput(nullptr, 0);
    this->View->render();
    if (readytoOp)
    {
      Q_EMIT this->operationRequested(m_edgeOp.lock());
      // keep picking mode. We need this to refresh the Selecter with new model geometry
      vtkPVRenderView* rv =
        vtkPVRenderView::SafeDownCast(this->View->getRenderViewProxy()->GetClientSideObject());
      if (rv != nullptr)
      {
        rv->InvalidateCachedSelection();
        rv->GetRenderWindow()->Render();
        rv->GetInteractor()->Render();
      }
    }
  }
}

void pqSplitEdgeWidget::onSelectionModeChanged()
{
  if (!this->View || !this->isActive())
  {
    return;
  }
  // check if the InteractionMode is changed, maybe from other selection reaction
  if (this->View)
  {
    vtkSMRenderViewProxy* rmp = this->View->getRenderViewProxy();
    int curSelMode = 0;
    vtkSMPropertyHelper(rmp, "InteractionMode").Get(&curSelMode);
    if (
      curSelMode != vtkPVRenderView::INTERACTION_MODE_SELECTION &&
      curSelMode != vtkPVRenderView::INTERACTION_MODE_2D &&
      curSelMode != vtkPVRenderView::INTERACTION_MODE_3D)
    {
      this->resetWidget();
    }
  }
}

bool pqSplitEdgeWidget::isActive()
{
  return m_edgePointPicker->m_isActive;
}

void pqSplitEdgeWidget::resetWidget()
{
  m_edgePointPicker->donePicking(this->View);
}
