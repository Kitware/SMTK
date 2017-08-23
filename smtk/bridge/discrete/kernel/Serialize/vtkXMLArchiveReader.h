//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkXMLArchiveReader - Reads an XML archive from input stream
// .SECTION Description
// This concrete subclass of vtkSerializer reads an XML archive from
// an input stream and create a collection of sub-classes of vtkObject.
// For example:
// \code
//  std::vector<vtkSmartPointer<vtkObject> > objs;
//  ifstream ifs(filename);
//
//  vtkSmartPointer<vtkXMLArchiveReader> reader =
//    vtkSmartPointer<vtkXMLArchiveReader>::New();
//  reader->Serialize(istr, "ObjectTree", objs);
// .. Do something with objs
// \endcode
// See vtkXMLArchiveWriter for details about the XML format.
// .SECTION See Also
// vtkSerializer vtkXMLArchiveWriter

#ifndef __smtkdiscrete_vtkXMLArchiveReader_h
#define __smtkdiscrete_vtkXMLArchiveReader_h

#include "smtk/bridge/discrete/kernel/vtkSMTKDiscreteModelModule.h" // For export macro
#include "vtkSerializer.h"

class vtkXMLElement;
struct vtkXMLArchiveReaderInternals;

class VTKSMTKDISCRETEMODEL_EXPORT vtkXMLArchiveReader : public vtkSerializer
{
public:
  static vtkXMLArchiveReader* New();
  vtkTypeMacro(vtkXMLArchiveReader, vtkSerializer);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // This method returns true if the serializer is an output
  // serializer (writer). Returns false.
  bool IsWriting() override { return false; }

  // Description:
  // Main entry point called to read an XML archive.
  // It populates the obj vector with the root objects in the
  // archive (under the RootObjects element).
  virtual void Serialize(
    istream& istr, const char* rootName, std::vector<vtkSmartPointer<vtkObject> >& objs);

  // Description:
  // Additional entry point called to read an XML archive from a
  // vtkXMLElement (as opposed to "from a stream"). It
  // populates the obj vector with the root objects in the
  // archive (under the RootObjects element).
  virtual void Serialize(vtkXMLElement* rootElement, const char* rootName,
    std::vector<vtkSmartPointer<vtkObject> >& objs);

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
  // Reads a vtkObject.  weakPtr should be set to true if this reference to the
  // object should NOT be reference counted.
  void Serialize(const char* name, vtkObject*& obj, bool weakPtr = false) override;

  // Description:
  // Reads a vtkInformationObject. Note that only keys registered
  // with the vtkInformationKeyMap are restored.
  void Serialize(const char* name, vtkInformation* info) override;

  // Description:
  // Reads a vector of vtkObjects.
  void Serialize(const char* name, std::vector<vtkSmartPointer<vtkObject> >& objs,
    bool weakPtr = false) override;

  // Description:
  // Reads a map from int to vector of vtkObject.
  void Serialize(
    const char* name, std::map<int, std::vector<vtkSmartPointer<vtkObject> > >& objs) override;

protected:
  vtkXMLArchiveReader();
  ~vtkXMLArchiveReader() override;

private:
  vtkXMLArchiveReader(const vtkXMLArchiveReader&); // Not implemented.
  void operator=(const vtkXMLArchiveReader&);      // Not implemented.

  void Serialize(std::vector<vtkSmartPointer<vtkObject> >& objs);
  int ParseStream(istream& str);
  vtkXMLElement* RootElement;

  // Description:
  // weakPtr is true if the object is NOT to be reference counted.
  vtkObject* ReadObject(int id, bool weakPtr);

  // Description:
  // Reads a vtkInformationObject. Note that only keys registered
  // with the vtkInformationKeyMap are restored.
  virtual void Serialize(vtkXMLElement* elem, vtkInformation* info);

  void SetRootElement(vtkXMLElement* re);

  vtkXMLArchiveReaderInternals* Internal;
};

#endif
