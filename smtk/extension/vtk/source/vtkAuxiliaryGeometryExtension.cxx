//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/source/vtkAuxiliaryGeometryExtension.h"
#include "smtk/extension/vtk/source/vtkCmbLayeredConeSource.h"
#include "smtk/extension/vtk/source/vtkModelAuxiliaryGeometry.h"
#include "smtk/extension/vtk/source/vtkModelAuxiliaryGeometry.txx"
#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h"

#include "smtk/extension/vtk/io/ReadVTKData.h"

#include "smtk/AutoInit.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"

#include "smtk/bridge/rgg/operators/CreatePin.h"

#include "vtkAppendPoints.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkClipClosedSurface.h"
#include "vtkCompositeDataSet.h"
#include "vtkDataObject.h"
#include "vtkDataObjectTreeIterator.h"
#include "vtkDataSet.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkGDALRasterReader.h"
#include "vtkGraph.h"
#include "vtkIdTypeArray.h"
#include "vtkImageData.h"
#include "vtkImageMapToColors.h"
#include "vtkImageSpacingFlip.h"
#include "vtkInformation.h"
#include "vtkInformationStringKey.h"
#include "vtkInformationVector.h"
#include "vtkLookupTable.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkNew.h"
#include "vtkOBJReader.h"
#include "vtkObjectFactory.h"
#include "vtkPLYReader.h"
#include "vtkPNGReader.h"
#include "vtkPTSReader.h"
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
#include "vtkXMLImageDataReader.h"
#include "vtkXMLMultiBlockDataReader.h"
#include "vtkXMLPolyDataReader.h"
#include "vtkXMLUnstructuredGridReader.h"
#include <vtkPlaneCollection.h>

#include "boost/filesystem.hpp"

#include <ctime>
#include <list>
#include <map>

#include <stdlib.h> // for atexit()

using namespace smtk::model;
using ::boost::filesystem::last_write_time;
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

class vtkAuxiliaryGeometryExtension::ClassInternal
{
public:
  double m_totalSize;
  double m_maxSize;
  enum TupleIndex
  {
    DATA = 0,
    TIMESTAMP = 1
  };
  typedef std::tuple<vtkSmartPointer<vtkDataObject>, std::time_t> CacheValue;
  std::map<AuxiliaryGeometry, CacheValue> m_cache;

  // m_recent is ordered, front to back, from least recent to most recent
  // TODO: Use something more efficient than a list.
  std::list<AuxiliaryGeometry> m_recent;

  /// Bump the entry to the front of the LRU
  void makeEntryMostRecent(const AuxiliaryGeometry& aux)
  {
    auto rit = m_recent.rbegin();
    if (rit != m_recent.rend() && *rit != aux)
    {
      m_recent.remove(aux);
      m_recent.push_back(aux);
    }
  }

  /// Insert (or, with a null vtkSmartPointer, remove) a cache entry associated with \a aux.
  bool insert(const AuxiliaryGeometry& aux, const CacheValue& entry, bool trimCache = true)
  {
    bool hadSomeEffect = false;
    auto dataset = std::get<DATA>(entry);
    auto replace = m_cache.find(aux);
    if (replace != m_cache.end())
    { // Entry replaces an older one for the same entity.
      hadSomeEffect = true;
      m_totalSize -= std::get<DATA>(replace->second)->GetActualMemorySize();
      m_recent.remove(aux);
      if (dataset)
      {
        replace->second = entry;
        m_totalSize += dataset->GetActualMemorySize();
        m_recent.push_back(aux);
      }
    }
    else if (dataset)
    {
      hadSomeEffect = true;
      m_cache.insert(std::make_pair(aux, entry));
      m_totalSize += dataset->GetActualMemorySize();
      m_recent.push_back(aux);
    }
    if (hadSomeEffect && trimCache)
    {
      this->trimCache();
    }
    return hadSomeEffect;
  }

  /**\brief Return the VTK dataset (and its metadata) associated with \a aux.
    *
    * This also marks \a aux as most recently used and thus last to be cleared from the cache.
    */
  CacheValue fetch(const AuxiliaryGeometry& aux)
  {
    // Find in the cache
    auto cit = m_cache.find(aux);
    if (cit == m_cache.end())
    {
      return std::make_tuple(vtkSmartPointer<vtkDataObject>(), std::time_t());
    }
    this->makeEntryMostRecent(aux);
    auto result = cit->second;
    return result;
  }

