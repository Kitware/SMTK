//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef vtkDiskRepresentation_h
#define vtkDiskRepresentation_h

#include "smtk/extension/vtk/widgets/vtkSMTKWidgetsExtModule.h" // For export macro
#include "vtkNew.h"
#include "vtkVector.h"
#include "vtkWidgetRepresentation.h"

#include <array>

class vtkActor;
class vtkPolyDataMapper;
class vtkHardwarePicker;
class vtkCellPicker;
class vtkDisk;
class vtkGlyph3DMapper;
class vtkImplicitDisk;
class vtkSphereSource;
class vtkTubeFilter;
class vtkProperty;
class vtkImageData;
class vtkOutlineFilter;
class vtkFeatureEdges;
class vtkPolyData;
class vtkPolyDataAlgorithm;
class vtkTransform;
class vtkBox;
class vtkLookupTable;

#define VTK_MAX_DISK_RESOLUTION 2048

/**
 * @class   vtkDiskRepresentation
 * @brief   defining the representation for a planar disk.
 *
 * This class is a concrete representation for the
 * vtkDiskWidget. It represents a finite disk
 * defined by a center point, a normal vector, and a radius.
 * This disk representation can be manipulated by using the
 * vtkDiskWidget.
 *
 * @sa
 * vtkDiskWidget
*/
class VTKSMTKWIDGETSEXT_EXPORT vtkDiskRepresentation : public vtkWidgetRepresentation
{
public:
  /**
   * Instantiate the class.
   */
  static vtkDiskRepresentation* New();

