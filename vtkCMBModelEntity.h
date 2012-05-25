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
// .NAME vtkCMBModelEntity - 
// .SECTION Description

#ifndef __vtkCMBModelEntity_h
#define __vtkCMBModelEntity_h

#include "vtkObject.h"

class vtkCMBModelEntityGroup;
class vtkModelEntity;
class vtkModelItemIterator;

class VTK_EXPORT vtkCMBModelEntity
{
public:
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get a pointer to this object that is a vtkModelEntity.
  virtual vtkModelEntity* GetThisModelEntity()=0;

  // Description:
  // Given a vtkModelEntity, return a vtkCMBModelEntity if
  // it is a vtkCMBModelEntity (for now vtkCMBModelRegion,
  // vtkCMBModelFace, or vtkCMBModelEdge).
  static vtkCMBModelEntity* GetThisCMBModelEntity(vtkModelEntity*);

  // Description:
  // Get information about the model entity groups associated 
  // with this object.
  int GetNumberOfModelEntityGroups();
  vtkModelItemIterator* NewModelEntityGroupIterator();

protected:
  vtkCMBModelEntity();
  virtual ~vtkCMBModelEntity();

  void CopyModelEntityGroups(vtkCMBModelEntity* SourceEntity);
  void RemoveAllModelEntityGroups();

private:
  vtkCMBModelEntity(const vtkCMBModelEntity&);  // Not implemented.
  void operator=(const vtkCMBModelEntity&);  // Not implemented.
};

#endif