  /**\brief Remove as many entries from the cache as needed to meet the size requirement.
    *
    * This will never remove the final (back) entry in m_recent, even if that entry
    * alone exceeds the size limit.
    */
  void trimCache()
  {
    if (m_maxSize <= 0)
    {
      return;
    }
    std::list<AuxiliaryGeometry>::iterator entry = m_recent.begin();
    for (; m_recent.size() > 1 && m_totalSize > m_maxSize; entry = m_recent.begin())
    {
      auto cit = m_cache.find(*entry);
      if (cit != m_cache.end())
      {
        m_totalSize -= std::get<DATA>(cit->second)->GetActualMemorySize();
        m_cache.erase(cit);
      }
      m_recent.erase(entry);
    }
  }
};

vtkAuxiliaryGeometryExtension::ClassInternal* vtkAuxiliaryGeometryExtension::s_p = nullptr;

vtkAuxiliaryGeometryExtension::vtkAuxiliaryGeometryExtension()
{
  vtkAuxiliaryGeometryExtension::ensureCache();
}

vtkAuxiliaryGeometryExtension::~vtkAuxiliaryGeometryExtension()
{
}

static bool updateBoundsFromDataSet(smtk::model::AuxiliaryGeometry& aux,
  std::vector<double>& bboxOut, vtkSmartPointer<vtkDataObject> dataobj)
{
  vtkDataSet* dataset;
  vtkGraph* graph;
  vtkCompositeDataSet* tree;
  if ((dataset = dynamic_cast<vtkDataSet*>(dataobj.GetPointer())))
  {
    bboxOut.resize(6);
    dataset->GetBounds(&bboxOut[0]);
    if (bboxOut[0] <= bboxOut[1])
    {
      aux.setBoundingBox(&bboxOut[0]);
    }
    return true;
  }
  else if ((graph = dynamic_cast<vtkGraph*>(dataobj.GetPointer())))
  {
    bboxOut.resize(6);
    dataset->GetBounds(&bboxOut[0]);
    if (bboxOut[0] <= bboxOut[1])
    {
      aux.setBoundingBox(&bboxOut[0]);
    }
    return true;
  }
  else if ((tree = dynamic_cast<vtkCompositeDataSet*>(dataobj.GetPointer())))
  {
    auto it = tree->NewIterator();
    it->SkipEmptyNodesOn();
    vtkBoundingBox bbox;
    bboxOut.resize(6);
    vtkDataSet* dset;
    vtkGraph* grph;
    for (it->InitTraversal(); !it->IsDoneWithTraversal(); it->GoToNextItem())
    {
      vtkDataObject* dobj = it->GetCurrentDataObject();
      if ((dset = dynamic_cast<vtkDataSet*>(dobj)))
      {
        dset->GetBounds(&bboxOut[0]);
        bbox.AddBounds(&bboxOut[0]);
      }
      else if ((grph = dynamic_cast<vtkGraph*>(dobj)))
      {
        grph->GetBounds(&bboxOut[0]);
        bbox.AddBounds(&bboxOut[0]);
      }
    }
    it->Delete();
    if (bbox.IsValid())
    {
      bbox.GetBounds(&bboxOut[0]);
      aux.setBoundingBox(&bboxOut[0]);
    }
    return true;
  }
  return false;
}

bool vtkAuxiliaryGeometryExtension::canHandleAuxiliaryGeometry(
  smtk::model::AuxiliaryGeometry& entity, std::vector<double>& bboxOut)
{
  //  std::cout << "--- check aux: " << entity.name() <<std::endl;
  if (!entity.isValid())
  {
    return false;
  }

  std::string url = entity.url();
  auto tuple = s_p->fetch(entity);
  auto dataset = std::get<ClassInternal::DATA>(tuple);
  std::time_t mtime;
  std::cout << "canHandleAuxiliaryGeometry ? " << entity.name();
  if (dataset)
  {
    // Check timestamp
    if (!url.empty())
    {
      try
      {
        mtime = last_write_time(url);
      }
      catch (...)
      {
        mtime = 0;
      }
      if (std::get<ClassInternal::TIMESTAMP>(tuple) >= mtime)
      {
        return updateBoundsFromDataSet(entity, bboxOut, dataset);
      }
    }
    else
    {
      if (entity.auxiliaryGeometries().size() == 0)
      { // For sub pin part/layer, just update bbox.
        std::cout << "  ->update BBOX" << std::endl;
        return updateBoundsFromDataSet(entity, bboxOut, dataset);
      }
    }
  }
  std::cout << "  ->generate new rep" << std::endl;

  // No cache entry for the data; we need to read it.
  bool trimCache(true);
  if (url.empty())
  {
    if (!entity.hasStringProperty("rggType") && entity.auxiliaryGeometries().empty())
    {
      return false;
    }
    else if (entity.hasStringProperty("rggType"))
    {
      // Currently only rgg session would create auxiliary geometry without url
      // We might add multiple auxgeoms. Don't trim it.
      trimCache = false;
      std::time(&mtime);
    }
    else
    {
      // Currently only rgg session would create auxiliary geometry without url
      // Create cache entry for pin
      std::time(&mtime);
    }
  }
  else
  {
    try
    {
      mtime = last_write_time(url);
    }
    catch (...)
    {
      mtime = 0;
    }
  }

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
  dataset = vtkAuxiliaryGeometryExtension::generateRepresentation(entity, genNormals);
  s_p->insert(entity, ClassInternal::CacheValue(dataset, mtime), trimCache);
  return updateBoundsFromDataSet(entity, bboxOut, dataset);
}

