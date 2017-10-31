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

#include "smtk/PublicPointerDefs.h"

#include "vtkMultiBlockDataSetAlgorithm.h"
#include "vtkNew.h"
#include "vtkSmartPointer.h"

class vtkModelMultiBlockSource;

/**\brief Use SMTK to provide a ParaView-friendly model source.
  */
class SMTKPVSERVEREXTPLUGIN_EXPORT vtkSMTKModelReader : public vtkMultiBlockDataSetAlgorithm
{
public:
  /// When the reader creates (true) and destroys (false) model resources, this function is called.
  using Observer = std::function<void(smtk::model::ManagerPtr, bool)>;

  vtkTypeMacro(vtkSMTKModelReader, vtkMultiBlockDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  static vtkSMTKModelReader* New();

  /// Set/get the URL of the SMTK model resource.
  vtkGetStringMacro(FileName);
  vtkSetStringMacro(FileName);

  /// Return the VTK algorithm used to read the SMTK file.
  vtkModelMultiBlockSource* GetModelSource() { return this->ModelSource.GetPointer(); }

  /// Return the SMTK model resource that holds data read from \a FileName.
  smtk::model::ManagerPtr GetSMTKResource() const;

  void ObserveResourceChanges(const Observer& fn) { this->ResourceObserver = fn; }
  void UnobserveResourceChanges() { this->ResourceObserver = nullptr; }

protected:
  vtkSMTKModelReader();
  ~vtkSMTKModelReader() override;

  int RequestData(
    vtkInformation* request, vtkInformationVector** inInfo, vtkInformationVector* outInfo) override;

  bool LoadFile();

  char* FileName;
  vtkNew<vtkModelMultiBlockSource> ModelSource;
  Observer ResourceObserver;

private:
  vtkSMTKModelReader(const vtkSMTKModelReader&) = delete;
  void operator=(const vtkSMTKModelReader&) = delete;
};

#endif
