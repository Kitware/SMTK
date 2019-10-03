//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_server_vtkSMTKResourceGenerator_h
#define smtk_extension_paraview_server_vtkSMTKResourceGenerator_h

#include "smtk/extension/paraview/server/smtkPVServerExtModule.h"

#include "smtk/PublicPointerDefs.h"

#include "vtkSMTKResource.h"

/**\brief A base class for generating SMTK resources in ParaView.
 */
class SMTKPVSERVEREXT_EXPORT vtkSMTKResourceGenerator : public vtkSMTKResource
{
public:
  vtkTypeMacro(vtkSMTKResourceGenerator, vtkSMTKResource);

  virtual smtk::resource::ResourcePtr GenerateResource() const = 0;

protected:
  vtkSMTKResourceGenerator() = default;
  ~vtkSMTKResourceGenerator() = default;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkSMTKResourceGenerator(const vtkSMTKResourceGenerator&) = delete;
  void operator=(const vtkSMTKResourceGenerator&) = delete;
};

#endif