vtkSmartPointer<vtkDataObject> vtkAuxiliaryGeometryExtension::fetchCachedGeometry(
  const smtk::model::AuxiliaryGeometry& entity)
{
  vtkAuxiliaryGeometryExtension::ensureCache();
  return std::get<ClassInternal::DATA>(s_p->fetch(entity));
}

double vtkAuxiliaryGeometryExtension::currentCacheSize()
{
  vtkAuxiliaryGeometryExtension::ensureCache();
  return s_p->m_totalSize;
}

double vtkAuxiliaryGeometryExtension::maximumCacheSize()
{
  vtkAuxiliaryGeometryExtension::ensureCache();
  return s_p->m_maxSize;
}

bool vtkAuxiliaryGeometryExtension::setMaximumCacheSize(double sz)
{
  vtkAuxiliaryGeometryExtension::ensureCache();
  if (sz < 0 || s_p->m_maxSize == sz)
  {
    return false;
  }

  s_p->m_maxSize = sz;
  s_p->trimCache();
  return true;
}

std::string vtkAuxiliaryGeometryExtension::getAuxiliaryFileType(
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
    fileType = vtkAuxiliaryGeometryExtension::inferFileTypeFromFileName(auxGeom.url());
  }
  return fileType;
}

std::string vtkAuxiliaryGeometryExtension::inferFileTypeFromFileName(const std::string& fname)
{
  ::boost::filesystem::path fp(fname);
  return fp.extension().string().substr(1);
}

void vtkAuxiliaryGeometryExtension::ensureCache()
{
  if (!s_p)
  {
    s_p = new vtkAuxiliaryGeometryExtension::ClassInternal;
    atexit(vtkAuxiliaryGeometryExtension::destroyCache);
  }
}

void vtkAuxiliaryGeometryExtension::destroyCache()
{
  delete s_p;
}

vtkSmartPointer<vtkDataObject> vtkAuxiliaryGeometryExtension::generateRepresentation(
  const smtk::model::AuxiliaryGeometry& auxGeom, bool genNormals)
{
  vtkAuxiliaryGeometryExtension::ensureCache();

  if (auxGeom.hasURL())
  {
    return vtkAuxiliaryGeometryExtension::readFromFile(auxGeom, genNormals);
  }

  // If there is no children and type is rggType, get the representation from somewhere
  if (auxGeom.hasStringProperty("rggType"))
  {
    // SMTK should check the pin auxgeom first. It would create corresponding
    // representation for its parts and layers.
    std::cout << " generateRep for aux " << auxGeom.name() << std::endl;
    return vtkAuxiliaryGeometryExtension::generateRGGRepresentation(auxGeom, genNormals);
  }

  AuxiliaryGeometries children(auxGeom.auxiliaryGeometries());
  if (!children.empty())
  {
    return vtkAuxiliaryGeometryExtension::createHierarchy(auxGeom, children, genNormals);
  }

  return vtkSmartPointer<vtkDataObject>();
}

vtkSmartPointer<vtkDataObject> vtkAuxiliaryGeometryExtension::readFromFile(
  const smtk::model::AuxiliaryGeometry& auxGeom, bool genNormals)
{
  vtkAuxiliaryGeometryExtension::ensureCache();

  (void)genNormals;

  std::string fileType = vtkAuxiliaryGeometryExtension::getAuxiliaryFileType(auxGeom);

  smtk::extension::vtk::io::ReadVTKData readVTKData;
  return readVTKData(std::make_pair(fileType, auxGeom.url()));
}

