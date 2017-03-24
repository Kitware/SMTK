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

#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Edge.h"
#include "smtk/model/EdgeUse.h"
#include "smtk/model/EntityIterator.h"
#include "smtk/model/Face.h"
#include "smtk/model/FaceUse.h"
#include "smtk/model/Group.h"
#include "smtk/model/Instance.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/ShellEntity.h"
#include "smtk/model/Tessellation.h"
#include "smtk/model/UseEntity.h"
#include "smtk/model/Volume.h"
#include "smtk/extension/vtk/filter/vtkImageSpacingFlip.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkDataObjectTreeIterator.h"
#include "vtkGDALRasterReader.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationStringKey.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkNew.h"
#include "vtkOBJReader.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkPoints.h"
#include "vtkProperty.h"
#include "vtkStringArray.h"
#include "vtkPTSReader.h"
#include "vtkPolyDataNormals.h"
#include "vtkPolyData.h"
#include "vtkUnstructuredGrid.h"
#include "vtkImageData.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkXMLPolyDataReader.h"
#include "vtkXMLUnstructuredGridReader.h"
#include "vtkXMLImageDataReader.h"
#include "vtkXMLMultiBlockDataReader.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "boost/filesystem.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

#include <inttypes.h>
#include <stdlib.h>
#include <errno.h>

using namespace smtk::model;

vtkInformationKeyMacro(vtkModelMultiBlockSource, ENTITYID, String);
vtkStandardNewMacro(vtkModelMultiBlockSource);
vtkCxxSetObjectMacro(vtkModelMultiBlockSource,CachedOutput,vtkMultiBlockDataSet);

smtk::common::UUIDGenerator vtkModelMultiBlockSource::UUIDGenerator;

vtkModelMultiBlockSource::vtkModelMultiBlockSource()
{
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(2);
  this->CachedOutput = NULL;
  for (int i = 0; i < 4; ++i)
    {
    this->DefaultColor[i] = 1.;
    }
  this->ModelEntityID = NULL;
  this->AllowNormalGeneration = 0;
  this->ShowAnalysisTessellation = 0;
}

vtkModelMultiBlockSource::~vtkModelMultiBlockSource()
{
  this->SetCachedOutput(NULL);
  this->SetModelEntityID(NULL);
}

void vtkModelMultiBlockSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Model: " << this->ModelMgr.get() << "\n";
  os << indent << "CachedOutput: " << this->CachedOutput << "\n";
  os << indent << "ModelEntityID: " << this->ModelEntityID << "\n";
  os << indent << "AllowNormalGeneration: " << (this->AllowNormalGeneration ? "ON" : "OFF") << "\n";
  os << indent << "ShowAnalysisTessellation: " << this->ShowAnalysisTessellation << "\n";
}

/**\brief For Python users, this method is the only way to bridge VTK and SMTK wrappings.
  *
  */
void vtkModelMultiBlockSource::SetModelManager(const char* pointerAsString)
{
  bool valid = true;
  vtkTypeUInt64 ptrInt;
  if (!pointerAsString || !pointerAsString[0])
    {
    valid = false;
    }
  else
    {
    int base = 16;
    if (pointerAsString[0] == '0' && pointerAsString[1] == 'x')
      {
      pointerAsString += 2;
      }
    char* endPtr;
    ptrInt = strtoll(pointerAsString, &endPtr, base);
    if (ptrInt == 0 && errno)
      valid = false;
    }
  if (valid)
    {
    if (ptrInt)
      {
#ifndef _MSC_VER
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored"-Wstrict-aliasing"
#endif
      Manager* direct =  *(reinterpret_cast<Manager**>(&ptrInt));
#ifndef _MSC_VER
#  pragma GCC diagnostic pop
#endif
      this->SetModelManager(direct->shared_from_this());
      }
    else
      {
      // Set to "NULL"
      vtkWarningMacro("Setting model manager to NULL");
      this->SetModelManager(ManagerPtr());
      }
    }
  else
    {
    vtkWarningMacro("Not setting model manager, errno = " << errno);
    }
}

/// Set the SMTK model to be displayed.
void vtkModelMultiBlockSource::SetModelManager(smtk::model::ManagerPtr model)
{
  if (this->ModelMgr == model)
    {
    return;
    }
  this->ModelMgr = model;
  this->Modified();
}

/// Get the SMTK model being displayed.
smtk::model::ManagerPtr vtkModelMultiBlockSource::GetModelManager()
{
  return this->ModelMgr;
}

