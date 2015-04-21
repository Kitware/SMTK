//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBParserV4 - Parse a vtkPolyData that was read in from a CMB version 1 file.
// .SECTION Description
// Parse a vtkPolyData that was read in from a CMB version 3 file.

#ifndef __smtkdiscrete_vtkCMBParserV4_h
#define __smtkdiscrete_vtkCMBParserV4_h

#include "smtk/bridge/discrete/Exports.h" // For export macro
#include "vtkCMBParserBase.h"
#include <vector>


class vtkDiscreteModel;
class vtkModelEntity;
class vtkPolyData;

class SMTKDISCRETESESSION_EXPORT vtkCMBParserV4 : public vtkCMBParserBase
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
