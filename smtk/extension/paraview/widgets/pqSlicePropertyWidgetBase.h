//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#ifndef pqSlicePropertyWidgetBase_h
#define pqSlicePropertyWidgetBase_h

#include "smtk/extension/paraview/widgets/pqSMTKInteractivePropertyWidget.h"
#include "smtk/extension/paraview/widgets/smtkPQWidgetsExtModule.h"

#include <QColor>
#include <QString>

// VTK's wrapper parser does not properly handle Qt macros on macos.
#if defined(__VTK_WRAP__) && !defined(Q_SLOTS)
#define Q_DISABLE_COPY(x)
#define Q_SLOTS
#define Q_SIGNALS public
#define Q_OBJECT
#endif

class pqPipelineSource;
class pqSlicePropertyWidgetBaseP;

/**
 * pqSlicePropertyWidgetBase is a custom property widget that uses
 * "SliceWidgetRepresentation" to help users interactively set the origin
 * and normal for a plane. To use this widget for a property group
 * (vtkSMPropertyGroup), use "InteractivePlane" as the "panel_widget" in the
 * XML configuration for the proxy. The property group should have properties for
 * following functions:
 * \li \c Origin: a 3-tuple vtkSMDoubleVectorProperty that will be linked to the
 * origin of the interactive plane.
 * \li \c Normal: a 3-tuple vtkSMDoubleVectorProperty that will be linked to the
 * normal for the interactive plane.
 * \li \c Input: (optional) a vtkSMInputProperty that is used to get data
 * information for bounds when placing/resetting the widget.
 */
class SMTKPQWIDGETSEXT_EXPORT pqSlicePropertyWidgetBase : public pqSMTKInteractivePropertyWidget
{
  Q_OBJECT
  typedef pqSMTKInteractivePropertyWidget Superclass;

public:
  /**\brief Create a widget for crinkle-slicing an \a input dataset.
    *
    * The \a proxy references some object whose \a smgroup contains
    * an Origin and a Normal property defining the slice plane.
    * Each of these properties is expected to hold 3 floating-point values.
    */
  pqSlicePropertyWidgetBase(
    const char* widgetGroup,
    const char* widgetName,
    pqPipelineSource* input,
    vtkSMProxy* proxy,
    vtkSMPropertyGroup* smgroup,
    QWidget* parent = nullptr);
  ~pqSlicePropertyWidgetBase() override;

  /**
   * Overridden to update the DrawPlane state.
   */
  void apply() override;
  void reset() override;

public Q_SLOTS:
  /**
   * Slots used to toggle the visibility of the translucent plane.
   */
  void showPlane() { this->setDrawPlane(true); }
  void hidePlane() { this->setDrawPlane(false); }
  void setDrawPlane(bool val);

  /**
   * Slots used to toggle the visibility of the plane-editing handles.
   */
  void setDrawHandles(bool val);

  /**
   * Set the widget normal to be along the X axis.
   */
  void useXNormal() { this->setNormal(1, 0, 0); }

  /**
   * Set the widget normal to be along the Y axis.
   */
  void useYNormal() { this->setNormal(0, 1, 0); }

  /**
   * Set the widget normal to be along the Z axis.
   */
  void useZNormal() { this->setNormal(0, 0, 1); }

  /**
   * Update the widget's origin and bounds using current data bounds.
   */
  void resetToDataBounds();

  /**
   * Reset the camera to be down the plane normal.
   */
  void resetCameraToNormal();

  /**
   * Set the widget normal to be along the camera view direction.
   */
  void useCameraNormal();

  /**\brief Update the representation used for color-by arrays as the view changes.
   */
  void setView(pqView* view) override;

  /**\brief Update the slice's array coloring.
   */
  void updateColorByArray();

  /**\brief Update the slice's edge visibility.
   */
  void setDrawCrinkleEdges(bool visible);

  /**\brief Update the slice's edge color.
   */
  void setCrinkleEdgeColor(const QColor& color);

protected Q_SLOTS:
  /**
   * Places the interactive widget using current data source information.
   */
  void placeWidget() override;

private Q_SLOTS:
  void setOrigin(double x, double y, double z);
  void setNormal(double x, double y, double z);

protected:
  friend class pqSlicePropertyWidgetBaseP;
  pqSlicePropertyWidgetBaseP* m_p;

private:
  Q_DISABLE_COPY(pqSlicePropertyWidgetBase)
};

#endif
