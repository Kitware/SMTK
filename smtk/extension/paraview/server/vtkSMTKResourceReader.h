//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_server_vtkSMTKResourceReader_h
#define smtk_extension_paraview_server_vtkSMTKResourceReader_h

#include "smtk/extension/paraview/server/smtkPVServerExtModule.h"
#include "smtk/extension/paraview/server/vtkSMTKResourceGenerator.h"

#include "smtk/PublicPointerDefs.h"

#include "vtkMultiBlockDataSetAlgorithm.h"
#include "vtkNew.h"
#include "vtkSmartPointer.h"

class vtkSMTKWrapper;

/**\brief Use SMTK to provide a ParaView-friendly resource.
  *
  * If the SMTK wrapper object is set, then the wrapper's resource and operation
  * managers are used to load the file. Otherwise, SMTK's default environment is
  * used.
  */
class SMTKPVSERVEREXT_EXPORT vtkSMTKResourceReader : public vtkSMTKResourceGenerator
{
public:
  vtkTypeMacro(vtkSMTKResourceReader, vtkSMTKResourceGenerator);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  static vtkSMTKResourceReader* New();

  vtkSMTKResourceReader(const vtkSMTKResourceReader&) = delete;
  vtkSMTKResourceReader& operator=(const vtkSMTKResourceReader&) = delete;

  /// Set/get the URL of the SMTK resource.
  vtkGetStringMacro(FileName);
  vtkSetStringMacro(FileName);

  /// Return the SMTK resource that holds data read from \a FileName.
  smtk::resource::ResourcePtr GenerateResource() const override;

protected:
  vtkSMTKResourceReader();
  ~vtkSMTKResourceReader() override;

  char* FileName;
};

#endif
