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
// .NAME vtkModelUserName - Helper class for setting/getting a user name
// .SECTION Description
// Helper class with functions for setting and getting a user name
// for a vtkModelEntity.

#ifndef __smtkcmb_vtkModelUserName_h
#define __smtkcmb_vtkModelUserName_h

#include "vtkSMTKDiscreteModelModule.h" // For export macro

#include "vtkObject.h"

class vtkInformationStringKey;
class vtkModelEntity;

class VTKSMTKDISCRETEMODEL_EXPORT vtkModelUserName : public vtkObject
{
public:
  static vtkModelUserName *New();
  vtkTypeMacro(vtkModelUserName,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  static void SetUserName(vtkModelEntity* entity, const char* userName);

  static const char* GetUserName(vtkModelEntity* entity);

  static vtkInformationStringKey* USERNAME();

protected:
  vtkModelUserName() {};
  ~vtkModelUserName() {};

private:
  vtkModelUserName(const vtkModelUserName&);  // Not implemented.
  void operator=(const vtkModelUserName&);  // Not implemented.
};
#endif

