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

#ifndef __smtkcmb_vtkCMBModelWriterV5_h
#define __smtkcmb_vtkCMBModelWriterV5_h

#include "smtk/bridge/cmb/cmbBridgeExports.h" // For export macro
#include "vtkCMBModelWriterV4.h"
#include <vector>


class vtkDiscreteModel;
class vtkCMBModelWriterBase;
class vtkModelEntity;
class vtkPolyData;

class SMTKCMBBRIDGE_EXPORT vtkCMBModelWriterV5 : public vtkCMBModelWriterV4
{
public:
  static vtkCMBModelWriterV5 * New();
  vtkTypeMacro(vtkCMBModelWriterV5,vtkCMBModelWriterV4);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual int GetVersion()
  {return 5;}

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
  vtkCMBModelWriterV5(const vtkCMBModelWriterV5&);  // Not implemented.
  void operator=(const vtkCMBModelWriterV5&);  // Not implemented.
};

#endif
