//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef vtkDiskWidget_h
#define vtkDiskWidget_h

#include "smtk/extension/vtk/widgets/vtkSMTKWidgetsExtModule.h" // For export macro
#include "vtkAbstractWidget.h"

class vtkDiskRepresentation;

/**
 * @class   vtkDiskWidget
 * @brief   3D widget for manipulating an infinite cylinder
 *
 * This 3D widget defines a planar, circular disk that can be
 * interactively placed in a scene. The widget is assumed to consist
 * of four parts: 1) a disk contained in a 2) bounding box, with a
 * 3) normal vector, which is rooted at a 4) center point in the bounding
 * box. (The representation paired with this widget determines the
 * actual geometry of the widget.)
 *
 * To use this widget, you generally pair it with a vtkDiskRepresentation
 * (or a subclass). Various options are available for controlling how the
 * representation appears, and how the widget functions.
 *
 * @par Event Bindings:
 * By default, the widget responds to the following VTK events (i.e., it
 * watches the vtkRenderWindowInteractor for these events):
 * <pre>
 * If the normal vector is selected:
 *   LeftButtonPressEvent - select normal
 *   LeftButtonReleaseEvent - release (end select) normal
 *   MouseMoveEvent - orient the normal vector
 * If the center point (handle) is selected:
 *   LeftButtonPressEvent - select handle (if on slider)
 *   LeftButtonReleaseEvent - release handle (if selected)
 *   MouseMoveEvent - move the center point (constrained to plane or on the
 *                    axis if CTRL key is pressed)
 * If the disk is selected:
 *   LeftButtonPressEvent - select disk
 *   LeftButtonReleaseEvent - release disk
 *   MouseMoveEvent - increase/decrease disk radius
 * If the outline is selected:
 *   LeftButtonPressEvent - select outline
 *   LeftButtonReleaseEvent - release outline
 *   MouseMoveEvent - move the outline
 * If the keypress characters are used
 *   'Down/Left' Move disk away from viewer
 *   'Up/Right' Move disk towards viewer
 * In all the cases, independent of what is picked, the widget responds to the
 * following VTK events:
 *   MiddleButtonPressEvent - move the disk
 *   MiddleButtonReleaseEvent - release the disk
 *   RightButtonPressEvent - scale the widget's representation
 *   RightButtonReleaseEvent - stop scaling the widget
 *   MouseMoveEvent - scale (if right button) or move (if middle button) the widget
 * </pre>
 *
 * @par Event Bindings:
 * Note that the event bindings described above can be changed using this
 * class's vtkWidgetEventTranslator. This class translates VTK events
 * into the vtkDiskWidget's widget events:
 * <pre>
 *   vtkWidgetEvent::Select -- some part of the widget has been selected
 *   vtkWidgetEvent::EndSelect -- the selection process has completed
 *   vtkWidgetEvent::Move -- a request for widget motion has been invoked
 *   vtkWidgetEvent::Up and vtkWidgetEvent::Down -- MoveConeAction
 * </pre>
 *
 * @par Event Bindings:
 * In turn, when these widget events are processed, the vtkDiskWidget
 * invokes the following VTK events on itself (which observers can listen for):
 * <pre>
 *   vtkCommand::StartInteractionEvent (on vtkWidgetEvent::Select)
 *   vtkCommand::EndInteractionEvent (on vtkWidgetEvent::EndSelect)
 *   vtkCommand::InteractionEvent (on vtkWidgetEvent::Move)
 * </pre>
 *
 *
 * @sa
 * vtk3DWidget vtkImplicitPlaneWidget
*/
class VTKSMTKWIDGETSEXT_EXPORT vtkDiskWidget : public vtkAbstractWidget
{
public:
  /**
   * Instantiate the object.
   */
  static vtkDiskWidget* New();

  //@{
  /**
   * Standard vtkObject methods
   */
  vtkTypeMacro(vtkDiskWidget, vtkAbstractWidget);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  //@}

  vtkDiskWidget(const vtkDiskWidget&) = delete;
  vtkDiskWidget& operator=(const vtkDiskWidget&) = delete;

  /**
   * Specify an instance of vtkWidgetRepresentation used to represent this
   * widget in the scene. Note that the representation is a subclass of vtkProp
   * so it can be added to the renderer independent of the widget.
   */
  void SetRepresentation(vtkDiskRepresentation* rep);

  /// Control widget interactivity, allowing users to interact with the camera or other widgets.
  ///
  /// The camera is unobserved when the widget is disabled.
  void SetEnabled(int enabling) override;

  /**
   * Return the representation as a vtkDiskRepresentation.
   */
  vtkDiskRepresentation* GetDiskRepresentation()
  {
    return reinterpret_cast<vtkDiskRepresentation*>(this->WidgetRep);
  }

  /**
   * Create the default widget representation if one is not set.
   */
  void CreateDefaultRepresentation() override;

protected:
  vtkDiskWidget();
  ~vtkDiskWidget() override;

  // Manage the state of the widget
  int WidgetState;
  enum _WidgetState
  {
    Start = 0,
    Active
  };

  // These methods handle events
  static void SelectAction(vtkAbstractWidget*);
  static void TranslateAction(vtkAbstractWidget*);
  static void ScaleAction(vtkAbstractWidget*);
  static void EndSelectAction(vtkAbstractWidget*);
  static void MoveAction(vtkAbstractWidget*);
  static void MoveDiskAction(vtkAbstractWidget*);
  static void PickNormalAction(vtkAbstractWidget*);

  /**
   * Update the cursor shape based on the interaction state. Returns 1
   * if the cursor shape requested is different from the existing one.
   */
  int UpdateCursorShape(int interactionState);
};

#endif
