//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBXMLBCSWriter - outputs a BCS file in XML format
// .SECTION Description
// Filter to output an XML file with the mesh facets, model face id
// they belong to and the BCSs defined over the model face. This
// filter takes in the CMB vtkMultiBlockDataSet structure.

#ifndef __vtkCMBXMLBCSWriter_h
#define __vtkCMBXMLBCSWriter_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkXMLWriter.h"
#include "cmbSystemConfig.h"
#include <ostream>

class vtkDiscreteModel;
class vtkDiscreteModelWrapper;
class vtkPolyData;
class vtkXMLDataElement;

// cannot derive from vtkXMLUnstructuredDataWriter unless the
// input is a subclass of vtkPointSet
class VTKCMBDISCRETEMODEL_EXPORT vtkCMBXMLBCSWriter : public vtkXMLWriter
{
public:
  static vtkCMBXMLBCSWriter *New();
  vtkTypeMacro(vtkCMBXMLBCSWriter,vtkXMLWriter);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual const char* GetDefaultFileExtension();

  // Description:
  // Methods to define the file's major and minor version numbers.
  virtual int GetDataSetMajorVersion();
  virtual int GetDataSetMinorVersion();

  // Description:
  // Set/get functions for the ModelWrapper.
  vtkGetMacro(ModelWrapper, vtkDiscreteModelWrapper*);
  void SetModelWrapper(vtkDiscreteModelWrapper* Wrapper);

  // Description:
  // If this is set w
  // This is mutual exclusive with SetStream. Setting this will mean that
  // we won't write to the Stream.
  void SetFileName(const char* fileName);

  // Description:
  // Write to buffer, which could be an ostringstream or ofstream allowing
  // writing to a file or a buffer.
  // This is mutual exclusive with SetFileName. Setting this will mean that
  // we won't write to the discrete FileName.
  void SetStream(std::ostream& stream);

  bool IsWritingToStream() const;
  bool IsWrittingToFile() const;

protected:
  vtkCMBXMLBCSWriter();
  ~vtkCMBXMLBCSWriter();

  virtual int WriteData();

  const char* GetDataSetName();


  // Description:
  // Add in the information for vtkCMBModelEdges as a nested element
  // in ParentElement.
  void AddHardPointsData(vtkXMLDataElement* ParentElement,
                         vtkDiscreteModel* Model, vtkIndent indent4);

private:
  vtkCMBXMLBCSWriter(const vtkCMBXMLBCSWriter&);  // Not implemented.
  void operator=(const vtkCMBXMLBCSWriter&);  // Not implemented.

  // Description:
  // The vtkDiscreteModelWrapper for the algorithm to extract the model
  // information from.
  vtkDiscreteModelWrapper* ModelWrapper;

  // Description:
  // Holds onto the stream that we will be writing too.
  std::ostream* Stream;
  bool WritingToFile;
};

#endif
