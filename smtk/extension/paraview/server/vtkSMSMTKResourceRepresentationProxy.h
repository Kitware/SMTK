//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
/**
 * @class   vtkSMSMTKResourceRepresentationProxy
 *
 * This is a proxy for a vtkPVCompositeRepresentation which parents a
 * vtkSMTKResourceRepresentation.
 */
#ifndef smtk_extension_paraview_representation_vtkSMSMTKResourceRepresentationProxy_h
#define smtk_extension_paraview_representation_vtkSMSMTKResourceRepresentationProxy_h

#include "smtk/common/UUID.h"                                     // needed for visibility API
#include "smtk/extension/paraview/server/smtkPVServerExtModule.h" // needed for exports
#include "vtkSMPVRepresentationProxy.h"

#include <map> // for visibility API

class vtkSMTKResourceRepresentation;

class SMTKPVSERVEREXT_EXPORT vtkSMSMTKResourceRepresentationProxy
  : public vtkSMPVRepresentationProxy
{
public:
  static vtkSMSMTKResourceRepresentationProxy* New();
  vtkTypeMacro(vtkSMSMTKResourceRepresentationProxy, vtkSMPVRepresentationProxy);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkSMSMTKResourceRepresentationProxy(const vtkSMSMTKResourceRepresentationProxy&) = delete;
  vtkSMSMTKResourceRepresentationProxy& operator=(const vtkSMSMTKResourceRepresentationProxy&) =
    delete;

  vtkSMProxy* GetResourceRepresentationSubProxy();

  void GetComponentVisibilities(std::map<smtk::common::UUID, int>& visibilities);

protected:
  vtkSMSMTKResourceRepresentationProxy();
  ~vtkSMSMTKResourceRepresentationProxy() override;

  friend class pqSMTKResourceRepresentation;
};

#endif
