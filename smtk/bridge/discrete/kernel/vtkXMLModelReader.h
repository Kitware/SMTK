//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkXMLModelReader - Reads an XML archive from input stream
// .SECTION Description
// This concrete sub-class of vtkSerializer reads an XML archive from
// an input stream and create a collection of sub-classes of
// vtkSerializableObject. For example:
// \code
//  std::vector<vtkSmartPointer<vtkSerializableObject> > objs;
//  ifstream ifs(filename);
//
//  vtkSmartPointer<vtkXMLModelReader> reader =
//    vtkSmartPointer<vtkXMLModelReader>::New();
//  reader->Serialize(istr, "ConceptualModel", objs);
// .. Do something with objs
// \endcode
// See vtkDiscreteModelWriter for details about the XML format.
// .SECTION See Also
// vtkSerializer vtkXMLArchiveWriter

#ifndef __smtkdiscrete_vtkXMLModelReader_h
#define __smtkdiscrete_vtkXMLModelReader_h

#include "Serialize/vtkSerializer.h"
#include "smtk/bridge/discrete/kernel/vtkSMTKDiscreteModelModule.h" // For export macro

class vtkDiscreteModel;
class vtkDiscreteModelEntityGroup;
class vtkCollection;
class vtkXMLElement;
class vtkModelEdge;
class vtkModelEdgeUse;
class vtkModelFace;
class vtkModelFaceUse;
class vtkModelLoopUse;
class vtkModelRegion;
class vtkModelShellUse;
class vtkModelVertex;
class vtkModelVertexUse;

class VTKSMTKDISCRETEMODEL_EXPORT vtkXMLModelReader : public vtkSerializer
{
public:
  static vtkXMLModelReader* New();
  vtkTypeMacro(vtkXMLModelReader, vtkSerializer);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // This method returns true if the serializer is an output
  // serializer (writer). Returns false.
  bool IsWriting() override { return false; }

  // Description:
  // Main entry point called to read an XML archive.
  // It populates the obj vector with the root objects in the
  // archive (under the RootObjects element).
  virtual void Serialize(istream& istr, const char* rootName);

  // Description:
  // Reads a single integer.
  void Serialize(const char* name, int& val) override;

  // Description:
  // Reads an array.
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
  // Reads a single double.
  void Serialize(const char* name, double& val) override;

  // Description:
  // Reads an array.
  void Serialize(const char* name, double*& val, unsigned int& length) override;

  // Description:
  // Serializes a string.
  void Serialize(const char* name, char*& str) override;

  // Description:
  // Serializes a string.
  void Serialize(const char* name, std::string& str) override;

  // Description:
  // Reads a vtkSerializableObject.
  void Serialize(const char* name, vtkObject*& obj, bool weakPtr = false) override;

  // Description:
  // Reads a vtkInformationObject. Note that only keys registered
  // with the vtkInformationKeyMap are restored.
  void Serialize(const char* name, vtkInformation* info) override;

  // Description:
  // Reads a vector of vtkSerializableObjects.
  virtual void Serialize(const char* name);

  // Description:
  // Set/get methods for the Model.  If Model is not set before Serialize
  // is called then a Model will be created internally.

  vtkGetMacro(Model, vtkDiscreteModel*);

  void SetModel(vtkDiscreteModel*);

  // Description:
  // Serializes a vector of vtkSerializableObjects.
  void Serialize(const char* name, std::vector<vtkSmartPointer<vtkObject> >& objs,
    bool weakPtr = false) override;

  // Description:
  // Serializes a map from int to vector of vtkSerializableObject.
  void Serialize(
    const char* name, std::map<int, std::vector<vtkSmartPointer<vtkObject> > >& objs) override;

  // Description:
  // Serialize the model.
  virtual void SerializeModel();

  // Description:
  // Get all XML elements that have a "type" attribute matching type.  The
  // elements collection will be appended with any matching objects.
  void GetElementsByType(vtkXMLElement* element, const char* type, vtkCollection* elements);

protected:
  vtkXMLModelReader();
  ~vtkXMLModelReader() override;

  // Description:
  // Gets the associations.
  void GetAssociations(vtkXMLElement* elem, std::map<int, std::vector<vtkIdType> >& objs);

  // Description:
  // Gets the associations for a given entity type.
  void GetAssociations(vtkXMLElement* elem, int entityType, std::vector<vtkIdType>& objs);

  vtkModelLoopUse* ConstructModelLoopUse(int id);
  vtkModelFace* ConstructModelFace(int id);
  vtkModelFaceUse* ConstructModelFaceUse(int id);
  vtkModelRegion* ConstructModelRegion(int id);
  vtkModelEdge* ConstructModelEdge(int id);
  vtkModelEdgeUse* ConstructModelEdgeUse(int id);
  vtkModelShellUse* ConstructModelShellUse(int id);
  vtkModelVertex* ConstructModelVertex(int id);
  vtkModelVertexUse* ConstructModelVertexUse(int id);
  vtkDiscreteModelEntityGroup* ConstructModelEntityGroup(int id);

private:
  vtkXMLModelReader(const vtkXMLModelReader&); // Not implemented.
  void operator=(const vtkXMLModelReader&);    // Not implemented.

  int ParseStream(istream& str);
  vtkObject* ReadObject(vtkIdType id, bool weakPtr);
  void SetRootElement(vtkXMLElement* re);

  vtkXMLElement* RootElement;
  vtkXMLElement* CurrentElement;
  vtkDiscreteModel* Model;
};

#endif
