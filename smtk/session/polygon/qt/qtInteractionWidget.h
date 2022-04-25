//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_qt_InteractionWidget_h
#define smtk_qt_InteractionWidget_h

#include <QPointer>
#include <QWidget>
#include <vtkNew.h>
#include <vtkSmartPointer.h>

class pqView;
class vtkSMNewWidgetRepresentationProxy;
class vtkEventQtSlotConnect;

/// qtInteractionWidget is a base class for QWidgets that use a
/// vtkAbstractWidget/vtkWidgetRepresentation subclasses (via Proxy's of course)
/// to let user interactive setup parameters.
class qtInteractionWidget : public QWidget
{
  Q_OBJECT;
  typedef QWidget Superclass;
  Q_PROPERTY(bool enableInteractivity READ isInteractivityEnabled WRITE setEnableInteractivity);

public:
  ~qtInteractionWidget() override;

  //@{
  /// Set the pqView (typically pqRenderView) which in which to show the widget
  /// for interaction. There can only be one view in which the widget can be
  /// shown at a time. If the view is changed, the widget will be remove/hidden
  /// from the previous view. Set to nullptr to hide the widget.
  void setView(pqView*);
  pqView* view() const;
  //@}

  bool isInteractivityEnabled() const { return this->Interactivity; }

  /// Provides access to vtkSMNewWidgetRepresentationProxy used.
  vtkSMNewWidgetRepresentationProxy* widgetProxy() const;

public Q_SLOTS:
  //@{
  /// Controls the visibility and interactivity of the vtkWidgetRepresentation
  /// in the view set using `setView`. Note, if no active view is set (or is of
  /// an incompatible type), the enabling interactivity will have no effect till
  /// a compatible view is provided.
  void setEnableInteractivity(bool val);
  void enableInteractivity() { this->setEnableInteractivity(true); }
  void disableInteractivity() { this->setEnableInteractivity(false); }
  //@}

Q_SIGNALS:
  /// fired when interactivity state is changed.
  void enableInteractivityChanged(bool enabled);

  /// Notifies observers that the user is dragging the 3D widget
  void widgetStartInteraction();

  /// Notifies observers that the user is done dragging the 3D widget
  void widgetEndInteraction();

  /// Notifies observers that the user is dragging the 3D widget
  void widgetInteraction();

protected Q_SLOTS:
  /// call to trigger a render.
  void render();

protected:
  qtInteractionWidget(
    const vtkSmartPointer<vtkSMNewWidgetRepresentationProxy>& proxy,
    QWidget* parent = nullptr);

  /// Convenience method to create a new proxy for the given type.
  static vtkSmartPointer<vtkSMNewWidgetRepresentationProxy> createWidget(
    const char* smgroup,
    const char* smname);

private:
  Q_DISABLE_COPY(qtInteractionWidget);
  QPointer<pqView> View;
  vtkSmartPointer<vtkSMNewWidgetRepresentationProxy> WidgetProxy;
  bool Interactivity;
  vtkNew<vtkEventQtSlotConnect> VTKConnect;
};

#endif
