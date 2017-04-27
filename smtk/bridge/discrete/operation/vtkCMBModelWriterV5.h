//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBModelWriterV5 - Writes out a CMB version 5.0 file.
// .SECTION Description
// Writes out a CMB version 5.0 file.  It currently gets written
// out as a vtkPolyData using vtkXMLPolyDataWriter with the necessary
// information included in the field data.
// The format for storing the model entity Ids for each model entity group
// is not that straightforward since every model entity group could potentially
// be grouping a different amount of model entities.  The way that I chose to do it
// is to have a single index flat array that stores all of the needed information.
// The format of the data in the array is:
// number of model entities in first model entity group
// the Ids of the model entities in the first group
// number of model entities in second model entity group
// the Ids of the model entities in the second group
// ...
// number of model entities in last model entity group
// the Ids of the model entities in the last group

#ifndef __smtkdiscrete_vtkCMBModelWriterV5_h
#define __smtkdiscrete_vtkCMBModelWriterV5_h

#include "smtk/bridge/discrete/Exports.h" // For export macro
#include "vtkCMBModelWriterV4.h"
#include <vector>

class vtkDiscreteModel;
class vtkCMBModelWriterBase;
class vtkModelEntity;
class vtkPolyData;

class SMTKDISCRETESESSION_EXPORT vtkCMBModelWriterV5 : public vtkCMBModelWriterV4
{
public:
  static vtkCMBModelWriterV5* New();
  vtkTypeMacro(vtkCMBModelWriterV5, vtkCMBModelWriterV4);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual int GetVersion() { return 5; }

protected:
  vtkCMBModelWriterV5();
  virtual ~vtkCMBModelWriterV5();

  // Description:
  // Set the vtkDiscreteModelEdge data in Poly.
  virtual void SetModelEdgeData(vtkDiscreteModel* model, vtkPolyData* poly);

  // Description:
  // Set the vtkDiscreteModelFace data in Poly.
  virtual void SetModelFaceData(vtkDiscreteModel* Model, vtkPolyData* Poly);

  // Description:
  // Set any information that maps the model grid to the analysis grid.
  virtual void SetAnalysisGridData(vtkDiscreteModel* model, vtkPolyData* poly);

private:
  vtkCMBModelWriterV5(const vtkCMBModelWriterV5&); // Not implemented.
  void operator=(const vtkCMBModelWriterV5&);      // Not implemented.
};

#endif
