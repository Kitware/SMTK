//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_widgets_vtkMeshInspectorRepresentation_h
#define smtk_extension_paraview_widgets_vtkMeshInspectorRepresentation_h

#include "smtk/common/UUID.h"
#include "smtk/extension/paraview/widgets/smtkPQWidgetsExtModule.h"

#include "vtkDisplaySizedImplicitPlaneRepresentation.h"
#include "vtkSmartPointer.h"

#include <array>
#include <set>

class vtkActor;
class vtkAlgorithmOutput;
class vtkCompositeDataSet;
class vtkCompositePolyDataMapper;
class vtkPlaneCutter;
class vtkExtractEdges;
class vtkExtractBlock;
class vtkInformation;
class vtkPVGeometryFilter;
class vtkPVMetaSliceDataSet;
class vtkScalarsToColors;

/**\brief A widget for viewing crinkle-slices of unstructured data.
 *
 * This class inherits vtkDisplaySizedImplicitPlaneRepresentation
 * and simply adds pipelines to accept an input composite dataset
 * to be crinkle-sliced by the plane.
 */
class SMTKPQWIDGETSEXT_EXPORT vtkMeshInspectorRepresentation
  : public vtkDisplaySizedImplicitPlaneRepresentation
{
public:
  vtkTypeMacro(vtkMeshInspectorRepresentation, vtkDisplaySizedImplicitPlaneRepresentation);
  static vtkMeshInspectorRepresentation* New();
  void PrintSelf(std::ostream& os, vtkIndent indent) override;
  vtkMeshInspectorRepresentation(const vtkMeshInspectorRepresentation&) = delete;
  void operator=(const vtkMeshInspectorRepresentation&) = delete;

  ///@{
  /**\brief Set/get the multiblock source holding renderable geometry for components.
   */
  virtual void SetInputConnection(vtkAlgorithmOutput* port);
  virtual void SetInput(vtkCompositeDataSet* input);
  vtkCompositeDataSet* GetInput();
  ///@}

  ///@{
  /**\brief Add/remove/reset UUIDs of components to inspect (crinkle-slice).
   */
  virtual bool AddId(const int* uid) VTK_SIZEHINT(4);
  virtual bool AddId(int i0, int i1, int i2, int i3)
  {
    int tmp[4] = { i0, i1, i2, i3 };
    return this->AddId(tmp);
  }
  virtual bool RemoveId(const int* uid) VTK_SIZEHINT(4);
  virtual bool RemoveId(int i0, int i1, int i2, int i3)
  {
    int tmp[4] = { i0, i1, i2, i3 };
    return this->RemoveId(tmp);
  }
  virtual void ResetIds();
  ///@}

  ///@{
  /**\brief Set/get whether to render the plane-editing handles.
   *
   * When true, the 3-d handles (ball, arrow, and disc) are drawn
   * in the scene and users can edit the plane by clicking and dragging
   * these objects.
   * When false, these objects are not drawn – which is suitable
   * for producing publication images.
   */
  virtual void SetDrawHandles(bool drawHandles);
  virtual bool GetDrawHandles() const;
  ///@}

  ///@{
  /**\brief Set/get the slice modality.
   *
   * The string should be either "Flat" or "Crinkle".
   */
  virtual void SetSliceType(const std::string& sliceType);
  ///@}

  ///@{
  /**\brief Set/get an array to color the extracted surface by.
   */
  virtual void
  SetInputArrayToProcess(int idx, int port, int connection, int fieldAssociation, const char* name);
  virtual vtkInformation* GetInputArrayInformation(int idx);
  virtual void SetSliceColorComponent(int comp);
  virtual int GetSliceColorComponent();
  ///@}

  ///@{
  /**\brief Set/get a lookup table for coloring the crinkle-slice.
   */
  virtual void SetSliceLookupTable(vtkScalarsToColors* lkup);
  vtkScalarsToColors* GetSliceLookupTable();
  ///@}

  ///@{
  /**\brief Set/get variables controlling edge rendering (visibility and color).
   */
  virtual void SetSliceEdgeVisibility(bool visible);
  virtual bool GetSliceEdgeVisibility();
  virtual void SetSliceEdgeColor(double r, double g, double b, double a = 1.0);
  virtual void SetSliceEdgeColor(double* rgba) VTK_SIZEHINT(4)
  {
    this->SetSliceEdgeColor(rgba[0], rgba[1], rgba[2], rgba[3]);
  }
  virtual double* GetSliceEdgeColor() VTK_SIZEHINT(4);
  ///@}

  ///@{
  /**\brief Methods to render the representation.
   */
  double* GetBounds() VTK_SIZEHINT(6) override;
  void GetActors(vtkPropCollection* pc) override;
  void ReleaseGraphicsResources(vtkWindow*) override;
  int RenderOpaqueGeometry(vtkViewport*) override;
  int RenderTranslucentPolygonalGeometry(vtkViewport*) override;
  vtkTypeBool HasTranslucentPolygonalGeometry() override;
  ///@}

protected:
  vtkMeshInspectorRepresentation();
  ~vtkMeshInspectorRepresentation() override;

  /// Prepare widget renderables given that the interaction/input-data state may have changed.
  void BuildRepresentation() override;

  /// Update list of block IDs to extract.
  virtual void UpdateBlockIds();

  vtkSmartPointer<vtkCompositeDataSet> Input;
  std::set<smtk::common::UUID> Components;
  bool ExtractNeedsUpdate;
  vtkNew<vtkExtractBlock> Extract;
  vtkNew<vtkPVMetaSliceDataSet> Crinkle;
  vtkNew<vtkPlaneCutter> Cutter;
  vtkNew<vtkPVGeometryFilter> Surface;
  vtkNew<vtkCompositePolyDataMapper> SurfaceMapper;
  vtkNew<vtkActor> SurfaceActor;
  vtkNew<vtkExtractEdges> Edges;
  vtkNew<vtkCompositePolyDataMapper> EdgeMapper;
  vtkNew<vtkActor> EdgeActor;
  bool DrawHandles{ true };
  std::array<double, 4> EdgeColor;
  std::array<double, 6> BoundsData;
};

#endif // smtk_extension_paraview_widgets_vtkMeshInspectorRepresentation_h
