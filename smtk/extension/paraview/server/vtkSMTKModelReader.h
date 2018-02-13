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

#include "smtk/extension/paraview/server/vtkSMTKResourceReader.h"

#include "smtk/PublicPointerDefs.h"

#include "vtkMultiBlockDataSetAlgorithm.h"
#include "vtkNew.h"
#include "vtkSmartPointer.h"

class vtkModelMultiBlockSource;
class vtkSMTKWrapper;

/**\brief Use SMTK to provide a ParaView-friendly model source.
  *
  * If the SMTK wrapper object is set, then the wrapper's resource and operation
  * manager are used to load the file (or perhaps in the future to create a new resource).
  * Otherwise SMTK's default environment is used.
  */
class SMTKPVSERVEREXT_EXPORT vtkSMTKModelReader : public vtkSMTKResourceReader
{
public:
  vtkTypeMacro(vtkSMTKModelReader, vtkSMTKResourceReader);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  static vtkSMTKModelReader* New();

  /// Return the VTK algorithm used to read the SMTK file.
  vtkModelMultiBlockSource* GetModelSource() { return this->ModelSource.GetPointer(); }

  /// Return the SMTK resource that holds data read from \a FileName.
  smtk::resource::ResourcePtr GetResource() const override;

  /// Return the SMTK model resource that holds data read from \a FileName.
  smtk::model::ManagerPtr GetSMTKResource() const;

protected:
  vtkSMTKModelReader();
  ~vtkSMTKModelReader() override;

  int RequestData(
    vtkInformation* request, vtkInformationVector** inInfo, vtkInformationVector* outInfo) override;

  bool LoadFile();

  vtkNew<vtkModelMultiBlockSource> ModelSource;

private:
  vtkSMTKModelReader(const vtkSMTKModelReader&) = delete;
  void operator=(const vtkSMTKModelReader&) = delete;
};

#endif
