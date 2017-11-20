//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include <vtkActor.h>
#include <vtkAlgorithmOutput.h>
#include <vtkCompositeDataDisplayAttributes.h>
#include <vtkCompositeDataIterator.h>
#include <vtkCompositePolyDataMapper2.h>
#include <vtkGlyph3DMapper.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkObjectFactory.h>
#include <vtkRenderer.h>
#include <vtkStreamingDemandDrivenPipeline.h>

#include <vtkPVCacheKeeper.h>
#include <vtkPVRenderView.h>
#include <vtkPVTrivialProducer.h>

#include "smtk/extension/paraview/representation/vtkSMTKModelRepresentation.h"
#include "smtk/extension/paraview/server/vtkSMTKResourceManagerWrapper.h"
#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h"
#include "smtk/resource/Component.h"
#include "smtk/resource/SelectionManager.h"
#include "vtkSMTKModelRepresentation.h"

vtkStandardNewMacro(vtkSMTKModelRepresentation);

vtkSMTKModelRepresentation::vtkSMTKModelRepresentation()
  : EntityMapper(vtkSmartPointer<vtkCompositePolyDataMapper2>::New())
  , SelectedEntityMapper(vtkSmartPointer<vtkCompositePolyDataMapper2>::New())
  , GlyphMapper(vtkSmartPointer<vtkGlyph3DMapper>::New())
  , SelectedGlyphMapper(vtkSmartPointer<vtkGlyph3DMapper>::New())
  , Entities(vtkSmartPointer<vtkActor>::New())
  , SelectedEntities(vtkSmartPointer<vtkActor>::New())
  , GlyphEntities(vtkSmartPointer<vtkActor>::New())
  , SelectedGlyphEntities(vtkSmartPointer<vtkActor>::New())
{
  this->SetupDefaults();
  this->SetNumberOfInputPorts(3);
}

vtkSMTKModelRepresentation::~vtkSMTKModelRepresentation() = default;

void vtkSMTKModelRepresentation::SetupDefaults()
{
  vtkNew<vtkCompositeDataDisplayAttributes> compAtt;
  this->EntityMapper->SetCompositeDataDisplayAttributes(compAtt);

  vtkNew<vtkCompositeDataDisplayAttributes> selCompAtt;
  this->SelectedEntityMapper->SetCompositeDataDisplayAttributes(selCompAtt);

  vtkNew<vtkCompositeDataDisplayAttributes> glyphAtt;
  this->GlyphMapper->SetBlockAttributes(glyphAtt);

  vtkNew<vtkCompositeDataDisplayAttributes> selGlyphAtt;
  this->SelectedGlyphMapper->SetBlockAttributes(selGlyphAtt);

  this->Entities->SetMapper(this->EntityMapper);
  this->SelectedEntities->SetMapper(this->SelectedEntityMapper);
  this->GlyphEntities->SetMapper(this->GlyphMapper);
  this->SelectedGlyphEntities->SetMapper(this->SelectedGlyphMapper);
}

void vtkSMTKModelRepresentation::SetOutputExtent(vtkAlgorithmOutput* output, vtkInformation* inInfo)
{
  if (inInfo->Has(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT()))
  {
    vtkPVTrivialProducer* prod = vtkPVTrivialProducer::SafeDownCast(output->GetProducer());
    if (prod)
    {
      prod->SetWholeExtent(inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT()));
    }
  }
}

bool vtkSMTKModelRepresentation::GetModelBounds()
{
  // Entity tessellation bounds
  double entityBounds[6];
  this->GetBounds(this->GetInternalOutputPort(0)->GetProducer()->GetOutputDataObject(0),
    entityBounds, this->EntityMapper->GetCompositeDataDisplayAttributes());

  // Instance placement bounds
  double* instanceBounds = this->GlyphMapper->GetBounds();

  vtkBoundingBox bbox;
  if (vtkBoundingBox::IsValid(entityBounds))
  {
    bbox.AddPoint(entityBounds[0], entityBounds[2], entityBounds[4]);
    bbox.AddPoint(entityBounds[1], entityBounds[3], entityBounds[5]);
  }

  if (vtkBoundingBox::IsValid(instanceBounds))
  {
    bbox.AddPoint(instanceBounds[0], instanceBounds[2], instanceBounds[4]);
    bbox.AddPoint(instanceBounds[1], instanceBounds[3], instanceBounds[5]);
  }

  if (bbox.IsValid())
  {
    bbox.GetBounds(this->DataBounds);
    return true;
  }

  vtkMath::UninitializeBounds(this->DataBounds);
  return false;
}

