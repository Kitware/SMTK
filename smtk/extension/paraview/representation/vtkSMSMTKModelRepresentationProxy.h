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

  /**
   * Overridden to ensure that whenever "Input" property changes, other input
   * properties are updated (glyph mapper inputs).
   */
  void SetPropertyModifiedFlag(const char* name, int flag) override;

private:
  vtkSMSMTKModelRepresentationProxy(const vtkSMSMTKModelRepresentationProxy&) = delete;
  void operator=(const vtkSMSMTKModelRepresentationProxy&) = delete;
};

#endif
