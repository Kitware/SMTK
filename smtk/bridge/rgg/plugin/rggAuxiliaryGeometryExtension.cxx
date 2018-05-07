//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/rgg/plugin/rggAuxiliaryGeometryExtension.h"
#include "smtk/extension/vtk/source/vtkCmbLayeredConeSource.h"
#include "smtk/extension/vtk/source/vtkModelAuxiliaryGeometry.h"
#include "smtk/extension/vtk/source/vtkModelAuxiliaryGeometry.txx"
#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h"

#include "smtk/AutoInit.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"

#include "smtk/bridge/rgg/operators/CreateDuct.h"
#include "smtk/bridge/rgg/operators/CreatePin.h"

#include "vtkBoundingBox.h"
#include "vtkClipClosedSurface.h"
#include "vtkCompositeDataSet.h"
#include "vtkDataObject.h"
#include "vtkDataObjectTreeIterator.h"
#include "vtkDataSet.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkGraph.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkNew.h"
#include "vtkPlaneCollection.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkPolyDataNormals.h"
#include "vtkProperty.h"
#include "vtkStringArray.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkUnstructuredGrid.h"

#include <ctime>
#include <list>
#include <map>

#include <stdlib.h> // for atexit()

using namespace smtk::model;
using smtk::bridge::rgg::RGGType;

namespace
{
const double cos30 = 0.86602540378443864676372317075294;
static void normalize(double* xyz)
{
  double sum = std::sqrt(xyz[0] * xyz[0] + xyz[1] * xyz[1] + xyz[2] * xyz[2]);
  if (sum == 0)
    return;
  xyz[0] /= sum;
  xyz[1] /= sum;
  xyz[2] /= sum;
}

static void transformNormal(double* xyz, double* xformR)
{
  if (xformR[0] != 0 || xformR[1] != 0 || xformR[2] != 0)
  {
    vtkSmartPointer<vtkTransform> xform = vtkSmartPointer<vtkTransform>::New();
    xform->RotateX(xformR[0]);
    xform->RotateY(xformR[1]);
    xform->RotateZ(xformR[2]);
    double tp[3];
    xform->TransformNormal(xyz, tp);
    xyz[0] = tp[0];
    xyz[1] = tp[1];
    xyz[2] = tp[2];
    normalize(xyz);
  }
}

static void clip(vtkSmartPointer<vtkPolyData> in, vtkSmartPointer<vtkPolyData>& out, double* normal,
  int offset = 0)
{
  vtkSmartPointer<vtkPolyData> tmpIn = in;
  out = vtkSmartPointer<vtkPolyData>::New();
  vtkNew<vtkPlane> plane;
  plane->SetOrigin(
    -normal[0] * 0.005 * offset, -normal[1] * 0.005 * offset, -normal[2] * 0.005 * offset);
  plane->SetNormal(normal[0], normal[1], normal[2]);

  vtkNew<vtkClipClosedSurface> clipper;
  vtkNew<vtkPlaneCollection> clipPlanes;
  vtkNew<vtkPolyDataNormals> normals;
  clipPlanes->AddItem(plane.GetPointer());
  clipper->SetClippingPlanes(clipPlanes.GetPointer());
  clipper->SetActivePlaneId(0);
  clipper->SetClipColor(1.0, 1.0, 1.0);
  clipper->SetActivePlaneColor(1.0, 1.0, 0.8);
  clipper->GenerateOutlineOff();
  clipper->SetInputData(tmpIn);
  clipper->GenerateFacesOn();
  normals->SetInputConnection(clipper->GetOutputPort());
  normals->Update();
  out->DeepCopy(normals->GetOutput());
}
}

rggAuxiliaryGeometryExtension::rggAuxiliaryGeometryExtension()
{
  rggAuxiliaryGeometryExtension::ensureCache();
}

rggAuxiliaryGeometryExtension::~rggAuxiliaryGeometryExtension()
{
}

