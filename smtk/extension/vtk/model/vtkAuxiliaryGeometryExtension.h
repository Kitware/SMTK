//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_vtk_model_vtkAuxiliaryGeometryExtension_h
#define smtk_extension_vtk_model_vtkAuxiliaryGeometryExtension_h
#ifndef __VTK_WRAP__

#include "smtk/PublicPointerDefs.h"

#include "smtk/model/AuxiliaryGeometryExtension.h"

#include "smtk/extension/vtk/model/vtkSMTKModelExtModule.h"

#include "vtkSmartPointer.h"

class vtkDataObject;

class VTKSMTKMODELEXT_EXPORT vtkAuxiliaryGeometryExtension
  : public smtk::model::AuxiliaryGeometryExtension
{
public:
  smtkTypeMacro(vtkAuxiliaryGeometryExtension);
  smtkCreateMacro(smtk::common::Extension);
  smtkSuperclassMacro(smtk::model::AuxiliaryGeometryExtension);
  ~vtkAuxiliaryGeometryExtension() override;

  /**\brief Implement the extension's API.
    *
    * This fetches (and caches) the auxiliary geometry for a given
    * aux. geom. entity, returning true if the entity had a
    * valid description (i.e., "url" or "shape" property with
    * additional properties as needed to fully specify a VTK data
    * object).
    *
    * If this method returns true, then (1) \a bboxOut will contain
    * 6 entries specifying a bounding box for the resulting geometry
    * and (2) you may call fetchCachedGeometry(entity) to obtain
    * the VTK dataset without recreating it.
    * The cached geometry is a least-recently-used (LRU) cache, so
    * you must call fetchCachedGeometry() before another
    * call to canHandleAuxiliaryGeometry() to be assured it is still
    * in the cache.
    *
    * This method will examine the \a entity's string "type" and
    * integer "generate normals" properties to determine how to
    * create the auxiliary geometry dataset.
    */
  bool canHandleAuxiliaryGeometry(
    smtk::model::AuxiliaryGeometry& entity,
    std::vector<double>& bboxOut) override;

  /// This can be used by other smtk::model::AuxiliaryGeometryExtension
  /// subclasses to provide VTK representations to the vtkModelMultiBlockSource.
  static void addCacheGeometry(
    const vtkSmartPointer<vtkDataObject> dataset,
    const smtk::model::AuxiliaryGeometry& entity,
    std::time_t& mtime,
    bool trimCache);

  /**\brief Return VTK data created by a previous call to canHandleAuxiliaryGeometry.
    *
    * The variant that accepts a reference to std::time_t will return the
    * timestamp provided when addCacheGeometry() was called (if a cache entry
    * is found; if no entry for \a entity is found, then its value is unmodified).
    */
  static vtkSmartPointer<vtkDataObject> fetchCachedGeometry(
    const smtk::model::AuxiliaryGeometry& entity);
  static vtkSmartPointer<vtkDataObject> fetchCachedGeometry(
    const smtk::model::AuxiliaryGeometry& entity,
    std::time_t& cachedTime);

  /// Return the size (in kiB) of the current geometry cache.
  static double currentCacheSize();
  /**\brief Return the maximum size (in kiB) that the cache is allowed to take before removal.
    *
    * This size can be exceeded if a the most recent auxiliary geometry exceeds
    * the maximum allowable size; the most recent entity is always held.
    */
  static double maximumCacheSize();
  /// Set the maximum allowable size (in kIB) that the cache is allowed to take before removal.
  static bool setMaximumCacheSize(double sz);

  static std::string getAuxiliaryFileType(const smtk::model::AuxiliaryGeometry&);
  static std::string inferFileTypeFromFileName(const std::string& fname);

  /// Read a data object of a given type from a URL.
  static vtkSmartPointer<vtkDataObject>
  readFromFile(const std::string& url, const std::string& fileType, bool genNormals = false);

protected:
  vtkAuxiliaryGeometryExtension();

  /// Add this to the top of each class-static method to ensure the cache is allocated.
  static void ensureCache();

  /// Once ensureCache succeeds, this method is registered with atexit() to prevent leaks.
  static void destroyCache();

  /// Internal method called by canHandleAuxiliaryGeometry.
  static vtkSmartPointer<vtkDataObject> generateRepresentation(
    const smtk::model::AuxiliaryGeometry& src,
    bool genNormals);

  /// Internal method called by generateRepresentation when \a src has a URL.
  static vtkSmartPointer<vtkDataObject> readFromFile(
    const smtk::model::AuxiliaryGeometry& src,
    bool genNormals);

  /// Internal method called by generateRepresentation when \a src has children.
  static vtkSmartPointer<vtkDataObject> createHierarchy(
    const smtk::model::AuxiliaryGeometry& src,
    const smtk::model::AuxiliaryGeometries& children,
    bool genNormals);

  static bool updateBoundsFromDataSet(
    smtk::model::AuxiliaryGeometry& aux,
    std::vector<double>& bboxOut,
    vtkSmartPointer<vtkDataObject> dataobj);

  class ClassInternal;
  static ClassInternal* s_p;

private:
  vtkAuxiliaryGeometryExtension(const vtkAuxiliaryGeometryExtension&); // Not implemented.
  void operator=(const vtkAuxiliaryGeometryExtension&);                // Not implemented.
};

#endif
#endif
