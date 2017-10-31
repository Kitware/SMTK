//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_server_vtkSMTKModelReader_h
#define smtk_extension_paraview_server_vtkSMTKModelReader_h

#include "smtk/extension/paraview/server/Exports.h"

#include "vtkMultiBlockDataSetAlgorithm.h"
#include "vtkNew.h"
#include "vtkSmartPointer.h"

class vtkModelMultiBlockSource;

/**\brief Use SMTK to provide a ParaView-friendly model source.
  */
class SMTKPVSERVEREXTPLUGIN_EXPORT vtkSMTKModelReader : public vtkMultiBlockDataSetAlgorithm
{
public:
  vtkTypeMacro(vtkSMTKModelReader, vtkMultiBlockDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  static vtkSMTKModelReader* New();

  vtkGetStringMacro(FileName);
  vtkSetStringMacro(FileName);

  vtkModelMultiBlockSource* GetModelSource() { return this->ModelSource.GetPointer(); }

protected:
  vtkSMTKModelReader();
  ~vtkSMTKModelReader() override;

  int RequestData(
    vtkInformation* request, vtkInformationVector** inInfo, vtkInformationVector* outInfo) override;

  bool LoadFile();

  char* FileName;
  vtkNew<vtkModelMultiBlockSource> ModelSource;

private:
  vtkSMTKModelReader(const vtkSMTKModelReader&) = delete;
  void operator=(const vtkSMTKModelReader&) = delete;
};

#endif
