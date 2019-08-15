//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_representation_vtkSMTKSelectionRepresentation_h
#define smtk_extension_paraview_representation_vtkSMTKSelectionRepresentation_h

#include "smtk/extension/paraview/server/Exports.h"
#include "vtkPVDataRepresentation.h"

class vtkDataLabelRepresentation;
class vtkSMTKCompositeRepresentation;
class vtkSMTKResourceRepresentation;

/**
 * @class   vtkSMTKSelectionRepresentation
 *
 * vtkSMTKSelectionRepresentation is a representation to convey selected
 * information to the active instance of vtkSMTKResourceRepresenation.
*/
class SMTKPVSERVEREXT_EXPORT vtkSMTKSelectionRepresentation : public vtkPVDataRepresentation
{
public:
  static vtkSMTKSelectionRepresentation* New();
  vtkTypeMacro(vtkSMTKSelectionRepresentation, vtkPVDataRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

  /**
   * One must change the parent composite representations only before the
   * representation is added to a view, after that it should not be touched.
   */
  void SetCompositeRepresentation(vtkSMTKCompositeRepresentation*);

  /**
   * Get the active resource representation through the composite
   * representation.
   */
  vtkSMTKResourceRepresentation* GetResourceRepresentation() const;

  /**
   * Get/Set the visibility for this representation. When the visibility of
   * representation of false, all view passes are ignored.
   * Overridden to propagate to the active representation.
   */
  void SetVisibility(bool val) override;

  //@{
  /**
   * Forwarded to the active resource representation
   */
  void SetColor(double r, double g, double b);
  void SetLineWidth(double val);
  void SetPointSize(double val);
  //@}

  /**
   * Override because of internal composite representations that need to be
   * initialized as well.
   */
  unsigned int Initialize(unsigned int minIdAvailable, unsigned int maxIdAvailable) override;

protected:
  vtkSMTKSelectionRepresentation();
  ~vtkSMTKSelectionRepresentation() override;

  int FillInputPortInformation(int port, vtkInformation* info) override;

  vtkSMTKCompositeRepresentation* CompositeRepresentation;

private:
  vtkSMTKSelectionRepresentation(const vtkSMTKSelectionRepresentation&) = delete;
  void operator=(const vtkSMTKSelectionRepresentation&) = delete;
};

#endif
