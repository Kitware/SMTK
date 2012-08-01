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
// .NAME vtkDiscreteModelRegion - 
// .SECTION Description

#ifndef __vtkDiscreteModelRegion_h
#define __vtkDiscreteModelRegion_h

#include "Model/vtkModelRegion.h"
#include "vtkDiscreteModelGeometricEntity.h"

class vtkInformationStringKey;

class VTK_EXPORT vtkDiscreteModelRegion : public vtkModelRegion,
  public vtkDiscreteModelGeometricEntity
{
public:
  vtkTypeMacro(vtkDiscreteModelRegion,vtkModelRegion);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual bool Destroy();

  // Description:
  // Functions for using a point inside the region used
  // for meshing.
  static vtkInformationDoubleVectorKey* POINTINSIDE();
  void SetPointInside(double PointInside[3]);
  double* GetPointInside();


  // Description:
  // Functions for caching the solid file the region is
  // created from.
  static vtkInformationStringKey* SOLIDFILENAME();
  void SetSolidFileName(const char* filename);
  const char* GetSolidFileName();

  // Description:
  // Reads the state of an instance from an archive OR
  // writes the state of an instance to an archive. See
  // the documentation for this class for details.
  virtual void Serialize(vtkSerializer* ser);

protected:
  static vtkDiscreteModelRegion *New();
//BTX
  friend class vtkDiscreteModel;
//ETX
  vtkDiscreteModelRegion();
  virtual ~vtkDiscreteModelRegion();
  virtual vtkModelEntity* GetThisModelEntity();

private:
  vtkDiscreteModelRegion(const vtkDiscreteModelRegion&);  // Not implemented.
  void operator=(const vtkDiscreteModelRegion&);  // Not implemented.
};

#endif

