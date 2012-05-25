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
// .NAME vtkCMBModelEntityGroup - An object to store a group of model entities.
// .SECTION Description
// An object that stores a group of model entities of the same EntityType.  
// This class stores an association to those types as well as those 
// types storing an association to this.  Note that a vtkModelEntity can
// belong to multiple vtkCMBModelEntityGroups (including none at all).

#ifndef __vtkCMBModelEntityGroup_h
#define __vtkCMBModelEntityGroup_h

#include "vtkModelEntity.h"

class vtkCMBModelEntity;

class VTK_EXPORT vtkCMBModelEntityGroup : public vtkModelEntity
{
public:
  vtkTypeRevisionMacro(vtkCMBModelEntityGroup,vtkModelEntity);
  void PrintSelf(ostream& os, vtkIndent indent);

  void AddModelEntity(vtkCMBModelEntity*);
  bool RemoveModelEntity(vtkCMBModelEntity*);
  
  // Description:
  // Returns the number of model entities of type this->EntityType
  // that is grouped by this object.
  int GetNumberOfModelEntities();

  // Description:
  // Returns an iterator over entities of type this->EntityType
  // that is grouped by this object.
  vtkModelItemIterator* NewModelEntityIterator();
  
  virtual int GetType();

  vtkSetMacro(EntityType, int);
  vtkGetMacro(EntityType, int);

  // Description:
  // Reads the state of an instance from an archive OR
  // writes the state of an instance to an archive. See
  // the documentation for this class for details.
  virtual void Serialize(vtkSerializer* ser);

protected:
  vtkCMBModelEntityGroup();
  virtual ~vtkCMBModelEntityGroup();
  static vtkCMBModelEntityGroup *New();
//BTX
  friend class vtkDiscreteModel;
//ETX

  virtual bool IsDestroyable();
  virtual bool Destroy();

  int EntityType;

private:
  vtkCMBModelEntityGroup(const vtkCMBModelEntityGroup&);  // Not implemented.
  void operator=(const vtkCMBModelEntityGroup&);  // Not implemented.
};

#endif

