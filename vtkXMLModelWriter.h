/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkCMBXMLModelWriter.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkCMBXMLModelWriter - Writes an XML archive to output stream
// .SECTION Description
// Given a vector of vtkSerializableObject, vtkCMBXMLModelWriter writes
// the object graph in an XML format. All objects are written in a flat
// collection under the root element. All references to these objects
// are stored using elements of type Pointer. All objects in the
// input vector are stored under a RootObjects element. For example:
// \code
// <ConceptualModel version="1">
//   <RootObjects>
//     <Item type="Pointer" to_id="1"/>
//   </RootObjects>
//   <Object type="vtkvtkCMBShell" id="1">
//     <Properties type="vtkInformation">
//       <vtkConceptualModelItem::COLOR values="0 0 0 0" length="4"/>
//     </Properties>
//     <Associations type="vtkSerializableObjectVectorMap">
//       <Key_1 type="vtkSerializableObjectVector">
//         <Item type="Pointer" to_id="2"/>
//       </Key_1>
//     </Associations>
//   </Object>
//   <Object type="vtkvtkCMBModelFaceUse" id="2">
//     <Properties type="vtkInformation">
//       <vtkConceptualModelItem::COLOR values="1 0 0 0" length="4"/>
//     </Properties>
//     <Associations type="vtkSerializableObjectVectorMap">
//       <Key_0 type="vtkSerializableObjectVector">
//         <Item type="Pointer" to_id="1"/>
//       </Key_0>
//     </Associations>
//   </Object>
// </ConceptualModel>
// \endcode

#ifndef __vtkCMBXMLModelWriter_h
#define __vtkCMBXMLModelWriter_h

#include "Serialize/vtkSerializer.h"

#include <vector> // Vector of smart pointers
#include <vtksys/ios/sstream>
#include "vtkSmartPointer.h" // Vector of smart pointers

//BTX
struct vtkCMBXMLModelWriterInternals;
class vtkXMLElement;
class vtkSerializableObject;
//ETX

class VTK_EXPORT vtkCMBXMLModelWriter : public vtkSerializer
{
public:
  static vtkCMBXMLModelWriter *New();
  vtkTypeRevisionMacro(vtkCMBXMLModelWriter,vtkSerializer);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // This method returns true if the serializer is an output
  // serializer (writer). Returns true.
  virtual bool IsWriting() {return true;}

//BTX
  // Description:
  // This is the main entry point used to write a vector of
  // objects to the XML archive. The rootName is the name
  // of the root XML element in the archive. This name is
  // not used when restoring the archive so it is for information
  // only. For example:
  // \code
  // vtkSmartPointer<vtkvtkCMBShell> shell = vtkSmartPointer<vtkvtkCMBShell>::New();
  // vtkvtkCMBModelFaceUse* fu1 = vtkvtkCMBModelFaceUse::New();
  // shell->AddModelFaceUse(fu1);
  // 
  // vtkSmartPointer<vtkvtkCMBModelFaceUse> fu2 = 
  //   vtkSmartPointer<vtkvtkCMBModelFaceUse>::New();
  // shell->AddModelFaceUse(fu2);
  // fu2->SetColor(1, 0, 0, 0);
  // 
  // vtkSmartPointer<vtkCMBXMLModelWriter> writer = 
  //   vtkSmartPointer<vtkCMBXMLModelWriter>::New();
  // vtksys_ios::ostringstream ostr;
  // writer->SetArchiveVersion(1);
  // std::vector<vtkSmartPointer<vtkSerializableObject> > objs;
  // objs.push_back(shell);
  // writer->Serialize(ostr, "ConceptualModel", objs);
  // \endcode
  virtual void Serialize(vtksys_ios::ostringstream& ostr, const char* rootName, 
    std::vector<vtkSmartPointer<vtkObject> >& objs);
//ETX

  // Description:
  // Serializes a single integer.
  virtual void Serialize(const char* name, int& val);

  // Description:
  // Serializes an array.
  virtual void Serialize(const char* name, int*& val, unsigned int& length);

  // Description:
  // Serializes a single unsigned long.
  virtual void Serialize(const char* name, unsigned long& val) ;
  
 // Description:
  // Serializes an array.
  virtual void Serialize(const char* name, unsigned long*& val, unsigned int& length);

  // Description:
  // Reads a single vtkIdType.
#if defined(VTK_USE_64BIT_IDS)
  virtual void Serialize(const char* name, vtkIdType& val);
#endif

  // Description:
  // Reads an array.
#if defined(VTK_USE_64BIT_IDS)
  virtual void Serialize(const char* name, vtkIdType*& val, unsigned int& length);
#endif

  // Description:
  // Serializes a single double.
  virtual void Serialize(const char* name, double& val);

  // Description:
  // Serializes an array.
  virtual void Serialize(const char* name, double*& val, unsigned int& length);

  // Description:
  // Serializes a string.
  virtual void Serialize(const char* name, char*& str);

//BTX
  // Description:
  // Serializes a string.
  virtual void Serialize(const char* name, std::string& str);
//ETX

  // Description:
  // Serializes a vtkSerializableObject.
  virtual void Serialize(const char* name, vtkObject*& obj,
    bool weakPtr = false);

  // Description:
  // Serializes a vtkInformationObject.
  virtual void Serialize(const char* name, vtkInformation* info);

//BTX
  // Description:
  // Serializes a vector of vtkSerializableObjects.
  virtual void Serialize(const char* name, 
                         std::vector<vtkSmartPointer<vtkObject> >& objs,
                         bool weakPtr = false);

  // Description:
  // Serializes a map from int to vector of vtkSerializableObject.
  virtual void Serialize(const char* name, 
    std::map<int, std::vector<vtkSmartPointer<vtkObject> > >& objs);
//ETX

protected:
  vtkCMBXMLModelWriter();
  ~vtkCMBXMLModelWriter();

  virtual void Initialize(const char* name);
  virtual vtkIdType Serialize(vtkSerializableObject*& obj);
  
private:
  vtkCMBXMLModelWriter(const vtkCMBXMLModelWriter&);  // Not implemented.
  void operator=(const vtkCMBXMLModelWriter&);  // Not implemented.

//BTX
  virtual vtkXMLElement* CreateDOM(const char* rootName, 
    std::vector<vtkSmartPointer<vtkObject> >& objs);
//ETX

  void SetRootElement(vtkXMLElement*);
  
  vtkCMBXMLModelWriterInternals* Internal;
  vtkXMLElement* RootElement;
};

#endif
