//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_extension_vtk_model_ModelAuxiliaryGeometry_h
#define __smtk_extension_vtk_model_ModelAuxiliaryGeometry_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/vtk/model/vtkAuxiliaryGeometryExtension.h"
#include "smtk/extension/vtk/source/vtkTracksAllInstances.h"

#include "smtk/extension/vtk/model/vtkSMTKModelExtModule.h"

#include "vtkMultiBlockDataSetAlgorithm.h"
#include "vtkNew.h"
#include "vtkSmartPointer.h"

#include <map>

class vtkPolyDataNormals;

/**\brief A VTK source for a single auxiliary geometry.
  *
  * This filter will parse the Url of auxiliary geometry entity referred by
  * \a AuxiliaryEntityID and generates a vtkMultiBlockDataSet, which could include polydata,
  * imagedata, unstructured grid, or multiblock depending which reader is used.
  */
class VTKSMTKMODELEXT_EXPORT vtkModelAuxiliaryGeometry : public vtkMultiBlockDataSetAlgorithm
{
public:
  smtkDeclareTracksAllInstances(vtkModelAuxiliaryGeometry);
  static vtkModelAuxiliaryGeometry* New();
  void PrintSelf(ostream& os, vtkIndent indent) override;
  vtkTypeMacro(vtkModelAuxiliaryGeometry, vtkMultiBlockDataSetAlgorithm);

  vtkGetObjectMacro(CachedOutput, vtkMultiBlockDataSet);

  smtk::model::ResourcePtr GetModelResource();
  void SetModelResource(smtk::model::ResourcePtr);

  // Description:
  // Auxiliary entity ID that this source will be building upon.
  vtkSetStringMacro(AuxiliaryEntityID);
  vtkGetStringMacro(AuxiliaryEntityID);

  void Dirty();

  /// A helper method to read data using a templated VTK reader and output data-object class.
  template<typename T, typename U>
  static vtkSmartPointer<T> ReadData(const smtk::model::AuxiliaryGeometry& auxGeom);

protected:
  vtkModelAuxiliaryGeometry();
  ~vtkModelAuxiliaryGeometry() override;

  int RequestInformation(
    vtkInformation* request,
    vtkInformationVector** inputVector,
    vtkInformationVector* outputVector) override;

  int RequestData(
    vtkInformation* request,
    vtkInformationVector** inInfo,
    vtkInformationVector* outInfo) override;

  void SetCachedOutput(vtkMultiBlockDataSet*);

  smtk::model::ResourcePtr ModelResource;
  vtkMultiBlockDataSet* CachedOutput;
  double DefaultColor[4];
  char* AuxiliaryEntityID; // Auxiliary Entity UUID
  int AllowNormalGeneration;
  vtkNew<vtkPolyDataNormals> NormalGenerator;
  vtkAuxiliaryGeometryExtension::Ptr AuxGeomHelper;

private:
  vtkModelAuxiliaryGeometry(const vtkModelAuxiliaryGeometry&); // Not implemented.
  void operator=(const vtkModelAuxiliaryGeometry&);            // Not implemented.
};

#endif
