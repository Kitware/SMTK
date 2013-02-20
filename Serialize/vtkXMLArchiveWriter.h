//=============================================================================
//   This file is part of VTKEdge. See vtkedge.org for more information.
//
//   Copyright (c) 2010 Kitware, Inc.
//
//   VTKEdge may be used under the terms of the BSD License
//   Please see the file Copyright.txt in the root directory of
//   VTKEdge for further information.
//
//   Alternatively, you may see: 
//
//   http://www.vtkedge.org/vtkedge/project/license.html
//
//
//   For custom extensions, consulting services, or training for
//   this or any other Kitware supported open source project, please
//   contact Kitware at sales@kitware.com.
//
//
//=============================================================================
// .NAME vtkXMLArchiveWriter - Writes an XML archive to output stream
// .SECTION Description
// Given a vector of vtkObject, vtkXMLArchiveWriter writes
// the object graph in an XML format. Note, to be written an object
// must either be a subclass of vtkSerializableObject or have a helper
// registered with vtkSerializationHelperMap which knows how to
// serialize the object type.  All objects are written in a flat collection
// under the root element. All references to these objects are stored using
// elements of type Pointer. All objects in the input vector are stored under
// a RootObjects element.  For example:
// \code
// <ConceptualModel version="1">
//   <RootObjects>
//     <Item type="Pointer" to_id="1"/>
//   </RootObjects>
//   <Object type="vtkCmbShell" id="1">
//     <Properties type="vtkInformation">
//       <vtkConceptualModelItem::COLOR values="0 0 0 0" length="4"/>
//     </Properties>
//     <Associations type="vtkObjectVectorMap">
//       <Key_1 type="vtkObjectVector">
//         <Item type="Pointer" to_id="2"/>
//       </Key_1>
//     </Associations>
//   </Object>
//   <Object type="vtkCmbModelFaceUse" id="2">
//     <Properties type="vtkInformation">
//       <vtkConceptualModelItem::COLOR values="1 0 0 0" length="4"/>
//     </Properties>
//     <Associations type="vtkObjectVectorMap">
//       <Key_0 type="vtkObjectVector">
//         <Item type="Pointer" to_id="1"/>
//       </Key_0>
//     </Associations>
//   </Object>
// </ConceptualModel>
// \endcode
// .SECTION See Also
// vtkSerializer

#ifndef __vtkXMLArchiveWriter_h
#define __vtkXMLArchiveWriter_h

#include "vtkDiscreteModelModule.h" // For export macro
#include "vtkSerializer.h"

#include <vector> // Vector of smart pointers
#include "vtkSmartPointer.h" // Vector of smart pointers

//BTX
struct vtkXMLArchiveWriterInternals;
class vtkXMLElement;
//ETX

class VTKDISCRETEMODEL_EXPORT vtkXMLArchiveWriter : public vtkSerializer
{
public:
  static vtkXMLArchiveWriter *New();
  vtkTypeMacro(vtkXMLArchiveWriter,vtkSerializer);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // This method returns true if the serializer is an output
  // serializer (writer). Returns true.
  virtual bool IsWriting() {return true;}

  // Description:
  // This is the main entry point used to write a vector of
  // objects to the XML archive. The rootName is the name
  // of the root XML element in the archive. This name is
  // not used when restoring the archive so it is for information
  // only. For example:
  // \code
  // vtkObjectTreeTransformableNode *root =
  //   vtkObjectTreeTransformableNode::New();
  // root->SetName("RootNode");
  //
  // vtkObjectTreeTransformableNode *child =
  //   vtkObjectTreeTransformableNode::New();
  // root->SetName("Child Node");
  // root->AddChild( child );
  //
  // vtkSmartPointer<vtkXMLArchiveWriter> writer =
  //   vtkSmartPointer<vtkXMLArchiveWriter>::New();
  // vtksys_ios::ostringstream ostr;
  // writer->SetArchiveVersion(1);
  // std::vector<vtkSmartPointer<vtkObject> > objs;
  // objs.push_back(root);
  // writer->Serialize(ostr, "ObjectTree", objs);
  // \endcode
  virtual void Serialize(ostream& ostr, const char* rootName,
    std::vector<vtkSmartPointer<vtkObject> >& objs);

  // Description:
  // Additional entry point, used to write a vector of
  // objects to the XML archive, in the form of a vtkXMLElement which
  // is assumed to have been allocated by the caller.
  virtual void Serialize(vtkXMLElement* elem, const char* rootName,
    std::vector<vtkSmartPointer<vtkObject> >& objs);

  // Description:
  // Additional entry point, used to write a single (root) object
  // to the XML archive, in the form of a vtkXMLElement which
  // is assumed to have been allocated by the caller.
  virtual void Serialize(vtkXMLElement* elem, const char* rootName,
    vtkObject *objs);

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

  // Description:
  // Serializes a string.
  virtual void Serialize(const char* name, std::string& str);

  // Description:
  // Serializes a vtkObject.  Note, the object must either be subclass of
  // vtkSerializableObject or have a helper registered with
  // vtkSerializableObjectManager which knows how to serialize the object
  // type. The weakPtr parameter is actually ignored (we don't write an
  // attribute indicating it is weak, if it is) because the reading code will
  // have the same parameter specifying the pointer is weak. Here for symmetry.
  virtual void Serialize(const char* name, vtkObject*& object, bool weakPtr = false);

  // Description:
  // Serializes a vtkInformationObject.
  virtual void Serialize(const char* name, vtkInformation* info);

  // Description:
  // Serializes a vector of vtkObjects.  The weakPtr parameter
  // is actually ignored (we don't write an attribute indicating it is weak,
  // if it is) because the reading code will have the same paremter specifying
  // the  pointer is weak.  Here for symmetry.
  virtual void Serialize(const char* name,
    std::vector<vtkSmartPointer<vtkObject> >& objs,
    bool weakPtr = false);

  // Description:
  // Serializes a map from int to vector of vtkObjects.
  virtual void Serialize(const char* name,
    std::map<int, std::vector<vtkSmartPointer<vtkObject> > >& objs);

protected:
  vtkXMLArchiveWriter();
  ~vtkXMLArchiveWriter();

  // Description:
  // Try to serialize the given vtkObject if it has not been serialized already.
  // If the object could not be serialized then return 0, else return the
  // id for serialized object's xml tag.  The id is always greater than or
  // equal to 1.  If the object has already been serialized then this will just
  // lookup and return the id.
  virtual unsigned int Serialize(vtkObject*& obj);

private:
  vtkXMLArchiveWriter(const vtkXMLArchiveWriter&);  // Not implemented.
  void operator=(const vtkXMLArchiveWriter&);  // Not implemented.

  virtual void CreateDOM(const char* rootName,
    std::vector<vtkSmartPointer<vtkObject> >& objs);

  void SetRootElement(vtkXMLElement*);

  // Description:
  // Serializes a vtkInformationObject.
  virtual void Serialize(vtkXMLElement* elem, vtkInformation* info);

  vtkXMLArchiveWriterInternals* Internal;
  vtkXMLElement* RootElement;
};

#endif
