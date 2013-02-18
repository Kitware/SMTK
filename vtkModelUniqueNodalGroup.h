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
// .NAME vtkModelUniqueNodalGroup - An object to store a set of point Ids.
// .SECTION Description
// An object that stores a group of point Ids.  All point Ids are
// stored uniquely.

#ifndef __vtkModelUniqueNodalGroup_h
#define __vtkModelUniqueNodalGroup_h

#include "vtkDiscreteModelModule.h" // For export macro
#include "vtkModelNodalGroup.h"


class VTKDISCRETEMODEL_EXPORT vtkModelUniqueNodalGroup : public vtkModelNodalGroup
{
public:
  vtkTypeMacro(vtkModelUniqueNodalGroup,vtkModelNodalGroup);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Add/remove point Ids to/from the set.
  virtual void AddPointId(vtkIdType pointId);
  virtual void RemovePointId(vtkIdType pointId);

  // Description:
  // Reads the state of an instance from an archive OR
  // writes the state of an instance to an archive. See
  // the documentation for this class for details.
  virtual void Serialize(vtkSerializer* ser);

  // Description:
  // Flag to indicate the type of vtkModelNodalGroup.
  virtual int GetNodalGroupType() {return UNIQUE_NODAL_GROUP;}

protected:
  vtkModelUniqueNodalGroup();
  virtual ~vtkModelUniqueNodalGroup();
  static vtkModelUniqueNodalGroup *New();
//BTX
  friend class vtkDiscreteModel;
//ETX

  virtual bool Destroy();

private:
  vtkModelUniqueNodalGroup(const vtkModelUniqueNodalGroup&);  // Not implemented.
  void operator=(const vtkModelUniqueNodalGroup&);  // Not implemented.
};

#endif

