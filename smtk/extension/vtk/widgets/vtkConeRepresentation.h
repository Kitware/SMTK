//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef vtkConeRepresentation_h
#define vtkConeRepresentation_h

#include "smtk/extension/vtk/widgets/vtkSMTKWidgetsExtModule.h" // For export macro
#include "vtkNew.h"
#include "vtkVector.h"
#include "vtkWidgetRepresentation.h"

#include <array>

class vtkActor;
class vtkPolyDataMapper;
class vtkCellPicker;
class vtkConeSource;
class vtkGlyph3DMapper;
class vtkImplicitConeFrustum;
class vtkSphereSource;
class vtkTubeFilter;
class vtkConeFrustum;
class vtkProperty;
class vtkImageData;
class vtkOutlineFilter;
class vtkFeatureEdges;
class vtkPolyData;
class vtkPolyDataAlgorithm;
class vtkTransform;
class vtkBox;
class vtkLookupTable;

#define VTK_MAX_CONE_RESOLUTION 2048

/**
 * @class   vtkConeRepresentation
 * @brief   defining the representation for a vtkConeFrustum
 *
 * This class is a concrete representation for the
 * vtkConeWidget. It represents a finite cylinder
 * defined by 2 radii and 2 endpoints.
 * This cylinder representation can be manipulated by using the
 * vtkConeWidget to adjust the radii and endpoints.
 *
 * @sa
 * vtkConeWidget
*/
class VTKSMTKWIDGETSEXT_EXPORT vtkConeRepresentation : public vtkWidgetRepresentation
{
public:
  /**
   * Instantiate the class.
   */
  static vtkConeRepresentation* New();

