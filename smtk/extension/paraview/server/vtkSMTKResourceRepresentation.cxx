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
#include <vtkFieldData.h>
#include <vtkGlyph3DMapper.h>
#include <vtkImageData.h>
#include <vtkImageSliceRepresentation.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkLookupTable.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkStringArray.h>
#include <vtkTransform.h>
#include <vtksys/SystemTools.hxx>

#include <vtkPVRenderView.h>
#include <vtkPVTrivialProducer.h>

#include "vtkDataObjectTreeIterator.h"

#include "smtk/extension/paraview/server/vtkSMTKRepresentationStyleGenerator.h"
#include "smtk/extension/paraview/server/vtkSMTKResourceReader.h"
#include "smtk/extension/paraview/server/vtkSMTKResourceRepresentation.h"
#include "smtk/extension/paraview/server/vtkSMTKSettings.h"
#include "smtk/extension/paraview/server/vtkSMTKWrapper.h"
#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h"
#include "smtk/extension/vtk/source/vtkResourceMultiBlockSource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ReferenceItem.h"

#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/Entity.h"
#include "smtk/model/Group.h"
#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/Manager.h"

#include "smtk/view/Selection.h"

vtkCxxSetObjectMacro(vtkSMTKResourceRepresentation, SliceXY, vtkImageSliceRepresentation);
vtkCxxSetObjectMacro(vtkSMTKResourceRepresentation, SliceYZ, vtkImageSliceRepresentation);
vtkCxxSetObjectMacro(vtkSMTKResourceRepresentation, SliceXZ, vtkImageSliceRepresentation);

namespace
{

void SetAttributeBlockColorToEntity(vtkCompositeDataDisplayAttributes* atts, vtkDataObject* block,
  const smtk::common::UUID& uuid, const smtk::resource::ResourcePtr& res)
{
  std::vector<double> color = { { 1., 1., 1., 1. } };

  auto component = res->find(uuid);
  if (component != nullptr && component->properties().contains<std::vector<double> >("color"))
  {
    color = component->properties().at<std::vector<double> >("color");
  }

  atts->SetBlockColor(block, color.data());
  if (color[3] < 1.0)
  {
    atts->SetBlockOpacity(block, color[3]);
  }
  else if (atts->HasBlockVisibility(block))
  {
    atts->RemoveBlockOpacity(block);
  }
}

void ColorBlockAsEntity(vtkGlyph3DMapper* mapper, vtkDataObject* block,
  const smtk::common::UUID& uuid, const smtk::resource::ResourcePtr& res)
{
  SetAttributeBlockColorToEntity(mapper->GetBlockAttributes(), block, uuid, res);
}

void ColorBlockAsEntity(vtkCompositePolyDataMapper2* mapper, vtkDataObject* block,
  const smtk::common::UUID& uuid, const smtk::resource::ResourcePtr& res)
{
  SetAttributeBlockColorToEntity(mapper->GetCompositeDataDisplayAttributes(), block, uuid, res);
}

void AddRenderables(
  vtkMultiBlockDataSet* data, vtkSMTKResourceRepresentation::RenderableDataMap& renderables)
{
  if (!data)
  {
    return;
  }
  auto mbit = data->NewTreeIterator();
  // Some entity might uses composite data sets.
  mbit->VisitOnlyLeavesOff();
  for (mbit->GoToFirstItem(); !mbit->IsDoneWithTraversal(); mbit->GoToNextItem())
  {
    auto obj = mbit->GetCurrentDataObject();
    auto uid = vtkResourceMultiBlockSource::GetDataObjectUUID(mbit->GetCurrentMetaData());
    if (!obj || !uid)
    {
      continue;
    }
    renderables[uid] = obj;
  }
  mbit->Delete();
}

} // anonymous namespace

///////////////////////////////////////////////////////////////////////////////
vtkStandardNewMacro(vtkSMTKResourceRepresentation);

vtkSMTKResourceRepresentation::vtkSMTKResourceRepresentation()
  : Wrapper(nullptr)
  , EntityMapper(vtkSmartPointer<vtkCompositePolyDataMapper2>::New())
  , SelectedEntityMapper(vtkSmartPointer<vtkCompositePolyDataMapper2>::New())
  , GlyphMapper(vtkSmartPointer<vtkGlyph3DMapper>::New())
  , SelectedGlyphMapper(vtkSmartPointer<vtkGlyph3DMapper>::New())
  , Entities(vtkSmartPointer<vtkActor>::New())
  , SelectedEntities(vtkSmartPointer<vtkActor>::New())
  , GlyphEntities(vtkSmartPointer<vtkActor>::New())
  , SelectedGlyphEntities(vtkSmartPointer<vtkActor>::New())
  , SliceXY(vtkImageSliceRepresentation::New())
  , SliceYZ(vtkImageSliceRepresentation::New())
  , SliceXZ(vtkImageSliceRepresentation::New())
  , EntitiesActorPickId(-1)
  , SelectedEntitiesActorPickId(-1)
  , GlyphEntitiesActorPickId(-1)
  , SelectedGlyphEntitiesActorPickId(-1)
{
  this->SetupDefaults();
  this->SetNumberOfInputPorts(3);
}

vtkSMTKResourceRepresentation::~vtkSMTKResourceRepresentation()
{
  this->SetWrapper(nullptr);
  this->SetSliceXY(nullptr);
  this->SetSliceYZ(nullptr);
  this->SetSliceXZ(nullptr);
}

void vtkSMTKResourceRepresentation::SetupDefaults()
{
  vtkNew<vtkCompositeDataDisplayAttributes> compAtt;
  this->EntityMapper->SetCompositeDataDisplayAttributes(compAtt);

  vtkNew<vtkCompositeDataDisplayAttributes> selCompAtt;
  this->SelectedEntityMapper->SetCompositeDataDisplayAttributes(selCompAtt);

  vtkNew<vtkCompositeDataDisplayAttributes> glyphAtt;
  this->GlyphMapper->SetBlockAttributes(glyphAtt);
  this->GlyphMapper->SetScaleModeToNoDataScaling(); // We use a per point scale array

  vtkNew<vtkCompositeDataDisplayAttributes> selGlyphAtt;
  this->SelectedGlyphMapper->SetBlockAttributes(selGlyphAtt);
  this->SelectedGlyphMapper->SetScaleModeToNoDataScaling(); // We use a per point scale array

  this->Entities->SetMapper(this->EntityMapper);
  this->SelectedEntities->SetMapper(this->SelectedEntityMapper);
  this->GlyphEntities->SetMapper(this->GlyphMapper);
  this->SelectedGlyphEntities->SetMapper(this->SelectedGlyphMapper);

  // Share vtkProperty between model mappers
  this->Property = this->Entities->GetProperty();
  this->GlyphEntities->SetProperty(this->Property);
}

void vtkSMTKResourceRepresentation::SetOutputExtent(
  vtkAlgorithmOutput* output, vtkInformation* inInfo)
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

bool vtkSMTKResourceRepresentation::GetModelBounds()
{
  // Entity tessellation bounds
  double entityBounds[6];
  this->GetEntityBounds(
    this->CurrentData, entityBounds, this->EntityMapper->GetCompositeDataDisplayAttributes());

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
  // must be uninitialized, or bounds will be treated as zero.
  vtkMath::UninitializeBounds(this->DataBounds);
  return false;
}

