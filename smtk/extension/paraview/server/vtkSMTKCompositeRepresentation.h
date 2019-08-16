//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_representation_vtkSMTKCompositeRepresentation_h
#define smtk_extension_paraview_representation_vtkSMTKCompositeRepresentation_h
/**
 * @class   vtkSMTKCompositeRepresentation
 * @brief   a data-representation used by SMTK.
 *
 * vtkSMTKCompositeRepresentation is nearly identical to
 * vtkPVCompositeRepresentation, with one exception:
 * vtkSMTKSelectionRepresentation is used in place of
 * vtkSelectionRepresentation.
 */

#include "smtk/extension/paraview/server/Exports.h"
#include "vtkCompositeRepresentation.h"

class vtkPolarAxesRepresentation;
class vtkPVGridAxes3DRepresentation;
class vtkSMTKSelectionRepresentation;

class SMTKPVSERVEREXT_EXPORT vtkSMTKCompositeRepresentation : public vtkCompositeRepresentation
{
public:
  static vtkSMTKCompositeRepresentation* New();
  vtkTypeMacro(vtkSMTKCompositeRepresentation, vtkCompositeRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

  /**
   * These must only be set during initialization before adding the
   * representation to any views or calling Update().
   */
  void SetSelectionRepresentation(vtkSMTKSelectionRepresentation*);
  void SetGridAxesRepresentation(vtkPVGridAxes3DRepresentation*);

  /**
   * This must only be set during initialization before adding the
   * representation to any views or calling Update().
   */
  void SetPolarAxesRepresentation(vtkPolarAxesRepresentation*);

  /**
   * Propagate the modification to all internal representations.
   */
  void MarkModified() VTK_OVERRIDE;

  /**
   * Set visibility of the representation.
   * Overridden to update the cube-axes and selection visibilities.
   */
  void SetVisibility(bool visible) VTK_OVERRIDE;

  /**
   * Set the selection visibility.
   */
  virtual void SetSelectionVisibility(bool visible);

  /**
   * Set the polar axes visibility.
   */
  virtual void SetPolarAxesVisibility(bool visible);

  //@{
  /**
   * Passed on to internal representations as well.
   */
  void SetUpdateTime(double time) VTK_OVERRIDE;
  void SetForceUseCache(bool val) VTK_OVERRIDE;
  void SetForcedCacheKey(double val) VTK_OVERRIDE;
  void SetInputConnection(int port, vtkAlgorithmOutput* input) VTK_OVERRIDE;
  void SetInputConnection(vtkAlgorithmOutput* input) VTK_OVERRIDE;
  void AddInputConnection(int port, vtkAlgorithmOutput* input) VTK_OVERRIDE;
  void AddInputConnection(vtkAlgorithmOutput* input) VTK_OVERRIDE;
  void RemoveInputConnection(int port, vtkAlgorithmOutput* input) VTK_OVERRIDE;
  void RemoveInputConnection(int port, int idx) VTK_OVERRIDE;
  //@}

  //@{
  /**
   * Forwarded to vtkSelectionRepresentation.
   */
  virtual void SetPointFieldDataArrayName(const char*);
  virtual void SetCellFieldDataArrayName(const char*);
  //@}

  /**
   * Override because of internal composite representations that need to be
   * initialized as well.
   */
  unsigned int Initialize(unsigned int minIdAvailable, unsigned int maxIdAvailable) VTK_OVERRIDE;

protected:
  vtkSMTKCompositeRepresentation();
  ~vtkSMTKCompositeRepresentation() override;

  /**
   * Adds the representation to the view.  This is called from
   * vtkView::AddRepresentation().  Subclasses should override this method.
   * Returns true if the addition succeeds.
   */
  bool AddToView(vtkView* view) VTK_OVERRIDE;

  /**
   * Removes the representation to the view.  This is called from
   * vtkView::RemoveRepresentation().  Subclasses should override this method.
   * Returns true if the removal succeeds.
   */
  bool RemoveFromView(vtkView* view) VTK_OVERRIDE;

  vtkSMTKSelectionRepresentation* SelectionRepresentation;
  vtkPVGridAxes3DRepresentation* GridAxesRepresentation;
  vtkPolarAxesRepresentation* PolarAxesRepresentation;

  bool SelectionVisibility;

private:
  vtkSMTKCompositeRepresentation(const vtkSMTKCompositeRepresentation&) = delete;
  void operator=(const vtkSMTKCompositeRepresentation&) = delete;
};

#endif
