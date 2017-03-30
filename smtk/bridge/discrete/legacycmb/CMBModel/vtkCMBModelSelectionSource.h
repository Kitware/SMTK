//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBModelSelectionSource - "Dummy" source so we can treat data as a source
// .SECTION Description
// The input Source data is shallow copied to the output

#ifndef __vtkCMBModelSelectionSource_h
#define __vtkCMBModelSelectionSource_h

#include "cmbSystemConfig.h"
#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkSelectionAlgorithm.h"

class vtkSelectionSource;
class vtkDiscreteModelWrapper;

class VTKCMBDISCRETEMODEL_EXPORT vtkCMBModelSelectionSource : public vtkSelectionAlgorithm
{
public:
  static vtkCMBModelSelectionSource* New();
  vtkTypeMacro(vtkCMBModelSelectionSource, vtkSelectionAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  void CopyData(vtkSelection* selection);
  vtkGetObjectMacro(Selection, vtkSelection);

  // Description:
  // Add the SelectedEntityId.
  void AddSelectedEntityId(vtkIdType SelectedEntityId);
  void RemoveAllSelectedEntityIds();

  // Description:
  // Set/get macros for the vtkDiscreteModelWrapper.

  void SetModelWrapper(vtkDiscreteModelWrapper*);
  vtkGetObjectMacro(ModelWrapper, vtkDiscreteModelWrapper);

protected:
  vtkCMBModelSelectionSource();
  ~vtkCMBModelSelectionSource();

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*);
  void RemoveAllSelectedEntityIdsInternal();

  // Description:
  // The vtkDiscreteModelWrapper for the algorithm to extract the model
  // information from.
  vtkDiscreteModelWrapper* ModelWrapper;

  vtkSelectionSource* Source;
  vtkSelection* Selection;

private:
  vtkCMBModelSelectionSource(const vtkCMBModelSelectionSource&); // Not implemented.
  void operator=(const vtkCMBModelSelectionSource&);             // Not implemented.

  class vtkInternal;
  vtkInternal* Internal;
};

#endif