bool vtkSMTKResourceRepresentation::GetEntityBounds(
  vtkDataObject* dataObject, double bounds[6], vtkCompositeDataDisplayAttributes* cdAttributes)
{
  vtkMath::UninitializeBounds(bounds);
  if (vtkCompositeDataSet* cd = vtkCompositeDataSet::SafeDownCast(dataObject))
  {
    // computing bounds with only visible blocks
    vtkCompositeDataDisplayAttributes::ComputeVisibleBounds(cdAttributes, cd, bounds);
    if (vtkBoundingBox::IsValid(bounds))
    {
      return true;
    }
  }
  return false;
}

int vtkSMTKResourceRepresentation::RequestData(
  vtkInformation* request, vtkInformationVector** inVec, vtkInformationVector* outVec)
{
  this->CurrentData->Initialize();
  if (inVec[0]->GetNumberOfInformationObjects() == 1)
  {
    vtkInformation* inInfo = inVec[0]->GetInformationObject(0);
    this->SetOutputExtent(this->GetInternalOutputPort(), inInfo);

    // Get each block from the top level multi block
    auto port = this->GetInternalOutputPort();
    auto mbds = vtkMultiBlockDataSet::SafeDownCast(port->GetProducer()->GetOutputDataObject(0));
    this->CurrentData->ShallowCopy(mbds);

    vtkSmartPointer<vtkMultiBlockDataSet> componentMultiBlock = vtkMultiBlockDataSet::SafeDownCast(
      mbds->GetBlock(vtkResourceMultiBlockSource::BlockId::Components));
    vtkSmartPointer<vtkMultiBlockDataSet> protoTypeMultiBlock = vtkMultiBlockDataSet::SafeDownCast(
      mbds->GetBlock(vtkResourceMultiBlockSource::BlockId::Prototypes));
    vtkSmartPointer<vtkMultiBlockDataSet> instanceMultiBlock = vtkMultiBlockDataSet::SafeDownCast(
      mbds->GetBlock(vtkResourceMultiBlockSource::BlockId::Instances));
    vtkSmartPointer<vtkMultiBlockDataSet> imageMultiBlock = vtkMultiBlockDataSet::SafeDownCast(
      mbds->GetBlock(vtkResourceMultiBlockSource::BlockId::Images));

    if (imageMultiBlock && imageMultiBlock->GetNumberOfBlocks() > 0)
    {
      vtkSmartPointer<vtkImageData> sliceVolume =
        vtkImageData::SafeDownCast(imageMultiBlock->GetBlock(0));
      if (sliceVolume)
      {
        std::string scalarName = "scalars";
        auto scalars = sliceVolume->GetPointData()->GetScalars();
        if (scalars)
        {
          scalarName = scalars->GetName();
          auto lkup = sliceVolume->GetPointData()->GetScalars()->GetLookupTable();
          this->SliceXY->SetLookupTable(lkup);
          this->SliceYZ->SetLookupTable(lkup);
          this->SliceXZ->SetLookupTable(lkup);
        }
        this->SliceXY->SetInputDataObject(0, sliceVolume);
        this->SliceYZ->SetInputDataObject(0, sliceVolume);
        this->SliceXZ->SetInputDataObject(0, sliceVolume);

        this->SliceXY->SetSliceMode(vtkImageSliceRepresentation::XY_PLANE);
        this->SliceYZ->SetSliceMode(vtkImageSliceRepresentation::YZ_PLANE);
        this->SliceXZ->SetSliceMode(vtkImageSliceRepresentation::XZ_PLANE);
        this->SliceXY->SetInputArrayToProcess(
          0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, scalarName.c_str());
        this->SliceYZ->SetInputArrayToProcess(
          0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, scalarName.c_str());
        this->SliceXZ->SetInputArrayToProcess(
          0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, scalarName.c_str());
      }
    }
    else
    {
      this->SliceXY->SetVisibility(false);
      this->SliceYZ->SetVisibility(false);
      this->SliceXZ->SetVisibility(false);
    }

    // Glyph points (2) and prototypes (1)
    this->GlyphMapper->SetInputData(instanceMultiBlock);
    this->GlyphMapper->SetSourceTableTree(protoTypeMultiBlock);
    this->ConfigureGlyphMapper(this->GlyphMapper.GetPointer());
    // Use per point color to render the normal glyphs
    this->GlyphMapper->SetScalarModeToUsePointFieldData();
    this->GlyphMapper->SelectColorArray(VTK_INSTANCE_COLOR);

    this->SelectedGlyphMapper->SetInputData(instanceMultiBlock);
    this->SelectedGlyphMapper->SetSourceTableTree(protoTypeMultiBlock);
    this->ConfigureGlyphMapper(this->SelectedGlyphMapper.GetPointer());
    // Use per block color to render the selected/hovered glyphs
    this->SelectedGlyphMapper->SetScalarModeToUseCellData();
  }

  this->EntityMapper->Modified();
  this->GlyphMapper->Modified();

  this->GetModelBounds();

  // New input data requires updated block colors:
  this->UpdateColorBy = true;
  return Superclass::RequestData(request, inVec, outVec);
}

unsigned int vtkSMTKResourceRepresentation::Initialize(unsigned int minId, unsigned int maxId)
{
  unsigned int result = this->Superclass::Initialize(minId, maxId);
  result = this->SliceXY->Initialize(result, maxId);
  result = this->SliceYZ->Initialize(result, maxId);
  result = this->SliceXZ->Initialize(result, maxId);
  return result;
}

