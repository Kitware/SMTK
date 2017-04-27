//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBModelOmicronMeshInputWriter - Merge a set of geometric model entities
// .SECTION Description
// Operator to merge a set of source geometric model entities into
// a target geometric entity on the server.  The properties of the target entity
// (e.g. color, BCS/ModelEntityGroup associations) will not be changed.
// Warning: This may only currently work from model faces.

#ifndef __vtkCMBModelOmicronMeshInputWriter_h
#define __vtkCMBModelOmicronMeshInputWriter_h

#include "cmbSystemConfig.h"
#include "vtkCMBModelOmicronMeshInputWriterBase.h"
#include "vtkCmbDiscreteModelModule.h" // For export macro
#include <iostream>

class vtkDiscreteModel;
class vtkCMBModelRegion;
class vtkMultiBlockDataSet;
class vtkPolyData;

class VTKCMBDISCRETEMODEL_EXPORT vtkCMBModelOmicronMeshInputWriter
  : public vtkCMBModelOmicronMeshInputWriterBase
{
public:
  static vtkCMBModelOmicronMeshInputWriter* New();
  vtkTypeMacro(vtkCMBModelOmicronMeshInputWriter, vtkCMBModelOmicronMeshInputWriterBase);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Write the CMB file out.
  bool Write(vtkDiscreteModel* model, vtkCMBModelOmicronMeshInputWriterBase* base);

  // Description:
  // Write out the CMB file to the provided buffer
  bool Write(vtkDiscreteModel* model, std::ostream& buffer);

protected:
  vtkCMBModelOmicronMeshInputWriter();
  virtual ~vtkCMBModelOmicronMeshInputWriter();

private:
  vtkCMBModelOmicronMeshInputWriter(const vtkCMBModelOmicronMeshInputWriter&); // Not implemented.
  void operator=(const vtkCMBModelOmicronMeshInputWriter&);                    // Not implemented.
};

#endif
