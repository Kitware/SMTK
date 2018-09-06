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

#include "smtk/extension/paraview/server/vtkSMTKResourceGenerator.h"

#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h"

#include "smtk/PublicPointerDefs.h"

#include "vtkMultiBlockDataSetAlgorithm.h"
#include "vtkNew.h"
#include "vtkSmartPointer.h"

class vtkSMTKWrapper;

/**\brief Use SMTK to provide a ParaView-friendly model source.
  *
  * If the SMTK wrapper object is set, then the wrapper's resource and operation
  * manager are used to load the file (or perhaps in the future to create a new resource).
  * Otherwise SMTK's default environment is used.
  */
class SMTKPVSERVEREXT_EXPORT vtkSMTKModelReader : public vtkSMTKResourceGenerator
{
public:
  vtkTypeMacro(vtkSMTKModelReader, vtkSMTKResourceGenerator);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  static vtkSMTKModelReader* New();

  /// Set/get the URL of the SMTK model resource.
  vtkGetStringMacro(FileName);
  vtkSetStringMacro(FileName);

  /// Return the SMTK resource that holds data read from \a FileName.
  smtk::resource::ResourcePtr GenerateResource() const override;

protected:
  vtkSMTKModelReader();
  ~vtkSMTKModelReader() override;

  char* FileName;

private:
  vtkSMTKModelReader(const vtkSMTKModelReader&) = delete;
  void operator=(const vtkSMTKModelReader&) = delete;
};

#endif
