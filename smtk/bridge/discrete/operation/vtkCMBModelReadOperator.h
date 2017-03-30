//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBModelReadOperator -
// .SECTION Description
// Front end for the readers.  Reads in a vtkPolyData and then figures
// out how to parse that vtkPolyData.

#ifndef __smtkdiscrete_vtkCMBModelReadOperator_h
#define __smtkdiscrete_vtkCMBModelReadOperator_h

#include "smtk/bridge/discrete/Exports.h" // For export macro
#include "vtkObject.h"

class vtkCMBParserBase;
class vtkDiscreteModelWrapper;
class vtkPolyData;
class vtkDiscreteModel;
namespace smtk
{
namespace bridge
{
namespace discrete
{
class Session;
}
}
}

class SMTKDISCRETESESSION_EXPORT vtkCMBModelReadOperator : public vtkObject
{
public:
  static vtkCMBModelReadOperator* New();
  vtkTypeMacro(vtkCMBModelReadOperator, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Load the file into Model.
  // The \a session will be used to assign UUIDs to entities if there are field arrays
  // for entity UUIDs; if there is no UUID array for certain entities, the session will create
  // and assign new UUIDs for them. This way cmb models will have consistent UUIDs across
  // different runs so that entity-attribute associations, which is recorded with UUIDs, will
  // be consistent, so is color-by entity UUIDs.
  // If the session is NULL, the UUIDs will be different everytime we load a cmb model.
  void Operate(vtkDiscreteModelWrapper* ModelWrapper, smtk::bridge::discrete::Session* session);

  // Description:
  // Load the file into Model.
  void Read(vtkDiscreteModel* model, smtk::bridge::discrete::Session* session);

  // Description:
  // Get/Set the name of the input file.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

  // Description:
  // Get the string used to reference the field data array that the
  // file version is stored in.
  static const char* GetCMBFileVersionString();

protected:
  vtkCMBModelReadOperator();
  virtual ~vtkCMBModelReadOperator();

  vtkCMBParserBase* NewParser(vtkPolyData* MasterPoly);

private:
  // Description:
  // The name of the file to be read in.
  char* FileName;

  vtkCMBModelReadOperator(const vtkCMBModelReadOperator&); // Not implemented.
  void operator=(const vtkCMBModelReadOperator&);          // Not implemented.

  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;
};

#endif
