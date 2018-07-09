//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h"

#include "smtk/extension/vtk/source/vtkAuxiliaryGeometryExtension.h"
#include "smtk/extension/vtk/source/vtkModelAuxiliaryGeometry.h"
#include "smtk/extension/vtk/source/vtkModelAuxiliaryGeometry.txx"

#include "smtk/extension/vtk/filter/vtkImageSpacingFlip.h"

#include "smtk/extension/vtk/io/mesh/ExportVTKData.h"

#include "smtk/mesh/core/Collection.h"
#include "smtk/mesh/core/MeshSet.h"

#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/Edge.h"
#include "smtk/model/EdgeUse.h"
#include "smtk/model/EntityIterator.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Face.h"
#include "smtk/model/FaceUse.h"
#include "smtk/model/Group.h"
#include "smtk/model/Instance.h"
#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"
#include "smtk/model/ShellEntity.h"
#include "smtk/model/Tessellation.h"
#include "smtk/model/UseEntity.h"
#include "smtk/model/Volume.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkDataObjectTreeIterator.h"
#include "vtkDoubleArray.h"
#include "vtkIdTypeArray.h"
#include "vtkImageData.h"
#include "vtkImageMapToColors.h"
#include "vtkInformation.h"
#include "vtkInformationStringKey.h"
#include "vtkInformationVector.h"
#include "vtkLookupTable.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkNew.h"
#include "vtkOBJReader.h"
#include "vtkObjectFactory.h"
#include "vtkPLYReader.h"
#include "vtkPTSReader.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkPolyData.h"
#include "vtkPolyDataNormals.h"
#include "vtkProperty.h"
#include "vtkStringArray.h"
#include "vtkUnstructuredGrid.h"
#include "vtkXMLImageDataReader.h"
#include "vtkXMLMultiBlockDataReader.h"
#include "vtkXMLPolyDataReader.h"
#include "vtkXMLUnstructuredGridReader.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "boost/filesystem.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>

using namespace smtk::model;

vtkStandardNewMacro(vtkModelMultiBlockSource);
vtkInformationKeyMacro(vtkModelMultiBlockSource, ENTITYID, String);
smtkImplementTracksAllInstances(vtkModelMultiBlockSource);

vtkModelMultiBlockSource::vtkModelMultiBlockSource()
{
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(NUMBER_OF_OUTPUT_PORTS);
  this->CachedOutputMBDS = nullptr;
  this->CachedOutputInst = nullptr;
  this->CachedOutputProto = nullptr;
  for (int i = 0; i < 4; ++i)
  {
    this->DefaultColor[i] = 1.;
  }
  this->ModelEntityID = NULL;
  this->AllowNormalGeneration = 0;
  this->ShowAnalysisTessellation = 0;
  this->linkInstance();
}

vtkModelMultiBlockSource::~vtkModelMultiBlockSource()
{
  this->unlinkInstance();
  this->SetCachedOutput(nullptr, nullptr, nullptr);
  this->SetModelEntityID(nullptr);
}

void vtkModelMultiBlockSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Model: " << this->ModelResource.get() << "\n";
  os << indent << "CachedOutputMBDS: " << this->CachedOutputMBDS << "\n";
  os << indent << "CachedOutputInst: " << this->CachedOutputInst << "\n";
  os << indent << "ModelEntityID: " << this->ModelEntityID << "\n";
  os << indent << "AllowNormalGeneration: " << (this->AllowNormalGeneration ? "ON" : "OFF") << "\n";
  os << indent << "ShowAnalysisTessellation: " << this->ShowAnalysisTessellation << "\n";
}

/// Set the SMTK model to be displayed.
void vtkModelMultiBlockSource::SetModelResource(smtk::model::ResourcePtr model)
{
  if (this->ModelResource == model)
  {
    return;
  }
  this->ModelResource = model;
  this->Modified();
}

/// Get the SMTK model being displayed.
smtk::model::ResourcePtr vtkModelMultiBlockSource::GetModelResource()
{
  return this->ModelResource;
}

/// Get the map from model entity UUID to the block index in multiblock output
void vtkModelMultiBlockSource::GetUUID2BlockIdMap(std::map<smtk::common::UUID, vtkIdType>& uuid2mid)
{
  uuid2mid.clear();
  uuid2mid.insert(this->UUID2BlockIdMap.begin(), this->UUID2BlockIdMap.end());
}

/// Indicate that the model has changed and should have its VTK representation updated.
void vtkModelMultiBlockSource::Dirty()
{
  // This both clears the output and marks this filter
  // as modified so that RequestData() will run the next
  // time the representation is updated:
  this->SetCachedOutput(nullptr, nullptr, nullptr);
}

void vtkModelMultiBlockSource::SetDataObjectUUID(
  vtkInformation* info, const smtk::common::UUID& uid)
{
  // FIXME: Eventually this should encode the UUID without string conversion
  info->Set(vtkModelMultiBlockSource::ENTITYID(), uid.toString().c_str());
}

