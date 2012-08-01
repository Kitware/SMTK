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
// .NAME vtkDiscreteModelVertex - CMB specific model vertex class.
// .SECTION Description

#ifndef __vtkDiscreteModelVertex_h
#define __vtkDiscreteModelVertex_h

#include "vtkModelVertex.h"
#include "vtkDiscreteModelGeometricEntity.h"

class vtkInformationIdTypeKey;

class VTK_EXPORT vtkDiscreteModelVertex : public vtkModelVertex
{
public:
  vtkTypeMacro(vtkDiscreteModelVertex,vtkModelVertex);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Pure virtual function to get the point location of the
  // model vertex.  Returns true for success and false for failure.
  // Fills the x, y, and z values in xyz if success.
  virtual bool GetPoint(double*);

  // Description:
  // The point id of the model vertex defined on the master polydata.
  // This will not create geometry automatically anymore.
  void SetPointId(vtkIdType Id);
  vtkIdType GetPointId();

  static vtkInformationIdTypeKey* POINTID();

  // Description:
  // Reads the state of an instance from an archive OR
  // writes the state of an instance to an archive. See
  // the documentation for this class for details.
  //virtual void Serialize(vtkSerializer* ser);

  // Description:
  // Create geometry using the point id of the model vertex defined on the master polydata.
  void CreateGeometry();

protected:
  static vtkDiscreteModelVertex* New();
  vtkDiscreteModelVertex();
  virtual ~vtkDiscreteModelVertex();

//BTX
  friend class vtkDiscreteModel;
  friend class vtkModel;
  friend class vtkModelVertexUse;
  friend class vtkModelEdgeUse;
  friend class vtkModelEdge;
  friend class vtkXMLModelReader;
  friend class vtkDiscreteModelWrapper;
  friend class vtkCmbMapToCmbModel;
//ETX


private:
  vtkDiscreteModelVertex(const vtkDiscreteModelVertex&);  // Not implemented.
  void operator=(const vtkDiscreteModelVertex&);  // Not implemented.
};

#endif

