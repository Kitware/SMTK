/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/
// .NAME vtkCMBModelOmicronMeshInputWriter - Merge a set of geometric model entities
// .SECTION Description
// Operator to merge a set of source geometric model entities into
// a target geometric entity on the server.  The properties of the target entity
// (e.g. color, BCS/ModelEntityGroup associations) will not be changed.
// Warning: This may only currently work from model faces.

#ifndef __vtkCMBModelOmicronMeshInputWriter_h
#define __vtkCMBModelOmicronMeshInputWriter_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkCMBModelOmicronMeshInputWriterBase.h"
#include "cmbSystemConfig.h"
#include <iostream>

class vtkDiscreteModel;
class vtkCMBModelRegion;
class vtkMultiBlockDataSet;
class vtkPolyData;

class VTKCMBDISCRETEMODEL_EXPORT vtkCMBModelOmicronMeshInputWriter : public vtkCMBModelOmicronMeshInputWriterBase
{
public:
  static vtkCMBModelOmicronMeshInputWriter * New();
  vtkTypeMacro(vtkCMBModelOmicronMeshInputWriter,vtkCMBModelOmicronMeshInputWriterBase);
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

  vtkCMBModelOmicronMeshInputWriter(const vtkCMBModelOmicronMeshInputWriter&);  // Not implemented.
  void operator=(const vtkCMBModelOmicronMeshInputWriter&);  // Not implemented.
};

#endif