smtk::common::UUID vtkModelMultiBlockSource::GetDataObjectUUID(vtkInformation* datainfo)
{
  // FIXME: Eventually this should decode the UUID without string conversion
  smtk::common::UUID uid;
  if (!datainfo)
  {
    return uid;
  }

  const char* uuidChar = datainfo->Get(vtkModelMultiBlockSource::ENTITYID());
  if (uuidChar)
  {
    uid = smtk::common::UUID(uuidChar);
  }
  return uid;
}

/*! \fn vtkModelMultiBlockSource::GetDefaultColor()
 *  \brief Get the RGBA color for model entities that do not have a color property set.
 *
 * The default value for DefaultColor is [1., 1., 1., 1.].
 *
 *  \fn vtkModelMultiBlockSource::GetDefaultColor(double& r, double& g, double& b, double& a)
 *  \brief Get the RGBA color for model entities that do not have a color property set.
 *
 * The default value for DefaultColor is [1., 1., 1., 1.].
 *
 *  \fn vtkModelMultiBlockSource::GetDefaultColor(double rgba[4])
 *  \brief Get the RGBA color for model entities that do not have a color property set.
 *
 * The default value for DefaultColor is [1., 1., 1., 1.].
 */

/*! \fn vtkModelMultiBlockSource::SetDefaultColor(double r, double g, double b, double a)
 *  \brief Set the default color (RGBA) for each model entity.
 *
 * This color will be assigned to each cell of each block for entities
 * that do not provide a color float-property.
 *
 * Setting the alpha component of the default color to a
 * negative number will turn off per-cell color array generation
 * to save space.
 *
 * \sa vtkModelMultiBlockSource::GetDefaultColor
 *
 *  \fn vtkModelMultiBlockSource::SetDefaultColor(double rgba[4])
 *  \brief Set the default color (RGBA) for each model entity.
 *
 * This color will be assigned to each cell of each block for entities
 * that do not provide a color float-property.
 *
 * Setting the alpha component of the default color to a
 * negative number will turn off per-cell color array generation
 * to save space.
 *
 * \sa vtkModelMultiBlockSource::GetDefaultColor
 */

/*! \fn vtkModelMultiBlockSource::GetShowAnalysisTessellation()
 *  \brief Get which tessellation to output.
 *
 * The default value is 0 (output the "display" tessellation, which
 * is not suitable for analysis but generally fast to compute).
 * A non-zero value indicates the analysis tessellation will be
 * output if present. If not present, then the display tessellation
 * will be output.
 */

/*! \fn vtkModelMultiBlockSource::SetShowAnalysisTessellation(int tess)
 *  \brief Set which tessellation to output.
 *
 * Setting this to a non-zero value will cause the
 * analysis tessellation (if present) to be output.
 * Otherwise, the display tessellation is output.
 *
 *  \fn vtkModelMultiBlockSource::ShowAnalysisTessellationOn()
 *  \brief Request the analysis tessellation be shown.
 *
 *  \fn vtkModelMultiBlockSource::ShowAnalysisTessellationOff()
 *  \brief Request the display tessellation be shown.
 */

static void AddEntityTessToPolyData(const smtk::model::EntityRef& entityref, vtkPoints* pts,
  vtkPolyData* pd, int showAnalysisTessellation)
{
  // gotMesh fetches Analysis mesh if it exists, falling back
  // to model tessellation if not.
  const smtk::model::Tessellation* tess =
    showAnalysisTessellation ? entityref.hasAnalysisMesh() : entityref.hasTessellation();
  if (!tess)
    return;

  vtkIdType i;
  //vtkIdType connOffset = pts->GetNumberOfPoints();
  std::vector<vtkIdType> conn;
  std::string uuidStr = entityref.entity().toString();
  vtkIdType npts = tess->coords().size() / 3;
  for (i = 0; i < npts; ++i)
  {
    pts->InsertNextPoint(&tess->coords()[3 * i]);
  }
  Tessellation::size_type off;
  vtkNew<vtkCellArray> verts;
  vtkNew<vtkCellArray> lines;
  vtkNew<vtkCellArray> polys;
  vtkNew<vtkCellArray> strip;
  bool have_verts = false;
  bool have_lines = false;
  bool have_polys = false;
  bool have_strip = false;
  for (off = tess->begin(); off != tess->end(); off = tess->nextCellOffset(off))
  {
    Tessellation::size_type cell_type = tess->cellType(off);
    Tessellation::size_type cell_shape = tess->cellShapeFromType(cell_type);
    std::vector<int> cell_conn;
    Tessellation::size_type num_verts = tess->vertexIdsOfCell(off, cell_conn);
    std::vector<vtkIdType> vtk_conn(cell_conn.begin(), cell_conn.end());
    switch (cell_shape)
    {
      case TESS_VERTEX:
        have_verts = true;
        verts->InsertNextCell(1, &vtk_conn[0]);
        break;
      case TESS_TRIANGLE:
        have_polys = true;
        polys->InsertNextCell(3, &vtk_conn[0]);
        break;
      case TESS_QUAD:
        have_polys = true;
        polys->InsertNextCell(4, &vtk_conn[0]);
        break;
      case TESS_POLYVERTEX:
        have_verts = true;
        verts->InsertNextCell(num_verts, &vtk_conn[0]);
        break;
      case TESS_POLYLINE:
        have_lines = true;
        lines->InsertNextCell(num_verts, &vtk_conn[0]);
        break;
      case TESS_POLYGON:
        have_polys = true;
        polys->InsertNextCell(num_verts, &vtk_conn[0]);
        break;
      case TESS_TRIANGLE_STRIP:
        have_strip = true;
        strip->InsertNextCell(num_verts, &vtk_conn[0]);
        break;
      default:
        std::cerr << "Invalid cell shape " << cell_shape << " at offset " << off << ". Skipping.\n";
        continue;
        break;
    }
  }
  if (have_verts)
    pd->SetVerts(verts.GetPointer());
  if (have_lines)
    pd->SetLines(lines.GetPointer());
  if (have_polys)
    pd->SetPolys(polys.GetPointer());
  if (have_strip)
    pd->SetStrips(strip.GetPointer());
}