int vtkSMTKResourceRepresentation::ProcessViewRequest(
  vtkInformationRequestKey* request_type, vtkInformation* inInfo, vtkInformation* outInfo)
{
  if (!Superclass::ProcessViewRequest(request_type, inInfo, outInfo))
  {
    // i.e. this->GetVisibility() == false, hence nothing to do.
    return 0;
  }

  if (request_type == vtkPVView::REQUEST_UPDATE())
  {
    // provide the "geometry" to the view so the view can deliver it to the
    // rendering nodes as and when needed.
    // When this process doesn't have any valid input, the cache-keeper is setup
    // to provide a place-holder dataset of the right type. This is essential
    // since the vtkPVRenderView uses the type specified to decide on the
    // delivery mechanism, among other things.
    vtkPVRenderView::SetPiece(inInfo, this, this->CurrentData, 0, 0);

    // Since we are rendering polydata, it can be redistributed when ordered
    // compositing is needed. So let the view know that it can feel free to
    // redistribute data as and when needed.
    vtkPVRenderView::MarkAsRedistributable(inInfo, this);

    // Tell the view if this representation needs ordered compositing. We need
    // ordered compositing when rendering translucent geometry. We need to extend
    // this condition to consider translucent LUTs once we start supporting them.
    if (this->Entities->HasTranslucentPolygonalGeometry() ||
      this->GlyphEntities->HasTranslucentPolygonalGeometry() ||
      this->SelectedEntities->HasTranslucentPolygonalGeometry() ||
      this->SelectedGlyphEntities->HasTranslucentPolygonalGeometry())
    {
      outInfo->Set(vtkPVRenderView::NEED_ORDERED_COMPOSITING(), 1);
    }

    // Finally, let the view know about the geometry bounds. The view uses this
    // information for resetting camera and clip planes. Since this
    // representation allows users to transform the geometry, we need to ensure
    // that the bounds we report include the transformation as well.
    vtkNew<vtkMatrix4x4> matrix;
    this->Entities->GetMatrix(matrix.GetPointer());
    vtkPVRenderView::SetGeometryBounds(inInfo, this, this->DataBounds, matrix.GetPointer());
  }
  else if (request_type == vtkPVView::REQUEST_UPDATE_LOD())
  {
    /// TODO Add LOD Mappers
  }
  else if (request_type == vtkPVView::REQUEST_RENDER())
  {
    auto port = this->GetInternalOutputPort();
    vtkSmartPointer<vtkMultiBlockDataSet> mbds =
      vtkMultiBlockDataSet::SafeDownCast(port->GetProducer()->GetOutputDataObject(0));

    vtkSmartPointer<vtkMultiBlockDataSet> componentMultiBlock = vtkMultiBlockDataSet::SafeDownCast(
      mbds->GetBlock(vtkResourceMultiBlockSource::BlockId::Components));
    vtkSmartPointer<vtkMultiBlockDataSet> instanceMultiBlock = vtkMultiBlockDataSet::SafeDownCast(
      mbds->GetBlock(vtkResourceMultiBlockSource::BlockId::Instances));
    vtkSmartPointer<vtkMultiBlockDataSet> imageMultiBlock = vtkMultiBlockDataSet::SafeDownCast(
      mbds->GetBlock(vtkResourceMultiBlockSource::BlockId::Images));

    {
      // In order to get consistent ordering for cell selection (and, therefore,
      // point picking), we must construct a multiblock dataset that has the
      // same hierarchical structure as the input data set. We don't want to
      // have component or instance rendering from our entity mapper, though, so
      // we simply leave those blocks out of our new dataset.
      vtkNew<vtkMultiBlockDataSet> mbds2;
      mbds2->SetBlock(vtkResourceMultiBlockSource::BlockId::Components, componentMultiBlock);
      this->EntityMapper->SetInputDataObject(mbds2);
      this->SelectedEntityMapper->SetInputDataObject(mbds2);
    }

    // Set up an internal slice representation for each image.
    if (imageMultiBlock && imageMultiBlock->GetNumberOfBlocks() > 0)
    {
      this->SliceXY->SetVisibility(true);
      this->SliceYZ->SetVisibility(true);
      this->SliceXZ->SetVisibility(true);
    }
    else
    {
      this->SliceXY->SetVisibility(false);
      this->SliceYZ->SetVisibility(false);
      this->SliceXZ->SetVisibility(false);
    }

    this->UpdateColoringParameters(componentMultiBlock);
    this->UpdateRepresentationSubtype();

    // If the input has changed, update the map of polydata pointers in RenderableData:
    this->UpdateRenderableData(componentMultiBlock, instanceMultiBlock);
    // If the selection has changed, update the visual properties of blocks:
    this->UpdateDisplayAttributesFromSelection(componentMultiBlock, instanceMultiBlock);
  }

  return 1;
}

void vtkSMTKResourceRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}

bool vtkSMTKResourceRepresentation::AddToView(vtkView* view)
{
  vtkPVRenderView* rview = vtkPVRenderView::SafeDownCast(view);
  if (rview)
  {
    rview->AddRepresentation(this->SliceXY);
    rview->AddRepresentation(this->SliceYZ);
    rview->AddRepresentation(this->SliceXZ);

    rview->GetRenderer()->AddActor(this->Entities);
    rview->GetRenderer()->AddActor(this->GlyphEntities);
    rview->GetRenderer()->AddActor(this->SelectedEntities);
    rview->GetRenderer()->AddActor(this->SelectedGlyphEntities);

    this->EntitiesActorPickId = rview->RegisterPropForHardwareSelection(this, this->Entities);
    this->GlyphEntitiesActorPickId =
      rview->RegisterPropForHardwareSelection(this, this->GlyphEntities);
    this->SelectedEntitiesActorPickId =
      rview->RegisterPropForHardwareSelection(this, this->SelectedEntities);
    this->SelectedGlyphEntitiesActorPickId =
      rview->RegisterPropForHardwareSelection(this, this->SelectedGlyphEntities);

    return Superclass::AddToView(view);
  }
  else
  {
    this->EntitiesActorPickId = -1;
    this->GlyphEntitiesActorPickId = -1;
    this->SelectedEntitiesActorPickId = -1;
    this->SelectedGlyphEntitiesActorPickId = -1;
  }
  return false;
}

bool vtkSMTKResourceRepresentation::RemoveFromView(vtkView* view)
{
  vtkPVRenderView* rview = vtkPVRenderView::SafeDownCast(view);
  if (rview)
  {
    rview->RemoveRepresentation(this->SliceXY);
    rview->RemoveRepresentation(this->SliceYZ);
    rview->RemoveRepresentation(this->SliceXZ);

    rview->GetRenderer()->RemoveActor(this->Entities);
    rview->GetRenderer()->RemoveActor(this->GlyphEntities);
    rview->GetRenderer()->RemoveActor(this->SelectedEntities);
    rview->GetRenderer()->RemoveActor(this->SelectedGlyphEntities);

    rview->UnRegisterPropForHardwareSelection(this, this->Entities);
    rview->UnRegisterPropForHardwareSelection(this, this->GlyphEntities);
    rview->UnRegisterPropForHardwareSelection(this, this->SelectedEntities);
    rview->UnRegisterPropForHardwareSelection(this, this->SelectedGlyphEntities);

    return Superclass::RemoveFromView(view);
  }
  return false;
}

void vtkSMTKResourceRepresentation::SetVisibility(bool val)
{
  this->SliceXY->SetVisibility(val);
  this->SliceYZ->SetVisibility(val);
  this->SliceXZ->SetVisibility(val);

  this->Entities->SetVisibility(val);
  this->GlyphEntities->SetVisibility(val);

  this->SelectedEntities->SetVisibility(val);
  this->SelectedGlyphEntities->SetVisibility(val);

  Superclass::SetVisibility(val);
}

void vtkSMTKResourceRepresentation::GetEntityVisibilities(
  std::map<smtk::common::UUID, int>& visdata)
{
  visdata.clear();
  for (const auto& entry : this->ComponentState)
  {
    visdata[entry.first] = entry.second.m_visibility;
  }
}

bool vtkSMTKResourceRepresentation::SetEntityVisibility(
  smtk::resource::PersistentObjectPtr ent, bool visible)
{
  bool didChange = false;
  if (!ent || !ent->id())
  {
    return didChange;
  }

  auto csit = this->ComponentState.find(ent->id());
  if (csit == this->ComponentState.end() && visible)
  { // No change from the presumed default.
    return didChange;
  }
  else if (csit == this->ComponentState.end())
  { // No previous visibility entry. Add one and update.
    csit = this->ComponentState.insert(std::make_pair(ent->id(), State())).first;
    didChange = true;
  }
  if (!!csit->second.m_visibility != visible || didChange)
  {
    csit->second.m_visibility = (visible ? 1 : 0);
    didChange = true;
    auto dataIt = this->RenderableData.find(csit->first);
    if (dataIt != this->RenderableData.end())
    {
      // Tell both the entity and glyph mappers that, should they encounter the data object,
      // use the provided visibility.
      this->EntityMapper->GetCompositeDataDisplayAttributes()->SetBlockVisibility(
        dataIt->second, !!csit->second.m_visibility);
      this->SelectedEntityMapper->GetCompositeDataDisplayAttributes()->SetBlockVisibility(
        dataIt->second, !csit->second.m_visibility);
      this->GlyphMapper->GetBlockAttributes()->SetBlockVisibility(
        dataIt->second, !!csit->second.m_visibility);
      this->SelectedGlyphMapper->GetBlockAttributes()->SetBlockVisibility(
        dataIt->second, !csit->second.m_visibility);
      // Mark the mappers as modified or the new visibility info will not be updated:
      this->EntityMapper->Modified();
      this->GlyphMapper->Modified();
      // mark the selection modified, so UpdateDisplayAttributesFromSelection will fix
      // the selection visibility
      this->SelectionModified();
    }
  }
  return didChange;
}