bool rggAuxiliaryGeometryExtension::canHandleAuxiliaryGeometry(
  smtk::model::AuxiliaryGeometry& entity, std::vector<double>& bboxOut)
{
  if (!entity.isValid() || !entity.hasStringProperty("rggType"))
  {
    return false;
  }

  auto dataset = vtkAuxiliaryGeometryExtension::fetchCachedGeometry(entity);
  if (dataset)
  {
    if (entity.auxiliaryGeometries().size() == 0)
    { // For sub pin part/layer and sub duct part/layer, just update bbox.
      return this->updateBoundsFromDataSet(entity, bboxOut, dataset);
    }
  }

  // No cache entry for the data; we need to create it.
  if (entity.auxiliaryGeometries().empty())
  { // Part/layer's dataset should be created by its parent
    return false;
  }
  // We might add multiple auxgeoms. Don't trim it.
  bool trimCache = false;

  Model parentModel = entity.owningModel();
  bool genNormals = false;
  if (parentModel.hasIntegerProperty("generate normals"))
  {
    const IntegerList& prop(parentModel.integerProperty("generate normals"));
    if (!prop.empty() && prop[0])
    {
      genNormals = true;
    }
  }
  dataset = rggAuxiliaryGeometryExtension::generateRGGRepresentation(entity, genNormals);
  std::time_t mtime;
  std::time(&mtime);
  this->addCacheGeometry(dataset, entity, mtime, trimCache);
  return this->updateBoundsFromDataSet(entity, bboxOut, dataset);
}

vtkSmartPointer<vtkDataObject> rggAuxiliaryGeometryExtension::generateRGGRepresentation(
  const AuxiliaryGeometry& rggEntity, bool genNormals)
{
  rggAuxiliaryGeometryExtension::ensureCache();
  if (rggEntity.stringProperty("rggType")[0] == SMTK_BRIDGE_RGG_PIN)
  {
    if (rggEntity.auxiliaryGeometries().size() == 0)
    {
      smtkErrorMacro(smtk::io::Logger::instance(), "Cannot create a representation for pin "
          << rggEntity.name() << "without any parts or layers. If itself is a part or layer,"
                                 "its parent should generate the rep for it");
      return vtkSmartPointer<vtkDataObject>();
    }
    return rggAuxiliaryGeometryExtension::generateRGGPinRepresentation(rggEntity, genNormals);
  }
  else if (rggEntity.stringProperty("rggType")[0] == SMTK_BRIDGE_RGG_DUCT)
  {
    if (rggEntity.auxiliaryGeometries().size() == 0)
    {
      smtkErrorMacro(smtk::io::Logger::instance(), "Cannot create a representation for duct "
          << rggEntity.name() << "without any segments or layers. If itself is a segment or layer,"
                                 "its parent should generate the rep for it");
      return vtkSmartPointer<vtkDataObject>();
    }
    return rggAuxiliaryGeometryExtension::generateRGGDuctRepresentation(rggEntity, genNormals);
  }
  return vtkSmartPointer<vtkDataObject>();
}