static bool AddColorWithDefault(
  vtkPolyData* pd, const smtk::model::EntityRef& entity, const double defaultColor[4])
{
  // Only create the color array if there is a valid default:
  if (defaultColor[3] >= 0.)
  {
    FloatList rgba = entity.color();
    vtkNew<vtkUnsignedCharArray> cellColor;
    cellColor->SetNumberOfComponents(4);
    cellColor->SetNumberOfTuples(1);
    cellColor->SetName("entity color");
    for (int i = 0; i < 4; ++i)
    {
      cellColor->FillComponent(i, (rgba[3] >= 0 ? rgba[i] : defaultColor[i]) * 255.);
    }
    pd->GetFieldData()->AddArray(cellColor.GetPointer());
    return true;
  }
  return false;
}

/// Add customized block info.
/// Mapping from UUID to block id
/// 'Volume' field array to color by volume
static void addBlockInfo(smtk::model::ResourcePtr resource, const smtk::model::EntityRef& entityref,
  const smtk::model::EntityRef& bordantCell, const vtkIdType& blockId, vtkDataObject* dobj,
  std::map<smtk::common::UUID, vtkIdType>& uuid2BlockId)
{
  resource->setIntegerProperty(entityref.entity(), "block_index", blockId);
  uuid2BlockId[entityref.entity()] = blockId;

  // Add Entity UUID to fieldData
  vtkNew<vtkStringArray> uuidArray;
  uuidArray->SetNumberOfComponents(1);
  uuidArray->SetNumberOfTuples(1);
  uuidArray->SetName(vtkModelMultiBlockSource::GetEntityTagName());
  uuidArray->SetValue(0, entityref.entity().toString());
  dobj->GetFieldData()->AddArray(uuidArray.GetPointer());

  smtk::model::EntityRefs vols;
  if (bordantCell.isValid() && bordantCell.isVolume())
    vols.insert(bordantCell);
  if (vols.size())
  {
    // Add volume UUID to fieldData
    vtkNew<vtkStringArray> volArray;
    volArray->SetNumberOfComponents(1);
    volArray->SetNumberOfTuples(vols.size());
    int ai = 0;
    for (smtk::model::EntityRefs::iterator it = vols.begin(); it != vols.end(); ++it, ++ai)
    {
      volArray->SetValue(ai, (*it).entity().toString());
    }
    volArray->SetName(vtkModelMultiBlockSource::GetVolumeTagName());
    dobj->GetFieldData()->AddArray(volArray.GetPointer());
  }
}

/// Generate a data object representing the entity. It may be polydata, image data, or a multiblock dataset.
vtkSmartPointer<vtkDataObject> vtkModelMultiBlockSource::GenerateRepresentationFromModel(
  const smtk::model::EntityRef& entity, bool genNormals)
{
  const smtk::model::Tessellation* tess;
  if ((tess = entity.hasTessellation()))
  {
    return this->GenerateRepresentationFromTessellation(entity, tess, genNormals);
  }
  else if (!entity.meshTessellation().is_empty())
  {
    return this->GenerateRepresentationFromMeshTessellation(entity, genNormals);
  }

  smtk::model::AuxiliaryGeometry aux(entity);
  std::string url;
  if (aux.isValid())
  {
    bool bShow = true;
    if (entity.hasIntegerProperty("display as separate representation"))
    {
      const IntegerList& prop(entity.integerProperty("display as separate representation"));
      // show this block if the property is not specified or equals to zero
      bShow = (prop.empty() || prop[0] == 0);
    }
    if (bShow)
    {
      auto ext = vtkAuxiliaryGeometryExtension::create();
      std::vector<double> bbox;
      if (ext->canHandleAuxiliaryGeometry(aux, bbox))
      {
        auto cgeom = ext->fetchCachedGeometry(aux);
        return cgeom;
      }
    }
  }
  return vtkSmartPointer<vtkDataObject>();
}

