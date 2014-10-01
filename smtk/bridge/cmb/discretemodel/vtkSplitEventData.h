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
// .NAME vtkSplitEventData - Information for a split event.
// .SECTION Description
// This class stores information for a split event that has occurred.
// The SourceEntity was the entity that was split and the
// CreatedModelEntityIds are the Ids of the objects created.
// Note that a model entity is always split into 2 entities of the same
// dimension.  If there are multiple splits as part of an operation
// (e.g. split based on angle) then the event will be triggered
// multiple times.

#ifndef __smtkcmb_vtkSplitEventData_h
#define __smtkcmb_vtkSplitEventData_h

#include "vtkSMTKDiscreteModelModule.h" // For export macro

#include "vtkObject.h"

class vtkIdList;
class vtkDiscreteModelGeometricEntity;

class VTKSMTKDISCRETEMODEL_EXPORT vtkSplitEventData : public vtkObject
{
public:
  static vtkSplitEventData * New();
  vtkTypeMacro(vtkSplitEventData,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/get the SourceEntity
  vtkGetMacro(SourceEntity, vtkDiscreteModelGeometricEntity*);
  vtkSetMacro(SourceEntity, vtkDiscreteModelGeometricEntity*);

  // Description:
  // Set/get the list of created model entity ids.  Values
  // can be for different model entity types.
  vtkGetMacro(CreatedModelEntityIds, vtkIdList*);
  void SetCreatedModelEntityIds(vtkIdList*);

protected:
  vtkSplitEventData();
  virtual ~vtkSplitEventData();

private:
  vtkDiscreteModelGeometricEntity* SourceEntity;
  vtkIdList* CreatedModelEntityIds;

  vtkSplitEventData(const vtkSplitEventData&);  // Not implemented.
  void operator=(const vtkSplitEventData&);  // Not implemented.
};

#endif
