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
// .NAME vtkCMBParserV4 - Parse a vtkPolyData that was read in from a CMB version 1 file.
// .SECTION Description
// Parse a vtkPolyData that was read in from a CMB version 3 file.

#ifndef __smtkcmb_vtkCMBParserV4_h
#define __smtkcmb_vtkCMBParserV4_h

#include "smtk/bridge/cmb/cmbBridgeExports.h" // For export macro
#include "vtkCMBParserBase.h"
#include <vector>


class vtkDiscreteModel;
class vtkModelEntity;
class vtkPolyData;

class SMTKCMBBRIDGE_EXPORT vtkCMBParserV4 : public vtkCMBParserBase
{
public:
  static vtkCMBParserV4* New();
  vtkTypeMacro(vtkCMBParserV4,vtkCMBParserBase);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual bool Parse(vtkPolyData* MasterPoly, vtkDiscreteModel* Model);

protected:
  vtkCMBParserV4();
  virtual ~vtkCMBParserV4();

  // Description:
  // Set the unique persistent Id,
  void SetModelEntityData(
    vtkPolyData* Poly, std::vector<vtkModelEntity*> & ModelEntities,
    const char* BaseArrayName, vtkDiscreteModel* Model);

private:
  vtkCMBParserV4(const vtkCMBParserV4&);  // Not implemented.
  void operator=(const vtkCMBParserV4&);  // Not implemented.
};

#endif