bool vtkSMTKResourceRepresentation::ApplyStyle(smtk::view::SelectionPtr seln,
  RenderableDataMap& renderables, vtkSMTKResourceRepresentation* self)
{
  // See if any plugins have registered a generator, and use that.
  auto resource = this->GetResource();
  if (resource)
  {
    vtkSMTKRepresentationStyleGenerator generator;
    StyleFromSelectionFunction doApplyStyle = generator(resource);
    if (doApplyStyle)
    {
      doApplyStyle(seln, renderables, self);
    }
    else
    {
      // otherwise use the default.
      vtkSMTKResourceRepresentation::ApplyDefaultStyle(seln, renderables, self);
    }
    return true;
  }
  return false;
}

bool vtkSMTKResourceRepresentation::ApplyDefaultStyle(smtk::view::SelectionPtr seln,
  RenderableDataMap& renderables, vtkSMTKResourceRepresentation* self)
{
  bool atLeastOneSelected = false;
  smtk::attribute::Attribute::Ptr attr;
  for (auto& item : seln->currentSelection())
  {
    if (item.second <= 0)
    {
      // Should never happen.
      continue;
    }

    // If the item is an attribute, it will not be directly renderable,
    // so preview its associated entities.
    attr = std::dynamic_pointer_cast<smtk::attribute::Attribute>(item.first);
    if (attr)
    {
      auto assoc = attr->associations();
      if (assoc)
      {
        assoc->visit([&](const smtk::resource::PersistentObjectPtr& obj) {
          atLeastOneSelected |=
            self->SelectComponentFootprint(obj, /*selnBit TODO*/ 1, renderables);
          return true;
        });
      }
    }
    else
    {
      atLeastOneSelected |= self->SelectComponentFootprint(item.first, item.second, renderables);
    }
  }

  return atLeastOneSelected;
}

bool vtkSMTKResourceRepresentation::SelectComponentFootprint(
  smtk::resource::PersistentObjectPtr item, int selnBits, RenderableDataMap& renderables)
{
  bool atLeastOneSelected = false;
  if (!item)
  {
    return atLeastOneSelected;
  }

  // Determine the actor-pair we are dealing with (normal or instanced):
  auto entity = item->as<smtk::model::Entity>();
  bool isGlyphed = entity && entity->isInstance();

  // Determine if the user has hidden the entity
  auto& smap = this->GetComponentState();
  auto cstate = smap.find(item->id());
  bool hidden = (cstate != smap.end() && !cstate->second.m_visibility);

  // TODO: A single persistent object may have a "footprint"
  // (i.e., a set<vtkDataObject*>) that represents it.
  // In this case, we should set the selected state for all
  // of the footprint. However, this is complicated by the fact
  // that some of the footprint entries might correspond to
  // children (boundary) objects that might be controlled
  // independently.
  //
  // For now, assume each persistent object has at most 1
  // vtkDataObject associated with it.
  auto dataIt = renderables.find(item->id());
  if (dataIt != renderables.end())
  {
    this->SetSelectedState(dataIt->second, hidden ? -1 : selnBits, isGlyphed);
    atLeastOneSelected |= !hidden;
  }
  else
  {
    // The component does not have any geometry of its own... but perhaps
    // we can render its children highlighted instead.
    auto ent = item->as<smtk::model::Entity>();
    if (ent)
    {
      if (ent->isGroup())
      {
        auto members = smtk::model::Group(ent).members<smtk::model::EntityRefs>();
        atLeastOneSelected |= this->SelectComponentFootprint(members, selnBits, renderables);
      }
      else if (ent->isModel())
      {
        auto model = smtk::model::Model(ent);
        auto cells = model.cellsAs<smtk::model::EntityRefs>();
        for (const auto& cell : cells)
        {
          // If the cell has no geometry, then add its boundary cells.
          if (renderables.find(cell.entity()) == renderables.end())
          {
            auto bdys = smtk::model::CellEntity(cell).boundingCellsAs<smtk::model::EntityRefs>();
            cells.insert(bdys.begin(), bdys.end());
          }
        }
        atLeastOneSelected |= this->SelectComponentFootprint(cells, selnBits, renderables);

        auto groups = model.groups();
        for (const auto& group : groups)
        {
          auto members = group.members<smtk::model::EntityRefs>();
          atLeastOneSelected |= this->SelectComponentFootprint(members, selnBits, renderables);
        }

        // TODO: Auxiliary geometry may also be handled by a separate representation.
        //       Need to ensure that representation also renders selection properly.
        auto auxGeoms = model.auxiliaryGeometry();
        // Convert auxGeoms to EntityRefs to match SelectComponentFootprint() API:
        smtk::model::EntityRefs auxEnts;
        for (const auto& auxGeom : auxGeoms)
        {
          auxEnts.insert(auxGeom);
        }
        atLeastOneSelected |= this->SelectComponentFootprint(auxEnts, selnBits, renderables);
      }
      else if (ent->isCellEntity())
      {
        auto bdys = smtk::model::CellEntity(ent).boundingCellsAs<smtk::model::EntityRefs>();
        atLeastOneSelected |= this->SelectComponentFootprint(bdys, selnBits, renderables);
      }
    }
  }
  return atLeastOneSelected;
}

bool vtkSMTKResourceRepresentation::SelectComponentFootprint(
  const smtk::model::EntityRefs& items, int selnBits, RenderableDataMap& renderables)
{
  bool atLeastOneSelected = false;
  auto& smap = this->GetComponentState();
  for (const auto& item : items)
  {
    auto dataIt = renderables.find(item.entity());
    auto cstate = smap.find(item.entity());
    bool hidden = (cstate != smap.end() && !cstate->second.m_visibility);
    if (dataIt != renderables.end())
    {
      this->SetSelectedState(dataIt->second, hidden ? -1 : selnBits, item.isInstance());
      atLeastOneSelected |= !hidden;
    }
  }
  return atLeastOneSelected;
}

int vtkSMTKResourceRepresentation::FillInputPortInformation(int port, vtkInformation* info)
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

void vtkSMTKResourceRepresentation::ConfigureGlyphMapper(vtkGlyph3DMapper* mapper)
{
  mapper->SetUseSourceTableTree(true);

  mapper->SetSourceIndexArray(VTK_INSTANCE_SOURCE);
  mapper->SetSourceIndexing(true);

  mapper->SetScaleArray(VTK_INSTANCE_SCALE);
  mapper->SetScaling(true);
  mapper->SetScaleModeToScaleByVectorComponents();

  mapper->SetOrientationArray(VTK_INSTANCE_ORIENTATION);
  mapper->SetOrientationMode(vtkGlyph3DMapper::ROTATION);

  mapper->SetMaskArray(VTK_INSTANCE_VISIBILITY);
  mapper->SetMasking(true);
}

void vtkSMTKResourceRepresentation::SetMapScalars(int val)
{
  if (val < 0 || val > 1)
  {
    vtkWarningMacro(<< "Invalid parameter for vtkSMTKResourceRepresentation::SetMapScalars: "
                    << val);
    val = 0;
  }

  int mapToColorMode[] = { VTK_COLOR_MODE_DIRECT_SCALARS, VTK_COLOR_MODE_MAP_SCALARS };
  this->EntityMapper->SetColorMode(mapToColorMode[val]);
  this->GlyphMapper->SetColorMode(mapToColorMode[val]);
}

