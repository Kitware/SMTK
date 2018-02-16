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

#include "smtk/extension/paraview/server/Exports.h"

#include "smtk/PublicPointerDefs.h"

#include "vtkMultiBlockDataSetAlgorithm.h"
#include "vtkNew.h"
#include "vtkSmartPointer.h"

class vtkSMTKWrapper;

/**\brief A base class for SMTK-based readers exposed in ParaView.
  *
  * If the SMTK wrapper object is set, then the wrapper's resource and operation
  * manager are used to load the file (or perhaps in the future to create a new resource).
  * Otherwise SMTK's default environment is used.
  */
class SMTKPVSERVEREXT_EXPORT vtkSMTKResourceReader : public vtkMultiBlockDataSetAlgorithm
{
public:
  vtkTypeMacro(vtkSMTKResourceReader, vtkMultiBlockDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Set/get the URL of the SMTK model resource.
  vtkGetStringMacro(FileName);
  vtkSetStringMacro(FileName);

  /// Return the SMTK resource that holds data read from \a FileName.
  virtual smtk::resource::ResourcePtr GetResource() const;

  /// Set/get the SMTK wrapper to which the resource is added (and whose operators are used to read)
  virtual void SetWrapper(vtkSMTKWrapper*);
  vtkGetObjectMacro(Wrapper, vtkSMTKWrapper);

  /// Remove any loaded resource from the resource manager being used by the reader.
  virtual void DropResource();

protected:
  vtkSMTKResourceReader();
  ~vtkSMTKResourceReader() override;

  char* FileName;
  vtkSMTKWrapper* Wrapper;

private:
  vtkSMTKResourceReader(const vtkSMTKResourceReader&) = delete;
  void operator=(const vtkSMTKResourceReader&) = delete;
};

#endif
