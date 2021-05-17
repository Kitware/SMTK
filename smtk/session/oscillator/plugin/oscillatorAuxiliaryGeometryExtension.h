//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_session_oscillator_plugin_oscillatorAuxiliaryGeometryExtension_h
#define smtk_session_oscillator_plugin_oscillatorAuxiliaryGeometryExtension_h

#include "smtk/PublicPointerDefs.h"

#include "smtk/extension/vtk/model/vtkAuxiliaryGeometryExtension.h"

#include "vtkSmartPointer.h"

class vtkDataObject;

class oscillatorAuxiliaryGeometryExtension : public vtkAuxiliaryGeometryExtension
{
public:
  smtkTypeMacro(oscillatorAuxiliaryGeometryExtension);
  smtkCreateMacro(smtk::common::Extension);
  smtkSuperclassMacro(vtkAuxiliaryGeometryExtension);
  ~oscillatorAuxiliaryGeometryExtension() override;

  /**\brief Implement the extension's API.
    *
    * This fetches (and caches) the auxiliary geometry for a given
    * aux. geom. entity, returning true if the entity had a
    * valid description (i.e., it was an oscillator source object).
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

protected:
  oscillatorAuxiliaryGeometryExtension();

  /// Internal method called by canHandleAuxiliaryGeometry.
  static vtkSmartPointer<vtkDataObject> generateOscillatorRepresentation(
    const smtk::model::AuxiliaryGeometry& src);
  // Helper function for generateOscillatorRepresentation
  static vtkSmartPointer<vtkDataObject> generateOscillatorSourceRepresentation(
    const smtk::model::AuxiliaryGeometry& src);

private:
  oscillatorAuxiliaryGeometryExtension(
    const oscillatorAuxiliaryGeometryExtension&);              // Not implemented.
  void operator=(const oscillatorAuxiliaryGeometryExtension&); // Not implemented.
};

#endif
