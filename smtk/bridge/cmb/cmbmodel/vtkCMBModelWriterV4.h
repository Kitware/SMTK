//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBModelWriterV4 - Writes out a CMB version 4.0 file.
// .SECTION Description
// Writes out a CMB version 4.0 file.  It currently gets written
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

#ifndef __smtkdiscrete_vtkCMBModelWriterV4_h
#define __smtkdiscrete_vtkCMBModelWriterV4_h

#include "smtk/bridge/cmb/discreteBridgeExports.h" // For export macro
#include "vtkCMBModelWriterV2.h"
#include <vector>


class vtkDiscreteModel;
class vtkCMBModelWriterBase;
class vtkModelEntity;
class vtkPolyData;

class SMTKDISCRETEBRIDGE_EXPORT vtkCMBModelWriterV4 : public vtkCMBModelWriterV2
{
public:
  static vtkCMBModelWriterV4 * New();
  vtkTypeMacro(vtkCMBModelWriterV4,vtkCMBModelWriterV2);
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual int GetVersion()
  {return 4;}


protected:
  vtkCMBModelWriterV4();
  virtual ~vtkCMBModelWriterV4();

  // Description:
  // Set the vtkDiscreteModelVertex data in Poly.
  virtual void SetModelVertexData(vtkDiscreteModel* Model, vtkPolyData* Poly);

  // Description:
  // Set the vtkDiscreteModelFace data in Edge.
  virtual void SetModelEdgeData(vtkDiscreteModel* Model, vtkPolyData* Poly);

  // Description:
  // Set the vtkDiscreteModelFace data in Poly.
  virtual void SetModelFaceData(vtkDiscreteModel* Model, vtkPolyData* Poly);

  // Description:
  // Set the vtkDiscreteModelEdge data in Poly. Floating edges act just like
  // every other edge so we should have it already taken care of.
  virtual void SetFloatingEdgeData(vtkDiscreteModel* /*Model*/, vtkPolyData* /*Poly*/) {};

private:
  vtkCMBModelWriterV4(const vtkCMBModelWriterV4&);  // Not implemented.
  void operator=(const vtkCMBModelWriterV4&);  // Not implemented.
};

#endif
