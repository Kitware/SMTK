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
 * @class   vtkSMSMTKModelRepresentationProxy
 *
 * Updates additional input properties of the representation
 * (GlyphPrototypes and GlyphPoints).
 */
#ifndef smtk_extension_paraview_representation_vtkSMSMTKModelRepresentationProxy_h
#define smtk_extension_paraview_representation_vtkSMSMTKModelRepresentationProxy_h

#include "smtk/extension/paraview/representation/Exports.h" //needed for exports
#include "vtkSMPVRepresentationProxy.h"

class vtkSMTKModelRepresentation;

class SMTKREPRESENTATIONPLUGIN_EXPORT vtkSMSMTKModelRepresentationProxy
  : public vtkSMPVRepresentationProxy
{
public:
  static vtkSMSMTKModelRepresentationProxy* New();
  vtkTypeMacro(vtkSMSMTKModelRepresentationProxy, vtkSMPVRepresentationProxy);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /**
   * Returns client side representation object.
   */
  vtkSMTKModelRepresentation* GetRepresentation();

protected:
  vtkSMSMTKModelRepresentationProxy();
  ~vtkSMSMTKModelRepresentationProxy() override;

  friend class pqSMTKModelRepresentation;

  /**
   * Connects additional input ports required by the representation (instance
   * placement inputs).
   */
  void ConnectAdditionalPorts();

  /**
   * Ensures that whenever the "Input" property changes, ConnectAdditionalPorts
   * is called. This is critical in cases when pqSMTKModelRepresentation has not
   * been constructed (e.g. Python invocations of SMTKModelRepresentation).
   */
  void SetPropertyModifiedFlag(const char* name, int flag) override;

  bool InitializedInputs = false;

private:
  vtkSMSMTKModelRepresentationProxy(const vtkSMSMTKModelRepresentationProxy&) = delete;
  void operator=(const vtkSMSMTKModelRepresentationProxy&) = delete;
};

#endif
