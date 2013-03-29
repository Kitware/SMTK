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
// .NAME vtkModelItem - Base class for a model object.
// .SECTION Description
// This class is meant to be used as the base class for model entities
// for storing associations to other model entities and iterating 
// over the associated objects.

#ifndef __vtkModelItem_h
#define __vtkModelItem_h

#include "vtkDiscreteModelModule.h" // For export macro
#include "../Serialize/vtkSerializableObject.h"

struct vtkModelItemInternals;
class vtkIdList;
class vtkInformation;
class vtkInformationObjectBaseKey;
class vtkModelItemIterator;

class VTKDISCRETEMODEL_EXPORT vtkModelItem : public vtkSerializableObject
{
public:
  vtkTypeMacro(vtkModelItem,vtkSerializableObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkGetObjectMacro(Properties, vtkInformation);
  
  // Description:
  // Return a vtkModelItemIterator to iterate through the
  // associated model entities of type itemType.  Note that
  // this cannot be used to iterate over the model faces that
  // are adjacent to a model region as they are associated 
  // through use objects instead of directly being associated
  // with each other.
  vtkModelItemIterator* NewIterator(int itemType);

  // Description:
  // Get the type of object that this entity is.  The base
  // enumeration of types are located in vtkModel.  Extensions
  // should separately enumerate their types.
  virtual int GetType() = 0;

  // Description:
  // Return the number of entities of type itemType that are
  // directly associated with this model entity.
  int GetNumberOfAssociations(int itemType);

  // Description:
  // Reads the state of an instance from an archive OR
  // writes the state of an instance to an archive. See
  // the documentation for this class for details.
  virtual void Serialize(vtkSerializer* ser);

protected:
  vtkModelItem();
  virtual ~vtkModelItem();

  // Description:
  // Add an association between this object and item.  This also
  // then calls AddReverseAssociation.
  void AddAssociation(vtkModelItem* item);

  // Description:
  // Remove an association between this object and items.  This also
  // then calls RemoveReverseAssociation for each item.
  void RemoveAssociation(vtkModelItem* item);
  void RemoveAllAssociations(int itemType);

  // Description:
  // Add an association between this object and item at a specified
  // ordered index.  This also then calls AddReverseAssociation.
  void AddAssociationInPosition(int index,
                                vtkModelItem* item);

  // Description:
  // Add an association between this object and item.  This also
  // then calls AddReverseAssociation.  Can be used in constructors
  // since GetType() is a virtual function that is not expected to
  // work in the constructor so we can use myType.
  void AddAssociationToType(vtkModelItem* item, int myType);


  // Description:
  // Since every association is symmetric, this is used to get
  // create the reverse association.  When an association
  // is added through AddAssociation 
  // then this function automatically gets rid of the reverse
  // association.
  void AddReverseAssociationToType(vtkModelItem* item, int itemType);

  // Description:
  // Since every association is symmetric, this is used to get
  // rid of the reverse association.  When an association
  // is removed through RemoveAssociation or RemoveAllAssociations
  // then this function automatically gets rid of the reverse
  // association.
  void RemoveReverseAssociationToType(vtkModelItem* item, int itemType);

  // Description:
  // Returns a list of item types that are stored.
  void GetItemTypesList(vtkIdList * ItemTypes);
  
private:
  vtkModelItem(const vtkModelItem&);  // Not implemented.
  void operator=(const vtkModelItem&);  // Not implemented.

  vtkInformation* Properties;
  vtkModelItemInternals* Internal;
  
//BTX
  friend class vtkModelItemListIterator;
  friend class vtkXMLModelReader;
//ETX
};

#endif