void vtkSMTKResourceRepresentation::SetInterpolateScalarsBeforeMapping(int val)
{
  this->EntityMapper->SetInterpolateScalarsBeforeMapping(val);
  this->GlyphMapper->SetInterpolateScalarsBeforeMapping(val);
}

void vtkSMTKResourceRepresentation::UpdateRenderableData(
  vtkMultiBlockDataSet* resourceData, vtkMultiBlockDataSet* instanceData)
{
  if ((resourceData && resourceData->GetMTime() > this->RenderableTime) ||
    (instanceData && instanceData->GetMTime() > this->RenderableTime) ||
    (this->SelectionTime > this->RenderableTime))
  {
    this->RenderableData.clear();
    AddRenderables(instanceData, this->RenderableData);
    AddRenderables(resourceData, this->RenderableData);
    this->RenderableTime.Modified();
  }
}

void vtkSMTKResourceRepresentation::UpdateDisplayAttributesFromSelection(
  vtkMultiBlockDataSet* resourceData, vtkMultiBlockDataSet* instanceData)
{
  auto rm = this->GetWrapper();
  auto sm = rm ? rm->GetSelection() : nullptr;
  if (!sm)
  {
    this->Entities->SetVisibility(1);
    this->GlyphEntities->SetVisibility(1);
    this->SelectedEntities->SetVisibility(0);
    this->SelectedGlyphEntities->SetVisibility(0);
    return;
  }
  else
  {
    this->SelectedEntities->SetVisibility(1);
    this->SelectedGlyphEntities->SetVisibility(1);
  }

  if (!resourceData)
  {
    return;
  }

  if (resourceData->GetMTime() < this->ApplyStyleTime &&
    (!instanceData || instanceData->GetMTime() < this->ApplyStyleTime) &&
    this->RenderableTime < this->ApplyStyleTime && this->SelectionTime < this->ApplyStyleTime)
  {
    return;
  }

  // We are about to manually set block visibilities for the selection,
  // so reset what's there now to reflect nothing being selected (i.e.,
  // only blocks hidden by user should have visibility entries and those
  // should be false).
  auto nrme = this->EntityMapper->GetCompositeDataDisplayAttributes();
  auto nrmg = this->GlyphMapper->GetBlockAttributes();
  nrme->RemoveBlockVisibilities();
  nrmg->RemoveBlockVisibilities();
  // Similarly, the selected-entity and selected-glyph block visibilities
  // should *all* be present but set to false.
  auto seda = this->SelectedEntityMapper->GetCompositeDataDisplayAttributes();
  auto sgda = this->SelectedGlyphMapper->GetBlockAttributes();
  for (const auto& entry : this->RenderableData)
  {
    seda->SetBlockVisibility(entry.second, false);
    sgda->SetBlockVisibility(entry.second, false);
  }

  // Add user-specified visibility
  for (const auto& entry : this->ComponentState)
  {
    auto rit = this->RenderableData.find(entry.first);
    if (rit == this->RenderableData.end())
    {
      continue;
    }
    nrme->SetBlockVisibility(rit->second, !!entry.second.m_visibility);
    nrmg->SetBlockVisibility(rit->second, !!entry.second.m_visibility);
    // We don't need to set visibility on seda/sgda here since
    // it will always be false (no selection being processed yet).
  }

  // Finally, ask our "style" functor to update selection visibility/color info
  // on the mappers by calling SetSelectedState() on entries in this->RenderableData.
  bool atLeastOneSelected = this->ApplyStyle(sm, this->RenderableData, this);
  (void)atLeastOneSelected;

  // This is necessary to force an update in the mapper
  this->Entities->GetMapper()->Modified();
  this->GlyphEntities->GetMapper()->Modified();
  this->SelectedEntities->GetMapper()->Modified();
  this->SelectedGlyphEntities->GetMapper()->Modified();

  this->ApplyStyleTime.Modified();
}

void vtkSMTKResourceRepresentation::UpdateSelection(
  vtkMultiBlockDataSet* data, vtkCompositeDataDisplayAttributes* blockAttr, vtkActor* actor)
{
  auto rm = this->GetWrapper(); // vtkSMTKWrapper::Instance(); // TODO: Remove the need for this.
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

  int propVis = 0;
  this->ClearSelection(actor->GetMapper()); // FIXME: ClearSelection does stupid things.
  // FIXME: This is the wrong thing to loop over -- since we don't have a map
  //        from component (or UUID) to block ID, the call to FindNode is slow.
  //        If we loop over blocks instead, we can search the selection map quickly!
  for (auto& item : selection)
  {
    if (item.second <= 0)
    {
      continue;
    }
    auto matchedBlock = this->FindNode(data, item.first->id().toString());
    if (matchedBlock)
    {
      propVis = 1;
      blockAttr->SetBlockVisibility(matchedBlock, true);
      blockAttr->SetBlockColor(
        matchedBlock, item.second > 1 ? this->HoverColor : this->SelectionColor);
    }
  }
  actor->SetVisibility(propVis);

  // This is necessary to force an update in the mapper
  actor->GetMapper()->Modified();
}

