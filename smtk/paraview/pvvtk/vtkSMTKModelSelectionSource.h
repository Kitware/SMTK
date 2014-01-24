/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkCmbModelSelectionSource.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkCmbModelSelectionSource - "Dummy" source so we can treat data as a source
// .SECTION Description
// The input Source data is shallow copied to the output

#ifndef __smtk_pv_vtkCmbModelSelectionSource_h
#define __smtk_pv_vtkCmbModelSelectionSource_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkSelectionAlgorithm.h"
#include "cmbSystemConfig.h"

class vtkSelectionSource;
class vtkDiscreteModelWrapper;

class PVSMTK_EXPORT vtkCmbModelSelectionSource : public vtkSelectionAlgorithm
{
public:
  static vtkCmbModelSelectionSource *New();
  vtkTypeMacro(vtkCmbModelSelectionSource,vtkSelectionAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  void CopyData(vtkSelection *selection);
  vtkGetObjectMacro(Selection, vtkSelection);

  // Description:
  // Add the SelectedEntityId.
  void AddSelectedEntityId(vtkIdType SelectedEntityId);
  void RemoveAllSelectedEntityIds();

  // Description:
  // Set/get macros for the vtkDiscreteModelWrapper.

  void SetModelWrapper(vtkDiscreteModelWrapper*);
  vtkGetObjectMacro(ModelWrapper, vtkDiscreteModelWrapper);

//BTX
protected:
  vtkCmbModelSelectionSource();
  ~vtkCmbModelSelectionSource();

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  void RemoveAllSelectedEntityIdsInternal();

  // Description:
  // The vtkDiscreteModelWrapper for the algorithm to extract the model
  // information from.
  vtkDiscreteModelWrapper* ModelWrapper;

  vtkSelectionSource *Source;
  vtkSelection* Selection;

private:
  vtkCmbModelSelectionSource(const vtkCmbModelSelectionSource&);  // Not implemented.
  void operator=(const vtkCmbModelSelectionSource&);  // Not implemented.

  class vtkInternal;
  vtkInternal* Internal;
//ETX
};

#endif
