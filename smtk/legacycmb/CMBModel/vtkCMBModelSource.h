/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkCMBModelSource.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkCMBModelSource - "Dummy" source so we can treat data as a source
// .SECTION Description
// The input Source data is shallow copied to the output

#ifndef __vtkCMBModelSource_h
#define __vtkCMBModelSource_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkCompositeDataSetAlgorithm.h"
#include "vtkWeakPointer.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModelWrapper;
class vtkDiscreteModel;
class vtkStringArray;
class vtkCallbackCommand;

class VTKCMBDISCRETEMODEL_EXPORT vtkCMBModelSource : public vtkCompositeDataSetAlgorithm
{
public:
  static vtkCMBModelSource *New();
  vtkTypeMacro(vtkCMBModelSource,vtkCompositeDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Copy a model source/wrapper
  void CopyData(vtkDiscreteModelWrapper *source);

  // Description:
  // Specify a model source.
  void SetSource(vtkDiscreteModelWrapper *source);
  vtkDiscreteModelWrapper* GetSource();

  // Description:
  // Get the modified time of this object.
  virtual unsigned long GetMTime();

protected:
  vtkCMBModelSource();
  ~vtkCMBModelSource();
  // Description:
  // This is called by the superclass.
  // This is the method you should override.
  virtual int RequestDataObject(vtkInformation* request,
    vtkInformationVector** inputData,
    vtkInformationVector* outputData);

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  // see algorithm for more info
  virtual int FillOutputPortInformation(int port, vtkInformation* info);

  vtkWeakPointer<vtkDiscreteModelWrapper> Source;

private:
  vtkCMBModelSource(const vtkCMBModelSource&);  // Not implemented.
  void operator=(const vtkCMBModelSource&);  // Not implemented.
};

#endif
