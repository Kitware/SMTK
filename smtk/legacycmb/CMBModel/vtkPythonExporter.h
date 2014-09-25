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
// .NAME vtkPythonExporter - Export info with a Python script.
// .SECTION Description
// Operator to export ModelBuilder, SimBuilder and SMTK information through
// a Python script.

#ifndef __vtkPythonExporter_h
#define __vtkPythonExporter_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include <vector> // for callback method for SMTK Model
#include <utility> // for pair in callback method for SMTK model
#include "vtkObject.h"
#include "smtk/attribute/Manager.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModelWrapper;
class vtkDiscreteModel;

class VTKCMBDISCRETEMODEL_EXPORT vtkPythonExporter : public vtkObject
{
public:
  static vtkPythonExporter * New();
  vtkTypeMacro(vtkPythonExporter,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // This method is for standard paraview client-server apps
  virtual void Operate(vtkDiscreteModelWrapper* modelWrapper,
                       const char* smtkContents);

  // This method is for *legacy* paraview client-server apps
  virtual void Operate(vtkDiscreteModelWrapper* modelWrapper,
                       const char* smtkContents,
                       const char* exportContents);

  // This method is for standalone & test apps
  virtual void Operate(vtkDiscreteModel* model,
                       smtk::attribute::Manager& simulationAttributes,
                       smtk::attribute::Manager& exportAttributes);

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

  // Description:
  // Get/set the script that is to be executed.
  vtkSetStringMacro(Script);
  vtkGetStringMacro(Script);

  // Description:
  // Get/set the PYTHONPATH.
  vtkSetStringMacro(PythonPath);
  vtkGetStringMacro(PythonPath);

  // Get/set the python executable. Should only be used by non-paraview apps
  vtkSetStringMacro(PythonExecutable);
  vtkGetStringMacro(PythonExecutable);

protected:
  vtkPythonExporter();
  virtual ~vtkPythonExporter();

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModelWrapper* modelWrapper);

  char *Script;
  char *PythonPath;
  char *PythonExecutable;

private:
  vtkPythonExporter(const vtkPythonExporter&);  // Not implemented.
  void operator=(const vtkPythonExporter&);  // Not implemented.

  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;
};
#endif
