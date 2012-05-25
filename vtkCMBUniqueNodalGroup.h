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
// .NAME vtkCMBUniqueNodalGroup - An object to store a set of point Ids.
// .SECTION Description
// An object that stores a group of point Ids.  All point Ids are
// stored uniquely.

#ifndef __vtkCMBUniqueNodalGroup_h
#define __vtkCMBUniqueNodalGroup_h

#include "vtkCMBNodalGroup.h"


class VTK_EXPORT vtkCMBUniqueNodalGroup : public vtkCMBNodalGroup
{
public:
  vtkTypeRevisionMacro(vtkCMBUniqueNodalGroup,vtkCMBNodalGroup);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Add/remove point Ids to/from the set.
  virtual void AddPointId(vtkIdType PointId);
  virtual void RemovePointId(vtkIdType PointId);
  
  // Description:
  // Reads the state of an instance from an archive OR
  // writes the state of an instance to an archive. See
  // the documentation for this class for details.
  virtual void Serialize(vtkSerializer* ser);

  // Description:
  // Flag to indicate the type of vtkCMBNodalGroup.
  virtual int GetNodalGroupType() {return UNIQUE_NODAL_GROUP;}

protected:
  vtkCMBUniqueNodalGroup();
  virtual ~vtkCMBUniqueNodalGroup();
  static vtkCMBUniqueNodalGroup *New();
//BTX
  friend class vtkDiscreteModel;
//ETX

  virtual bool Destroy();
  
private:
  vtkCMBUniqueNodalGroup(const vtkCMBUniqueNodalGroup&);  // Not implemented.
  void operator=(const vtkCMBUniqueNodalGroup&);  // Not implemented.
};

#endif

