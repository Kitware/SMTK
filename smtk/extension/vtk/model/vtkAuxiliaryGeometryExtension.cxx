//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/model/vtkAuxiliaryGeometryExtension.h"

#include "smtk/extension/vtk/io/ImportAsVTKData.h"

#include "smtk/AutoInit.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/Model.h"

#include "vtkAppendPoints.h"
#include "vtkBoundingBox.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkClipClosedSurface.h"
#include "vtkCompositeDataSet.h"
#include "vtkDataObject.h"
#include "vtkDataObjectTreeIterator.h"
#include "vtkDataSet.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkGraph.h"
#include "vtkIdTypeArray.h"
#include "vtkImageData.h"
#include "vtkImageSpacingFlip.h"
#include "vtkInformation.h"
#include "vtkInformationStringKey.h"
#include "vtkInformationVector.h"
#include "vtkLookupTable.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPlaneCollection.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkPolyDataNormals.h"
#include "vtkStringArray.h"
#include "vtkTransform.h"
#include "vtkTransformFilter.h"
#include "vtkUnstructuredGrid.h"

#include "boost/filesystem.hpp"

#include <ctime>
#include <list>
#include <map>

#include <cstdlib> // for atexit()

using namespace smtk::model;
using ::boost::filesystem::last_write_time;

class vtkAuxiliaryGeometryExtension::ClassInternal
{
public:
  double m_totalSize;
  double m_maxSize = 8196;
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
    const auto& dataset = std::get<DATA>(entry);
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

vtkAuxiliaryGeometryExtension::~vtkAuxiliaryGeometryExtension() = default;

bool vtkAuxiliaryGeometryExtension::canHandleAuxiliaryGeometry(
  smtk::model::AuxiliaryGeometry& entity,
  std::vector<double>& bboxOut)
{
  smtk::extension::vtk::io::ImportAsVTKData importAsVTKData;
  if (
    !entity.isValid() || (entity.url().empty() && entity.auxiliaryGeometries().empty()) ||
    (!entity.url().empty() && !importAsVTKData.valid(entity.url())))
  {
    return false;
  }

  std::string url = entity.url();
  auto tuple = s_p->fetch(entity);
  auto dataset = std::get<ClassInternal::DATA>(tuple);
  std::time_t mtime;
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
        return vtkAuxiliaryGeometryExtension::updateBoundsFromDataSet(entity, bboxOut, dataset);
      }
    }
    else
    { // TODO: No URL, so just assume the data is still good?
      return vtkAuxiliaryGeometryExtension::updateBoundsFromDataSet(entity, bboxOut, dataset);
    }
  }

  // No cache entry for the data; we need to read it.
  bool trimCache(true);
  if (url.empty())
  { // Can't read from non-existent URL
    if (entity.auxiliaryGeometries().empty())
    {
      return false;
    }
    std::time(&mtime);
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
  vtkAuxiliaryGeometryExtension::addCacheGeometry(dataset, entity, mtime, trimCache);
  return vtkAuxiliaryGeometryExtension::updateBoundsFromDataSet(entity, bboxOut, dataset);
}

void vtkAuxiliaryGeometryExtension::addCacheGeometry(
  const vtkSmartPointer<vtkDataObject> dataset,
  const AuxiliaryGeometry& entity,
  std::time_t& mtime,
  bool trimCache)
{
  s_p->insert(entity, ClassInternal::CacheValue(dataset, mtime), trimCache);
}

vtkSmartPointer<vtkDataObject> vtkAuxiliaryGeometryExtension::fetchCachedGeometry(
  const smtk::model::AuxiliaryGeometry& entity)
{
  std::time_t dummy;
  return vtkAuxiliaryGeometryExtension::fetchCachedGeometry(entity, dummy);
}

vtkSmartPointer<vtkDataObject> vtkAuxiliaryGeometryExtension::fetchCachedGeometry(
  const smtk::model::AuxiliaryGeometry& entity,
  std::time_t& cachedTime)
{
  vtkAuxiliaryGeometryExtension::ensureCache();
  auto entry = s_p->fetch(entity);
  auto result = std::get<ClassInternal::DATA>(entry);
  if (result)
  {
    cachedTime = std::get<ClassInternal::TIMESTAMP>(entry);
  }
  return result;
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
  const smtk::model::AuxiliaryGeometry& auxGeom,
  bool genNormals)
{
  vtkAuxiliaryGeometryExtension::ensureCache();

  if (auxGeom.hasURL())
  {
    return vtkAuxiliaryGeometryExtension::readFromFile(auxGeom, genNormals);
  }

  AuxiliaryGeometries children(auxGeom.auxiliaryGeometries());
  if (!children.empty())
  {
    return vtkAuxiliaryGeometryExtension::createHierarchy(auxGeom, children, genNormals);
  }

  return vtkSmartPointer<vtkDataObject>();
}

