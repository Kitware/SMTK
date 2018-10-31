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
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkObjectFactory.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkStringArray.h>
#include <vtkTransform.h>
#include <vtksys/SystemTools.hxx>

#include <vtkPVCacheKeeper.h>
#include <vtkPVRenderView.h>
#include <vtkPVTrivialProducer.h>

#include "vtkDataObjectTreeIterator.h"

#include "smtk/extension/paraview/server/vtkSMTKModelReader.h"
#include "smtk/extension/paraview/server/vtkSMTKModelRepresentation.h"
#include "smtk/extension/paraview/server/vtkSMTKWrapper.h"
#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h"

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

namespace
{

void ColorBlockAsEntity(vtkCompositePolyDataMapper2* mapper, vtkDataObject* block,
  const std::string& uuid, const smtk::resource::ResourcePtr& res)
{
  using namespace smtk::model;
  auto modelResource = std::static_pointer_cast<Resource>(res);
  EntityRef entity(modelResource, smtk::common::UUID(uuid));
  FloatList color = entity.color();
  color = color[3] < 0 ? FloatList({ 1., 1., 1., 1. }) : color;

  // FloatList is a typedef for std::vector<double>, so it is safe to
  // pass the raw pointer to its data.
  auto atts = mapper->GetCompositeDataDisplayAttributes();
  atts->SetBlockColor(block, color.data());
}

void AddRenderables(
  vtkMultiBlockDataSet* data, vtkSMTKModelRepresentation::RenderableDataMap& renderables)
{
  if (!data)
  {
    return;
  }
  auto mbit = data->NewTreeIterator();
  mbit->VisitOnlyLeavesOn();
  for (mbit->GoToFirstItem(); !mbit->IsDoneWithTraversal(); mbit->GoToNextItem())
  {
    auto obj = mbit->GetCurrentDataObject();
    auto uid = vtkModelMultiBlockSource::GetDataObjectUUID(mbit->GetCurrentMetaData());
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
vtkStandardNewMacro(vtkSMTKModelRepresentation);

vtkSMTKModelRepresentation::vtkSMTKModelRepresentation()
  : Superclass()
  , Wrapper(nullptr)
  , SelectionObserver(-1)
  , EntityMapper(vtkSmartPointer<vtkCompositePolyDataMapper2>::New())
  , SelectedEntityMapper(vtkSmartPointer<vtkCompositePolyDataMapper2>::New())
  , EntityCacheKeeper(vtkSmartPointer<vtkPVCacheKeeper>::New())
  , GlyphMapper(vtkSmartPointer<vtkGlyph3DMapper>::New())
  , SelectedGlyphMapper(vtkSmartPointer<vtkGlyph3DMapper>::New())
  , Entities(vtkSmartPointer<vtkActor>::New())
  , SelectedEntities(vtkSmartPointer<vtkActor>::New())
  , GlyphEntities(vtkSmartPointer<vtkActor>::New())
  , SelectedGlyphEntities(vtkSmartPointer<vtkActor>::New())
  , ApplyStyle(ApplyDefaultStyle)
{
  this->SetupDefaults();
  this->SetNumberOfInputPorts(3);
}

vtkSMTKModelRepresentation::~vtkSMTKModelRepresentation()
{
  this->SetWrapper(nullptr);
}

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

  // Share vtkProperty between model mappers
  this->Property = this->Entities->GetProperty();
  this->GlyphEntities->SetProperty(this->Property);
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
  this->GetEntityBounds(this->GetInternalOutputPort(0)->GetProducer()->GetOutputDataObject(0),
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

bool vtkSMTKModelRepresentation::GetEntityBounds(
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

int vtkSMTKModelRepresentation::RequestData(
  vtkInformation* request, vtkInformationVector** inVec, vtkInformationVector* outVec)
{
  this->EntityCacheKeeper->RemoveAllCaches();
  if (inVec[0]->GetNumberOfInformationObjects() == 1)
  {
    vtkInformation* inInfo = inVec[0]->GetInformationObject(0);
    this->SetOutputExtent(this->GetInternalOutputPort(0), inInfo);

    // Model entities
    this->EntityCacheKeeper->SetInputConnection(this->GetInternalOutputPort(0));

    // Glyph points (2) and prototypes (1)
    this->GlyphMapper->SetInputConnection(this->GetInternalOutputPort(2));
    this->GlyphMapper->SetInputConnection(1, this->GetInternalOutputPort(1));
    this->ConfigureGlyphMapper(this->GlyphMapper.GetPointer());

    this->SelectedGlyphMapper->SetInputConnection(this->GetInternalOutputPort(2));
    this->SelectedGlyphMapper->SetInputConnection(1, this->GetInternalOutputPort(1));
    this->ConfigureGlyphMapper(this->SelectedGlyphMapper.GetPointer());
  }
  this->EntityCacheKeeper->Update();

  this->EntityMapper->Modified();
  this->GlyphMapper->Modified();

  this->GetModelBounds();

  // New input data requires updated block colors:
  this->UpdateColorBy = true;
  return Superclass::RequestData(request, inVec, outVec);
}

int vtkSMTKModelRepresentation::ProcessViewRequest(
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
    vtkPVRenderView::SetPiece(inInfo, this, this->EntityCacheKeeper->GetOutputDataObject(0), 0, 0);

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
    vtkPVRenderView::SetGeometryBounds(inInfo, this->DataBounds, matrix.GetPointer());
  }
  else if (request_type == vtkPVView::REQUEST_UPDATE_LOD())
  {
    /// TODO Add LOD Mappers
  }
  else if (request_type == vtkPVView::REQUEST_RENDER())
  {
    // Update model entity and glyph attributes
    // vtkDataObject* (blocks) in the multi-block may have changed after
    // updating the pipeline, so UpdateEntityAttributes and UpdateInstanceAttributes
    // are called here to ensure the block attributes are updated with the current
    // block pointers. To do this, it uses the flat-index mapped attributes stored in
    // this class.
    auto producerPort = vtkPVRenderView::GetPieceProducer(inInfo, this, 0);
    this->EntityMapper->SetInputConnection(0, producerPort);
    this->SelectedEntityMapper->SetInputConnection(0, producerPort);
    auto producer = producerPort->GetProducer();
    auto data = producer->GetOutputDataObject(vtkModelMultiBlockSource::MODEL_ENTITY_PORT);
    this->UpdateColoringParameters(data);
    this->UpdateRepresentationSubtype();

    // Entities
    auto multiblockModel = vtkMultiBlockDataSet::SafeDownCast(data);
    // Glyphs
    data = producer->GetNumberOfOutputPorts() > 1
      ? producer->GetOutputDataObject(vtkModelMultiBlockSource::INSTANCE_PORT)
      : nullptr;
    auto multiblockInstance = vtkMultiBlockDataSet::SafeDownCast(data);

    // If the input has changed, update the map of polydata pointers in RenderableData:
    this->UpdateRenderableData(multiblockModel, multiblockInstance);
    // If the selection has changed, update the visual properties of blocks:
    this->UpdateDisplayAttributesFromSelection(multiblockModel, multiblockInstance);
  }

  return 1;
}

void vtkSMTKModelRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}

bool vtkSMTKModelRepresentation::AddToView(vtkView* view)
{
  vtkPVRenderView* rview = vtkPVRenderView::SafeDownCast(view);
  if (rview)
  {
    rview->GetRenderer()->AddActor(this->Entities);
    rview->GetRenderer()->AddActor(this->GlyphEntities);
    rview->GetRenderer()->AddActor(this->SelectedEntities);
    rview->GetRenderer()->AddActor(this->SelectedGlyphEntities);

    rview->RegisterPropForHardwareSelection(this, this->Entities);
    rview->RegisterPropForHardwareSelection(this, this->GlyphEntities);
    rview->RegisterPropForHardwareSelection(this, this->SelectedEntities);
    rview->RegisterPropForHardwareSelection(this, this->SelectedGlyphEntities);

    return Superclass::AddToView(view);
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

void vtkSMTKModelRepresentation::SetVisibility(bool val)
{
  this->Entities->SetVisibility(val);
  this->GlyphEntities->SetVisibility(val);

  this->SelectedEntities->SetVisibility(val);
  this->SelectedGlyphEntities->SetVisibility(val);

  Superclass::SetVisibility(val);
}

vtkDataObject* FindEntityData(vtkMultiBlockDataSet* mbds, smtk::model::EntityPtr ent)
{
  if (mbds)
  {
    auto it = mbds->NewTreeIterator();
    it->VisitOnlyLeavesOn();
    for (it->GoToFirstItem(); !it->IsDoneWithTraversal(); it->GoToNextItem())
    {
      vtkDataObject* data = it->GetCurrentDataObject();
      auto uid = vtkModelMultiBlockSource::GetDataObjectUUID(
        it->GetCurrentMetaData()); // data->GetInformation());
      // std::cout << "    " << uid << " == " << ent->id() << "\n";
      if (uid == ent->id())
      {
        it->Delete();
        return data;
      }
    }
    it->Delete();
  }
  return nullptr;
}

void vtkSMTKModelRepresentation::GetEntityVisibilities(std::map<smtk::common::UUID, int>& visdata)
{
  visdata.clear();
  for (auto entry : this->ComponentState)
  {
    visdata[entry.first] = entry.second.m_visibility;
  }
}

bool vtkSMTKModelRepresentation::SetEntityVisibility(
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
    }
  }
  return didChange;
}

bool vtkSMTKModelRepresentation::ApplyDefaultStyle(
  smtk::view::SelectionPtr seln, RenderableDataMap& renderables, vtkSMTKModelRepresentation* self)
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
    attr = item.first->as<smtk::attribute::Attribute>();
    if (attr)
    {
      for (auto obj : *attr->associations())
      {
        atLeastOneSelected |= self->SelectComponentFootprint(obj, /*selnBit TODO*/ 1, renderables);
      }
    }
    else
    {
      atLeastOneSelected |= self->SelectComponentFootprint(item.first, item.second, renderables);
    }
  }

  return atLeastOneSelected;
}

bool vtkSMTKModelRepresentation::SelectComponentFootprint(
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
        for (auto cell : cells)
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
        for (auto group : groups)
        {
          auto members = group.members<smtk::model::EntityRefs>();
          atLeastOneSelected |= this->SelectComponentFootprint(members, selnBits, renderables);
        }

        // TODO: Auxiliary geometry may also be handled by a separate representation.
        //       Need to ensure that representation also renders selection properly.
        auto auxGeoms = model.auxiliaryGeometry();
        // Convert auxGeoms to EntityRefs to match SelectComponentFootprint() API:
        smtk::model::EntityRefs auxEnts;
        for (auto auxGeom : auxGeoms)
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

bool vtkSMTKModelRepresentation::SelectComponentFootprint(
  const smtk::model::EntityRefs& items, int selnBits, RenderableDataMap& renderables)
{
  bool atLeastOneSelected = false;
  auto& smap = this->GetComponentState();
  for (auto item : items)
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

void vtkSMTKModelRepresentation::SetInterpolateScalarsBeforeMapping(int val)
{
  this->EntityMapper->SetInterpolateScalarsBeforeMapping(val);
  this->GlyphMapper->SetInterpolateScalarsBeforeMapping(val);
}

void vtkSMTKModelRepresentation::UpdateRenderableData(
  vtkMultiBlockDataSet* modelData, vtkMultiBlockDataSet* instanceData)
{
  if ((modelData && modelData->GetMTime() > this->RenderableTime) ||
    (instanceData && instanceData->GetMTime() > this->RenderableTime) ||
    (this->SelectionTime > this->RenderableTime))
  {
    this->RenderableData.clear();
    AddRenderables(instanceData, this->RenderableData);
    AddRenderables(modelData, this->RenderableData);
    this->RenderableTime.Modified();
  }
}

void vtkSMTKModelRepresentation::UpdateDisplayAttributesFromSelection(
  vtkMultiBlockDataSet* modelData, vtkMultiBlockDataSet* instanceData)
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

  if (!modelData)
  {
    return;
  }

  if (modelData->GetMTime() < this->ApplyStyleTime &&
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
  for (auto entry : this->RenderableData)
  {
    seda->SetBlockVisibility(entry.second, false);
    sgda->SetBlockVisibility(entry.second, false);
  }

  // Add user-specified visibility
  for (auto entry : this->ComponentState)
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

void vtkSMTKModelRepresentation::UpdateSelection(
  vtkMultiBlockDataSet* data, vtkCompositeDataDisplayAttributes* blockAttr, vtkActor* actor)
{
  auto rm = this->GetWrapper(); // vtkSMTKWrapper::Instance(); // TODO: Remove the need for this.
  auto sm = rm ? rm->GetSelection() : nullptr;
  // std::cout << "rep " << this << " wrapper " << rm << " seln " << sm << "\n";
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

void vtkSMTKModelRepresentation::SetResource(smtk::resource::ResourcePtr res)
{
  this->Resource = res;
}

void vtkSMTKModelRepresentation::SetColorBy(const char* type)
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

void vtkSMTKModelRepresentation::SelectionModified()
{
  this->SelectionTime.Modified();
}

void vtkSMTKModelRepresentation::SetWrapper(vtkSMTKWrapper* wrapper)
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
      oldSeln->unobserve(this->SelectionObserver);
    }
    this->SelectionObserver = -1;
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
      ? newSeln->observe(
          [this](const std::string&, smtk::view::Selection::Ptr) { this->SelectionModified(); })
      : -1;
  }
  this->Modified();
}

void vtkSMTKModelRepresentation::SetRepresentation(const char* type)
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

void vtkSMTKModelRepresentation::SetSelectionPointSize(double val)
{
  this->SelectedEntities->GetProperty()->SetPointSize(val);
  this->SelectedGlyphEntities->GetProperty()->SetPointSize(val);
}

void vtkSMTKModelRepresentation::SetLookupTable(vtkScalarsToColors* val)
{
  this->EntityMapper->SetLookupTable(val);
  this->GlyphMapper->SetLookupTable(val);
}

void vtkSMTKModelRepresentation::SetSelectionLineWidth(double val)
{
  this->SelectedEntities->GetProperty()->SetLineWidth(val);
  this->SelectedGlyphEntities->GetProperty()->SetLineWidth(val);
}

void vtkSMTKModelRepresentation::SetPointSize(double val)
{
  this->Property->SetPointSize(val);
}

void vtkSMTKModelRepresentation::SetLineWidth(double val)
{
  this->Property->SetLineWidth(val);
}

void vtkSMTKModelRepresentation::SetLineColor(double r, double g, double b)
{
  this->Property->SetEdgeColor(r, g, b);
}

void vtkSMTKModelRepresentation::SetOpacity(double val)
{
  this->Property->SetOpacity(val);
}

void vtkSMTKModelRepresentation::SetPosition(double x, double y, double z)
{
  this->Entities->SetPosition(x, y, z);
  this->SelectedEntities->SetPosition(x, y, z);
  this->GlyphEntities->SetPosition(x, y, z);
  this->SelectedGlyphEntities->SetPosition(x, y, z);
}

void vtkSMTKModelRepresentation::SetScale(double x, double y, double z)
{
  this->Entities->SetScale(x, y, z);
  this->SelectedEntities->SetScale(x, y, z);
  this->GlyphEntities->SetScale(x, y, z);
  this->SelectedGlyphEntities->SetScale(x, y, z);
}

void vtkSMTKModelRepresentation::SetOrientation(double x, double y, double z)
{
  this->Entities->SetOrientation(x, y, z);
  this->SelectedEntities->SetOrientation(x, y, z);
  this->GlyphEntities->SetOrientation(x, y, z);
  this->SelectedGlyphEntities->SetOrientation(x, y, z);
}

void vtkSMTKModelRepresentation::SetOrigin(double x, double y, double z)
{
  this->Entities->SetOrigin(x, y, z);
  this->SelectedEntities->SetOrigin(x, y, z);
  this->GlyphEntities->SetOrigin(x, y, z);
  this->SelectedGlyphEntities->SetOrigin(x, y, z);
}

void vtkSMTKModelRepresentation::SetUserTransform(const double matrix[16])
{
  vtkNew<vtkTransform> transform;
  transform->SetMatrix(matrix);
  this->Entities->SetUserTransform(transform.GetPointer());
  this->SelectedEntities->SetUserTransform(transform.GetPointer());
  this->GlyphEntities->SetUserTransform(transform.GetPointer());
  this->SelectedGlyphEntities->SetUserTransform(transform.GetPointer());
}

void vtkSMTKModelRepresentation::SetPickable(int val)
{
  this->Entities->SetPickable(val);
  this->GlyphEntities->SetPickable(val);
}

void vtkSMTKModelRepresentation::SetTexture(vtkTexture* val)
{
  this->Entities->SetTexture(val);
  this->GlyphEntities->SetTexture(val);
}

void vtkSMTKModelRepresentation::SetSpecularPower(double val)
{
  this->Property->SetSpecularPower(val);
}

void vtkSMTKModelRepresentation::SetSpecular(double val)
{
  this->Property->SetSpecular(val);
}

void vtkSMTKModelRepresentation::SetAmbient(double val)
{
  this->Property->SetAmbient(val);
}

void vtkSMTKModelRepresentation::SetDiffuse(double val)
{
  this->Property->SetDiffuse(val);
}

void vtkSMTKModelRepresentation::UpdateRepresentationSubtype()
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

void vtkSMTKModelRepresentation::UpdateColoringParameters(vtkDataObject* data)
{
  auto multiBlock = vtkMultiBlockDataSet::SafeDownCast(data);
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

void vtkSMTKModelRepresentation::ColorByField()
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

void vtkSMTKModelRepresentation::ColorByVolume(vtkCompositeDataSet* data)
{
  if (!this->UpdateColorBy)
    return;

  // Traverse the blocks and set the volume's color
  this->EntityMapper->GetCompositeDataDisplayAttributes()->RemoveBlockColors();
  vtkCompositeDataIterator* it = data->NewIterator();
  it->GoToFirstItem();
  while (!it->IsDoneWithTraversal())
  {
    auto dataObj = it->GetCurrentDataObject();
    auto arr = vtkStringArray::SafeDownCast(
      dataObj->GetFieldData()->GetAbstractArray(vtkModelMultiBlockSource::GetVolumeTagName()));
    if (arr)
    {
      if (!this->Resource)
      {
        vtkErrorMacro(<< "Invalid Resource!");
        return;
      }

      // FIXME Do something with additional volumes this block might be bounding
      // (currently only using the first one)
      ColorBlockAsEntity(this->EntityMapper, dataObj, arr->GetValue(0), this->Resource);
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

void vtkSMTKModelRepresentation::ColorByEntity(vtkMultiBlockDataSet* data)
{
  if (!this->UpdateColorBy)
    return;

  // Traverse the blocks and set the entity's color
  this->EntityMapper->GetCompositeDataDisplayAttributes()->RemoveBlockColors();
  vtkCompositeDataIterator* it = data->NewIterator();
  it->GoToFirstItem();
  while (!it->IsDoneWithTraversal())
  {
    auto dataObj = it->GetCurrentDataObject();
    if (data->HasMetaData(it))
    {
      auto uuid = data->GetMetaData(it)->Get(vtkModelMultiBlockSource::ENTITYID());
      if (uuid)
      {
        // FIXME? Check whether UUID corresponds to an instance or not.
        //        Instances should use the GlyphMapper rather than the EntityMapper.
        ColorBlockAsEntity(this->EntityMapper, dataObj, uuid, this->Resource);
      }
    }
    it->GoToNextItem();
  }
  it->Delete();
  this->UpdateColorBy = false;
}

void vtkSMTKModelRepresentation::ApplyInternalBlockAttributes()
{
  // Update glyph attributes
  auto data = this->GetInternalOutputPort(0)->GetProducer()->GetOutputDataObject(0);
  if (this->BlockAttributeTime < data->GetMTime() || this->BlockAttrChanged)
  {
    this->ApplyEntityAttributes(this->EntityMapper.GetPointer());
    this->BlockAttributeTime.Modified();
    this->BlockAttrChanged = false;
  }

  data = this->GetInternalOutputPort(2)->GetProducer()->GetOutputDataObject(0);
  if (this->InstanceAttributeTime < data->GetMTime() || this->InstanceAttrChanged)
  {
    this->ApplyGlyphBlockAttributes(this->GlyphMapper.GetPointer());
    this->InstanceAttributeTime.Modified();
    this->InstanceAttrChanged = false;
  }
}

void vtkSMTKModelRepresentation::ApplyEntityAttributes(vtkMapper* mapper)
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

void vtkSMTKModelRepresentation::ApplyGlyphBlockAttributes(vtkGlyph3DMapper* mapper)
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

void vtkSMTKModelRepresentation::SetBlockVisibility(unsigned int index, bool visible)
{
  this->BlockVisibilities[index] = visible;
  this->BlockAttrChanged = true;
}

bool vtkSMTKModelRepresentation::GetBlockVisibility(unsigned int index) const
{
  auto it = this->BlockVisibilities.find(index);
  if (it == this->BlockVisibilities.cend())
  {
    return true;
  }
  return it->second;
}

void vtkSMTKModelRepresentation::RemoveBlockVisibility(unsigned int index, bool)
{
  auto it = this->BlockVisibilities.find(index);
  if (it == this->BlockVisibilities.cend())
  {
    return;
  }
  this->BlockVisibilities.erase(it);
  this->BlockAttrChanged = true;
}

void vtkSMTKModelRepresentation::RemoveBlockVisibilities()
{
  this->BlockVisibilities.clear();
  this->BlockAttrChanged = true;
}

void vtkSMTKModelRepresentation::SetBlockColor(unsigned int index, double r, double g, double b)
{
  std::array<double, 3> color = { { r, g, b } };
  this->BlockColors[index] = color;
  this->BlockAttrChanged = true;
}

void vtkSMTKModelRepresentation::SetBlockColor(unsigned int index, double* color)
{
  if (color)
  {
    this->SetBlockColor(index, color[0], color[1], color[2]);
  }
}

double* vtkSMTKModelRepresentation::GetBlockColor(unsigned int index)
{
  auto it = this->BlockColors.find(index);
  if (it == this->BlockColors.cend())
  {
    return nullptr;
  }
  return it->second.data();
}

void vtkSMTKModelRepresentation::RemoveBlockColor(unsigned int index)
{
  auto it = this->BlockColors.find(index);
  if (it == this->BlockColors.cend())
  {
    return;
  }
  this->BlockColors.erase(it);
  this->BlockAttrChanged = true;
}

void vtkSMTKModelRepresentation::RemoveBlockColors()
{
  this->BlockColors.clear();
  this->BlockAttrChanged = true;
}

void vtkSMTKModelRepresentation::SetBlockOpacity(unsigned int index, double opacity)
{
  this->BlockOpacities[index] = opacity;
  this->BlockAttrChanged = true;
}

void vtkSMTKModelRepresentation::SetBlockOpacity(unsigned int index, double* opacity)
{
  if (opacity)
  {
    this->SetBlockOpacity(index, *opacity);
  }
}

double vtkSMTKModelRepresentation::GetBlockOpacity(unsigned int index)
{
  auto it = this->BlockOpacities.find(index);
  if (it == this->BlockOpacities.cend())
  {
    return 0.0;
  }
  return it->second;
}

void vtkSMTKModelRepresentation::RemoveBlockOpacity(unsigned int index)
{
  auto it = this->BlockOpacities.find(index);
  if (it == this->BlockOpacities.cend())
  {
    return;
  }
  this->BlockOpacities.erase(it);
  this->BlockAttrChanged = true;
}

void vtkSMTKModelRepresentation::RemoveBlockOpacities()
{
  this->BlockOpacities.clear();
  this->BlockAttrChanged = true;
}

void vtkSMTKModelRepresentation::SetSelectedState(
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

void vtkSMTKModelRepresentation::SetUseInternalAttributes(bool enable)
{
  this->UseInternalAttributes = enable;

  // Force update of internal attributes
  this->BlockAttrChanged = true;
  this->InstanceAttrChanged = true;

  // Force update of ColorBy
  this->UpdateColorBy = true;
}
