//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkDiscreteModelRegion -
// .SECTION Description

#ifndef __smtkdiscrete_vtkDiscreteModelRegion_h
#define __smtkdiscrete_vtkDiscreteModelRegion_h

#include "smtk/bridge/discrete/kernel/vtkSMTKDiscreteModelModule.h" // For export macro
#include "Model/vtkModelRegion.h"
#include "vtkDiscreteModelGeometricEntity.h"


class vtkInformationStringKey;

class VTKSMTKDISCRETEMODEL_EXPORT vtkDiscreteModelRegion : public vtkModelRegion,
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
  void SetPointInside(double pointInside[3]);
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

