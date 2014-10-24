//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBParserV2 - Parse a vtkPolyData that was read in from a CMB version 1 file.
// .SECTION Description
// Parse a vtkPolyData that was read in from a CMB version 1 file.

#ifndef __smtkdiscrete_vtkCMBParserV2_h
#define __smtkdiscrete_vtkCMBParserV2_h

#include "smtk/bridge/discrete/discreteBridgeExports.h" // For export macro
#include "vtkCMBParserBase.h"
#include <vector>


class vtkDiscreteModel;
class vtkModelEntity;
class vtkPolyData;

class SMTKDISCRETEBRIDGE_EXPORT vtkCMBParserV2 : public vtkCMBParserBase
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