/// Get the map from model entity UUID to the block index in multiblock output
void vtkModelMultiBlockSource::GetUUID2BlockIdMap(std::map<smtk::common::UUID, unsigned int>& uuid2mid)
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
  this->SetCachedOutput(NULL);
}

/**\brief Return a UUID for the data object, adding one if it was not present.
  *
  * UUIDs are stored in the vtkInformation object associated with each
  * data object.
  */
smtk::common::UUID vtkModelMultiBlockSource::GetDataObjectUUID(vtkInformation* datainfo)
{
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

static void AddEntityTessToPolyData(
  const smtk::model::EntityRef& entityref, vtkPoints* pts, vtkPolyData* pd,
  int showAnalysisTessellation)
{
  // gotMesh fetches Analysis mesh if it exists, falling back
  // to model tessellation if not.
  const smtk::model::Tessellation* tess = showAnalysisTessellation ?
    entityref.hasAnalysisMesh() :
    entityref.hasTessellation();
  if (!tess)
    return;

  vtkIdType i;
  //vtkIdType connOffset = pts->GetNumberOfPoints();
  std::vector<vtkIdType> conn;
  std::string uuidStr = entityref.entity().toString();
  vtkIdType npts = tess->coords().size() / 3;
  for (i = 0; i < npts; ++i)
    {
    pts->InsertNextPoint(&tess->coords()[3*i]);
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
    case TESS_VERTEX:          have_verts = true; verts->InsertNextCell(1, &vtk_conn[0]); break;
    case TESS_TRIANGLE:        have_polys = true; polys->InsertNextCell(3, &vtk_conn[0]); break;
    case TESS_QUAD:            have_polys = true; polys->InsertNextCell(4, &vtk_conn[0]); break;
    case TESS_POLYVERTEX:      have_verts = true; verts->InsertNextCell(num_verts, &vtk_conn[0]); break;
    case TESS_POLYLINE:        have_lines = true; lines->InsertNextCell(num_verts, &vtk_conn[0]); break;
    case TESS_POLYGON:         have_polys = true; polys->InsertNextCell(num_verts, &vtk_conn[0]); break;
    case TESS_TRIANGLE_STRIP:  have_strip = true; strip->InsertNextCell(num_verts, &vtk_conn[0]); break;
    default:
      std::cerr << "Invalid cell shape " << cell_shape << " at offset " << off << ". Skipping.\n";
      continue;
      break;
      }
    }
  if (have_verts) pd->SetVerts(verts.GetPointer());
  if (have_lines) pd->SetLines(lines.GetPointer());
  if (have_polys) pd->SetPolys(polys.GetPointer());
  if (have_strip) pd->SetStrips(strip.GetPointer());
}

/// Add customized block info.
/// Mapping from UUID to block id
/// 'Volume' field array to color by volume
static void addBlockInfo(
  smtk::model::ManagerPtr manager,
  const smtk::model::EntityRef& entityref,
  const smtk::model::EntityRef& bordantCell,
  const vtkIdType& blockId,
  vtkDataObject* dobj,
  std::map<smtk::common::UUID, unsigned int>& uuid2BlockId)
{
  manager->setIntegerProperty(entityref.entity(), "block_index", blockId);
  uuid2BlockId[entityref.entity()] = static_cast<unsigned int>(blockId);

  // Add Entity UUID to fieldData
  vtkNew<vtkStringArray> uuidArray;
  uuidArray->SetNumberOfComponents(1);
  uuidArray->SetNumberOfTuples(1);
  uuidArray->SetName(vtkModelMultiBlockSource::GetEntityTagName());
  uuidArray->SetValue(0, entityref.entity().toString());
  dobj->GetFieldData()->AddArray(uuidArray.GetPointer());

  smtk::model::EntityRefs vols;
  if(bordantCell.isValid() && bordantCell.isVolume())
    vols.insert(bordantCell);
 if(vols.size())
    {
    // Add volume UUID to fieldData
    vtkNew<vtkStringArray> volArray;
    volArray->SetNumberOfComponents(1);
    volArray->SetNumberOfTuples(vols.size());
    int ai = 0;
    for (smtk::model::EntityRefs::iterator it = vols.begin();
         it != vols.end(); ++it, ++ai)
      {
      volArray->SetValue(ai, (*it).entity().toString());
      }
    volArray->SetName(vtkModelMultiBlockSource::GetVolumeTagName());
    dobj->GetFieldData()->AddArray(volArray.GetPointer());
    }
}

