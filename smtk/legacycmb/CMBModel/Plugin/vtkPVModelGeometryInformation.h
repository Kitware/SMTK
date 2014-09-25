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
// .NAME vtkPVModelGeometryInformation - Light object for holding
// geometry information about a model face object.
// .SECTION Description
// .SECTION Caveats

#ifndef __vtkPVModelGeometryInformation_h
#define __vtkPVModelGeometryInformation_h

#include "vtkPVInformation.h"
#include "cmbSystemConfig.h"
#include <string>
#include <map>

class vtkIdTypeArray;

class VTK_EXPORT vtkPVModelGeometryInformation : public vtkPVInformation
{
public:
  static vtkPVModelGeometryInformation* New();
  vtkTypeMacro(vtkPVModelGeometryInformation, vtkPVInformation);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Transfer information about a single object into this object.
  virtual void CopyFromObject(vtkObject*);

  // Description:
  // Get the number of points or cells for the model or a model entity.
  vtkGetMacro(NumberOfPoints, int);
  vtkGetMacro(NumberOfCells, int);

  // Description:
  // Get the bounds for the model or a geometric model entity as
  // (Xmin,Xmax,Ymin,Ymax,Zmin,Zmax).
  void GetBounds(double bounds[6]);
  double *GetBounds();

  virtual int GetMasterCellId(unsigned int flatidx, int idx);
  virtual vtkIdType GetModelEntityId(unsigned int flatidx);

  // Description:
  // Manage a serialized version of the information.
  virtual void CopyToStream(vtkClientServerStream*);
  virtual void CopyFromStream(const vtkClientServerStream*);

  //BTX
protected:
  vtkPVModelGeometryInformation();
  ~vtkPVModelGeometryInformation();

  // Data information collected from remote processes.
  int    NumberOfPoints;
  int    NumberOfCells;
  double Bounds[6];

  std::map<int, vtkIdTypeArray*> CellIdsMap;
  std::map<int, vtkIdType> EnityIdsMap;

private:

  vtkPVModelGeometryInformation(const vtkPVModelGeometryInformation&); // Not implemented
  void operator=(const vtkPVModelGeometryInformation&); // Not implemented
  //ETX
};

#endif
