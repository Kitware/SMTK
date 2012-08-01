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
// .NAME vtkCmbMeshToModelReader - reading a MeshToModelMapFile file
// .SECTION Description

#ifndef __vtkCmbMeshToModelReader_h
#define __vtkCmbMeshToModelReader_h

#include "vtkXMLDataReader.h"

class vtkXMLDataElement;
class vtkFieldData;
class vtkDiscreteModelWrapper;
class vtkIdTypeArray;
class vtkDataArray;

// derive from vtkXMLDataReader so that we can use the  ReadArrayValues function
class VTK_EXPORT vtkCmbMeshToModelReader : public vtkXMLDataReader
{
public:
  static vtkCmbMeshToModelReader *New();
  vtkTypeMacro(vtkCmbMeshToModelReader,vtkXMLReader);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get the number of points in the output.
  virtual vtkIdType GetNumberOfPoints(){return 0;}

  // Description:
  // Get the number of cells in the output.
  virtual vtkIdType GetNumberOfCells(){return 0;}

  // Description:
  // Methods to define the file's major and minor version numbers.
  virtual int GetDataSetMajorVersion();
  virtual int GetDataSetMinorVersion();

  // Description:
  // Set/get functions for the ModelWrapper.
  vtkGetMacro(ModelWrapper, vtkDiscreteModelWrapper*);
  void SetModelWrapper(vtkDiscreteModelWrapper* Wrapper);
  bool IsReadSuccessful(){return !this->DataError;}

protected:
  vtkCmbMeshToModelReader();
  ~vtkCmbMeshToModelReader();

  // see algorithm for more info
  virtual int FillOutputPortInformation(int port, vtkInformation* info);

  // Called by corresponding RequestData methods after appropriate
  // setup has been done.
  virtual void ReadXMLData();

  // Setup the output's information.
  virtual void SetupOutputInformation(vtkInformation *vtkNotUsed(outInfo)) {}

  // Load the analysis grid info to the model.
  int LoadAnalysisGridInfo(vtkFieldData* fieldData);
  int Load2DAnalysisGridInfo(vtkFieldData* fieldData);
  int Load3DAnalysisGridInfo(vtkFieldData* fieldData);

  // Get the name of the data set being read.
  virtual const char* GetDataSetName();

  // Read the primary element from the file.  This is the element
  // whose name is the value returned by GetDataSetName().
  virtual int ReadPrimaryElement(vtkXMLDataElement* ePrimary);

  // Setup the output with no data available.  Used in error cases.
  virtual void SetupEmptyOutput();

  // Test if the reader can read a file with the given version number.
  virtual int CanReadFileVersion(int major, int minor);

  // Description:
  // copied from vtkCMBParserBase.cxx
  // Function to output vtkIdTypeArray given a data array.  This is needed
  // since the array may be read in as an vtkIntArray (they are essentially the
  // same on 32 bit machines) but cannot be cast to a vtkIdTypeArray.
  // If Array is not a vtkIdTypeArray, it will allocate a new array
  // which the caller must delete.  Otherwise it will increase the reference
  // count to the original array such that the caller must delete the array as well.
  vtkIdTypeArray* NewIdTypeArray(vtkDataArray* Array);

  // Description:
  // The vtkDiscreteModelWrapper for the algorithm to reload the mesh
  // information to.
  vtkDiscreteModelWrapper* ModelWrapper;

  // Description:
  // Get/Set the name of the AnalysisGridFileName file.
  vtkSetStringMacro(AnalysisGridFileName);
  vtkGetStringMacro(AnalysisGridFileName);

  int ModelDimension;
  char* AnalysisGridFileName;

private:
  vtkCmbMeshToModelReader(const vtkCmbMeshToModelReader&);  // Not implemented.
  void operator=(const vtkCmbMeshToModelReader&);  // Not implemented.
};

#endif