vtkSmartPointer<vtkDataObject> vtkAuxiliaryGeometryExtension::createHierarchy(
  const smtk::model::AuxiliaryGeometry& src, const smtk::model::AuxiliaryGeometries& children,
  bool genNormals)
{
  (void)src;

  vtkAuxiliaryGeometryExtension::ensureCache();
  vtkNew<vtkMultiBlockDataSet> mbds;
  int nblk = static_cast<int>(children.size());
  mbds->SetNumberOfBlocks(nblk);
  auto childIt = children.begin();
  for (int ii = 0; ii < nblk; ++ii, ++childIt)
  {
    mbds->SetBlock(ii, vtkAuxiliaryGeometryExtension::generateRepresentation(*childIt, genNormals));
  }
  return mbds.GetPointer();
}

vtkSmartPointer<vtkDataObject> vtkAuxiliaryGeometryExtension::generateRGGRepresentation(
  const AuxiliaryGeometry& pin, bool genNormals)
{
  vtkAuxiliaryGeometryExtension::ensureCache();
  if (pin.stringProperty("rggType")[0] == SMTK_BRIDGE_RGG_PIN)
  {
    if (pin.auxiliaryGeometries().size() == 0)
    {
      std::cerr << "Cannot create a representation for a pin " << pin.name()
                << "without any parts or layers. If itself is a part or layer,"
                   "its parent should generate the rep for it"
                << std::endl;
      return vtkSmartPointer<vtkDataObject>();
    }
    return vtkAuxiliaryGeometryExtension::generateRGGPinRepresentation(pin, genNormals);
  }
  return vtkSmartPointer<vtkDataObject>();
}

