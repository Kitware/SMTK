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
// .NAME vtkModelEntityOperatorBase - Change properties of model entities.
// .SECTION Description
// Operator to change the color (RGBA), user name and/or visibility of a
// vtkModelEntity.  Also can be used to build or destroy a
// vtkDiscreteModelEntityGroup.

#ifndef __vtkModelEntityOperatorBase_h
#define __vtkModelEntityOperatorBase_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkObject.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModel;
class vtkModelEntity;

class VTKCMBDISCRETEMODEL_EXPORT vtkModelEntityOperatorBase : public vtkObject
{
public:
  static vtkModelEntityOperatorBase * New();
  vtkTypeMacro(vtkModelEntityOperatorBase,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/get the model entity unique persistent id to operate on.
  // This is required to be set before calling Operate.
  vtkGetMacro(IsIdSet, int);
  vtkGetMacro(Id, vtkIdType);
  void SetId(vtkIdType id);

  // Description:
  // Set/get the model entity type.  This is not required to be
  // set before calling Operate but makes the lookup of the
  // entity faster.  Derived classes can modify SetItemType
  // if they only deal with specific model entity types.
  vtkGetMacro(IsItemTypeSet, int);
  vtkGetMacro(ItemType, int);
  virtual void SetItemType(int itemType);

  // Description:
  // Functions to set/get the visibility that will be set.
  vtkGetMacro(IsVisibilitySet, int);
  vtkGetMacro(Visibility, int);
  void SetVisibility(int visibility);

  // Description:
  // Functions to set/get the Pickable that will be set.
  vtkGetMacro(IsPickableSet, int);
  vtkGetMacro(Pickable, int);
  void SetPickable(int pickable);

  // Description:
  // Functions to set/get the ShowTexture that will be set.
  vtkGetMacro(IsShowTextureSet, int);
  vtkGetMacro(ShowTexture, int);
  void SetShowTexture(int show);

  // Description:
  // Functions to set/get the RGBA that will be set.
  vtkGetMacro(IsRGBASet, int);
  vtkGetMacro(IsRepresentationRGBASet, int);
//BTX
  vtkGetMacro(RGBA, double*);
  vtkGetMacro(RepresentationRGBA, double*);
//ETX
  void SetRGBA(double *Color);
  void SetRGBA(double R, double G, double B, double A);
  void SetRepresentationRGBA(double *Color);
  void SetRepresentationRGBA(double R, double G, double B, double A);

  // Description:
  // Get/Set the name of the ModelEntity.
  vtkSetStringMacro(UserName);
  vtkGetStringMacro(UserName);

//BTX
  // Description:
  // Return the model entity.
  virtual vtkModelEntity* GetModelEntity(vtkDiscreteModel* Model);


  // Description:
  // Do the basic operation to modify the model.  This should be
  // able to be done on both the server and the client.
  virtual bool Operate(vtkDiscreteModel* Model);
//ETX

protected:
  vtkModelEntityOperatorBase();
  virtual ~vtkModelEntityOperatorBase();

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModel* Model);

  // Description:
  // The type of the model entity to be operated on.  This is not
  // required to be set before calling Operate but makes the lookup of the
  // entity faster.  This is protected so that derived classes can
  // set these values in the constructor if desired.
  int ItemType;
  int IsItemTypeSet;

  // Description:
  // Operator will modify Representation color stored in vtkProperty
  // of the ModelWrapper's composite child.
  double RepresentationRGBA[4];
  int IsRepresentationRGBASet;

private:
  // Description:
  // The unique persistent id of the model entity to be operated on.
  // This is required to be set before calling Operate.
  vtkIdType Id;
  int IsIdSet;

  // Description:
  // Operator will modify the vtkModelEntity to be invisible
  // (Visibility=0) or visible (should be Visibility=1).
  int Visibility;
  int IsVisibilitySet;
  int Pickable;
  int IsPickableSet;
  int ShowTexture;
  int IsShowTextureSet;

  // Description:
  // Operator will modify RGBA of the vtkModelEntity.
  double RGBA[4];
  int IsRGBASet;

  // Description:
  // Operator will modify the UserName of the vtkModelEntity
  // if UserName is not NULL.
  char* UserName;

  vtkModelEntityOperatorBase(const vtkModelEntityOperatorBase&);  // Not implemented.
  void operator=(const vtkModelEntityOperatorBase&);  // Not implemented.
};

#endif
