//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBMeshToModelWriter - outputs a m2m file in XML format
// .SECTION Description
// Filter to output an XML file with mapping from analysis mesh info to
// model topology.

#ifndef __vtkCMBMeshToModelWriter_h
#define __vtkCMBMeshToModelWriter_h

#include "cmbSystemConfig.h"
#include "vtkXMLWriter.h"

class vtkDiscreteModelWrapper;

class VTK_EXPORT vtkCMBMeshToModelWriter : public vtkXMLWriter
{
public:
  static vtkCMBMeshToModelWriter* New();
  vtkTypeMacro(vtkCMBMeshToModelWriter, vtkXMLWriter);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual const char* GetDefaultFileExtension();

  // Description:
  // Methods to define the file's major and minor version numbers.
  virtual int GetDataSetMajorVersion();
  virtual int GetDataSetMinorVersion();

  // Description:
  // Set/get functions for the ModelWrapper.
  vtkGetMacro(ModelWrapper, vtkDiscreteModelWrapper*);
  void SetModelWrapper(vtkDiscreteModelWrapper* Wrapper);

protected:
  vtkCMBMeshToModelWriter();
  ~vtkCMBMeshToModelWriter();

  virtual int WriteData();
  virtual int WriteHeader(vtkIndent* parentindent);
  virtual int Write3DModelMeshInfo(vtkIndent* indent);
  virtual int Write2DModelMeshInfo(vtkIndent* indent);
  virtual int WriteFooter(vtkIndent* parentindent);

  const char* GetDataSetName();

private:
  vtkCMBMeshToModelWriter(const vtkCMBMeshToModelWriter&); // Not implemented.
  void operator=(const vtkCMBMeshToModelWriter&);          // Not implemented.

  // Description:
  // The vtkDiscreteModelWrapper for the algorithm to extract the model
  // information from.
  vtkDiscreteModelWrapper* ModelWrapper;
};

#endif
