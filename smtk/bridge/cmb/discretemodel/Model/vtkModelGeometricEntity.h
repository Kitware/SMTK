//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkModelGeometricEntity -
// .SECTION Description
// The vtkPolyData/vtkSMProxy object that is stored on the
// server/client, respectively in the GEOMETRIC() vtkInformationObjectBaseKey.
// Use a SafeDownCast() to determine which one.

#ifndef __smtkcmb_vtkModelGeometricEntity_h
#define __smtkcmb_vtkModelGeometricEntity_h

#include "vtkSMTKDiscreteModelModule.h" // For export macro
#include "vtkModelEntity.h"


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