vtkSmartPointer<vtkPolyData> vtkModelMultiBlockSource::GenerateRepresentationFromTessellation(
  const smtk::model::EntityRef& entity, const smtk::model::Tessellation* tess, bool genNormals)
{
  vtkSmartPointer<vtkPolyData> pd = vtkSmartPointer<vtkPolyData>::New();
  vtkNew<vtkPoints> pts;
  pts->SetDataTypeToDouble();
  pd->SetPoints(pts.GetPointer());

  vtkIdType npts = tess->coords().size() / 3;
  pts->Allocate(npts);
  smtk::model::EntityPtr entrec;
  if (entity.isValid(&entrec))
  {
    AddEntityTessToPolyData(entity, pts.GetPointer(), pd, this->ShowAnalysisTessellation);
    AddColorWithDefault(pd, entity, this->DefaultColor);
    if (this->AllowNormalGeneration && pd->GetPolys()->GetSize() > 0)
    {
      bool reallyNeedNormals = genNormals;
      if (entity.hasIntegerProperty("generate normals"))
      { // Allow per-entity setting to override per-model setting
        const IntegerList& prop(entity.integerProperty("generate normals"));
        reallyNeedNormals = (!prop.empty() && prop[0]);
      }
      if (reallyNeedNormals)
      {
        this->NormalGenerator->SetInputDataObject(pd);
        this->NormalGenerator->Update();
        pd->ShallowCopy(this->NormalGenerator->GetOutput());
      }
    }

    vtkModelMultiBlockSource::AddPointsAsAttribute(pd);
  }
  return pd;
}

vtkSmartPointer<vtkPolyData> vtkModelMultiBlockSource::GenerateRepresentationFromMeshTessellation(
  const smtk::model::EntityRef& entity, bool genNormals)
{
  vtkSmartPointer<vtkPolyData> pd = vtkSmartPointer<vtkPolyData>::New();
  if (!entity.isValid())
  {
    return pd;
  }
  smtk::extension::vtk::io::mesh::ExportVTKData exportVTKData;
  if (entity.dimension() == 3)
  {
    //To preserve the state of the mesh database, we track
    //whether or not a new meshset was created to represent
    //the 3d shell; if it was created, we delete it when we
    //are finished with it.
    bool created;
    smtk::mesh::MeshSet shell = entity.meshTessellation().extractShell(created);
    exportVTKData(shell, pd);
    if (created)
    {
      entity.meshTessellation().collection()->removeMeshes(shell);
    }
  }
  else
  {
    exportVTKData(entity.meshTessellation(), pd);
  }

  AddColorWithDefault(pd, entity, this->DefaultColor);
  if (this->AllowNormalGeneration && pd->GetPolys()->GetSize() > 0)
  {
    bool reallyNeedNormals = genNormals;
    if (entity.hasIntegerProperty("generate normals"))
    { // Allow per-entity setting to override per-model setting
      const IntegerList& prop(entity.integerProperty("generate normals"));
      reallyNeedNormals = (!prop.empty() && prop[0]);
    }
    if (reallyNeedNormals)
    {
      this->NormalGenerator->SetInputDataObject(pd);
      this->NormalGenerator->Update();
      pd->ShallowCopy(this->NormalGenerator->GetOutput());
    }
  }

  vtkModelMultiBlockSource::AddPointsAsAttribute(pd);
  return pd;
}

/// Loop over the model generating blocks of polydata.
void vtkModelMultiBlockSource::GenerateRepresentationFromModel(
  vtkPolyData* pd, const smtk::model::EntityRef& entityref, bool genNormals)
{
  vtkNew<vtkPoints> pts;
  pts->SetDataTypeToDouble();
  pd->SetPoints(pts.GetPointer());
  const smtk::model::Tessellation* tess;
  if (!(tess = entityref.hasTessellation()))
  { // Oops.
    return;
  }
  vtkIdType npts = tess->coords().size() / 3;
  pts->Allocate(npts);
  smtk::model::EntityPtr entity;
  if (entityref.isValid(&entity))
  {
    AddEntityTessToPolyData(entityref, pts.GetPointer(), pd, this->ShowAnalysisTessellation);
    AddColorWithDefault(pd, entity, this->DefaultColor);
    if (this->AllowNormalGeneration && pd->GetPolys()->GetSize() > 0)
    {
      bool reallyNeedNormals = genNormals;
      if (entityref.hasIntegerProperty("generate normals"))
      { // Allow per-entity setting to override per-model setting
        const IntegerList& prop(entityref.integerProperty("generate normals"));
        reallyNeedNormals = (!prop.empty() && prop[0]);
      }
      if (reallyNeedNormals)
      {
        this->NormalGenerator->SetInputDataObject(pd);
        this->NormalGenerator->Update();
        pd->ShallowCopy(this->NormalGenerator->GetOutput());
      }
    }

    vtkModelMultiBlockSource::AddPointsAsAttribute(pd);
  }
}

