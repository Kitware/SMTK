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

#ifndef __smtkcmb_vtkCMBModelWriterV4_h
#define __smtkcmb_vtkCMBModelWriterV4_h

#include "SMTKCMBBridgeExports.h" // For export macro
#include "vtkCMBModelWriterV2.h"
#include <vector>


class vtkDiscreteModel;
class vtkCMBModelWriterBase;
class vtkModelEntity;
class vtkPolyData;

class SMTKCMBBRIDGE_EXPORT vtkCMBModelWriterV4 : public vtkCMBModelWriterV2
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
