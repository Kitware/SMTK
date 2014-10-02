//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBParserV5 - Parse a vtkPolyData that was read in from a CMB version 1 file.
// .SECTION Description
// Parse a vtkPolyData that was read in from a CMB version 5 file.

#ifndef __smtkcmb_vtkCMBParserV5_h
#define __smtkcmb_vtkCMBParserV5_h

#include "smtk/bridge/cmb/cmbBridgeExports.h" // For export macro
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