static smtk::model::Volume volumeOfEntity(const smtk::model::EntityRef& ein)
{
  // If we are a instance entity, find the volume owning the prototype (if one exists):
  smtk::model::EntityRef ent(ein.isInstance() ? ein.as<smtk::model::Instance>().prototype() : ein);

  if (!ent.isValid() || ent.isModel() || ent.isGroup() || ent.isConcept())
  {
    return smtk::model::Volume();
  }

  if (ent.isVolume())
  {
    return ent.as<smtk::model::Volume>();
  }

  // From here, we should be able to get to a cell, which may or may not have a parent volume.
  smtk::model::CellEntity cent(ent);
  if (ent.isShellEntity())
  {
    cent = ent.as<smtk::model::ShellEntity>().boundingCell();
  }
  else if (ent.isUseEntity())
  {
    cent = ent.as<smtk::model::UseEntity>().cell();
  }

  while (cent.isValid() && !cent.isVolume())
  {
    EntityRefs bord = cent.bordantEntities();
    if (bord.empty())
    {
      return smtk::model::Volume();
    }

    auto it =
      std::find_if(bord.cbegin(), bord.cend(), [](EntityRef const& tst) { return tst.isVolume(); });
    cent = it == bord.cend() ? bord.cbegin()->as<smtk::model::CellEntity>()
                             : it->as<smtk::model::Volume>();
  }
  return cent.as<smtk::model::Volume>();
}

/// Called by GenerateRepresentationFromModel to sum placements across all instances.
void vtkModelMultiBlockSource::AddInstanceMetadata(vtkIdType& npts,
  smtk::model::InstanceSet& modelInstances, const smtk::model::Instance& inst,
  std::map<smtk::model::EntityRef, vtkIdType>& instancePrototypes)
{
  const smtk::model::Tessellation* tess;
  std::map<smtk::common::UUID, vtkIdType>::iterator it;
  if (!inst.isValid() ||                // Is the instance in the model resource?
    !(tess = inst.hasTessellation()) || // Does it have a tessellation that places instances?
    tess->coords().size() < 3           // Does the tessellation contain at least 1 placement?
    )
  {
    return;
  }
  npts += tess->coords().size() / 3;
  modelInstances.insert(inst);
  instancePrototypes[inst.prototype()] = -1; // We don't have an output block ID yet
}

/// Called by GenerateRepresentationFromModel to create hierarchical
/// multiblock for rendering instance glyphs.
void vtkModelMultiBlockSource::PreparePrototypeOutput(vtkMultiBlockDataSet* mbds,
  vtkMultiBlockDataSet* protoBlocks,
  std::map<smtk::model::EntityRef, vtkIdType>& instancePrototypes)
{
  auto iter = mbds->NewTreeIterator();
  iter->VisitOnlyLeavesOff();
  protoBlocks->SetNumberOfBlocks(static_cast<int>(instancePrototypes.size()));
  vtkIdType nextProtoIndex = 0;
  for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
  {
    smtk::common::UUID uid = this->GetDataObjectUUID(iter->GetCurrentMetaData());
    if (uid)
    {
      smtk::model::EntityRef proto(this->GetModelResource(), uid);
      if (instancePrototypes.find(proto) != instancePrototypes.end())
      {
        protoBlocks->SetBlock(nextProtoIndex, iter->GetCurrentDataObject());
        instancePrototypes[proto] = nextProtoIndex;
        ++nextProtoIndex;
      }
    }
  }
  iter->Delete();
}

/// Called by GenerateRepresentationFromModel to create a polydata per instance
void vtkModelMultiBlockSource::PrepareInstanceOutput(vtkMultiBlockDataSet* instanceBlocks,
  const smtk::model::InstanceSet& modelInstances,
  std::map<smtk::model::EntityRef, vtkIdType>& instancePrototypes)
{
  instanceBlocks->SetNumberOfBlocks(static_cast<int>(modelInstances.size()));
  int block = 0;
  for (auto instance : modelInstances)
  {
    const smtk::model::Tessellation* tess = instance.hasTessellation();
    vtkIdType numPoints = static_cast<vtkIdType>(tess->coords().size() / 3);

    vtkNew<vtkPolyData> instancePoly;
    vtkNew<vtkPoints> instancePts;
    instancePts->Allocate(numPoints);
    instancePoly->SetPoints(instancePts.GetPointer());
    instanceBlocks->SetBlock(block, instancePoly.GetPointer());

    // const smtk::model::EntityRef entRef = instance.prototype();
    this->SetDataObjectUUID(instanceBlocks->GetMetaData(block), instance.entity());
    this->SetDataObjectUUID(instancePoly->GetInformation(), instance.entity());
    block++;

    vtkNew<vtkDoubleArray> instanceOrient;
    vtkNew<vtkDoubleArray> instanceScale;
    vtkNew<vtkIdTypeArray> instancePrototype;  // block ID of prototype object
    vtkNew<vtkUnsignedCharArray> instanceMask; // visibility control

    instanceOrient->SetName(VTK_INSTANCE_ORIENTATION);
    instanceScale->SetName(VTK_INSTANCE_SCALE);
    instancePrototype->SetName(VTK_INSTANCE_SOURCE);
    instanceMask->SetName(VTK_INSTANCE_VISIBILITY);

    instanceOrient->SetNumberOfComponents(3);
    instanceScale->SetNumberOfComponents(3);

    //instanceOrient->SetNumberOfTuples(numPoints);
    //instanceScale->SetNumberOfTuples(numPoints);
    //instancePrototype->SetNumberOfTuples(numPoints);
    //instanceMask->SetNumberOfTuples(numPoints);

    instanceOrient->Allocate(numPoints);
    instanceScale->Allocate(numPoints);
    instancePrototype->Allocate(numPoints);
    instanceMask->Allocate(numPoints);

    // WARNING: Pointdata-array indices are used blindly in AddInstancePoints. Do not reorder:
    auto pd = instancePoly->GetPointData();
    pd->AddArray(instanceOrient.GetPointer());
    pd->AddArray(instanceScale.GetPointer());
    pd->AddArray(instancePrototype.GetPointer());
    pd->AddArray(instanceMask.GetPointer());

    this->AddInstancePoints(instancePoly.GetPointer(), instance, instancePrototypes);
  }
}

