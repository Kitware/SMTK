//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_vtk_AttributeMultiBlockSource_h
#define smtk_vtk_AttributeMultiBlockSource_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/common/UUID.h"
#include "smtk/extension/vtk/source/Exports.h"
#include "smtk/extension/vtk/source/vtkResourceMultiBlockSource.h"
#include "smtk/extension/vtk/source/vtkTracksAllInstances.h"
#include "smtk/model/CellEntity.h" // for CellEntities

#include "vtkNew.h"
#include "vtkSmartPointer.h"

#include <map>

class vtkPolyData;
class vtkPolyDataNormals;
class vtkInformationStringKey;

/**\brief A VTK source for exposing attribute data in SMTK Resource as multiblock data.
  *
  * This filter does nothing for now.
  * Eventually it might provide some visualization capabilities related to
  * attributes but for now it is just needed so that every SMTK resource
  * class can have a matching pqPipelineSource in ParaView.
  */
class VTKSMTKSOURCEEXT_EXPORT vtkAttributeMultiBlockSource : public vtkResourceMultiBlockSource
{
public:
  smtkDeclareTracksAllInstances(vtkAttributeMultiBlockSource);
  static vtkAttributeMultiBlockSource* New();
  void PrintSelf(ostream& os, vtkIndent indent) override;
  vtkTypeMacro(vtkAttributeMultiBlockSource, vtkResourceMultiBlockSource);

  smtk::attribute::ResourcePtr GetAttributeResource();
  void SetAttributeResource(const smtk::attribute::ResourcePtr&);

protected:
  vtkAttributeMultiBlockSource();
  ~vtkAttributeMultiBlockSource() override;

  int RequestData(
    vtkInformation* request, vtkInformationVector** inInfo, vtkInformationVector* outInfo) override;

private:
  vtkAttributeMultiBlockSource(const vtkAttributeMultiBlockSource&); // Not implemented.
  void operator=(const vtkAttributeMultiBlockSource&);               // Not implemented.
};

#endif // smtk_vtk_AttributeMultiBlockSource_h