int vtkSMTKModelRepresentation::RequestData(
  vtkInformation* request, vtkInformationVector** inVec, vtkInformationVector* outVec)
{
  if (inVec[0]->GetNumberOfInformationObjects() == 1)
  {
    vtkInformation* inInfo = inVec[0]->GetInformationObject(0);
    this->SetOutputExtent(this->GetInternalOutputPort(0), inInfo);

    // Model entities
    this->CacheKeeper->SetInputConnection(this->GetInternalOutputPort(0));

    // Glyph points (2) and prototypes (1)
    this->GlyphMapper->SetInputConnection(this->GetInternalOutputPort(2));
    this->GlyphMapper->SetInputConnection(1, this->GetInternalOutputPort(1));
    this->ConfigureGlyphMapper(this->GlyphMapper.GetPointer());

    this->SelectedGlyphMapper->SetInputConnection(this->GetInternalOutputPort(2));
    this->SelectedGlyphMapper->SetInputConnection(1, this->GetInternalOutputPort(1));
    this->ConfigureGlyphMapper(this->SelectedGlyphMapper.GetPointer());
  }
  this->CacheKeeper->Update();

  this->EntityMapper->Modified();
  this->GlyphMapper->Modified();

  this->GetModelBounds();
  return vtkPVDataRepresentation::RequestData(request, inVec, outVec);
}

int vtkSMTKModelRepresentation::ProcessViewRequest(
  vtkInformationRequestKey* request_type, vtkInformation* inInfo, vtkInformation* outInfo)
{
  if (!vtkPVDataRepresentation::ProcessViewRequest(request_type, inInfo, outInfo))
  {
    // i.e. this->GetVisibility() == false, hence nothing to do.
    return 0;
  }

  if (request_type == vtkPVView::REQUEST_UPDATE())
  {
    // provide the "geometry" to the view so the view can delivery it to the
    // rendering nodes as and when needed.
    // When this process doesn't have any valid input, the cache-keeper is setup
    // to provide a place-holder dataset of the right type. This is essential
    // since the vtkPVRenderView uses the type specified to decide on the
    // delivery mechanism, among other things.
    vtkPVRenderView::SetPiece(inInfo, this, this->CacheKeeper->GetOutputDataObject(0), 0, 0);

    // Since we are rendering polydata, it can be redistributed when ordered
    // compositing is needed. So let the view know that it can feel free to
    // redistribute data as and when needed.
    vtkPVRenderView::MarkAsRedistributable(inInfo, this);

    // Tell the view if this representation needs ordered compositing. We need
    // ordered compositing when rendering translucent geometry. We need to extend
    // this condition to consider translucent LUTs once we start supporting them.
    if (this->Entities->HasTranslucentPolygonalGeometry() ||
      this->GlyphEntities->HasTranslucentPolygonalGeometry())
    {
      outInfo->Set(vtkPVRenderView::NEED_ORDERED_COMPOSITING(), 1);
      // Pass partitioning information to the render view.
      if (this->UseDataPartitions == true)
      {
        vtkPVRenderView::SetOrderedCompositingInformation(inInfo, this->DataBounds);
      }
    }

    // Finally, let the view know about the geometry bounds. The view uses this
    // information for resetting camera and clip planes. Since this
    // representation allows users to transform the geometry, we need to ensure
    // that the bounds we report include the transformation as well.
    vtkNew<vtkMatrix4x4> matrix;
    this->Entities->GetMatrix(matrix.GetPointer());
    vtkPVRenderView::SetGeometryBounds(inInfo, this->DataBounds, matrix.GetPointer());
  }
  else if (request_type == vtkPVView::REQUEST_UPDATE_LOD())
  {
    /// TODO Add LOD Mappers
  }
  else if (request_type == vtkPVView::REQUEST_RENDER())
  {
    // Update model entity attributes
    auto producerPort = vtkPVRenderView::GetPieceProducer(inInfo, this, 0);
    this->EntityMapper->SetInputConnection(0, producerPort);
    auto data = producerPort->GetProducer()->GetOutputDataObject(0);

    if (this->BlockAttributeTime < data->GetMTime() || this->BlockAttrChanged)
    {
      this->UpdateBlockAttributes(this->EntityMapper.GetPointer());
      this->BlockAttributeTime.Modified();
      this->BlockAttrChanged = false;
    }

    // Update selected entities
    /// TODO Selection should only updated after a change
    auto multiBlock = vtkMultiBlockDataSet::SafeDownCast(data);
    if (multiBlock)
    {
      this->SelectedEntityMapper->SetInputConnection(0, producerPort);
      auto attr = this->SelectedEntityMapper->GetCompositeDataDisplayAttributes();
      this->UpdateSelection(multiBlock, attr, this->SelectedEntities);
    }

    // Update glyph attributes
    data = this->GetInternalOutputPort(2)->GetProducer()->GetOutputDataObject(0);
    if (this->InstanceAttributeTime < data->GetMTime() || this->InstanceAttrChanged)
    {
      this->UpdateGlyphBlockAttributes(this->GlyphMapper.GetPointer());
      this->InstanceAttributeTime.Modified();
      this->InstanceAttrChanged = false;
    }

    // Update selected glyphs
    /// TODO Selection should only updated after a change
    multiBlock = vtkMultiBlockDataSet::SafeDownCast(data);
    if (multiBlock)
    {
      auto attr = this->SelectedGlyphMapper->GetBlockAttributes();
      this->UpdateSelection(multiBlock, attr, this->SelectedGlyphEntities);
    }
  }

  return 1;
}

void vtkSMTKModelRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkPVDataRepresentation::PrintSelf(os, indent);
}

bool vtkSMTKModelRepresentation::AddToView(vtkView* view)
{
  vtkPVRenderView* rview = vtkPVRenderView::SafeDownCast(view);
  if (rview)
  {
    rview->GetRenderer()->AddActor(this->Entities);
    rview->GetRenderer()->AddActor(this->GlyphEntities);
    rview->RegisterPropForHardwareSelection(this, this->Entities);
    rview->RegisterPropForHardwareSelection(this, this->GlyphEntities);

    rview->GetRenderer()->AddActor(this->SelectedEntities);
    rview->GetRenderer()->AddActor(this->SelectedGlyphEntities);

    return this->vtkPVDataRepresentation::AddToView(view);
  }
  return false;
}

bool vtkSMTKModelRepresentation::RemoveFromView(vtkView* view)
{
  vtkPVRenderView* rview = vtkPVRenderView::SafeDownCast(view);
  if (rview)
  {
    rview->GetRenderer()->RemoveActor(this->Entities);
    rview->GetRenderer()->RemoveActor(this->GlyphEntities);
    rview->UnRegisterPropForHardwareSelection(this, this->Entities);
    rview->UnRegisterPropForHardwareSelection(this, this->GlyphEntities);

    rview->GetRenderer()->RemoveActor(this->SelectedEntities);
    rview->GetRenderer()->RemoveActor(this->SelectedGlyphEntities);

    return this->vtkPVDataRepresentation::RemoveFromView(view);
  }
  return false;
}

void vtkSMTKModelRepresentation::SetVisibility(bool val)
{
  this->Entities->SetVisibility(val);
  this->GlyphEntities->SetVisibility(val);

  this->SelectedEntities->SetVisibility(val);
  this->SelectedGlyphEntities->SetVisibility(val);

  this->vtkPVDataRepresentation::SetVisibility(val);
}