/// Called by GenerateRepresentationFromModel to add a glyph point per instance location.
void vtkModelMultiBlockSource::AddInstancePoints(vtkPolyData* instancePoly,
  const smtk::model::Instance& inst,
  std::map<smtk::model::EntityRef, vtkIdType>& instancePrototypes)
{
  EntityRef proto;
  const smtk::model::Tessellation* tess;
  std::map<smtk::model::EntityRef, vtkIdType>::iterator it;
  if (!inst.isValid() ||                    // Is the instance in the model resource?
    !(tess = inst.hasTessellation()) ||     // Does it have a tessellation that places instances?
    tess->coords().size() < 3 ||            // Does the tessellation contain at least 1 placement?
    !((proto = inst.prototype()).isValid()) // Does it have a prototype entity?
    )
  {
    smtkWarningMacro(this->ModelResource->log(), "Instance "
        << inst.entity() << " was invalid, has no tessellation, or has no prototype.");
    return;
  }
  if (((it = instancePrototypes.find(proto)) == instancePrototypes.end()) ||
    it->second < 0 // Does the prototype have a valid block ID (i.e., a tessellation)?
    )
  {
    smtkWarningMacro(this->ModelResource->log(), "Prototype ("
        << proto.name() << ") for instance (" << inst.name() << ") has no VTK dataset");
    return;
  }
  vtkPoints* pts = instancePoly->GetPoints();
  auto pd = instancePoly->GetPointData();
  // WARNING: Array indices are hardcoded here for speed. See PrepareInstanceOutput above.
  auto orient = vtkDoubleArray::SafeDownCast(pd->GetArray(0));
  auto scale = vtkDoubleArray::SafeDownCast(pd->GetArray(1));
  auto prototype = vtkIdTypeArray::SafeDownCast(pd->GetArray(2));
  auto mask = vtkUnsignedCharArray::SafeDownCast(pd->GetArray(3));

  std::vector<double>::const_iterator pit = tess->coords().begin();
  vtkIdType nptsThisInst = static_cast<vtkIdType>(tess->coords().size() / 3);
  double ptOrient[3] = { 0, 0, 0 };
  double ptScale[3] = { 1, 1, 1 };
  for (vtkIdType ii = 0; ii < nptsThisInst; ++ii, pit += 3)
  {
    /*
    std::cout << "Adding " << *pit << " " << *(pit + 1) << " " << *(pit + 2) << " as pt " <<
      pts->GetNumberOfPoints() << " with glyph index " << it->second << "\n";
      */
    pts->InsertNextPoint(*pit, *(pit + 1), *(pit + 2));
    prototype->InsertNextValue(it->second); // block ID
    orient->InsertNextTuple(ptOrient);
    scale->InsertNextTuple(ptScale);
    mask->InsertNextValue(1);
  }
}

