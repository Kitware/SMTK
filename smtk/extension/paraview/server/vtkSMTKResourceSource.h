//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_server_vtkSMTKResourceSource_h
#define smtk_extension_paraview_server_vtkSMTKResourceSource_h

#include "smtk/extension/paraview/server/Exports.h"

#include "smtk/PublicPointerDefs.h"

#include "vtkMultiBlockDataSetAlgorithm.h"
#include "vtkNew.h"
#include "vtkSmartPointer.h"

class vtkSMTKWrapper;

/**\brief A base class for SMTK-based sources exposed in ParaView.
  */
class SMTKPVSERVEREXT_EXPORT vtkSMTKResourceSource : public vtkMultiBlockDataSetAlgorithm
{
public:
  vtkTypeMacro(vtkSMTKResourceSource, vtkMultiBlockDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Return the SMTK resource that holds data read from \a FileName.
  virtual smtk::resource::ResourcePtr GetResource() const;

  /// Set/get the SMTK wrapper to which the resource is added (and whose operators are used to read)
  virtual void SetWrapper(vtkSMTKWrapper*);
  vtkGetObjectMacro(Wrapper, vtkSMTKWrapper);

  /// Remove any loaded resource from the resource manager being used by the reader.
  virtual void DropResource();

protected:
  vtkSMTKResourceSource();
  ~vtkSMTKResourceSource() override;

  vtkSMTKWrapper* Wrapper;

private:
  vtkSMTKResourceSource(const vtkSMTKResourceSource&) = delete;
  void operator=(const vtkSMTKResourceSource&) = delete;
};

#endif