  //@{
  /**
   * Standard methods for the class.
   */
  vtkTypeMacro(vtkDiskRepresentation, vtkWidgetRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  //@}

  vtkDiskRepresentation(const vtkDiskRepresentation&) = delete;
  vtkDiskRepresentation& operator=(const vtkDiskRepresentation&) = delete;

  //@{
  /**
   * Set/get the center of the disk.
   */
  bool SetCenter(double x, double y, double z);
  bool SetCenter(const vtkVector3d& pt);
  vtkVector3d GetCenter() const;
  void SetCenterPoint(double* x) VTK_SIZEHINT(3) { this->SetCenter(x[0], x[1], x[2]); }
  double* GetCenterPoint() VTK_SIZEHINT(3);
  //@}

  //@{
  /**
   * Set/get the normal of the disk.
   */
  bool SetNormal(double x, double y, double z);
  bool SetNormal(const vtkVector3d& nm);
  vtkVector3d GetNormal() const;
  void SetNormal(double* x) VTK_SIZEHINT(3) { this->SetNormal(x[0], x[1], x[2]); }
  double* GetNormalVector() VTK_SIZEHINT(3);
  //@}

  //@{
  /**
   * Set/get the radius of the disk.
   *
   * Negative values are generally a bad idea but not prohibited at this point.
   * They will result in bad surface normals, though.
   */
  bool SetRadius(double r);
  double GetRadius() const;
  //@}

  //@{
  /**
   * Force the disk widget's normal to be aligned with one of the x-y-z axes.
   * If one axis is set on, the other two will be set off.
   * Remember that when the state changes, a ModifiedEvent is invoked.
   * This can be used to snap the disk to the axes if it is originally
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
   * Enable/disable the drawing of the disk. In some cases the disk
   * interferes with the object that it is operating on (e.g., the
   * disk interferes with the cut surface it produces resulting in
   * z-buffer artifacts.) By default it is off.
   */
  void SetDrawDisk(vtkTypeBool drawCyl);
  vtkGetMacro(DrawDisk, vtkTypeBool);
  vtkBooleanMacro(DrawDisk, vtkTypeBool);
  //@}

  //@{
  /**
   * Set/Get the resolution of the disk. This is the number of
   * triangles used to approximate the circular face and line
   * segments used to approximate the circular edge.
   * A vtkDisk is used under the hood.
   */
  vtkSetClampMacro(Resolution, int, 3, VTK_MAX_DISK_RESOLUTION);
  vtkGetMacro(Resolution, int);
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

  /**
   * Get the implicit function for the disk. The user must provide an instance
   * vtkImplicitDisk; upon return, it will be set to the difference between
   * an infinite disk and the two cutting planes.
   * The returned implicit can be used by a variety of filters
   * to perform clipping, cutting, and selection of data.
   */
  void GetDisk(vtkImplicitDisk* disk);

  /**
   * Satisfies the superclass API.  This will change the state of the widget
   * to match changes that have been made to the underlying PolyDataSource.
   */
  void UpdatePlacement();

  //@{
  /**
   * Get the properties used to render the center point
   * when selected or not.
   */
  vtkGetObjectMacro(HandleProperty, vtkProperty);
  vtkGetObjectMacro(SelectedHandleProperty, vtkProperty);
  //@}

  //@{
  /**
   * Get the disk-face properties. The properties of the disk when selected
   * and unselected can be manipulated.
   */
  vtkGetObjectMacro(DiskProperty, vtkProperty);
  vtkGetObjectMacro(SelectedDiskProperty, vtkProperty);
  //@}

  //@{
  /**
   * Get the property of the boundary edge and normal line. (This property also
   * applies to the edges when tubed.)
   */
  vtkGetObjectMacro(EdgeProperty, vtkProperty);
  //@}

  //@{
  /**
   * Methods to interface with the vtkDiskWidget.
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
   * Specify a translation distance used by the BumpDisk() method. Note that the
   * distance is normalized; it is the fraction of the length of the bounding
   * box of the wire outline.
   */
  vtkSetClampMacro(BumpDistance, double, 0.000001, 1);
  vtkGetMacro(BumpDistance, double);
  //@}

  /**
   * Translate the disk in the direction of the view vector by the
   * specified BumpDistance. The dir parameter controls which
   * direction the pushing occurs, either in the same direction as the
   * view vector, or when negative, in the opposite direction.  The factor
   * controls what percentage of the bump is used.
   */
  void BumpDisk(int dir, double factor);

  /**
   * Push the disk the distance specified along the view
   * vector. Positive values are in the direction of the view vector;
   * negative values are in the opposite direction. The distance value
   * is expressed in world coordinates.
   */
  void PushDisk(double distance);

  /**\brief Set the disk normal to either the surface normal below the pointer
   *        or the opposite of the camera view vector (pointing back toward the camera).
   */
  bool PickNormal(int X, int Y, bool snapToMeshPoint);

  // Manage the state of the widget
  enum _InteractionState
  {
    Outside = 0,
    Moving,
    PushingDiskFace,
    AdjustingRadius,
    MovingCenterVertex,
    MovingWhole,
    RotatingNormal
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
  vtkSetClampMacro(InteractionState, int, Outside, RotatingNormal);
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

protected:
  vtkDiskRepresentation();
  ~vtkDiskRepresentation() override;

  /// Visual elements of the representation.
  enum ElementType
  {
    DiskFace = 0,
    DiskNormal,
    DiskEdge,
    CenterVertex,
    NumberOfElements
  };

  void HighlightElement(ElementType elem, int highlight);

  void HighlightDisk(int highlight);
  void HighlightAxis(int highlight);
  void HighlightCurve(int highlight);
  void HighlightHandle(int highlight);

  // Methods to manipulate the disk
  void Rotate(double X, double Y, double* p1, double* p2, double* vpn);
  void TranslateDisk(double* p1, double* p2);
  void PushFace(double* p1, double* p2);
  void AdjustRadius(double X, double Y, double* p1, double* p2);
  void TranslateCenter(double* p1, double* p2);
  void TranslateCenterInPlane(double* p1, double* p2);
  void TranslateHandle(double* p1, double* p2);
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
  double BumpDistance{ 0.01 };
  // Controlling ivars
  vtkTypeBool AlongXAxis{ 0 };
  vtkTypeBool AlongYAxis{ 0 };
  vtkTypeBool AlongZAxis{ 0 };
  // The actual disk which is being manipulated
  vtkNew<vtkDisk> Disk;
  // The facet resolution for rendering purposes.
  int Resolution{ 128 };
  vtkTypeBool DrawDisk{ 1 };
  vtkNew<vtkTubeFilter> AxisTuber; // Used to style edges.
  vtkTypeBool Tubing{ 1 };         //control whether tubing is on

  // Source of center-point handle geometry
  vtkNew<vtkSphereSource> Sphere;

  // Do the picking
  vtkNew<vtkCellPicker> Picker;
  vtkNew<vtkCellPicker> FacePicker;
  vtkNew<vtkHardwarePicker> HardwarePicker; // For picking surface normals.
  vtkTypeBool PickCameraFocalInfo{ true };  // Allow picking the camera projection direction.

  // Properties used to control the appearance of selected objects and
  // the manipulator in general.
  vtkNew<vtkProperty> HandleProperty;
  vtkNew<vtkProperty> SelectedHandleProperty;
  vtkNew<vtkProperty> DiskProperty;
  vtkNew<vtkProperty> SelectedDiskProperty;
  vtkNew<vtkProperty> EdgeProperty;
  vtkNew<vtkProperty> SelectedEdgeProperty;

  // Support GetBounds() method
  vtkNew<vtkBox> BoundingBox;

  // Overrides for the point-handle polydata mappers
  vtkNew<vtkGlyph3DMapper> CenterVertexMapper;

  vtkNew<vtkTransform> Transform;
};

#endif
