//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_pq_3DWidget_h
#define __smtk_pq_3DWidget_h

#include "smtk/extension/paraview/widgets/Exports.h"
#include "pqProxyPanel.h"

class pq3DWidgetInternal;
class pqPipelineSource;
class pqProxy;
class pqRenderViewBase;
class QKeySequence;
class vtkObject;
class vtkPVXMLElement;
class vtkSMNewWidgetRepresentationProxy;
class vtkSMProperty;

/// pq3DWidget is the abstract superclass for all 3D widgets.
/// This class represents a 3D Widget proxy as well as the GUI for the
/// widget.
class SMTKPQWIDGETSEXT_EXPORT pq3DWidget : public pqProxyPanel
{
  Q_OBJECT
  typedef pqProxyPanel Superclass;
public:
  pq3DWidget(vtkSMProxy* referenceProxy, vtkSMProxy* proxy, QWidget* parent=0);
  virtual ~pq3DWidget();

  /// Controlled proxy is a proxy which is controlled by the 3D widget.
  /// A controlled proxy must provide "Hints" describing how
  /// the properties of the controlled proxy are controlled by the
  /// 3D widget.
  vtkSMProxy* getControlledProxy() const;
  
  vtkSMProxy* getReferenceProxy() const;

  /// Set the hints XML to be using to map the 3D widget to the controlled
  /// proxy. This method must be called only after the controlled
  /// proxy has been set. The argument is the element
  /// <PropertyGroup /> which will be controlled by this widget.
  void setHints(vtkPVXMLElement* element);
  vtkPVXMLElement* getHints() const;

  /// Return the 3D Widget proxy.
  vtkSMNewWidgetRepresentationProxy* getWidgetProxy() const;

  /// Returns true if 3D widget visibility is enabled.
  /// Note: this *does not* indicate that the 3D widget is currently visible
  /// in the display, since this widget's panel might not be visible.
  bool widgetVisible() const;

  /// Returns true if 3D widget is selected.
  bool widgetSelected() const;
  
  /// Returns the current render view.
  pqRenderViewBase* renderView() const;

  /// Reset the bounds to the specified bounds.
  /// This typically calls PlaceWidget on the underlying 3D Widget 
  /// with reference proxy bounds.
  virtual void resetBounds(double bounds[6])=0;

signals:
  /// Notifies observers that widget visibility has changed
  void widgetVisibilityChanged(bool);

  /// Notifies observers that the user is dragging the 3D widget
  void widgetStartInteraction();

  /// Notifies observers that the user is done dragging the 3D widget
  void widgetEndInteraction();

  /// Notifies observers that the user is dragging the 3D widget
  void widgetInteraction();

public slots:
  /// Sets 3D widget visibility
  void setWidgetVisible(bool);
  /// Makes the 3D widget visible. 
  void showWidget();
  /// Hides the 3D widget.
  void hideWidget();

  /// Activates the widget. Respects the visibility flag.
  virtual void select();

  /// Deactivates the widget.
  virtual void deselect();

  /// Accepts pending changes. Default implementation
  /// pushes the 3D widget information property values to
  /// the corresponding properties on the controlled widget.
  /// The correspondence is determined from the <Hints />
  /// associated with the controlled proxy.
  virtual void accept();

  /// Resets pending changes. Default implementation
  /// pushes the property values of the controlled widget to the
  /// 3D widget properties.
  /// The correspondence is determined from the <Hints />
  /// associated with the controlled proxy.
  virtual void reset();

  /// Set the view that this panel works with
  virtual void setView(pqView*);

  /// Resets the bounds of the 3D widget to the reference proxy bounds.
  /// This typically calls PlaceWidget on the underlying 3D Widget 
  /// with reference proxy bounds.
  /// This should be explicitly called after the panel is created
  /// and the widget is initialized i.e. the reference proxy, controlled proxy
  /// and hints have been set.
  /// Default implementation uses the getReferenceInputBounds() to get the
  /// bounds and then calls resetBounds(double bounds[]). Subclasses generally
  /// need to override this resetBounds(double*) method.
  virtual void resetBounds();

  /// When set to true, instead of using the referenceProxy to obtain the
  /// default bounds to reset to, it will use the bounds for the selected sources
  /// as indicated by
  /// pqApplicationCore::getSelectionModel()->getSelectionDataBounds().
  /// Default is false.
  virtual void setUseSelectionDataBounds(bool use)
    { this->UseSelectionDataBounds = use; }
  bool useSelectionDataBounds()
    {return this->UseSelectionDataBounds; }

protected slots:
  /// Called to request a render.
  void render();

  // When set to true, instead of intersecting with the mesh surface for picking, it will only get a close point from the mesh
  void setPickOnMeshPoint(bool);

  /// triggers a pick action using the current location of the mouse.
  void pickPoint();
  
  /// Called on each pick, default implementation does nothing.
  virtual void pick(double, double, double) {};

  /// Called when master/slave change
  virtual void updateMasterEnableState(bool);

  /// Handle custom user notification to show/hide corresponding widget
  void handleReferenceProxyUserEvent(vtkObject*, unsigned long, void*);

protected:
 
  // Return true if picking is on mesh point only
  bool pickOnMeshPoint() const;

  /// Subclasses can override this method to map properties to
  /// GUI. Default implementation updates the internal datastructures
  /// so that default implementations can be provided for 
  /// accept/reset.
  virtual void setControlledProperty(const char* function,
    vtkSMProperty * controlled_property);
  
  /// Subclasses should call this method if they support picking.
  /// When the user picks a position,
  /// the virtual method pick(double, double, double) will be called.
  void pickingSupported(const QKeySequence& key);

  void setControlledProperty(vtkSMProperty* widget_property, vtkSMProperty* controlled_property);

  // Subclasses must set the widget proxy.
  void setWidgetProxy(vtkSMNewWidgetRepresentationProxy*);

  /// Called when one of the controlled properties change (e.g: by undo/redo)
  virtual void onControlledPropertyChanged();

  /// Used to get the input bounds on for the reference proxy, if any.
  /// returns 1 on success, 0 otherwise.
  int getReferenceInputBounds(double bounds[6]) const;
  
  /// Update the widget visibility according to the WidgetVisible and Selected flags
  virtual void updateWidgetVisibility();
  
  /// Update the widget visibility and enable state
  virtual void updateWidgetState(bool visible, bool enable);
  
  /// updates the enable state of the picking shortcut.
  virtual void updatePickShortcut();
  virtual void updatePickShortcut(bool pickable);
  
private:
  void setControlledProxy(vtkSMProxy*);

  pq3DWidgetInternal* const Internal;

  bool UseSelectionDataBounds;
};

#endif
