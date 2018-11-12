//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_session_rgg_plugin_rggAuxiliaryGeometryExtension_h
#define smtk_session_rgg_plugin_rggAuxiliaryGeometryExtension_h

#include "smtk/PublicPointerDefs.h"

#include "smtk/extension/vtk/source/vtkAuxiliaryGeometryExtension.h"

#include "smtk/session/rgg/plugin/Exports.h"

#include "vtkSmartPointer.h"

class vtkDataObject;

class SMTKRGGSESSIONPLUGIN_EXPORT rggAuxiliaryGeometryExtension
  : public vtkAuxiliaryGeometryExtension
{
public:
  smtkTypeMacro(rggAuxiliaryGeometryExtension);
  smtkCreateMacro(smtk::common::Extension);
  smtkSuperclassMacro(vtkAuxiliaryGeometryExtension);
  virtual ~rggAuxiliaryGeometryExtension();

  /**\brief Implement the extension's API.
    *
    * This fetches (and caches) the auxiliary geometry for a given
    * aux. geom. entity, returning true if the entity had a
    * valid description (rgg pin and rgg duct object).
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
    smtk::model::AuxiliaryGeometry& entity, std::vector<double>& bboxOut) override;

protected:
  rggAuxiliaryGeometryExtension();

  /// Internal method called by canHandleAuxiliaryGeometry.
  static vtkSmartPointer<vtkDataObject> generateRGGRepresentation(
    const smtk::model::AuxiliaryGeometry& pin, bool genNormals);
  // Helper function for generateRGGRepresentation
  static vtkSmartPointer<vtkDataObject> generateRGGPinRepresentation(
    const smtk::model::AuxiliaryGeometry& pin, bool genNormals);
  // Helper function for generateRGGRepresentation
  static vtkSmartPointer<vtkDataObject> generateRGGDuctRepresentation(
    const smtk::model::AuxiliaryGeometry& duct, bool genNormals);

private:
  rggAuxiliaryGeometryExtension(const rggAuxiliaryGeometryExtension&); // Not implemented.
  void operator=(const rggAuxiliaryGeometryExtension&);                // Not implemented.
};

#endif
