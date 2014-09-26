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

#ifndef __vtkXMLArchiveReader_h
#define __vtkXMLArchiveReader_h

#include "vtkSMTKDiscreteModelModule.h" // For export macro
#include "vtkSerializer.h"
#include "cmbSystemConfig.h"

//BTX
class vtkXMLElement;
struct vtkXMLArchiveReaderInternals;
//ETX

class VTKSMTKDISCRETEMODEL_EXPORT vtkXMLArchiveReader : public vtkSerializer
{
public:
  static vtkXMLArchiveReader *New();
  vtkTypeMacro(vtkXMLArchiveReader,vtkSerializer);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // This method returns true if the serializer is an output
  // serializer (writer). Returns false.
  virtual bool IsWriting() {return false;}

  // Description:
  // Main entry point called to read an XML archive.
  // It populates the obj vector with the root objects in the
  // archive (under the RootObjects element).
  virtual void Serialize(istream& istr, const char* rootName,
    std::vector<vtkSmartPointer<vtkObject> >& objs);

  // Description:
  // Additional entry point called to read an XML archive from a
  // vtkXMLElement (as opposed to "from a stream"). It
  // populates the obj vector with the root objects in the
  // archive (under the RootObjects element).
  virtual void Serialize(vtkXMLElement *rootElement, const char* rootName,
    std::vector<vtkSmartPointer<vtkObject> >& objs);

  // Description:
  // Reads a single integer.
  virtual void Serialize(const char* name, int& val);

  // Description:
  // Reads an array.
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
  // Reads a single double.
  virtual void Serialize(const char* name, double& val);

  // Description:
  // Reads an array.
  virtual void Serialize(const char* name, double*& val, unsigned int& length);

  // Description:
  // Serializes a string.
  virtual void Serialize(const char* name, char*& str);

  // Description:
  // Serializes a string.
  virtual void Serialize(const char* name, std::string& str);

  // Description:
  // Reads a vtkObject.  weakPtr should be set to true if this reference to the
  // object should NOT be reference counted.
  virtual void Serialize(const char* name, vtkObject*& obj, bool weakPtr = false);

  // Description:
  // Reads a vtkInformationObject. Note that only keys registered
  // with the vtkInformationKeyMap are restored.
  virtual void Serialize(const char* name, vtkInformation* info);

  // Description:
  // Reads a vector of vtkObjects.
  virtual void Serialize(const char* name,
    std::vector<vtkSmartPointer<vtkObject> >& objs,
    bool weakPtr = false);

  // Description:
  // Reads a map from int to vector of vtkObject.
  virtual void Serialize(const char* name,
    std::map<int, std::vector<vtkSmartPointer<vtkObject> > >& objs);

protected:
  vtkXMLArchiveReader();
  ~vtkXMLArchiveReader();

private:
  vtkXMLArchiveReader(const vtkXMLArchiveReader&);  // Not implemented.
  void operator=(const vtkXMLArchiveReader&);  // Not implemented.

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