vtkSmartPointer<vtkDataObject> rggAuxiliaryGeometryExtension::generateRGGPinRepresentation(
  const AuxiliaryGeometry& pin, bool genNormals)
{
  rggAuxiliaryGeometryExtension::ensureCache();
  //Extract info from pin
  int materialIndex;
  bool isMaterialSet;
  if (pin.hasIntegerProperty("cell material"))
  {
    materialIndex = pin.integerProperty("cell material")[0];
    isMaterialSet = materialIndex > 0 ? true : false;
  }
  int zOrigin;
  if (pin.hasFloatProperty("z origin"))
  {
    zOrigin = pin.floatProperty("z origin")[0];
  }
  int isHex;
  if (pin.owningModel().hasIntegerProperty("hex"))
  {
    isHex = pin.owningModel().integerProperty("hex")[0];
  }

  int isCutAway;
  if (pin.hasIntegerProperty("cut away"))
  {
    isCutAway = pin.integerProperty("cut away")[0];
  }
  // Pieces
  // Segment type of each piece. 0 means cylinder and 1 means frustum
  smtk::model::IntegerList segTypes;
  if (pin.hasIntegerProperty("pieces"))
  {
    segTypes = pin.integerProperty("pieces");
  }
  smtk::model::FloatList typeParas;
  // For each piece, it would 3 parameters as: length, base radius, top radius,
  if (pin.hasFloatProperty("pieces"))
  {
    typeParas = pin.floatProperty("pieces");
  }
  // Layer materials
  smtk::model::IntegerList subMaterials;
  if (pin.hasIntegerProperty("layer materials"))
  {
    subMaterials = pin.integerProperty("layer materials");
  }
  smtk::model::FloatList radiusNs;
  if (pin.hasFloatProperty("layer materials"))
  {
    radiusNs = pin.floatProperty("layer materials");
  }

  // Follow logic in cmbNucRender::createGeo function. L249
  // Create a name-auxgeom map so that we can assign the right rep
  AuxiliaryGeometries childrenAux = pin.auxiliaryGeometries();
  std::map<std::string, AuxiliaryGeometry*> nameToChildAux;
  for (auto aux : childrenAux)
  {
    nameToChildAux[aux.name()] = &aux;
  }

  size_t numParts = segTypes.size();
  size_t numLayers = subMaterials.size();
  size_t baseCenter(zOrigin);

  // FIXME: Provide a handle for the user to change resolution
  const int PinCellResolution = 20;

  // Assemble all child polydatas into one for the pin
  vtkNew<vtkMultiBlockDataSet> mbds;
  int nblk = static_cast<int>(childrenAux.size());
  mbds->SetNumberOfBlocks(nblk);

  for (size_t j = 0; j < numParts; ++j)
  {
    // TODO: For now I just blindly follow the generation logic in RGG. It's not
    // straightforward and if we have time, we should simplify it.
    /// Create a vtkCmbLayeredConeSource for current part
    vtkSmartPointer<vtkCmbLayeredConeSource> coneSource =
      vtkSmartPointer<vtkCmbLayeredConeSource>::New();
    //  Add a material layer if needed
    coneSource->SetNumberOfLayers(static_cast<int>(numLayers + isMaterialSet));
    double height(typeParas[j * 3]), baseR(typeParas[j * 3 + 1]), topR(typeParas[j * 3 + 2]);
    //    double layer42 = numLayers + isMaterialSet;
    //    std::cout << "Processing part " << j << " with base center as " <<
    //                 baseCenter << " ,height as " << height <<
    //                 " and number of layers to be " << layer42<<
    //                 " baseR=" << baseR << " topR=" << topR<<std::endl;
    // baseCenter would be updated at the end of the loop
    coneSource->SetBaseCenter(0, 0, baseCenter);
    coneSource->SetHeight(height);
    double largestRadius = 0;

    for (size_t k = 0; k < numLayers; k++)
    {
      // Calculate the baseR and topR at current layer
      double baseRL = baseR * radiusNs[k];
      double topRL = topR * radiusNs[k];
      coneSource->SetBaseRadius(static_cast<int>(k), baseRL);
      coneSource->SetTopRadius(static_cast<int>(k), topRL);
      coneSource->SetResolution(static_cast<int>(k), PinCellResolution);
      // Update largest raidus for cell material visulization purprose
      if (largestRadius < baseRL)
      {
        largestRadius = baseRL;
      }
      if (largestRadius < topRL)
      {
        largestRadius = topRL;
      }
    }
    if (isMaterialSet) // We have a valid material assigned( 0 means no material)
    {
      largestRadius *= 2.50;
      double r[] = { largestRadius * 0.5, largestRadius * 0.5 };
      int res = 4;
      if (isHex)
      {
        res = 6;
        r[0] = r[1] = r[0] / cos30;
      }
      coneSource->SetBaseRadius(static_cast<int>(numLayers), r[0], r[1]);
      coneSource->SetTopRadius(static_cast<int>(numLayers), r[0], r[1]);
      coneSource->SetResolution(static_cast<int>(numLayers), res);
    }
    double direction[] = { 0, 0, 1 };
    coneSource->SetDirection(direction);

    coneSource->SetGenerateNormals(genNormals);

    if (coneSource == nullptr)
    {
      continue;
    }
    // Cache child auxgeom(layer and part) with their polydata
    for (size_t k = 0; k < numLayers + isMaterialSet; k++)
    {
      std::string subName = pin.name() + SMTK_BRIDGE_RGG_PIN_SUBPART + std::to_string(j) +
        SMTK_BRIDGE_RGG_PIN_LAYER + std::to_string(k);
      if (isMaterialSet && k == numLayers)
      {
        subName = pin.name() + SMTK_BRIDGE_RGG_PIN_SUBPART + std::to_string(j) +
          SMTK_BRIDGE_RGG_PIN_MATERIAL;
      }
      // Follow logic in L263 cmbNucRender
      vtkSmartPointer<vtkPolyData> dataset = coneSource->CreateUnitLayer(static_cast<int>(k));
      // Since it's a unit layer, proper trasformation should be applied
      vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
      // Translate, rotate then scale
      double xyz[3];
      coneSource->GetBaseCenter(xyz);
      transform->Translate(xyz[0], xyz[1], xyz[2]);

      transform->RotateZ((isHex) ? 30 : 0);
      double angle = (isHex) ? 30 : 0;

      if (segTypes[j] == RGGType::CYLINDER && k == 0)
      { // Cylinder in the 0 layer should be handled differently(Following RGG's logic)
        transform->Scale(coneSource->GetTopRadius(static_cast<int>(k)),
          coneSource->GetBaseRadius(static_cast<int>(k)), height);
      }
      else
      {
        transform->Scale(1, 1, height);
      }
      vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter =
        vtkSmartPointer<vtkTransformPolyDataFilter>::New();
      transformFilter->SetInputData(dataset);
      transformFilter->SetTransform(transform);
      transformFilter->Update();
      vtkSmartPointer<vtkPolyData> transformed = transformFilter->GetOutput();

      // Check cut away flag
      if (isCutAway)
      {
        double normal[] = { 0, 1, 0 };
        double xform[] = { 0, 0, -angle };

        transformNormal(normal, xform);
        clip(transformed, transformed, normal);
      }
      // Find the right auxgeom
      // FIXME: use the nameToChildAux map. For now if I deference the pointer
      // to a const ref, model builder would crash
      std::time_t mtime;
      std::time(&mtime);
      for (const auto& childAux : childrenAux)
      {
        if (childAux.name() == subName)
        {
          vtkAuxiliaryGeometryExtension::addCacheGeometry(transformed, childAux, mtime, false);
        }
      }
      int blockIndex = static_cast<int>(k + numLayers * j);
      vtkSmartPointer<vtkPolyData> pinSubDataset = vtkSmartPointer<vtkPolyData>::New();
      pinSubDataset->DeepCopy(transformed);
      mbds->SetBlock(blockIndex, pinSubDataset);
    }
    // Check if needed to create a boundary layer for the pin
    // Update current baseCenter
    baseCenter += height;
  }
  // Instead of return mbds.GetPointer(), now we do not aggregate the multiblocks
  // on the pin geometry.
  return vtkSmartPointer<vtkDataObject>();
}

