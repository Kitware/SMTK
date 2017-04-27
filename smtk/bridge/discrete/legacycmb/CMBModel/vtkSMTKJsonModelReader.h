//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkSMTKJsonModelReader -
// .SECTION Description

#ifndef __vtkSMTKJsonModelReader_h
#define __vtkSMTKJsonModelReader_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkMultiBlockDataSetAlgorithm.h"
#include <map>
#include <string>

class VTKCMBDISCRETEMODEL_EXPORT vtkSMTKJsonModelReader : public vtkMultiBlockDataSetAlgorithm
{
public:
  static vtkSMTKJsonModelReader* New();
  vtkTypeMacro(vtkSMTKJsonModelReader, vtkMultiBlockDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get/Set the name of the input file.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Get json string of the smtk model
  virtual const char* GetJSONModel() { return this->JSONModel.c_str(); }

  // Description:
  // Get the map of entityid (UUID) to blockindex of multiblock
  void GetEntityId2BlockIdMap(std::map<std::string, unsigned int>& uuid2mid);

protected:
  vtkSMTKJsonModelReader();
  virtual ~vtkSMTKJsonModelReader();

  int RequestInformation(vtkInformation*, vtkInformationVector**, vtkInformationVector*);
  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*);

private:
  // Description:
  // The name of the file to be read in.
  char* FileName;

  // Description:
  // smtk model in json
  std::string JSONModel;

  class vtkInternal;
  vtkInternal* Internal;

  vtkSMTKJsonModelReader(const vtkSMTKJsonModelReader&); // Not implemented.
  void operator=(const vtkSMTKJsonModelReader&);         // Not implemented.
};

#endif
