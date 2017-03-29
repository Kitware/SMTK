//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkModelItem - Base class for a model object.
// .SECTION Description
// This class is meant to be used as the base class for model entities
// for storing associations to other model entities and iterating
// over the associated objects.

#ifndef __smtkdiscrete_vtkModelItem_h
#define __smtkdiscrete_vtkModelItem_h

#include "../Serialize/vtkSerializableObject.h"
#include "smtk/bridge/discrete/kernel/vtkSMTKDiscreteModelModule.h" // For export macro


struct vtkModelItemInternals;
class vtkIdList;
class vtkInformation;
class vtkInformationObjectBaseKey;
class vtkModelItemIterator;

class VTKSMTKDISCRETEMODEL_EXPORT vtkModelItem : public vtkSerializableObject
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
  void GetItemTypesList(vtkIdList * itemTypes);

private:
  vtkModelItem(const vtkModelItem&);  // Not implemented.
  void operator=(const vtkModelItem&);  // Not implemented.

  vtkInformation* Properties;
  vtkModelItemInternals* Internal;

  friend class vtkModelItemListIterator;
  friend class vtkXMLModelReader;

};

#endif

