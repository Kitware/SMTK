//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_server_vtkSMTKModelImporter_h
#define smtk_extension_paraview_server_vtkSMTKModelImporter_h

#include "smtk/extension/paraview/server/vtkSMTKResourceGenerator.h"

#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h"

#include "smtk/PublicPointerDefs.h"

#include "smtk/resource/Resource.h"

#include "vtkMultiBlockDataSetAlgorithm.h"
#include "vtkNew.h"
#include "vtkSmartPointer.h"

class vtkSMTKWrapper;

/**\brief Use SMTK to import a model as a ParaView-friendly model source.
  *
  * If the SMTK wrapper object is set, then the wrapper's resource and operation
  * manager are used to import the file (or perhaps in the future to create a
  * new resource). Otherwise SMTK's default environment is used.
  */
class SMTKPVSERVEREXT_EXPORT vtkSMTKModelImporter : public vtkSMTKResourceGenerator
{
public:
  vtkTypeMacro(vtkSMTKModelImporter, vtkSMTKResourceGenerator);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  static vtkSMTKModelImporter* New();

  /// Set/get the URL of the SMTK model resource.
  vtkGetStringMacro(FileName);
  vtkSetStringMacro(FileName);

  /// Set/get the unique name of the SMTK resource.
  vtkGetStringMacro(ResourceName);
  vtkSetStringMacro(ResourceName);

  /// Return the SMTK resource that holds data imported from \a FileName.
  smtk::resource::ResourcePtr GenerateResource() const override;

protected:
  vtkSMTKModelImporter();
  ~vtkSMTKModelImporter() override;

  char* FileName;
  char* ResourceName;

private:
  vtkSMTKModelImporter(const vtkSMTKModelImporter&) = delete;
  void operator=(const vtkSMTKModelImporter&) = delete;
};

#endif
