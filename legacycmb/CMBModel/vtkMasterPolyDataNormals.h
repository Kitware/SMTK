/*=========================================================================

Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive,
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

// .NAME vtkMasterPolyDataNormals - compute normals pointing out for all shells
// .SECTION Description
// This filter expects vtkPolyData, ordering the point ids making up each cell
// such that the normals are pointing out.  This is done by taking each shell
// individually and passing it through vtkPolyDataNormals (with no splitting or
// computation of point normals, as only interested in cell normals).
// Note: Only Polys are passed through this filter.  Any Verts or Lines on the
// input are removed, and we don't handle Strips.

#ifndef __vtkMasterPolyDataNormals_h
#define __vtkMasterPolyDataNormals_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"

class vtkIdList;

class VTKCMBDISCRETEMODEL_EXPORT vtkMasterPolyDataNormals : public vtkPolyDataAlgorithm
{
public:
  static vtkMasterPolyDataNormals* New();
  vtkTypeMacro(vtkMasterPolyDataNormals, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

//BTX
protected:
  vtkMasterPolyDataNormals();
  ~vtkMasterPolyDataNormals();

  // Description:
  // This is called within ProcessRequest when a request asks the algorithm
  // to do its work. This is the method you should override to do whatever the
  // algorithm is designed to do. This happens during the fourth pass in the
  // pipeline execution process.
  virtual int RequestData(vtkInformation*,
                          vtkInformationVector**,
                          vtkInformationVector*);

private:
  vtkMasterPolyDataNormals(const vtkMasterPolyDataNormals&); // Not implemented.
  void operator=(const vtkMasterPolyDataNormals&); // Not implemented.
//ETX
};

#endif