vtkSmartPointer<vtkDataObject> vtkAuxiliaryGeometryExtension::readFromFile(
  const smtk::model::AuxiliaryGeometry& auxGeom,
  bool genNormals)
{
  vtkAuxiliaryGeometryExtension::ensureCache();

  (void)genNormals;

  std::string fileType = vtkAuxiliaryGeometryExtension::getAuxiliaryFileType(auxGeom);

  smtk::extension::vtk::io::ImportAsVTKData importAsVTKData;
  vtkSmartPointer<vtkDataObject> data = importAsVTKData(std::make_pair(fileType, auxGeom.url()));

  smtk::resource::Component::Ptr component = auxGeom.component();
  if (
    component->properties().contains<std::vector<double>>("rotate") ||
    component->properties().contains<std::vector<double>>("translate") ||
    component->properties().contains<std::vector<double>>("scale"))
  {
    vtkNew<vtkTransform> transform;
    if (component->properties().contains<std::vector<double>>("translate"))
    {
      const std::vector<double>& translate =
        component->properties().get<std::vector<double>>()["translate"];
      transform->Translate(translate[0], translate[1], translate[2]);
    }
    if (component->properties().contains<std::vector<double>>("scale"))
    {
      const std::vector<double>& scale =
        component->properties().get<std::vector<double>>()["scale"];
      transform->Scale(scale[0], scale[1], scale[2]);
    }
    if (component->properties().contains<std::vector<double>>("rotate"))
    {
      const std::vector<double>& rotate =
        component->properties().get<std::vector<double>>()["rotate"];

      // From https://en.wikipedia.org/wiki/Euler_angles#Intrinsic_rotations :
      // VTK uses Tait-Bryan Y_1 X_2 Z_3 angles to store orientation;
      // This is the corresponding direction cosine matrix (DCM) for
      // theta = X, phi = Y, psi = Z:
      transform->RotateY(rotate[1]);
      transform->RotateX(rotate[0]);
      transform->RotateZ(rotate[2]);
    }

    vtkSmartPointer<vtkTransformFilter> transformFilter =
      vtkSmartPointer<vtkTransformFilter>::New();
    transformFilter->SetInputData(data);
    transformFilter->SetTransform(transform);
    transformFilter->Update();
    data = transformFilter->GetOutput();
  }
  return data;
}

vtkSmartPointer<vtkDataObject> vtkAuxiliaryGeometryExtension::createHierarchy(
  const smtk::model::AuxiliaryGeometry& src,
  const smtk::model::AuxiliaryGeometries& children,
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

bool vtkAuxiliaryGeometryExtension::updateBoundsFromDataSet(
  smtk::model::AuxiliaryGeometry& aux,
  std::vector<double>& bboxOut,
  vtkSmartPointer<vtkDataObject> dataobj)
{
  if (auto* dataset = dynamic_cast<vtkDataSet*>(dataobj.GetPointer()))
  {
    bboxOut.resize(6);
    dataset->GetBounds(bboxOut.data());
    if (bboxOut[0] <= bboxOut[1])
    {
      aux.setBoundingBox(bboxOut.data());
    }
    return true;
  }
  else if (auto* graph = dynamic_cast<vtkGraph*>(dataobj.GetPointer()))
  {
    bboxOut.resize(6);
    graph->GetBounds(bboxOut.data());
    if (bboxOut[0] <= bboxOut[1])
    {
      aux.setBoundingBox(bboxOut.data());
    }
    return true;
  }
  else if (auto* tree = dynamic_cast<vtkCompositeDataSet*>(dataobj.GetPointer()))
  {
    auto* it = tree->NewIterator();
    it->SkipEmptyNodesOn();
    vtkBoundingBox bbox;
    bboxOut.resize(6);
    for (it->InitTraversal(); !it->IsDoneWithTraversal(); it->GoToNextItem())
    {
      vtkDataObject* dobj = it->GetCurrentDataObject();
      if (auto* dset = dynamic_cast<vtkDataSet*>(dobj))
      {
        dset->GetBounds(bboxOut.data());
        bbox.AddBounds(bboxOut.data());
      }
      else if (auto* grph = dynamic_cast<vtkGraph*>(dobj))
      {
        grph->GetBounds(bboxOut.data());
        bbox.AddBounds(bboxOut.data());
      }
    }
    it->Delete();
    if (bbox.IsValid())
    {
      bbox.GetBounds(bboxOut.data());
      aux.setBoundingBox(bboxOut.data());
    }
    return true;
  }
  return false;
}

smtkDeclareExtension(VTKSMTKMODELEXT_EXPORT, vtk_auxiliary_geometry, vtkAuxiliaryGeometryExtension);
smtkComponentInitMacro(smtk_vtk_auxiliary_geometry_extension);