  //@{
  /**
   * Standard methods for the class.
   */
  vtkTypeMacro(vtkConeRepresentation, vtkWidgetRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  //@}

  vtkConeRepresentation(const vtkConeRepresentation&) = delete;
  vtkConeRepresentation& operator=(const vtkConeRepresentation&) = delete;

  //@{
  /**
   * Set/get an endpoint of the cone.
   */
  bool SetEndpoint(bool isBottom, double x, double y, double z);
  bool SetEndpoint(bool isBottom, const vtkVector3d& pt);
  vtkVector3d GetEndpoint(bool isBottom) const;

  bool SetBottomEndpoint(double x, double y, double z) { return this->SetEndpoint(true, x, y, z); }
  bool SetBottomEndpoint(const vtkVector3d& pt) { return this->SetEndpoint(true, pt); }
  vtkVector3d GetBottomEndpoint() const { return this->GetEndpoint(true); }
  void SetBottomPoint(double x, double y, double z) { this->SetEndpoint(true, x, y, z); }
  void SetBottomPoint(double* x) VTK_SIZEHINT(3) { this->SetEndpoint(true, x[0], x[1], x[2]); }
  double* GetBottomPoint() VTK_SIZEHINT(3);

  bool SetTopEndpoint(double x, double y, double z) { return this->SetEndpoint(false, x, y, z); }
  bool SetTopEndpoint(const vtkVector3d& pt) { return this->SetEndpoint(false, pt); }
  vtkVector3d GetTopEndpoint() const { return this->GetEndpoint(false); }
  void SetTopPoint(double x, double y, double z) { this->SetEndpoint(false, x, y, z); }
  void SetTopPoint(double* x) VTK_SIZEHINT(3) { this->SetEndpoint(false, x[0], x[1], x[2]); }
  double* GetTopPoint() VTK_SIZEHINT(3);
  //@}

  //@{
  /**
   * Set/get a radius of the cylinder.
   *
   * Negative values are generally a bad idea but not prohibited at this point.
   * They will result in bad surface normals, though.
   */
  bool SetRadius(bool isBottom, double r);
  double GetRadius(bool isBottom) const;

  bool SetBottomRadius(double r) { return this->SetRadius(true, r); }
  double GetBottomRadius() const { return this->GetRadius(true); }

  bool SetTopRadius(double r) { return this->SetRadius(false, r); }
  double GetTopRadius() const { return this->GetRadius(false); }
  //@}

  //@{
  /**
   * Force the cone widget to produce cylinders (i.e., make the two
   * radii identical).
   * When set, if the two radii are not already identical, the
   * average radius will be used.
   */
  bool SetCylindrical(int);
  vtkGetMacro(Cylindrical, int);
  vtkBooleanMacro(Cylindrical, int);
  //@}

  //@{
  /**
   * Force the cone widget to be aligned with one of the x-y-z axes.
   * If one axis is set on, the other two will be set off.
   * Remember that when the state changes, a ModifiedEvent is invoked.
   * This can be used to snap the cylinder to the axes if it is originally
   * not aligned.
   */
  void SetAlongXAxis(vtkTypeBool);
  vtkGetMacro(AlongXAxis, vtkTypeBool);
  vtkBooleanMacro(AlongXAxis, vtkTypeBool);
  void SetAlongYAxis(vtkTypeBool);
  vtkGetMacro(AlongYAxis, vtkTypeBool);
  vtkBooleanMacro(AlongYAxis, vtkTypeBool);
  void SetAlongZAxis(vtkTypeBool);
  vtkGetMacro(AlongZAxis, vtkTypeBool);
  vtkBooleanMacro(AlongZAxis, vtkTypeBool);
  //@}

  //@{
  /**
   * Enable/disable the drawing of the cylinder. In some cases the cylinder
   * interferes with the object that it is operating on (e.g., the
   * cylinder interferes with the cut surface it produces resulting in
   * z-buffer artifacts.) By default it is off.
   */
  void SetDrawCone(vtkTypeBool drawCyl);
  vtkGetMacro(DrawCone, vtkTypeBool);
  vtkBooleanMacro(DrawCone, vtkTypeBool);
  //@}

  //@{
  /**
   * Set/Get the resolution of the cylinder. This is the number of
   * polygonal facets used to approximate the curved cylindrical
   * surface (for rendering purposes). A vtkConeFrustum is used under
   * the hood to provide an exact surface representation.
   */
  vtkSetClampMacro(Resolution, int, 8, VTK_MAX_CONE_RESOLUTION);
  vtkGetMacro(Resolution, int);
  //@}

  //@{
  /**
   * Set/Get the tolerance of the cylinder.
   *
   * This how close the endpoints are allowed to be.
   * It is initially set to 1e-8.
   *
   * Note that this is an absolute distance in world coordinates,
   * so if your scene is very small, you may need to adjust this.
   */
  vtkSetClampMacro(Tolerance, double, 0.0, VTK_DOUBLE_MAX);
  vtkGetMacro(Tolerance, double);
  //@}

  //@{
  /**
   * Turn on/off tubing of the wire outline of the cylinder
   * intersection (against the bounding box). The tube thickens the
   * line by wrapping with a vtkTubeFilter.
   */
  vtkSetMacro(Tubing, vtkTypeBool);
  vtkGetMacro(Tubing, vtkTypeBool);
  vtkBooleanMacro(Tubing, vtkTypeBool);
  //@}

  //@{
  /**
   * Turn on/off the ability to scale the widget with the mouse.
   */
  vtkSetMacro(ScaleEnabled, vtkTypeBool);
  vtkGetMacro(ScaleEnabled, vtkTypeBool);
  vtkBooleanMacro(ScaleEnabled, vtkTypeBool);
  //@}

  /**
   * Get the implicit function for the cone. The user must provide an instance
   * vtkImplicitConeFrustum; upon return, it will be set to the difference between
   * an infinite cone and the two cutting planes.
   * The returned implicit can be used by a variety of filters
   * to perform clipping, cutting, and selection of data.
   */
  void GetCone(vtkImplicitConeFrustum* cone);

  /**
   * Satisfies the superclass API.  This will change the state of the widget
   * to match changes that have been made to the underlying PolyDataSource.
   */
  void UpdatePlacement();

  //@{
  /**
   * Get the properties on the axis (line and cone).
   */
  vtkGetObjectMacro(HandleProperty, vtkProperty);
  vtkGetObjectMacro(SelectedHandleProperty, vtkProperty);
  //@}

  //@{
  /**
   * Get the cylinder properties. The properties of the cylinder when selected
   * and unselected can be manipulated.
   */
  vtkGetObjectMacro(ConeProperty, vtkProperty);
  vtkGetObjectMacro(SelectedConeProperty, vtkProperty);
  //@}

  //@{
  /**
   * Get the property of the intersection edges. (This property also
   * applies to the edges when tubed.)
   */
  vtkGetObjectMacro(EdgeProperty, vtkProperty);
  //@}

  //@{
  /**
   * Methods to interface with the vtkImplicitCylinderWidget.
   */
  int ComputeInteractionState(int X, int Y, int modify = 0) override;
  void PlaceWidget(double bounds[6]) override;
  void BuildRepresentation() override;
  void StartWidgetInteraction(double eventPos[2]) override;
  void WidgetInteraction(double newEventPos[2]) override;
  void EndWidgetInteraction(double newEventPos[2]) override;
  //@}

  //@{
  /**
   * Methods supporting the rendering process.
   */
  double* GetBounds() override;
  void GetActors(vtkPropCollection* pc) override;
  void ReleaseGraphicsResources(vtkWindow*) override;
  int RenderOpaqueGeometry(vtkViewport*) override;
  int RenderTranslucentPolygonalGeometry(vtkViewport*) override;
  vtkTypeBool HasTranslucentPolygonalGeometry() override;
  //@}

  //@{
  /**
   * Specify a translation distance used by the BumpCone() method. Note that the
   * distance is normalized; it is the fraction of the length of the bounding
   * box of the wire outline.
   */
  vtkSetClampMacro(BumpDistance, double, 0.000001, 1);
  vtkGetMacro(BumpDistance, double);
  //@}

  /**
   * Translate the cylinder in the direction of the view vector by the
   * specified BumpDistance. The dir parameter controls which
   * direction the pushing occurs, either in the same direction as the
   * view vector, or when negative, in the opposite direction.  The factor
   * controls what percentage of the bump is used.
   */
  void BumpCone(int dir, double factor);

  /**
   * Push the cylinder the distance specified along the view
   * vector. Positive values are in the direction of the view vector;
   * negative values are in the opposite direction. The distance value
   * is expressed in world coordinates.
   */
  void PushCone(double distance);

  // Manage the state of the widget
  enum _InteractionState
  {
    Outside = 0,
    Moving,
    PushingBottomFace,
    PushingTopFace,
    AdjustingBottomRadius,
    AdjustingTopRadius,
    MovingBottomHandle,
    MovingTopHandle,
    MovingWhole,
    RotatingAxis,
    Scaling,
    TranslatingCenter
  };

  static std::string InteractionStateToString(int);

  //@{
  /**
   * The interaction state may be set from a widget (e.g.,
   * vtkImplicitCylinderWidget) or other object. This controls how the
   * interaction with the widget proceeds. Normally this method is used as
   * part of a handshaking process with the widget: First
   * ComputeInteractionState() is invoked that returns a state based on
   * geometric considerations (i.e., cursor near a widget feature), then
   * based on events, the widget may modify this further.
   */
  vtkSetClampMacro(InteractionState, int, Outside, TranslatingCenter);
  //@}

  //@{
  /**
   * Sets the visual appearance of the representation based on the
   * state it is in. This state is usually the same as InteractionState.
   */
  virtual void SetRepresentationState(int);
  vtkGetMacro(RepresentationState, int);
  //@}

  /*
  * Register internal Pickers within PickingManager
  */
  void RegisterPickers() override;

  /// Swap the top and bottom points and radii.
  void SwapTopAndBottom();

protected:
  vtkConeRepresentation();
  ~vtkConeRepresentation() override;

  /// Visual elements of the representation.
  enum ElementType
  {
    ConeFace = 0,
    BottomFace,
    TopFace,
    ConeAxis,
    BottomCurve,
    TopCurve,
    BottomHandle,
    TopHandle,
    NumberOfElements
  };

  void HighlightElement(ElementType elem, int highlight);

  void HighlightCone(int highlight);
  void HighlightAxis(int highlight);
  void HighlightCap(bool isBottom, int highlight);
  void HighlightCurve(bool isBottom, int highlight);
  void HighlightHandle(bool isBottom, int highlight);

  // Methods to manipulate the cylinder
  void Rotate(double X, double Y, double* p1, double* p2, double* vpn);
  void TranslateCone(double* p1, double* p2);
  void PushCap(bool isBottom, double* p1, double* p2);
  void AdjustBottomRadius(double X, double Y, double* p1, double* p2);
  void AdjustTopRadius(double X, double Y, double* p1, double* p2);
  void TranslateCenter(double* p1, double* p2);
  void TranslateCenterOnAxis(double* p1, double* p2);
  void TranslateHandle(bool isBottom, double* p1, double* p2);
  void Scale(double* p1, double* p2, double X, double Y);
  void SizeHandles();

  void CreateDefaultProperties();

  struct Element
  {
    vtkNew<vtkActor> Actor;
    vtkNew<vtkPolyDataMapper> Mapper;
  };

  // Actors and mappers for all visual elements of the representation.
  std::array<Element, NumberOfElements> Elements;

  int RepresentationState;
  // Keep track of event positions
  double LastEventPosition[3];
  // Controlling the push operation
  double BumpDistance;
  // Controlling ivars
  vtkTypeBool AlongXAxis;
  vtkTypeBool AlongYAxis;
  vtkTypeBool AlongZAxis;
  // The actual cylinder which is being manipulated
  vtkNew<vtkConeFrustum> Cone;
  // The facet resolution for rendering purposes.
  int Resolution;
  double Tolerance;         // How close are endpoints allowed to be?
  vtkTypeBool ScaleEnabled; //whether the widget can be scaled
  vtkTypeBool DrawCone;
  vtkNew<vtkTubeFilter> AxisTuber; // Used to style edges.
  vtkTypeBool Tubing;              //control whether tubing is on
  int Cylindrical;                 // control whether the cone is a cylinder (apex at infinity)

  // Source of endpoint handle geometry
  vtkNew<vtkSphereSource> Sphere;

  // Do the picking
  vtkNew<vtkCellPicker> Picker;
  vtkNew<vtkCellPicker> CylPicker;

  // Properties used to control the appearance of selected objects and
  // the manipulator in general.
  vtkNew<vtkProperty> HandleProperty;
  vtkNew<vtkProperty> SelectedHandleProperty;
  vtkNew<vtkProperty> ConeProperty;
  vtkNew<vtkProperty> SelectedConeProperty;
  vtkNew<vtkProperty> EdgeProperty;
  vtkNew<vtkProperty> SelectedEdgeProperty;

  // Support GetBounds() method
  vtkNew<vtkBox> BoundingBox;

  // Overrides for the point-handle polydata mappers
  vtkNew<vtkGlyph3DMapper> BottomHandleMapper;
  vtkNew<vtkGlyph3DMapper> TopHandleMapper;

  vtkNew<vtkTransform> Transform;
};

#endif