/// Generate a data object representing the entity. It may be polydata, image data, or a multiblock dataset.
vtkSmartPointer<vtkDataObject> vtkModelMultiBlockSource::GenerateRepresentationFromModel(
  const smtk::model::EntityRef& entity,
  bool genNormals)
{
  const smtk::model::Tessellation* tess;
  if ((tess = entity.hasTessellation()))
    {
    return this->GenerateRepresentationFromTessellation(entity, tess, genNormals);
    }
  smtk::model::AuxiliaryGeometry aux(entity);
  std::string url;
  if (aux.isValid() && !(url = aux.url()).empty())
    {
    bool bShow = true;
    if ( entity.hasIntegerProperty("display as separate representation"))
      {
      const IntegerList& prop(entity.integerProperty(
          "display as separate representation"));
      // show this block if the property is not specified or equals to zero
      bShow = (prop.empty() || prop[0] == 0);
      }
    if(bShow)
      {
      return this->GenerateRepresentationFromURL(aux, genNormals);
      }
    }
  return vtkSmartPointer<vtkDataObject>();
}

vtkSmartPointer<vtkPolyData> vtkModelMultiBlockSource::GenerateRepresentationFromTessellation(
  const smtk::model::EntityRef& entity,
  const smtk::model::Tessellation* tess,
  bool genNormals)
{
  vtkSmartPointer<vtkPolyData> pd = vtkSmartPointer<vtkPolyData>::New();
  vtkNew<vtkPoints> pts;
  pts->SetDataTypeToDouble();
  pd->SetPoints(pts.GetPointer());

  vtkIdType npts = tess->coords().size() / 3;
  pts->Allocate(npts);
  smtk::model::Entity* entrec;
  if (entity.isValid(&entrec))
    {
    AddEntityTessToPolyData(
      entity, pts.GetPointer(), pd, this->ShowAnalysisTessellation);
    // Only create the color array if there is a valid default:
    if (this->DefaultColor[3] >= 0.)
      {
      FloatList rgba = entity.color();
      vtkNew<vtkUnsignedCharArray> cellColor;
      cellColor->SetNumberOfComponents(4);
      cellColor->SetNumberOfTuples(pd->GetNumberOfCells());
      cellColor->SetName("Entity Color");
      for (int i = 0; i < 4; ++i)
        {
        cellColor->FillComponent(i,
          (rgba[3] >= 0 ? rgba[i] : this->DefaultColor[i])* 255.);
        }
      pd->GetCellData()->AddArray(cellColor.GetPointer());
      pd->GetCellData()->SetScalars(cellColor.GetPointer());
      }
    if (this->AllowNormalGeneration && pd->GetPolys()->GetSize() > 0)
      {
      bool reallyNeedNormals = genNormals;
      if ( entity.hasIntegerProperty("generate normals"))
        { // Allow per-entity setting to override per-model setting
        const IntegerList& prop(
          entity.integerProperty(
            "generate normals"));
        reallyNeedNormals = (!prop.empty() && prop[0]);
        }
      if (reallyNeedNormals)
        {
        this->NormalGenerator->SetInputDataObject(pd);
        this->NormalGenerator->Update();
        pd->ShallowCopy(this->NormalGenerator->GetOutput());
        }
      }
    }
  return pd;
}

template<typename T, typename U>
vtkSmartPointer<T> ReadData(const smtk::model::AuxiliaryGeometry& auxGeom)
{
  vtkNew<U> rdr;
  rdr->SetFileName(auxGeom.url().c_str());
  rdr->Update();
  vtkSmartPointer<T> data = vtkSmartPointer<T>::New();
  data->ShallowCopy(rdr->GetOutput());
  return data;
}

std::string vtkModelMultiBlockSource::GetAuxiliaryFileType(
  const smtk::model::AuxiliaryGeometry& auxGeom)
{
  std::string fileType;
  if (auxGeom.hasStringProperty("type"))
    {
    const StringList& prop(auxGeom.stringProperty("type"));
    if (!prop.empty())
      {
      fileType = prop[0];
      }
    }
  if (fileType.empty())
    {
    fileType = vtkModelMultiBlockSource::InferFileTypeFromFileName(auxGeom.url());
    }
  return fileType;
}

