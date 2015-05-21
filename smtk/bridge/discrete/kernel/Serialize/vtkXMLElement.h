//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkXMLElement represents an XML element and those nested inside.
// .SECTION Description
// This is used by vtkModelXMLParser to represent an XML document starting
// at the root element.
#ifndef __smtkdiscrete_vtkXMLElement_h
#define __smtkdiscrete_vtkXMLElement_h

#include "smtk/bridge/discrete/kernel/vtkSMTKDiscreteModelModule.h" // For export macro
#include "vtkObject.h"
#include "vtkStdString.h" // needed for vtkStdString.


class vtkCollection;
class vtkModelXMLParser;

//BTX
struct vtkXMLElementInternals;
//ETX

class VTKSMTKDISCRETEMODEL_EXPORT vtkXMLElement : public vtkObject
{
public:
  vtkTypeMacro(vtkXMLElement,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkXMLElement* New();

  // Description:
  // Set/Get the name of the element.  This is its XML tag.
  // (<Name />).
  vtkSetStringMacro(Name);
  vtkGetStringMacro(Name);

  // Description:
  // Get the id of the element. This is assigned by the XML parser
  // and can be used as an identifier to an element.
  vtkGetStringMacro(Id);

  // Description:
  // Get the attribute with the given name.  If it doesn't exist,
  // returns 0.
  const char* GetAttribute(const char* name);

  // Description:
  // Get the character data for the element.
  const char* GetCharacterData();

  // Description:
  // Get the attribute with the given name converted to a scalar
  // value.  Returns whether value was extracted.
  unsigned int GetScalarAttribute(const char* name, int* value);
  unsigned int GetScalarAttribute(const char* name, unsigned int* value);
  unsigned int GetScalarAttribute(const char* name, unsigned long* value);
  unsigned int GetScalarAttribute(const char* name, float* value);
  unsigned int GetScalarAttribute(const char* name, double* value);
#if defined(VTK_USE_64BIT_IDS)
  unsigned int GetScalarAttribute(const char* name, vtkIdType* value);
#endif

  // Description:
  // Get the attribute with the given name converted to a scalar
  // value.  Returns length of vector read.
  unsigned int GetVectorAttribute(const char* name, unsigned int length, int* value);
  unsigned int GetVectorAttribute(const char* name, unsigned int length, unsigned int* value);
  unsigned int GetVectorAttribute(const char* name, unsigned int length, unsigned long* value);
  unsigned int GetVectorAttribute(const char* name, unsigned int length, float* value);
  unsigned int GetVectorAttribute(const char* name, unsigned int length, double* value);
#if defined(VTK_USE_64BIT_IDS)
  unsigned int GetVectorAttribute(const char* name, unsigned int length, vtkIdType* value);
#endif

  // Description:
  // Get the character data converted to a scalar
  // value.  Returns length of vector read.
  unsigned int GetCharacterDataAsVector(unsigned int length, int* value);
  unsigned int GetCharacterDataAsVector(unsigned int length, unsigned long* value);
  unsigned int GetCharacterDataAsVector(unsigned int length, float* value);
  unsigned int GetCharacterDataAsVector(unsigned int length, double* value);
#if defined(VTK_USE_64BIT_IDS)
  unsigned int GetCharacterDataAsVector(unsigned int length, vtkIdType* value);
#endif

  // Description:
  // Get the parent of this element.
  vtkXMLElement* GetParent();

  // Description:
  // Get the number of elements nested in this one.
  unsigned int GetNumberOfNestedElements();

  // Description:
  // Get the element nested in this one at the given index.
  vtkXMLElement* GetNestedElement(unsigned int index);

  // Description:
  // Find a nested element with the given id.
  // Note that this searches only the immediate children of this
  // vtkXMLElement.
  vtkXMLElement* FindNestedElement(const char* id);

  // Description:
  // Locate a nested element with the given tag name.
  vtkXMLElement* FindNestedElementByName(const char* name);

  // Description:
  // Removes all nested elements.
  void RemoveAllNestedElements();

  // Description:
  // Remove a particular element.
  void RemoveNestedElement(vtkXMLElement*);

  // Description:
  // Lookup the element with the given id, starting at this scope.
  vtkXMLElement* LookupElement(const char* id);

  // Description:
  // Given it's name and value, add an attribute.
  void AddAttribute(const char* attrName, const char* attrValue);
  void AddAttribute(const char* attrName, unsigned int attrValue);
  void AddAttribute(const char* attrName, double attrValue);
  void AddAttribute(const char* attrName, int attrValue);
  void AddAttribute(const char* attrName, unsigned long attrValue);
  void AddAttribute(const char* attrName, double* vals, unsigned int length);
  void AddAttribute(const char* attrName, int* vals, unsigned int length);
  void AddAttribute(const char* attrName, unsigned long* vals, unsigned int length);
#if defined(VTK_USE_64BIT_IDS)
  void AddAttribute(const char* attrName, vtkIdType attrValue);
  void AddAttribute(const char* attrName, vtkIdType* vals, unsigned int length);
#endif

  // Description:
  // Given it's name and value, set an attribute.
  // If an attribute with the given name already exists,
  // it replaces the old attribute.
  // chars that need to be XML escaped will be done so internally
  // for example " will be converted to &quot;
  void SetAttribute(const char* attrName, const char* attrValue);

  // Description:
  // Add a sub-element. The parent element keeps a reference to
  // sub-element. If setParent is true, the nested element's parent
  // is set as this.
  void AddNestedElement(vtkXMLElement* element, int setPrent);
  void AddNestedElement(vtkXMLElement* element);

  // Description:
  // Serialize (as XML) in the given stream.
  void PrintXML(ostream& os, vtkIndent indent);
  void PrintXML();

  // Description:
  // Merges another element with this one, both having the same name.
  // If any attribute, character data or nested element exists in both,
  // the passed in one will override this one's.  If they don't exist,
  // they'll be added.  If nested elements have the same names, the
  // optional attributeName maybe passed in as another criteria to determine
  // what to merge in case of same names.
  void Merge(vtkXMLElement* element, const char* attributeName);

  // Description:
  // Similar to DOM sepecific getElementsByTagName().
  // Returns a list of vtkXMLElements with the given name in the order
  // in which they will be encountered in a preorder traversal
  // of the sub-tree under this node. The elements are populated
  // in the vtkCollection passed as an argument.
  void GetElementsByName(const char* name, vtkCollection* elements);

  // Description:
  // Encode a string.
  static vtkStdString Encode(const char* plaintext);

protected:
  vtkXMLElement();
  ~vtkXMLElement();

  vtkXMLElementInternals* Internal;

  char* Name;
  char* Id;

  // The parent of this element.
  vtkXMLElement* Parent;

  // Method used by vtkModelXMLParser to setup the element.
  vtkSetStringMacro(Id);
  void ReadXMLAttributes(const char** atts);
  void AddCharacterData(const char* data, int length);


  // Internal utility methods.
  vtkXMLElement* LookupElementInScope(const char* id);
  vtkXMLElement* LookupElementUpScope(const char* id);
  void SetParent(vtkXMLElement* parent);

  //BTX
  friend class vtkModelXMLParser;
  //ETX

private:
  vtkXMLElement(const vtkXMLElement&);  // Not implemented.
  void operator=(const vtkXMLElement&);  // Not implemented.
};

#endif