int vtkSMTKModelRepresentation::FillInputPortInformation(int port, vtkInformation* info)
{
  // Saying INPUT_IS_OPTIONAL() is essential, since representations don't have
  // any inputs on client-side (in client-server, client-render-server mode) and
  // render-server-side (in client-render-server mode).
  if (port == 0)
  {
    // Entity tessellations
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
    info->Append(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkCompositeDataSet");
    return 1;
  }
  if (port == 1)
  {
    // Glyph vertices
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
    info->Append(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkCompositeDataSet");
    return 1;
  }
  else if (port == 2)
  {
    // Glyph sources
    info->Set(vtkAlgorithm::INPUT_IS_REPEATABLE(), 1);
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataObjectTree");
    info->Append(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
    return 1;
  }

  return 0;
}

void vtkSMTKModelRepresentation::ConfigureGlyphMapper(vtkGlyph3DMapper* mapper)
{
  mapper->SetUseSourceTableTree(true);

  mapper->SetSourceIndexArray(VTK_INSTANCE_SOURCE);
  mapper->SetSourceIndexing(true);

  mapper->SetScaleArray(VTK_INSTANCE_SCALE);
  mapper->SetScaling(true);

  mapper->SetOrientationArray(VTK_INSTANCE_ORIENTATION);
  mapper->SetOrientationMode(vtkGlyph3DMapper::ROTATION);

  mapper->SetMaskArray(VTK_INSTANCE_VISIBILITY);
  mapper->SetMasking(true);
}

void vtkSMTKModelRepresentation::SetMapScalars(int val)
{
  if (val < 0 || val > 1)
  {
    vtkWarningMacro(<< "Invalid parameter for vtkSMTKModelRepresentation::SetMapScalars: " << val);
    val = 0;
  }

  int mapToColorMode[] = { VTK_COLOR_MODE_DIRECT_SCALARS, VTK_COLOR_MODE_MAP_SCALARS };
  this->EntityMapper->SetColorMode(mapToColorMode[val]);
  this->GlyphMapper->SetColorMode(mapToColorMode[val]);
}

void vtkSMTKModelRepresentation::UpdateSelection(
  vtkMultiBlockDataSet* data, vtkCompositeDataDisplayAttributes* blockAttr, vtkActor* actor)
{
  auto rm = vtkSMTKResourceManagerWrapper::Instance(); // TODO: Remove the need for this.
  auto sm = rm ? rm->GetSelection() : nullptr;
  if (!sm)
  {
    actor->SetVisibility(0);
    return;
  }

  auto selection = sm->currentSelection();
  if (selection.empty())
  {
    actor->SetVisibility(0);
    return;
  }

  // std::cout << "Updating rep selection from " << sm << ", have " << selection.size() << " entries in map\n";

  int propVis = 0;
  this->ClearSelection(actor->GetMapper());
  for (auto& item : selection)
  {
    auto matchedBlock = this->FindNode(data, item.first->id().toString());
    if (matchedBlock)
    {
      propVis = 1;
      blockAttr->SetBlockVisibility(matchedBlock, true);
      blockAttr->SetBlockColor(matchedBlock, this->SelectionColor);
    }
  }
  actor->SetVisibility(propVis);

  // This is necessary to force an update in the mapper
  actor->GetMapper()->Modified();
}

vtkDataObject* vtkSMTKModelRepresentation::FindNode(
  vtkMultiBlockDataSet* data, const std::string& uuid)
{
  const int numBlocks = data->GetNumberOfBlocks();
  for (int index = 0; index < numBlocks; index++)
  {
    auto currentBlock = data->GetBlock(index);
    if (data->HasMetaData(index))
    {
      auto currentId = data->GetMetaData(index)->Get(vtkModelMultiBlockSource::ENTITYID());
      if (currentId)
      {
        const std::string currentIdStr = currentId;
        if (currentIdStr.compare(uuid) == 0)
        {
          return currentBlock;
        }
      }
    }

    auto childBlock = vtkMultiBlockDataSet::SafeDownCast(currentBlock);
    if (childBlock)
    {
      auto matchedNode = this->FindNode(childBlock, uuid);
      if (matchedNode)
      {
        return matchedNode;
      }
    }
  }

  return nullptr;
}

void vtkSMTKModelRepresentation::ClearSelection(vtkMapper* mapper)
{
  auto clearAttributes = [](vtkCompositeDataDisplayAttributes* attr) {
    attr->RemoveBlockVisibilities();
    attr->RemoveBlockColors();
  };

  auto cpdm = vtkCompositePolyDataMapper2::SafeDownCast(mapper);
  if (cpdm)
  {
    auto blockAttr = cpdm->GetCompositeDataDisplayAttributes();
    clearAttributes(blockAttr);
    auto data = cpdm->GetInputDataObject(0, 0);

    // For vtkCompositePolyDataMapper2, setting the top node as false is enough
    // since the state of the top node will stream down to its nodes.
    blockAttr->SetBlockVisibility(data, false);
    return;
  }

  auto gm = vtkGlyph3DMapper::SafeDownCast(mapper);
  if (gm)
  {
    auto blockAttr = gm->GetBlockAttributes();
    clearAttributes(blockAttr);

    // Glyph3DMapper does not behave as vtkCompositePolyDataMapper2, hence it is
    // necessary to update the block visibility of each node directly.
    auto mbds = vtkMultiBlockDataSet::SafeDownCast(gm->GetInputDataObject(0, 0));
    vtkCompositeDataIterator* iter = mbds->NewIterator();

    iter->GoToFirstItem();
    while (!iter->IsDoneWithTraversal())
    {
      auto dataObj = iter->GetCurrentDataObject();
      blockAttr->SetBlockVisibility(dataObj, false);
      iter->GoToNextItem();
    }
    iter->Delete();
    return;
  }
}

void vtkSMTKModelRepresentation::SetSelectionPointSize(double val)
{
  this->SelectedEntities->GetProperty()->SetPointSize(val);
  this->SelectedGlyphEntities->GetProperty()->SetPointSize(val);
}

void vtkSMTKModelRepresentation::SetPointSize(double val)
{
  this->Entities->GetProperty()->SetPointSize(val);
  this->GlyphEntities->GetProperty()->SetPointSize(val);
}

void vtkSMTKModelRepresentation::UpdateGlyphBlockAttributes(vtkGlyph3DMapper* mapper)
{
  auto instanceData = mapper->GetInputDataObject(0, 0);
  auto blockAttr = mapper->GetBlockAttributes();

  blockAttr->RemoveBlockVisibilities();
  for (auto const& item : this->InstanceVisibilities)
  {
    unsigned int currentIdx = 0;
    auto dob = blockAttr->DataObjectFromIndex(item.first, instanceData, currentIdx);

    if (dob)
    {
      blockAttr->SetBlockVisibility(dob, item.second);
    }
  }

  blockAttr->RemoveBlockColors();
  for (auto const& item : this->InstanceColors)
  {
    unsigned int currentIdx = 0;
    auto dob = blockAttr->DataObjectFromIndex(item.first, instanceData, currentIdx);

    if (dob)
    {
      auto& arr = item.second;
      double color[3] = { arr[0], arr[1], arr[2] };
      blockAttr->SetBlockColor(dob, color);
    }
  }
  // Opacity currently not supported by vtkGlyph3DMapper

  mapper->Modified();
}

void vtkSMTKModelRepresentation::SetInstanceVisibility(unsigned int index, bool visible)
{
  this->InstanceVisibilities[index] = visible;
  this->InstanceAttrChanged = true;
}

bool vtkSMTKModelRepresentation::GetInstanceVisibility(unsigned int index) const
{
  auto it = this->InstanceVisibilities.find(index);
  if (it == this->InstanceVisibilities.cend())
  {
    return true;
  }
  return it->second;
}

void vtkSMTKModelRepresentation::RemoveInstanceVisibility(unsigned int index, bool)
{
  auto it = this->InstanceVisibilities.find(index);
  if (it == this->InstanceVisibilities.cend())
  {
    return;
  }
  this->InstanceVisibilities.erase(it);
  this->InstanceAttrChanged = true;
}

void vtkSMTKModelRepresentation::RemoveInstanceVisibilities()
{
  this->InstanceVisibilities.clear();
  this->InstanceAttrChanged = true;
}

void vtkSMTKModelRepresentation::SetInstanceColor(unsigned int index, double r, double g, double b)
{
  std::array<double, 3> color = { { r, g, b } };
  this->InstanceColors[index] = color;
  this->InstanceAttrChanged = true;
}

void vtkSMTKModelRepresentation::SetInstanceColor(unsigned int index, double* color)
{
  if (color)
  {
    this->SetInstanceColor(index, color[0], color[1], color[2]);
  }
}

double* vtkSMTKModelRepresentation::GetInstanceColor(unsigned int index)
{
  auto it = this->InstanceColors.find(index);
  if (it == this->InstanceColors.cend())
  {
    return nullptr;
  }
  return it->second.data();
}

void vtkSMTKModelRepresentation::RemoveInstanceColor(unsigned int index)
{
  auto it = this->InstanceColors.find(index);
  if (it == this->InstanceColors.cend())
  {
    return;
  }
  this->InstanceColors.erase(it);
  this->InstanceAttrChanged = true;
}

void vtkSMTKModelRepresentation::RemoveInstanceColors()
{
  this->InstanceColors.clear();
  this->InstanceAttrChanged = true;
}
