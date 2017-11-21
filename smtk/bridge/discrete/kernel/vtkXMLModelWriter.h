//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkXMLModelWriter - Writes an XML archive to output stream
// .SECTION Description
// Given a vector of vtkSerializableObject, vtkXMLModelWriter writes
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
//       <vtkConceptualModelItem::ColorType values="0 0 0 0" length="4"/>
//     </Properties>
//     <Associations type="vtkSerializableObjectVectorMap">
//       <Key_1 type="vtkSerializableObjectVector">
//         <Item type="Pointer" to_id="2"/>
//       </Key_1>
//     </Associations>
//   </Object>
//   <Object type="vtkvtkDiscreteModelFaceUse" id="2">
//     <Properties type="vtkInformation">
//       <vtkConceptualModelItem::ColorType values="1 0 0 0" length="4"/>
//     </Properties>
//     <Associations type="vtkSerializableObjectVectorMap">
//       <Key_0 type="vtkSerializableObjectVector">
//         <Item type="Pointer" to_id="1"/>
//       </Key_0>
//     </Associations>
//   </Object>
// </ConceptualModel>
// \endcode

#ifndef __smtkdiscrete_vtkXMLModelWriter_h
#define __smtkdiscrete_vtkXMLModelWriter_h

#include "Serialize/vtkSerializer.h"
#include "smtk/bridge/discrete/kernel/vtkSMTKDiscreteModelModule.h" // For export macro

#include "vtkSmartPointer.h" // Vector of smart pointers
#include <sstream>
#include <vector> // Vector of smart pointers

struct vtkXMLModelWriterInternals;
class vtkXMLElement;
class vtkSerializableObject;

class VTKSMTKDISCRETEMODEL_EXPORT vtkXMLModelWriter : public vtkSerializer
{
public:
  static vtkXMLModelWriter* New();
  vtkTypeMacro(vtkXMLModelWriter, vtkSerializer);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // This method returns true if the serializer is an output
  // serializer (writer). Returns true.
  bool IsWriting() override { return true; }

  // Description:
  // This is the main entry point used to write a vector of
  // objects to the XML archive. The rootName is the name
  // of the root XML element in the archive. This name is
  // not used when restoring the archive so it is for information
  // only. For example:
  // \code
  // vtkSmartPointer<vtkvtkCMBShell> shell = vtkSmartPointer<vtkvtkCMBShell>::New();
  // vtkvtkDiscreteModelFaceUse* fu1 = vtkvtkDiscreteModelFaceUse::New();
  // shell->AddModelFaceUse(fu1);
  //
  // vtkSmartPointer<vtkvtkDiscreteModelFaceUse> fu2 =
  //   vtkSmartPointer<vtkvtkDiscreteModelFaceUse>::New();
  // shell->AddModelFaceUse(fu2);
  // fu2->SetColor(1, 0, 0, 0);
  //
  // vtkSmartPointer<vtkXMLModelWriter> writer =
  //   vtkSmartPointer<vtkXMLModelWriter>::New();
  // std::ostringstream ostr;
  // writer->SetArchiveVersion(1);
  // std::vector<vtkSmartPointer<vtkSerializableObject> > objs;
  // objs.push_back(shell);
  // writer->Serialize(ostr, "ConceptualModel", objs);
  // \endcode
  virtual void Serialize(
    std::ostringstream& ostr, const char* rootName, std::vector<vtkSmartPointer<vtkObject> >& objs);

  // Description:
  // Serializes a single integer.
  void Serialize(const char* name, int& val) override;

  // Description:
  // Serializes an array.
  void Serialize(const char* name, int*& val, unsigned int& length) override;

  // Description:
  // Serializes a single unsigned long.
  void Serialize(const char* name, unsigned long& val) override;

  // Description:
  // Serializes an array.
  void Serialize(const char* name, unsigned long*& val, unsigned int& length) override;

// Description:
// Reads a single vtkIdType.
#if defined(VTK_USE_64BIT_IDS)
  void Serialize(const char* name, vtkIdType& val) override;
#endif

// Description:
// Reads an array.
#if defined(VTK_USE_64BIT_IDS)
  void Serialize(const char* name, vtkIdType*& val, unsigned int& length) override;
#endif

  // Description:
  // Serializes a single double.
  void Serialize(const char* name, double& val) override;

  // Description:
  // Serializes an array.
  void Serialize(const char* name, double*& val, unsigned int& length) override;

  // Description:
  // Serializes a string.
  void Serialize(const char* name, char*& str) override;

  // Description:
  // Serializes a string.
  void Serialize(const char* name, std::string& str) override;

  // Description:
  // Serializes a vtkSerializableObject.
  void Serialize(const char* name, vtkObject*& obj, bool weakPtr = false) override;

  // Description:
  // Serializes a vtkInformationObject.
  void Serialize(const char* name, vtkInformation* info) override;

  // Description:
  // Serializes a vector of vtkSerializableObjects.
  void Serialize(const char* name, std::vector<vtkSmartPointer<vtkObject> >& objs,
    bool weakPtr = false) override;

  // Description:
  // Serializes a map from int to vector of vtkSerializableObject.
  void Serialize(
    const char* name, std::map<int, std::vector<vtkSmartPointer<vtkObject> > >& objs) override;

protected:
  vtkXMLModelWriter();
  ~vtkXMLModelWriter() override;

  virtual void Initialize(const char* name);
  virtual vtkIdType Serialize(vtkSerializableObject*& obj);

private:
  vtkXMLModelWriter(const vtkXMLModelWriter&); // Not implemented.
  void operator=(const vtkXMLModelWriter&);    // Not implemented.

  virtual vtkXMLElement* CreateDOM(
    const char* rootName, std::vector<vtkSmartPointer<vtkObject> >& objs);

  void SetRootElement(vtkXMLElement*);

  vtkXMLModelWriterInternals* Internal;
  vtkXMLElement* RootElement;
};

#endif