vtkSmartPointer<vtkDataObject> vtkAuxiliaryGeometryExtension::generateRGGPinRepresentation(
  const AuxiliaryGeometry& pin, bool genNormals)
{
  vtkAuxiliaryGeometryExtension::ensureCache();
  std::cout << "generateRGGPinRpresentation:\nChecking input first " << std::endl;
  //Extract info from pin
  int materialIndex;
  bool isMaterialSet;
  if (pin.hasIntegerProperty("cell material"))
  {
    materialIndex = pin.integerProperty("cell material")[0];
    isMaterialSet = materialIndex > 0 ? true : false;
    std::cout << "  cell material: " << materialIndex << std::endl;
  }
  int zOrigin;
  if (pin.hasFloatProperty("z origin"))
  {
    zOrigin = pin.floatProperty("z origin")[0];
    std::cout << "  zOrigin: " << zOrigin << std::endl;
  }
  int isHex;
  if (pin.hasIntegerProperty("hex"))
  {
    isHex = pin.integerProperty("hex")[0];
    std::cout << "  hex: " << isHex << std::endl;
  }

  int isCutAway;
  if (pin.hasIntegerProperty("cut away"))
  {
    isCutAway = pin.integerProperty("cut away")[0];
    std::cout << "  cut away: " << isCutAway << std::endl;
  }
  // Pieces
  // Segment type of each piece. 0 means cylinder and 1 means frustum
  smtk::model::IntegerList segTypes;
  if (pin.hasIntegerProperty("pieces"))
  {
    segTypes = pin.integerProperty("pieces");
    std::cout << "  segment type: ";
    for (auto item : segTypes)
    {
      std::cout << " " << item;
    }
    std::cout << std::endl;
  }
  smtk::model::FloatList typeParas;
  // For each piece, it would 3 parameters as: length, base radius, top radius,
  if (pin.hasFloatProperty("pieces"))
  {
    typeParas = pin.floatProperty("pieces");
    std::cout << "  type parameters: ";
    for (auto item : typeParas)
    {
      std::cout << " " << item;
    }
    std::cout << std::endl;
  }
  // Layer materials
  smtk::model::IntegerList subMaterials;
  if (pin.hasIntegerProperty("layer materials"))
  {
    subMaterials = pin.integerProperty("layer materials");
    std::cout << "  sub material: ";
    for (auto item : subMaterials)
    {
      std::cout << " " << item;
    }
    std::cout << std::endl;
  }
  smtk::model::FloatList radiusNs;
  if (pin.hasFloatProperty("layer materials"))
  {
    radiusNs = pin.floatProperty("layer materials");
    std::cout << "  radius(normalized): ";
    for (auto item : radiusNs)
    {
      std::cout << " " << item;
    }
    std::cout << std::endl;
  }

  // Follow logic in cmbNucRender::createGeo function. L249
  AuxiliaryGeometries childrenAux = pin.auxiliaryGeometries();
  // Create a name-auxgeom map so that we can assign the right rep
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

  std::time_t mtime;
  std::time(&mtime);

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
    coneSource->SetNumberOfLayers(numLayers + isMaterialSet);
    double height(typeParas[j * 3]), baseR(typeParas[j * 3 + 1]), topR(typeParas[j * 3 + 2]);
    /****************************************************************/
    double layer42 = numLayers + isMaterialSet;
    std::cout << "Processing part " << j << " with base center as " << baseCenter << " ,height as "
              << height << " and number of layers to be " << layer42 << " baseR=" << baseR
              << " topR=" << topR << std::endl;
    /****************************************************************/
    // baseCenter would be updated at the end of the loop
    coneSource->SetBaseCenter(0, 0, baseCenter);
    coneSource->SetHeight(height);
    double largestRadius = 0;

    for (size_t k = 0; k < numLayers; k++)
    {
      // Calculate the baseR and topR at current layer
      double baseRL = baseR * radiusNs[k];
      double topRL = topR * radiusNs[k];
      coneSource->SetBaseRadius(k, baseRL);
      coneSource->SetTopRadius(k, topRL);
      coneSource->SetResolution(k, PinCellResolution);
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
      std::cout << "Creating material layer!" << std::endl;
      largestRadius *= 2.50;
      double r[] = { largestRadius * 0.5, largestRadius * 0.5 };
      int res = 4;
      if (isHex)
      {
        res = 6;
        r[0] = r[1] = r[0] / cos30;
      }
      coneSource->SetBaseRadius(numLayers, r[0], r[1]);
      coneSource->SetTopRadius(numLayers, r[0], r[1]);
      coneSource->SetResolution(numLayers, res);
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
        std::cout << "searching for material layer! for part " << j << std::endl;
      }
      // Follow logic in L263 cmbNucRender
      vtkSmartPointer<vtkPolyData> dataset = coneSource->CreateUnitLayer(k);
      // Since it's a unit layer, proper trasformation should be applied
      vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
      // Translate, rotate then scale
      double xyz[3];
      coneSource->GetBaseCenter(xyz);
      transform->Translate(xyz[0], xyz[1], xyz[2]);
      std::cout << "  translate: x=" << xyz[0] << " y=" << xyz[1] << " z=" << xyz[2] << std::endl;

      transform->RotateZ((isHex) ? 30 : 0);
      double angle = (isHex) ? 30 : 0;
      std::cout << "  rotate: z=" << angle << std::endl;

      if (segTypes[j] == RGGType::CYLINDER && k == 0)
      { // Cylinder in the 0 layer should be handled differently(Following RGG's logic)
        transform->Scale(coneSource->GetTopRadius(k), coneSource->GetBaseRadius(k), height);
        std::cout << "  scale: x=" << coneSource->GetTopRadius(k)
                  << " y=" << coneSource->GetBaseRadius(k) << " z=" << height << std::endl;
      }
      else
      {
        transform->Scale(1, 1, height);
        std::cout << "  scale: x=" << 1 << " y=" << 1 << " z=" << height << std::endl;
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
      for (const auto& childAux : childrenAux)
      {
        if (childAux.name() == subName)
        {
          std::cout << "  insert aux " << childAux.name() << " to s_p" << std::endl;
          s_p->insert(childAux, ClassInternal::CacheValue(transformed, mtime), false);
        }
      }
      int blockIndex = k + numLayers * j;
      vtkSmartPointer<vtkPolyData> pinSubDataset = vtkSmartPointer<vtkPolyData>::New();
      pinSubDataset->DeepCopy(transformed);
      mbds->SetBlock(blockIndex, pinSubDataset);
    }
    // Check if needed to create a boundary layer for the pin
    // Update current baseCenter
    baseCenter += height;
  }
  return mbds.GetPointer();
  // FIXME
  //return vtkSmartPointer<vtkDataObject>();
}

smtkDeclareExtension(
  VTKSMTKSOURCEEXT_EXPORT, vtk_auxiliary_geometry, vtkAuxiliaryGeometryExtension);

smtkComponentInitMacro(smtk_vtk_auxiliary_geometry_extension);