vtkDataObject* vtkSMTKResourceRepresentation::FindNode(
  vtkMultiBlockDataSet* data, const smtk::common::UUID& uuid)
{
  const int numBlocks = data->GetNumberOfBlocks();
  for (int index = 0; index < numBlocks; index++)
  {
    auto currentBlock = data->GetBlock(index);
    auto currentId = vtkResourceMultiBlockSource::GetDataObjectUUID(data->GetMetaData(index));
    if (currentId == uuid)
    {
      return currentBlock;
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

void vtkSMTKResourceRepresentation::ClearSelection(vtkMapper* mapper)
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
    vtkDataObjectTreeIterator* iter = mbds->NewTreeIterator();
    iter->VisitOnlyLeavesOff();

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

void vtkSMTKResourceRepresentation::SetResource(const smtk::resource::ResourcePtr& res)
{
  this->Resource = res;
}

smtk::resource::ResourcePtr vtkSMTKResourceRepresentation::GetResource() const
{
  return this->Resource.lock();
}

void vtkSMTKResourceRepresentation::SetColorBy(const char* type)
{
  if (vtksys::SystemTools::Strucmp(type, "Entity") == 0)
  {
    this->SetColorBy(ENTITY);
  }
  else if (vtksys::SystemTools::Strucmp(type, "Volume") == 0)
  {
    this->SetColorBy(VOLUME);
  }
  else if (vtksys::SystemTools::Strucmp(type, "Field") == 0)
  {
    this->SetColorBy(FIELD);
  }
  else
  {
    vtkErrorMacro("Invalid type: " << type);
  }

  // Foce update of ColorBy
  this->UpdateColorBy = true;

  // Force update of internal attributes
  this->BlockAttrChanged = true;
  this->InstanceAttrChanged = true;
}

void vtkSMTKResourceRepresentation::SelectionModified()
{
  this->SelectionTime.Modified();
}

void vtkSMTKResourceRepresentation::SetWrapper(vtkSMTKWrapper* wrapper)
{
  if (wrapper == this->Wrapper)
  {
    return;
  }
  if (this->Wrapper)
  {
    auto oldSeln = this->Wrapper->GetSelection();
    if (oldSeln)
    {
      oldSeln->observers().erase(this->SelectionObserver);
    }
    this->SelectionObserver = smtk::view::SelectionObservers::Key();
    this->Wrapper->UnRegister(this);
  }
  this->Wrapper = wrapper;
  if (this->Wrapper)
  {
    this->Wrapper->Register(this);
    // Observe the Wrapper's selection and mark when we need
    // to rebuild visual properties (due to selection changes).
    auto newSeln = this->Wrapper->GetSelection();
    this->SelectionObserver = newSeln
      ? newSeln->observers().insert(
          [this](const std::string& /*unused*/, smtk::view::Selection::Ptr /*unused*/) {
            this->SelectionModified();
          },
          "vtkSMTKResourceRepresentation: Update visual properties to reflect selection change.")
      : smtk::view::SelectionObservers::Key();
  }
  this->Modified();
}

void vtkSMTKResourceRepresentation::SetRepresentation(const char* type)
{
  if (vtksys::SystemTools::Strucmp(type, "Points") == 0)
  {
    this->SetRepresentation(POINTS);
  }
  else if (vtksys::SystemTools::Strucmp(type, "Wireframe") == 0)
  {
    this->SetRepresentation(WIREFRAME);
  }
  else if (vtksys::SystemTools::Strucmp(type, "Surface") == 0)
  {
    this->SetRepresentation(SURFACE);
  }
  else if (vtksys::SystemTools::Strucmp(type, "Surface With Edges") == 0)
  {
    this->SetRepresentation(SURFACE_WITH_EDGES);
  }
  else
  {
    vtkErrorMacro("Invalid type: " << type);
  }
}

void vtkSMTKResourceRepresentation::SetSelectionPointSize(double val)
{
  this->SelectedEntities->GetProperty()->SetPointSize(val);
  this->SelectedGlyphEntities->GetProperty()->SetPointSize(val);
}

void vtkSMTKResourceRepresentation::SetLookupTable(vtkScalarsToColors* val)
{
  this->EntityMapper->SetLookupTable(val);
  this->GlyphMapper->SetLookupTable(val);
}

void vtkSMTKResourceRepresentation::SetSelectionLineWidth(double val)
{
  this->SelectedEntities->GetProperty()->SetLineWidth(val);
  this->SelectedGlyphEntities->GetProperty()->SetLineWidth(val);
}

void vtkSMTKResourceRepresentation::SetSelectionRenderStyle(int style)
{
  switch (style)
  {
    case vtkSMTKSettings::SolidSelectionStyle:
      this->SelectedEntities->GetProperty()->SetRepresentation(VTK_SURFACE);
      break;
    case vtkSMTKSettings::WireframeSelectionStyle:
      this->SelectedEntities->GetProperty()->SetRepresentation(VTK_WIREFRAME);
      break;
    default:
      vtkWarningMacro("Unknown selection render style \"" << style << "\" provided. Ignoring.");
      break;
  }
}

void vtkSMTKResourceRepresentation::SetPointSize(double val)
{
  this->Property->SetPointSize(val);
}

void vtkSMTKResourceRepresentation::SetLineWidth(double val)
{
  this->Property->SetLineWidth(val);
}

void vtkSMTKResourceRepresentation::SetLineColor(double r, double g, double b)
{
  this->Property->SetEdgeColor(r, g, b);
}

void vtkSMTKResourceRepresentation::SetOpacity(double val)
{
  this->Property->SetOpacity(val);
}

void vtkSMTKResourceRepresentation::SetPosition(double x, double y, double z)
{
  this->Entities->SetPosition(x, y, z);
  this->SelectedEntities->SetPosition(x, y, z);
  this->GlyphEntities->SetPosition(x, y, z);
  this->SelectedGlyphEntities->SetPosition(x, y, z);
}

void vtkSMTKResourceRepresentation::SetScale(double x, double y, double z)
{
  this->Entities->SetScale(x, y, z);
  this->SelectedEntities->SetScale(x, y, z);
  this->GlyphEntities->SetScale(x, y, z);
  this->SelectedGlyphEntities->SetScale(x, y, z);
}

void vtkSMTKResourceRepresentation::SetOrientation(double x, double y, double z)
{
  this->Entities->SetOrientation(x, y, z);
  this->SelectedEntities->SetOrientation(x, y, z);
  this->GlyphEntities->SetOrientation(x, y, z);
  this->SelectedGlyphEntities->SetOrientation(x, y, z);
}

void vtkSMTKResourceRepresentation::SetOrigin(double x, double y, double z)
{
  this->Entities->SetOrigin(x, y, z);
  this->SelectedEntities->SetOrigin(x, y, z);
  this->GlyphEntities->SetOrigin(x, y, z);
  this->SelectedGlyphEntities->SetOrigin(x, y, z);
}

void vtkSMTKResourceRepresentation::SetUserTransform(const double matrix[16])
{
  vtkNew<vtkTransform> transform;
  transform->SetMatrix(matrix);
  this->Entities->SetUserTransform(transform.GetPointer());
  this->SelectedEntities->SetUserTransform(transform.GetPointer());
  this->GlyphEntities->SetUserTransform(transform.GetPointer());
  this->SelectedGlyphEntities->SetUserTransform(transform.GetPointer());
}

void vtkSMTKResourceRepresentation::SetPickable(int val)
{
  this->Entities->SetPickable(val);
  this->GlyphEntities->SetPickable(val);
}

void vtkSMTKResourceRepresentation::SetTexture(vtkTexture* val)
{
  this->Entities->SetTexture(val);
  this->GlyphEntities->SetTexture(val);
}

void vtkSMTKResourceRepresentation::SetSpecularPower(double val)
{
  this->Property->SetSpecularPower(val);
}

void vtkSMTKResourceRepresentation::SetSpecular(double val)
{
  this->Property->SetSpecular(val);
}

void vtkSMTKResourceRepresentation::SetAmbient(double val)
{
  this->Property->SetAmbient(val);
}

void vtkSMTKResourceRepresentation::SetDiffuse(double val)
{
  this->Property->SetDiffuse(val);
}

void vtkSMTKResourceRepresentation::UpdateRepresentationSubtype()
{
  // Adjust material properties.
  double diffuse = this->Diffuse;
  double specular = this->Specular;
  double ambient = this->Ambient;

  if (this->Representation != SURFACE && this->Representation != SURFACE_WITH_EDGES)
  {
    diffuse = 0.0;
    ambient = 1.0;
  }

  this->Property->SetAmbient(ambient);
  this->Property->SetSpecular(specular);
  this->Property->SetDiffuse(diffuse);

  switch (this->Representation)
  {
    case SURFACE_WITH_EDGES:
      this->Property->SetEdgeVisibility(1);
      this->Property->SetRepresentation(VTK_SURFACE);
      break;

    default:
      this->Property->SetEdgeVisibility(0);
      this->Property->SetRepresentation(this->Representation);
  }
}

void vtkSMTKResourceRepresentation::UpdateColoringParameters(vtkDataObject* data)
{
  auto multiBlock = vtkMultiBlockDataSet::SafeDownCast(data);
  if (!multiBlock)
  {
    return;
  }
  switch (this->ColorBy)
  {
    case VOLUME:
      this->ColorByVolume(multiBlock);
      break;
    case ENTITY:
      this->ColorByEntity(multiBlock);
      break;
    case FIELD:
      this->ColorByField();
      break;
    default:
      this->ColorByEntity(multiBlock);
      break;
  }

  if (this->UseInternalAttributes)
  {
    this->ApplyInternalBlockAttributes();
  }
}

void vtkSMTKResourceRepresentation::ColorByField()
{
  this->EntityMapper->GetCompositeDataDisplayAttributes()->RemoveBlockColors();

  bool using_scalar_coloring = false;
  vtkInformation* info = this->GetInputArrayInformation(0);
  if (info && info->Has(vtkDataObject::FIELD_ASSOCIATION()) &&
    info->Has(vtkDataObject::FIELD_NAME()))
  {
    const char* colorArrayName = info->Get(vtkDataObject::FIELD_NAME());
    int fieldAssociation = info->Get(vtkDataObject::FIELD_ASSOCIATION());
    if (colorArrayName && colorArrayName[0])
    {
      this->EntityMapper->SetScalarVisibility(1);
      this->EntityMapper->SelectColorArray(colorArrayName);
      this->EntityMapper->SetUseLookupTableScalarRange(1);
      this->GlyphMapper->SetScalarVisibility(1);
      this->GlyphMapper->SelectColorArray(colorArrayName);
      this->GlyphMapper->SetUseLookupTableScalarRange(1);
      switch (fieldAssociation)
      {
        case vtkDataObject::FIELD_ASSOCIATION_CELLS:
          this->EntityMapper->SetScalarMode(VTK_SCALAR_MODE_USE_CELL_FIELD_DATA);
          this->GlyphMapper->SetScalarMode(VTK_SCALAR_MODE_USE_CELL_FIELD_DATA);
          break;

        case vtkDataObject::FIELD_ASSOCIATION_NONE:
          this->EntityMapper->SetScalarMode(VTK_SCALAR_MODE_USE_FIELD_DATA);
          this->GlyphMapper->SetScalarMode(VTK_SCALAR_MODE_USE_FIELD_DATA);
          // Color entire block by zeroth tuple in the field data
          this->EntityMapper->SetFieldDataTupleId(0);
          this->GlyphMapper->SetFieldDataTupleId(0);
          break;

        case vtkDataObject::FIELD_ASSOCIATION_POINTS:
        default:
          this->EntityMapper->SetScalarMode(VTK_SCALAR_MODE_USE_POINT_FIELD_DATA);
          this->GlyphMapper->SetScalarMode(VTK_SCALAR_MODE_USE_POINT_FIELD_DATA);
          break;
      }
      using_scalar_coloring = true;
    }
  }

  if (!using_scalar_coloring)
  {
    this->EntityMapper->SetScalarVisibility(0);
    this->EntityMapper->SelectColorArray(nullptr);
    this->GlyphMapper->SetScalarVisibility(0);
    this->GlyphMapper->SelectColorArray(nullptr);
  }
}

void vtkSMTKResourceRepresentation::ColorByVolume(vtkMultiBlockDataSet* data)
{
  if (!this->UpdateColorBy)
    return;

  // Traverse the blocks and set the volume's color
  this->EntityMapper->GetCompositeDataDisplayAttributes()->RemoveBlockColors();
  this->EntityMapper->GetCompositeDataDisplayAttributes()->RemoveBlockOpacities();
  vtkDataObjectTreeIterator* it = data->NewTreeIterator();
  it->VisitOnlyLeavesOff();
  it->GoToFirstItem();
  while (!it->IsDoneWithTraversal())
  {
    auto dataObj = it->GetCurrentDataObject();
    auto arr = vtkStringArray::SafeDownCast(
      dataObj->GetFieldData()->GetAbstractArray(vtkModelMultiBlockSource::GetVolumeTagName()));
    if (arr)
    {
      auto resource = this->Resource.lock();
      if (!resource)
      {
        vtkErrorMacro(<< "Invalid Resource!");
        return;
      }

      // FIXME Do something with additional volumes this block might be bounding
      // (currently only using the first one)
      ColorBlockAsEntity(this->EntityMapper, dataObj, arr->GetValue(0), resource);
    }
    else
    {
      // Set a default color
      std::array<double, 3> white = { { 1., 1., 1. } };
      this->EntityMapper->GetCompositeDataDisplayAttributes()->SetBlockColor(dataObj, white.data());
    }
    it->GoToNextItem();
  }
  it->Delete();
  this->UpdateColorBy = false;
}

void vtkSMTKResourceRepresentation::ColorByEntity(vtkMultiBlockDataSet* data)
{
  if (!this->UpdateColorBy)
    return;

  auto resource = this->Resource.lock();

  if (!resource)
    return;

  // Traverse the blocks and set the entity's color
  this->EntityMapper->GetCompositeDataDisplayAttributes()->RemoveBlockColors();
  this->EntityMapper->GetCompositeDataDisplayAttributes()->RemoveBlockOpacities();
  vtkDataObjectTreeIterator* it = data->NewTreeIterator();
  it->VisitOnlyLeavesOff();
  it->GoToFirstItem();
  while (!it->IsDoneWithTraversal())
  {
    auto dataObj = it->GetCurrentDataObject();
    auto uuid = vtkResourceMultiBlockSource::GetDataObjectUUID(data->GetMetaData(it));
    if (uuid)
    {
      auto ent = std::dynamic_pointer_cast<smtk::model::Entity>(resource->find(uuid));
      if (ent && ent->isInstance())
      {
        ColorBlockAsEntity(this->GlyphMapper, dataObj, uuid, resource);
      }
      else
      {
        ColorBlockAsEntity(this->EntityMapper, dataObj, uuid, resource);
      }
    }
    it->GoToNextItem();
  }
  it->Delete();
  this->UpdateColorBy = false;
}

void vtkSMTKResourceRepresentation::ApplyInternalBlockAttributes()
{
  // Update glyph attributes
  auto data = this->GetInternalOutputPort(0)->GetProducer()->GetOutputDataObject(0);
  if (this->BlockAttributeTime < data->GetMTime() || this->BlockAttrChanged)
  {
    this->ApplyEntityAttributes(this->EntityMapper.GetPointer());
    this->BlockAttributeTime.Modified();
    this->BlockAttrChanged = false;
  }

  auto outPort = this->GetInternalOutputPort(2);
  // some representations don't use output port 2 - avoid a crash.
  if (outPort)
  {
    data = outPort->GetProducer()->GetOutputDataObject(0);
    if (this->InstanceAttributeTime < data->GetMTime() || this->InstanceAttrChanged)
    {
      this->ApplyGlyphBlockAttributes(this->GlyphMapper.GetPointer());
      this->InstanceAttributeTime.Modified();
      this->InstanceAttrChanged = false;
    }
  }
}

void vtkSMTKResourceRepresentation::ApplyEntityAttributes(vtkMapper* mapper)
{
  auto cpm = vtkCompositePolyDataMapper2::SafeDownCast(mapper);
  if (!cpm)
  {
    vtkErrorMacro(<< "Invalid mapper!");
    return;
  }

  // TODO: Should we re-apply BlockVisibilities or ComponentState?
  //       If restoring visibility from a file, ParaView will probably
  //       provide via BlockVisibilities. Otherwise we should use
  //       ComponentState.

  // Do not call RemoveBlockColors, since some block attributes could
  // have been set through ColorBy mode
  for (auto const& item : this->BlockColors)
  {
    auto& arr = item.second;
    double color[3] = { arr[0], arr[1], arr[2] };
    cpm->SetBlockColor(item.first, color);
  }

  cpm->RemoveBlockOpacities();
  for (auto const& item : this->BlockOpacities)
  {
    cpm->SetBlockOpacity(item.first, item.second);
  }
}

void vtkSMTKResourceRepresentation::ApplyGlyphBlockAttributes(vtkGlyph3DMapper* mapper)
{
  auto instanceData = mapper->GetInputDataObject(0, 0);
  auto blockAttr = mapper->GetBlockAttributes();

  blockAttr->RemoveBlockVisibilities();
  for (auto const& item : this->InstanceVisibilities)
  {
    unsigned int currentIdx = 0;
    auto dob =
      vtkCompositeDataDisplayAttributes::DataObjectFromIndex(item.first, instanceData, currentIdx);

    if (dob)
    {
      blockAttr->SetBlockVisibility(dob, item.second);
    }
  }

  blockAttr->RemoveBlockColors();
  for (auto const& item : this->InstanceColors)
  {
    unsigned int currentIdx = 0;
    auto dob =
      vtkCompositeDataDisplayAttributes::DataObjectFromIndex(item.first, instanceData, currentIdx);

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

void vtkSMTKResourceRepresentation::SetBlockVisibility(unsigned int index, bool visible)
{
  this->BlockVisibilities[index] = visible;
  this->BlockAttrChanged = true;
}

bool vtkSMTKResourceRepresentation::GetBlockVisibility(unsigned int index) const
{
  auto it = this->BlockVisibilities.find(index);
  if (it == this->BlockVisibilities.cend())
  {
    return true;
  }
  return it->second;
}

void vtkSMTKResourceRepresentation::RemoveBlockVisibility(unsigned int index, bool /*unused*/)
{
  auto it = this->BlockVisibilities.find(index);
  if (it == this->BlockVisibilities.cend())
  {
    return;
  }
  this->BlockVisibilities.erase(it);
  this->BlockAttrChanged = true;
}

void vtkSMTKResourceRepresentation::RemoveBlockVisibilities()
{
  this->BlockVisibilities.clear();
  this->BlockAttrChanged = true;
}

void vtkSMTKResourceRepresentation::SetBlockColor(unsigned int index, double r, double g, double b)
{
  std::array<double, 3> color = { { r, g, b } };
  this->BlockColors[index] = color;
  this->BlockAttrChanged = true;
}

void vtkSMTKResourceRepresentation::SetBlockColor(unsigned int index, double* color)
{
  if (color)
  {
    this->SetBlockColor(index, color[0], color[1], color[2]);
  }
}

double* vtkSMTKResourceRepresentation::GetBlockColor(unsigned int index)
{
  auto it = this->BlockColors.find(index);
  if (it == this->BlockColors.cend())
  {
    return nullptr;
  }
  return it->second.data();
}

void vtkSMTKResourceRepresentation::RemoveBlockColor(unsigned int index)
{
  auto it = this->BlockColors.find(index);
  if (it == this->BlockColors.cend())
  {
    return;
  }
  this->BlockColors.erase(it);
  this->BlockAttrChanged = true;
}

void vtkSMTKResourceRepresentation::RemoveBlockColors()
{
  this->BlockColors.clear();
  this->BlockAttrChanged = true;
}

void vtkSMTKResourceRepresentation::SetBlockOpacity(unsigned int index, double opacity)
{
  this->BlockOpacities[index] = opacity;
  this->BlockAttrChanged = true;
}

void vtkSMTKResourceRepresentation::SetBlockOpacity(unsigned int index, double* opacity)
{
  if (opacity)
  {
    this->SetBlockOpacity(index, *opacity);
  }
}

double vtkSMTKResourceRepresentation::GetBlockOpacity(unsigned int index)
{
  auto it = this->BlockOpacities.find(index);
  if (it == this->BlockOpacities.cend())
  {
    return 0.0;
  }
  return it->second;
}

void vtkSMTKResourceRepresentation::RemoveBlockOpacity(unsigned int index)
{
  auto it = this->BlockOpacities.find(index);
  if (it == this->BlockOpacities.cend())
  {
    return;
  }
  this->BlockOpacities.erase(it);
  this->BlockAttrChanged = true;
}

void vtkSMTKResourceRepresentation::RemoveBlockOpacities()
{
  this->BlockOpacities.clear();
  this->BlockAttrChanged = true;
}

void vtkSMTKResourceRepresentation::SetSelectedState(
  vtkDataObject* data, int selectionValue, bool isGlyph)
{
  auto nrm = isGlyph ? this->GlyphMapper->GetBlockAttributes()
                     : this->EntityMapper->GetCompositeDataDisplayAttributes();
  auto sel = isGlyph ? this->SelectedGlyphMapper->GetBlockAttributes()
                     : this->SelectedEntityMapper->GetCompositeDataDisplayAttributes();
  if (selectionValue > 0)
  {
    sel->SetBlockColor(data, selectionValue > 1 ? this->HoverColor : this->SelectionColor);
  }
  sel->SetBlockVisibility(data, selectionValue > 0);
  nrm->SetBlockVisibility(data, selectionValue == 0);
}

void vtkSMTKResourceRepresentation::SetInstanceVisibility(unsigned int index, bool visible)
{
  this->InstanceVisibilities[index] = visible;
  this->InstanceAttrChanged = true;
}

bool vtkSMTKResourceRepresentation::GetInstanceVisibility(unsigned int index) const
{
  auto it = this->InstanceVisibilities.find(index);
  if (it == this->InstanceVisibilities.cend())
  {
    return true;
  }
  return it->second;
}

void vtkSMTKResourceRepresentation::RemoveInstanceVisibility(unsigned int index, bool /*unused*/)
{
  auto it = this->InstanceVisibilities.find(index);
  if (it == this->InstanceVisibilities.cend())
  {
    return;
  }
  this->InstanceVisibilities.erase(it);
  this->InstanceAttrChanged = true;
}

void vtkSMTKResourceRepresentation::RemoveInstanceVisibilities()
{
  this->InstanceVisibilities.clear();
  this->InstanceAttrChanged = true;
}

void vtkSMTKResourceRepresentation::SetInstanceColor(
  unsigned int index, double r, double g, double b)
{
  std::array<double, 3> color = { { r, g, b } };
  this->InstanceColors[index] = color;
  this->InstanceAttrChanged = true;
}

void vtkSMTKResourceRepresentation::SetInstanceColor(unsigned int index, double* color)
{
  if (color)
  {
    this->SetInstanceColor(index, color[0], color[1], color[2]);
  }
}

double* vtkSMTKResourceRepresentation::GetInstanceColor(unsigned int index)
{
  auto it = this->InstanceColors.find(index);
  if (it == this->InstanceColors.cend())
  {
    return nullptr;
  }
  return it->second.data();
}

void vtkSMTKResourceRepresentation::RemoveInstanceColor(unsigned int index)
{
  auto it = this->InstanceColors.find(index);
  if (it == this->InstanceColors.cend())
  {
    return;
  }
  this->InstanceColors.erase(it);
  this->InstanceAttrChanged = true;
}

void vtkSMTKResourceRepresentation::RemoveInstanceColors()
{
  this->InstanceColors.clear();
  this->InstanceAttrChanged = true;
}

void vtkSMTKResourceRepresentation::SetUseInternalAttributes(bool enable)
{
  this->UseInternalAttributes = enable;

  // Force update of internal attributes
  this->BlockAttrChanged = true;
  this->InstanceAttrChanged = true;

  // Force update of ColorBy
  this->UpdateColorBy = true;
}
