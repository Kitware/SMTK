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
// .NAME vtkCMBParserV5 - Parse a vtkPolyData that was read in from a CMB version 1 file.
// .SECTION Description
// Parse a vtkPolyData that was read in from a CMB version 5 file.

#ifndef __smtkcmb_vtkCMBParserV5_h
#define __smtkcmb_vtkCMBParserV5_h

#include "SMTKCMBBridgeExports.h" // For export macro
#include "vtkCMBParserBase.h"
#include <vector>

#include "smtk/SharedPtr.h"

class vtkDiscreteModel;
class vtkModelEntity;
class vtkPolyData;

namespace smtk {
  namespace bridge {
    namespace cmb {
      class Bridge;
    }
  }
}

class SMTKCMBBRIDGE_EXPORT vtkCMBParserV5 : public vtkCMBParserBase
{
public:
  static vtkCMBParserV5* New();
  vtkTypeMacro(vtkCMBParserV5,vtkCMBParserBase);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual bool Parse(vtkPolyData* MasterPoly, vtkDiscreteModel* Model);

protected:
  vtkCMBParserV5();
  virtual ~vtkCMBParserV5();

  // Description:
  // Set the unique persistent Id, color, user name.
  void SetModelEntityData(
    vtkPolyData* Poly, std::vector<vtkModelEntity*> & ModelEntities,
    const char* BaseArrayName, vtkDiscreteModel* Model);

  // Description:
  // Set the mapping from the model grid to analysis grid info
  // if possible.  If the information from a 3dm file and a
  // BC file exists, use the information from the 3dm file since it
  // is "more" complete.
  void SetAnalysisGridData(vtkPolyData* masterPoly, vtkDiscreteModel* model);

  static smtk::shared_ptr<smtk::bridge::cmb::Bridge> s_bridge;

private:
  vtkCMBParserV5(const vtkCMBParserV5&);  // Not implemented.
  void operator=(const vtkCMBParserV5&);  // Not implemented.
};

#endif
