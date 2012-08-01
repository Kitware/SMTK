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
// .NAME vtkMergeEventData - Information for a merge event.
// .SECTION Description
// This class stores information for a merge event that is about to take
// place.  The SourceEntity is merged into the TargetEntity and the
// LowerDimensionalIds is the list of ids that are only on the boundary
// of the SourceEntity and TargetEntity and will be destroyed during
// the merge operation.

#ifndef __vtkMergeEventData_h
#define __vtkMergeEventData_h

#include "vtkObject.h"

class vtkIdTypeArray;
class vtkDiscreteModelGeometricEntity;

class VTK_EXPORT vtkMergeEventData : public vtkObject
{
public:
  static vtkMergeEventData * New();
  vtkTypeMacro(vtkMergeEventData,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/get the SourceEntity
  vtkGetMacro(SourceEntity, vtkDiscreteModelGeometricEntity*);
  vtkSetMacro(SourceEntity, vtkDiscreteModelGeometricEntity*);

  // Description:
  // Set/get the TargetEntity
  vtkGetMacro(TargetEntity, vtkDiscreteModelGeometricEntity*);
  vtkSetMacro(TargetEntity, vtkDiscreteModelGeometricEntity*);

  // Description:
  // Set/get the LowerDimensionalIds
  vtkGetMacro(LowerDimensionalIds, vtkIdTypeArray*);
  void SetLowerDimensionalIds(vtkIdTypeArray* lowerDimensionalIds);

protected:
  vtkMergeEventData();
  virtual ~vtkMergeEventData();

private:
  vtkDiscreteModelGeometricEntity* SourceEntity;
  vtkDiscreteModelGeometricEntity* TargetEntity;
  vtkIdTypeArray* LowerDimensionalIds;

  vtkMergeEventData(const vtkMergeEventData&);  // Not implemented.
  void operator=(const vtkMergeEventData&);  // Not implemented.
};

#endif