vtkSmartPointer<vtkDataObject> rggAuxiliaryGeometryExtension::generateRGGDuctRepresentation(
  const AuxiliaryGeometry& duct, bool /*genNormals*/)
{
  rggAuxiliaryGeometryExtension::ensureCache();
  //Extract info from duct
  bool isHex(false);
  if (duct.owningModel().hasIntegerProperty("hex"))
  {
    isHex = (duct.owningModel().integerProperty("hex")[0] != 0);
  }

  bool isCrossSection(false);
  if (duct.hasIntegerProperty("cross section"))
  {
    isCrossSection = (duct.integerProperty("cross section")[0] != 0);
  }

  smtk::model::FloatList pitch;
  if (duct.owningModel().hasFloatProperty("duct thickness"))
  {
    pitch = duct.owningModel().floatProperty("duct thickness");
  }

  smtk::model::FloatList ductHeight;
  if (duct.hasFloatProperty("duct height"))
  {
    ductHeight = duct.floatProperty("duct height");
  }

  smtk::model::IntegerList numMaterialsPerSeg;
  if (duct.hasIntegerProperty("material nums per segment"))
  {
    numMaterialsPerSeg = duct.integerProperty("material nums per segment");
  }

  smtk::model::FloatList zValues;
  if (duct.hasFloatProperty("z values"))
  {
    zValues = duct.floatProperty("z values");
  }

  smtk::model::IntegerList materials;
  if (duct.hasIntegerProperty("materials"))
  {
    materials = duct.integerProperty("materials");
  }

  smtk::model::FloatList thicknessesN;
  if (duct.hasFloatProperty("thicknesses(normalized)"))
  {
    thicknessesN = duct.floatProperty("thicknesses(normalized)");
  }

  // Follow logic in cmbNucRender::createGeo function. L168
  // Create a name-auxgeom map so that we can assign the right rep
  AuxiliaryGeometries childrenAux = duct.auxiliaryGeometries();
  std::map<std::string, AuxiliaryGeometry*> nameToChildAux;
  for (auto aux : childrenAux)
  {
    nameToChildAux[aux.name()] = &aux;
  }

  // Assemble all child polydatas into one for the pin
  vtkNew<vtkMultiBlockDataSet> mbds;
  int nblk = static_cast<int>(childrenAux.size());
  mbds->SetNumberOfBlocks(nblk);

  size_t numSegs = numMaterialsPerSeg.size();
  size_t thicknessOffset = 0;
  for (size_t i = 0; i < numSegs; i++)
  {
    // Create layer manager
    // TODO: For now I just blindly follow the generation logic in RGG. It's not
    // straightforward and if we have time, we should simplify it.
    double z1 = zValues[2 * i];
    double z2 = zValues[2 * i + 1];
    double height = z2 - z1;
    double deltaZ = height * 0.0005; // Magic number used in rgg
    if (i == 0)
    {
      z1 = z1 + deltaZ;
      // if more than one duct, first duct height need to be reduced by deltaZ
      height = (numSegs > 1) ? height - deltaZ : height - 2 * deltaZ;
    }
    else if (i == (numSegs - 1)) //last duct
    {
      height -= 2 * deltaZ;
    }
    else
    {
      z1 += deltaZ;
    }
    size_t numLayers = numMaterialsPerSeg[i];
    vtkSmartPointer<vtkCmbLayeredConeSource> coneSource =
      vtkSmartPointer<vtkCmbLayeredConeSource>::New();
    coneSource->SetNumberOfLayers(static_cast<int>(numLayers));
    coneSource->SetBaseCenter(0, 0, z1);
    double direction[] = { 0, 0, 1 };
    coneSource->SetDirection(direction);
    coneSource->SetHeight(height);

    int res = 4;
    double mult = 0.5;

    if (isHex)
    {
      res = 6;
      mult = 0.5 / cos30;
    }

    for (size_t k = 0; k < numLayers; k++)
    { // For each layer based on is hex or not,
      // it might have two different thicknesses
      size_t tNIndex = thicknessOffset + k * 2;
      double tx = thicknessesN[tNIndex] * pitch[0] - thicknessesN[tNIndex] * pitch[0] * 0.0005;
      double ty =
        thicknessesN[tNIndex + 1] * pitch[1] - thicknessesN[tNIndex + 1] * pitch[1] * 0.0005;
      coneSource->SetBaseRadius(static_cast<int>(k), tx * mult, ty * mult);
      coneSource->SetTopRadius(static_cast<int>(k), tx * mult, ty * mult);
      coneSource->SetResolution(static_cast<int>(k), res);
    }
    thicknessOffset += numMaterialsPerSeg[i] * 2; // each layer has two thicknesses

    // Cache child auxgeom(layer and part) with their polydata
    for (size_t k = 0; k < numLayers; k++)
    {
      std::string subName = duct.name() + SMTK_BRIDGE_RGG_DUCT_SEGMENT + std::to_string(i) +
        SMTK_BRIDGE_RGG_DUCT_LAYER + std::to_string(k);

      // Follow logic in L168 cmbNucRender
      vtkSmartPointer<vtkPolyData> dataset = coneSource->CreateUnitLayer(static_cast<int>(k));
      // Since it's a unit layer, proper trasformation should be applied
      vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
      // Translate, rotate then scale
      double xyz[3];
      coneSource->GetBaseCenter(xyz);
      transform->Translate(xyz[0], xyz[1], xyz[2]);

      if (k == 0)
      { // Cylinder in the 0 layer should be handled differently(Following RGG's logic)
        transform->Scale(coneSource->GetTopRadius(static_cast<int>(k), 0),
          coneSource->GetBaseRadius(static_cast<int>(k), 1), height);
      }
      else
      {
        transform->Scale(1, 1, height);
      }

      vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter =
        vtkSmartPointer<vtkTransformPolyDataFilter>::New();
      transformFilter->SetInputData(dataset);
      transformFilter->SetTransform(transform);
      transformFilter->Update();
      vtkSmartPointer<vtkPolyData> transformed = transformFilter->GetOutput();

      // Check cut away flag
      if (isCrossSection)
      {
        double normal[] = { 0, 1, 0 };
        clip(transformed, transformed, normal);
      }
      // Find the right auxgeom
      // FIXME: use the nameToChildAux map. For now if I deference the pointer
      // to a const ref, model builder would crash
      std::time_t mtime;
      std::time(&mtime);
      for (const auto& childAux : childrenAux)
      {
        if (childAux.name() == subName)
        {
          vtkAuxiliaryGeometryExtension::addCacheGeometry(transformed, childAux, mtime, false);
        }
      }
      int blockIndex = static_cast<int>(k + thicknessOffset);
      vtkSmartPointer<vtkPolyData> pinSubDataset = vtkSmartPointer<vtkPolyData>::New();
      pinSubDataset->DeepCopy(transformed);
      mbds->SetBlock(blockIndex, pinSubDataset);
    }
  }
  // Instead of return mbds.GetPointer(), now we do not aggregate the multiblocks
  // on the duct geometry.
  return vtkSmartPointer<vtkDataObject>();
}

smtkDeclareExtension(SMTKRGGSESSION_EXPORT, rgg_auxiliary_geometry, rggAuxiliaryGeometryExtension);
smtkComponentInitMacro(smtk_rgg_auxiliary_geometry_extension);