/// Create a multiblock with the right structure, find entities with tessellations, and add them.
void vtkModelMultiBlockSource::GenerateRepresentationFromModel(vtkMultiBlockDataSet* mbds,
  vtkMultiBlockDataSet* instanceBlocks, vtkMultiBlockDataSet* protoBlocks,
  smtk::model::ResourcePtr resource)
{
  if (this->ModelEntityID && this->ModelEntityID[0])
  {
    smtk::common::UUID uid(this->ModelEntityID);
    smtk::model::EntityRef entity(resource, uid);
    Model modelEntity = entity.isModel() ? entity.as<smtk::model::Model>() : entity.owningModel();
    if (modelEntity.isValid())
    {
      // See if the model has any instructions about
      // whether to generate surface normals.
      bool modelRequiresNormals = false;
      if (modelEntity.hasIntegerProperty("generate normals"))
      {
        const IntegerList& prop(modelEntity.integerProperty("generate normals"));
        if (!prop.empty() && prop[0])
          modelRequiresNormals = true;
      }

      // TODO: how do we handle submodels in a multiblock dataset? We could have
      //       a cycle in the submodels, so treating them as trees would not work.
      // Finally, if nothing has any tessellation information, see if any is associated
      // with the model itself.

      // Create top-level blocks for model:
      if (1)
      {
        smtk::model::EntityRefArray blockEntities[NUMBER_OF_BLOCK_TYPES];
        std::vector<vtkSmartPointer<vtkDataObject> > blockDatasets[NUMBER_OF_BLOCK_TYPES];
        vtkSmartPointer<vtkMultiBlockDataSet> topBlocks[NUMBER_OF_BLOCK_TYPES];
        mbds->SetNumberOfBlocks(NUMBER_OF_BLOCK_TYPES);
        int bb;
        for (bb = 0; bb < NUMBER_OF_BLOCK_TYPES; ++bb)
        {
          topBlocks[bb] = vtkSmartPointer<vtkMultiBlockDataSet>::New();
          mbds->SetBlock(bb, topBlocks[bb].GetPointer());
        }
        smtk::model::InstanceSet modelInstances;
        // Map from an entity serving as an instance's prototype to its block ID on PROTOTYPE_PORT:
        std::map<smtk::model::EntityRef, vtkIdType> instancePrototypes;
        vtkIdType numInstancePts = 0;
        smtk::model::EntityIterator eit;
        eit.traverse(modelEntity, smtk::model::ITERATE_CHILDREN);
        for (eit.begin(); !eit.isAtEnd(); eit.advance())
        {
          BitFlags etype = eit->entityFlags();
          if (smtk::model::isVertex(etype))
          {
            bb = VERTICES;
          }
          else if (smtk::model::isEdge(etype))
          {
            bb = EDGES;
          }
          else if (smtk::model::isFace(etype))
          {
            bb = FACES;
          }
          else if (smtk::model::isVolume(etype))
          {
            bb = VOLUMES;
          }
          else if (smtk::model::isGroup(etype))
          {
            bb = GROUPS;
          }
          else if (smtk::model::isInstance(etype))
          {
            // Instances are special and do not get added to the multiblock.
            // Remember for later (once we have UUID2BlockIdMap) and skip for now:
            this->AddInstanceMetadata(
              numInstancePts, modelInstances, eit->as<smtk::model::Instance>(), instancePrototypes);
            continue;
          }
          else if (smtk::model::isAuxiliaryGeometry(etype))
          {
            switch (etype & smtk::model::ANY_DIMENSION)
            {
              case DIMENSION_0:
                bb = AUXILIARY_POINTS;
                break;
              case DIMENSION_1:
                bb = AUXILIARY_CURVES;
                break;
              case DIMENSION_2:
                bb = AUXILIARY_SURFACES;
                break;
              case DIMENSION_3:
                bb = AUXILIARY_VOLUMES;
                break;
              default:
                bb = AUXILIARY_MIXED;
                break;
            }
          }
          else if (smtk::model::isModel(etype) && !eit->hasTessellation())
          {
            continue; // silently ignore models without tessellation
          }
          else
          { // skip anything not listed above... we don't know where to put it.
            if (eit->hasTessellation())
            {
              smtkWarningMacro(resource->log(), "MultiBlockDataSet: Entity \""
                  << eit->name() << "\" (" << eit->flagSummary() << ")"
                  << " had a tessellation but was skipped because we don't know what block to put "
                     "it in.");
            }
            continue;
          }

          vtkSmartPointer<vtkDataObject> data =
            this->GenerateRepresentationFromModel(*eit, modelRequiresNormals);
          if (data.GetPointer())
          {
            blockDatasets[bb].push_back(data);
            blockEntities[bb].push_back(*eit);
          }
        }
        // We have all the output, now set up the level-2 multiblock datasets.
        for (bb = 0; bb < NUMBER_OF_BLOCK_TYPES; ++bb)
        {
          int nlb = static_cast<int>(blockDatasets[bb].size());
          topBlocks[bb]->SetNumberOfBlocks(nlb);
          if (nlb == 0)
          {
            mbds->SetBlock(bb, NULL);
          }
          for (int lb = 0; lb < nlb; ++lb)
          {
            topBlocks[bb]->SetBlock(lb, blockDatasets[bb][lb].GetPointer());
            topBlocks[bb]->GetMetaData(lb)->Set(
              vtkCompositeDataSet::NAME(), blockEntities[bb][lb].name().c_str());
            this->SetDataObjectUUID(topBlocks[bb]->GetMetaData(lb), blockEntities[bb][lb].entity());
            this->SetDataObjectUUID(
              blockDatasets[bb][lb]->GetInformation(), blockEntities[bb][lb].entity());
          }
        }

        // Now all blocks are set on mbds, so we can get the flat index of each block.
        // Annotate each block with its flat index.
        vtkDataObjectTreeIterator* miter = mbds->NewTreeIterator();
        miter->VisitOnlyLeavesOff();
        for (miter->GoToFirstItem(); !miter->IsDoneWithTraversal(); miter->GoToNextItem())
        {
          if (!miter->HasCurrentMetaData())
            continue;
          smtk::model::EntityRef ent = this->GetDataObjectEntityAs<smtk::model::EntityRef>(
            resource, miter->GetCurrentMetaData());
          if (ent.isValid())
          {
            addBlockInfo(resource, ent, volumeOfEntity(ent), miter->GetCurrentFlatIndex(),
              miter->GetCurrentDataObject(), this->UUID2BlockIdMap);
          }
        }
        miter->Delete();

        // Now that we have the UUID2BlockIdMap constructed, we can iterate
        // over our instances and add points for them to the polydata.
        this->PreparePrototypeOutput(mbds, protoBlocks, instancePrototypes);
        // Now that we have the UUID2BlockIdMap constructed, we can iterate
        // over our instances and add points for them to the polydata.
        this->PrepareInstanceOutput(instanceBlocks, modelInstances, instancePrototypes);
      }
    }
    else
    {
      vtkGenericWarningMacro(<< "Can not find the model entity with UUID: " << this->ModelEntityID);
    }
  }
  else
  {

    mbds->SetNumberOfBlocks(static_cast<unsigned>(resource->tessellations().size()));
    vtkIdType i;
    smtk::model::UUIDWithTessellation it;
    for (i = 0, it = resource->tessellations().begin(); it != resource->tessellations().end();
         ++it, ++i)
    {
      vtkNew<vtkPolyData> poly;
      mbds->SetBlock(i, poly.GetPointer());
      smtk::model::EntityRef entityref(resource, it->first);
      // Set the block name to the entity UUID.
      mbds->GetMetaData(i)->Set(vtkCompositeDataSet::NAME(), entityref.name().c_str());
      this->SetDataObjectUUID(mbds->GetMetaData(i), entityref.entity());
      this->SetDataObjectUUID(poly->GetInformation(), entityref.entity());
      //mbds->GetMetaData(i)->Set(vtkModelMultiBlockSource::ENTITYID(), entityref.entity().toString().c_str());
      this->GenerateRepresentationFromModel(
        poly.GetPointer(), entityref, this->AllowNormalGeneration != 0);

      // as a convenient method to get the flat block_index in multiblock
      addBlockInfo(
        resource, entityref, smtk::model::EntityRef(), i, poly.GetPointer(), this->UUID2BlockIdMap);
    }
  }
}

