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
// .NAME vtkModelGeometricEntity -
// .SECTION Description
// The vtkPolyData/vtkSMProxy object that is stored on the
// server/client, respectively in the GEOMETRIC() vtkInformationObjectBaseKey.
// Use a SafeDownCast() to determine which one.

#ifndef __vtkModelGeometricEntity_h
#define __vtkModelGeometricEntity_h

#include "vtkSMTKDiscreteModelModule.h" // For export macro
#include "vtkModelEntity.h"
#include "cmbSystemConfig.h"

class vtkIdList;
class vtkProperty;

class VTKSMTKDISCRETEMODEL_EXPORT vtkModelGeometricEntity : public vtkModelEntity
{
public:
  vtkTypeMacro(vtkModelGeometricEntity,vtkModelEntity);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Definition:
  // Get/set the representation for the geometry. On the client
  // this will be a vtkSMProxy* and on the server it will be
  // a vtkDataObject*.
  void SetGeometry(vtkObject* geometry);
  virtual vtkObject* GetGeometry();

  // Description:
  // Static function for getting the information object
  // used to store the geometric representation of a
  // model item.
  static vtkInformationObjectBaseKey* GEOMETRY();

  // Definition:
  // Function to get the bounds of the implemented object.
  // For now it assumes that the geometric entity is a
  // vtkPolyData to get the bounds.
  virtual bool GetBounds(double bounds[6]);

  // Description:
  // Get the model that this vtkModelItem is part of.
  vtkModel* GetModel();

  // Definition:
  // Get/set the display property for this entity.
  void SetDisplayProperty(vtkProperty* prop);
  vtkProperty* GetDisplayProperty();

  // Description:
  // Set/get Pickable (non-zero is Pickable).
  void SetPickable(int pickable);
  int GetPickable();

  // Description:
  // Set/get ShowTexture (non-zero is ShowTexture).
  void SetShowTexture(int show);
  int GetShowTexture();

  // Description:
  // Reads the state of an instance from an archive OR
  // writes the state of an instance to an archive. See
  // the documentation for this class for details.
  virtual void Serialize(vtkSerializer* ser);

protected:
  vtkModelGeometricEntity();
  virtual ~vtkModelGeometricEntity();

  // Description:
  // Determine if this object and everything it aggregates is
  // destroyable/removable (e.g. is it used by anything
  // higher order than itself).
  virtual bool IsDestroyable() = 0;

  // Description:
  // Initialize a default opengl display property
  void InitDefaultDisplayProperty();

//BTX
  friend class vtkModel;
//ETX

private:
  vtkModelGeometricEntity(const vtkModelGeometricEntity&);  // Not implemented.
  void operator=(const vtkModelGeometricEntity&);  // Not implemented.
};

#endif