/// Create a reader and copy its output into a new data object to serve as the representation for auxiliary geometry.
vtkSmartPointer<vtkDataObject> vtkModelMultiBlockSource::GenerateRepresentationFromURL(
  const smtk::model::AuxiliaryGeometry& auxGeom,
  bool genNormals)
{
  (void)genNormals;
  smtkDebugMacro(auxGeom.manager()->log(),
    "Need to load " << auxGeom.url() << " for " << auxGeom.name());
  std::string fileType = vtkModelMultiBlockSource::GetAuxiliaryFileType(
                       auxGeom);
  if (fileType == "vtp") { return ReadData<vtkPolyData, vtkXMLPolyDataReader>(auxGeom); }
  else if (fileType == "vtu") { return ReadData<vtkUnstructuredGrid, vtkXMLUnstructuredGridReader>(auxGeom); }
  else if (fileType == "vti") { return ReadData<vtkImageData, vtkXMLImageDataReader>(auxGeom); }
  else if (fileType == "vtm") { return ReadData<vtkMultiBlockDataSet, vtkXMLMultiBlockDataReader>(auxGeom); }
  else if (fileType == "obj") { return ReadData<vtkPolyData, vtkOBJReader>(auxGeom); }
  else if (fileType == "pts") { return ReadData<vtkPolyData, vtkPTSReader>(auxGeom); }
  else if (fileType == "dem" || fileType == "tif" || fileType == "tiff")
  {
    vtkSmartPointer<vtkImageData> outImage = ReadData<vtkImageData, vtkGDALRasterReader>(auxGeom);
    if(outImage.GetPointer())
    {
      vtkNew<vtkImageSpacingFlip> flipImage;
      flipImage->SetInputData(outImage);
      flipImage->Update();
      vtkSmartPointer<vtkImageData> data = vtkSmartPointer<vtkImageData>::New();
      data->ShallowCopy(flipImage->GetOutput());
      return data;
    }   
  }

  return vtkSmartPointer<vtkDataObject>();
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
  smtk::model::Entity* entity;
  if (entityref.isValid(&entity))
    {
    AddEntityTessToPolyData(
      entityref, pts.GetPointer(), pd, this->ShowAnalysisTessellation);
    // Only create the color array if there is a valid default:
    if (this->DefaultColor[3] >= 0.)
      {
      FloatList rgba = entityref.color();
      vtkNew<vtkUnsignedCharArray> cellColor;
      cellColor->SetNumberOfComponents(4);
      cellColor->SetNumberOfTuples(pd->GetNumberOfCells());
      cellColor->SetName("Entity Color");
      for (int i = 0; i < 4; ++i)
        {
        cellColor->FillComponent(i,
          (rgba[3] >= 0 ? rgba[i] : this->DefaultColor[i])* 255.);
        }
      pd->GetCellData()->AddArray(cellColor.GetPointer());
      pd->GetCellData()->SetScalars(cellColor.GetPointer());
      }
    if (this->AllowNormalGeneration && pd->GetPolys()->GetSize() > 0)
      {
      bool reallyNeedNormals = genNormals;
      if ( entityref.hasIntegerProperty("generate normals"))
        { // Allow per-entity setting to override per-model setting
        const IntegerList& prop(
          entityref.integerProperty(
            "generate normals"));
        reallyNeedNormals = (!prop.empty() && prop[0]);
        }
      if (reallyNeedNormals)
        {
        this->NormalGenerator->SetInputDataObject(pd);
        this->NormalGenerator->Update();
        pd->ShallowCopy(this->NormalGenerator->GetOutput());
        }
      }
    }
}

static smtk::model::Volume volumeOfEntity(const smtk::model::EntityRef& ein)
{
  // If we are a instance entity, find the volume owning the prototype (if one exists):
  smtk::model::EntityRef ent(
    ein.isInstance() ?
      ein.as<smtk::model::Instance>().prototype() :
      ein);

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
    cent = bord.begin()->as<smtk::model::CellEntity>();
    }
  return cent.as<smtk::model::Volume>();
}