/// Generate polydata from an smtk::model with tessellation information.
int vtkModelMultiBlockSource::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inInfo), vtkInformationVector* outInfo)
{
  this->UUID2BlockIdMap.clear();
  auto output = vtkMultiBlockDataSet::GetData(outInfo, static_cast<int>(MODEL_ENTITY_PORT));
  auto instanceSource = vtkMultiBlockDataSet::GetData(outInfo, static_cast<int>(PROTOTYPE_PORT));
  if (!output || !instanceSource)
  {
    vtkErrorMacro("No output dataset");
    return 0;
  }
  auto instancePlacement = vtkMultiBlockDataSet::GetData(outInfo, static_cast<int>(INSTANCE_PORT));
  if (!instancePlacement)
  {
    vtkErrorMacro("No output instance-placement dataset");
    return 0;
  }

  if (!this->ModelResource)
  {
    vtkErrorMacro("No input model");
    return 0;
  }

  // Destroy the cache if the parameters have changed since it was generated.
  if (this->CachedOutputMBDS && this->GetMTime() > this->CachedOutputMBDS->GetMTime())
    this->SetCachedOutput(nullptr, nullptr, nullptr);

  if (!this->CachedOutputMBDS)
  { // Populate a polydata with tessellation information from the model.
    vtkNew<vtkMultiBlockDataSet> rep;
    vtkNew<vtkMultiBlockDataSet> proto;
    vtkNew<vtkMultiBlockDataSet> inst;
    this->GenerateRepresentationFromModel(
      rep.GetPointer(), inst.GetPointer(), proto.GetPointer(), this->ModelResource);
    this->SetCachedOutput(rep.GetPointer(), inst.GetPointer(), proto.GetPointer());
  }
  output->ShallowCopy(this->CachedOutputMBDS);
  instancePlacement->ShallowCopy(this->CachedOutputInst);
  instanceSource->ShallowCopy(this->CachedOutputProto);
  return 1;
}

void vtkModelMultiBlockSource::SetCachedOutput(vtkMultiBlockDataSet* entityTess,
  vtkMultiBlockDataSet* instances, vtkMultiBlockDataSet* protoBlocks)
{
  if (this->CachedOutputMBDS == entityTess && this->CachedOutputInst == instances &&
    this->CachedOutputProto == protoBlocks)
  {
    return;
  }
  if (this->CachedOutputMBDS && this->CachedOutputMBDS != entityTess)
  {
    this->CachedOutputMBDS->Delete();
  }
  if (this->CachedOutputInst && this->CachedOutputInst != instances)
  {
    this->CachedOutputInst->Delete();
  }
  if (this->CachedOutputProto && this->CachedOutputProto != protoBlocks)
  {
    this->CachedOutputProto->Delete();
  }
  this->CachedOutputMBDS = entityTess;
  this->CachedOutputInst = instances;
  this->CachedOutputProto = protoBlocks;
  if (this->CachedOutputMBDS)
  {
    this->CachedOutputMBDS->Register(this);
  }
  if (this->CachedOutputInst)
  {
    this->CachedOutputInst->Register(this);
  }
  if (this->CachedOutputProto)
  {
    this->CachedOutputProto->Register(this);
  }
  this->Modified();
}

void vtkModelMultiBlockSource::AddPointsAsAttribute(vtkPolyData* data)
{
  // If a data read prior to this call failed, the data passed to this method
  // may be incomplete. So, we guard against bad data to prevent the subsequent
  // logic from causing the program to crash.
  if (data == nullptr || data->GetNumberOfPoints() == 0)
  {
    return;
  }

  vtkNew<vtkDoubleArray> pointCoords;
  pointCoords->ShallowCopy(data->GetPoints()->GetData());
  pointCoords->SetName("PointCoordinates");
  data->GetPointData()->AddArray(pointCoords.GetPointer());
}
