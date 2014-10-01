/*=========================================================================

Copyright (c) 1998-205 Kitware Inc. 28 Corporate Drive, Suite 24,
Clifton Park, NY, 1265, USA.

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
// .NAME vtkCMBParserV2 - Parse a vtkPolyData that was read in from a CMB version 1 file.
// .SECTION Description
// Parse a vtkPolyData that was read in from a CMB version 1 file.

#ifndef __smtkcmb_vtkCMBParserV2_h
#define __smtkcmb_vtkCMBParserV2_h

#include "SMTKCMBBridgeExports.h" // For export macro
#include "vtkCMBParserBase.h"
#include <vector>


class vtkDiscreteModel;
class vtkModelEntity;
class vtkPolyData;

class SMTKCMBBRIDGE_EXPORT vtkCMBParserV2 : public vtkCMBParserBase
{
public:
  static vtkCMBParserV2* New();
  vtkTypeMacro(vtkCMBParserV2,vtkCMBParserBase);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual bool Parse(vtkPolyData* MasterPoly, vtkDiscreteModel* Model);

protected:
  vtkCMBParserV2();
  virtual ~vtkCMBParserV2();

  // Description:
  // Set the unique persistent Id,
  void SetModelEntityData(
    vtkPolyData* Poly, std::vector<vtkModelEntity*> & ModelEntities,
    const char* BaseArrayName, vtkDiscreteModel* Model);

private:
  vtkCMBParserV2(const vtkCMBParserV2&);  // Not implemented.
  void operator=(const vtkCMBParserV2&);  // Not implemented.
};

#endif