/// Do the actual work of grabbing primitives from the model.
void vtkModelMultiBlockSource::GenerateRepresentationFromModel(
  vtkMultiBlockDataSet* mbds, smtk::model::ManagerPtr manager)
{
  if (this->ModelEntityID && this->ModelEntityID[0])
    {
    smtk::common::UUID uid(this->ModelEntityID);
    smtk::model::EntityRef entity(manager, uid);
    Model modelEntity = entity.isModel() ?
      entity.as<smtk::model::Model>() : entity.owningModel();
    if (modelEntity.isValid())
      {
      // See if the model has any instructions about
      // whether to generate surface normals.
      bool modelRequiresNormals = false;
      if ( modelEntity.hasIntegerProperty("generate normals"))
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
          else if (smtk::model::isAuxiliaryGeometry(etype))
            {
            switch (etype & smtk::model::ANY_DIMENSION)
              {
            case DIMENSION_0: bb = AUXILIARY_POINTS; break;
            case DIMENSION_1: bb = AUXILIARY_CURVES; break;
            case DIMENSION_2: bb = AUXILIARY_SURFACES; break;
            case DIMENSION_3: bb = AUXILIARY_VOLUMES; break;
            default: bb = AUXILIARY_MIXED; break;
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
              smtkWarningMacro(manager->log(),
                "MultiBlockDataSet: Entity \"" << eit->name() << "\" (" << eit->flagSummary() << ")" <<
                " had a tessellation but was skipped because we don't know what block to put it in.");
              }
            continue;
            }

          vtkSmartPointer<vtkDataObject> data =
            this->GenerateRepresentationFromModel(
              *eit, modelRequiresNormals);
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
            topBlocks[bb]->GetMetaData(lb)->Set(vtkCompositeDataSet::NAME(), blockEntities[bb][lb].name().c_str());
            topBlocks[bb]->GetMetaData(lb)->Set(vtkModelMultiBlockSource::ENTITYID(),
              blockEntities[bb][lb].entity().toString().c_str());
            /*
            topBlocks[bb]->GetMetaData(lb)->Set(
              vtkModelMultiBlockSource::ENTITYID(), blockEntities[lb].entity().toString().c_str());
              */
            }
          }

        // Now all blocks are set on mbds, so we can get the flat index of each block.
        // Annotate each block with its flat index.
        vtkDataObjectTreeIterator* miter = mbds->NewTreeIterator();
        for (miter->GoToFirstItem(); !miter->IsDoneWithTraversal(); miter->GoToNextItem())
          {
          if(!miter->HasCurrentMetaData())
            continue;
          smtk::model::EntityRef ent = this->GetDataObjectEntityAs<smtk::model::EntityRef>(
            manager, miter->GetCurrentMetaData());
          if (ent.isValid())
            {
            addBlockInfo(
              manager,
              ent,
              volumeOfEntity(ent),
              miter->GetCurrentFlatIndex(),
              miter->GetCurrentDataObject(),
              this->UUID2BlockIdMap);
            }
          }
        miter->Delete();
        }
      }
    else
      {
      vtkGenericWarningMacro(<< "Can not find the model entity with UUID: " << this->ModelEntityID);
      }
    }
  else
    {
    vtkGenericWarningMacro(<< "A valid model entity id was not set, so all tessellations are used.");

    mbds->SetNumberOfBlocks(static_cast<unsigned>(manager->tessellations().size()));
    vtkIdType i;
    smtk::model::UUIDWithTessellation it;
    for (i = 0, it = manager->tessellations().begin(); it != manager->tessellations().end(); ++it, ++i)
      {
      vtkNew<vtkPolyData> poly;
      mbds->SetBlock(i, poly.GetPointer());
      smtk::model::EntityRef entityref(manager, it->first);
      // Set the block name to the entity UUID.
      mbds->GetMetaData(i)->Set(vtkCompositeDataSet::NAME(), entityref.name().c_str());
      mbds->GetMetaData(i)->Set(vtkModelMultiBlockSource::ENTITYID(), entityref.entity().toString().c_str());
      this->GenerateRepresentationFromModel(poly.GetPointer(), entityref, this->AllowNormalGeneration != 0);

      // as a convenient method to get the flat block_index in multiblock
      addBlockInfo(manager, entityref, smtk::model::EntityRef(), i, poly.GetPointer(), this->UUID2BlockIdMap);
      }
    }
}

std::string vtkModelMultiBlockSource::InferFileTypeFromFileName(const std::string& fname)
{
  ::boost::filesystem::path fp(fname);
  return fp.extension().string().substr(1);
}

/// Generate polydata from an smtk::model with tessellation information.
int vtkModelMultiBlockSource::RequestData(
  vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inInfo),
  vtkInformationVector* outInfo)
{
  this->UUID2BlockIdMap.clear();
  vtkMultiBlockDataSet* output = vtkMultiBlockDataSet::GetData(outInfo, 0);
  if (!output)
    {
    vtkErrorMacro("No output dataset");
    return 0;
    }

  if (!this->ModelMgr)
    {
    vtkErrorMacro("No input model");
    return 0;
    }

  // Destroy the cache if the parameters have changed since it was generated.
  if (this->CachedOutput && this->GetMTime() > this->CachedOutput->GetMTime())
    this->SetCachedOutput(NULL);

  if (!this->CachedOutput)
    { // Populate a polydata with tessellation information from the model.
    vtkNew<vtkMultiBlockDataSet> rep;
    this->GenerateRepresentationFromModel(rep.GetPointer(), this->ModelMgr);
    this->SetCachedOutput(rep.GetPointer());
    }
  output->ShallowCopy(this->CachedOutput);
  return 1;
}
