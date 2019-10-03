//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_server_vtkSMTKResourceImporter_h
#define smtk_extension_paraview_server_vtkSMTKResourceImporter_h

#include "smtk/extension/paraview/server/smtkPVServerExtModule.h"
#include "smtk/extension/paraview/server/vtkSMTKResourceGenerator.h"

#include "smtk/PublicPointerDefs.h"

#include "smtk/resource/Resource.h"

#include "vtkMultiBlockDataSetAlgorithm.h"
#include "vtkNew.h"
#include "vtkSmartPointer.h"

class vtkSMTKWrapper;

/**\brief Use SMTK to import a resource as a ParaView-friendly source.
  *
  * If the SMTK wrapper object is set, then the wrapper's resource and operation
  * manager are used to import the file (or perhaps in the future to create a
  * new resource). Otherwise SMTK's default environment is used.
  */
class SMTKPVSERVEREXT_EXPORT vtkSMTKResourceImporter : public vtkSMTKResourceGenerator
{
public:
  vtkTypeMacro(vtkSMTKResourceImporter, vtkSMTKResourceGenerator);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  static vtkSMTKResourceImporter* New();

  /// Set/get the URL of the SMTK resource.
  vtkGetStringMacro(FileName);
  vtkSetStringMacro(FileName);

  /// Set/get the unique name of the SMTK resource.
  vtkGetStringMacro(ResourceName);
  vtkSetStringMacro(ResourceName);

  /// Return the SMTK resource that holds data imported from \a FileName.
  smtk::resource::ResourcePtr GenerateResource() const override;

protected:
  vtkSMTKResourceImporter();
  ~vtkSMTKResourceImporter() override;

  char* FileName;
  char* ResourceName;

private:
  vtkSMTKResourceImporter(const vtkSMTKResourceImporter&) = delete;
  void operator=(const vtkSMTKResourceImporter&) = delete;
};

#endif
