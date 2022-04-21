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
/*=========================================================================

   Program: ParaView
   Module:  pqPointPropertyWidget.h

   Copyright (c) 2005,2006 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2.

   See License_v1.2.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

========================================================================*/
#ifndef pqPointPropertyWidget_h
#define pqPointPropertyWidget_h

#include "smtk/extension/paraview/widgets/pqSMTKInteractivePropertyWidget.h"
#include "smtk/extension/paraview/widgets/smtkPQWidgetsExtModule.h"

// VTK's wrapper parser does not properly handle Qt macros on macos.
#if defined(__VTK_WRAP__) && !defined(Q_SLOTS)
#define Q_DISABLE_COPY(x)
#define Q_SLOTS
#define Q_SIGNALS public
#define Q_OBJECT
#endif

class QCheckBox;
class pqPointPickingHelper;

/**
* pqPointPropertyWidget is a custom property widget that uses
* "HandleWidgetRepresentation" to help users interactively set a 3D point in
* space. To use this widget for a property group
* (vtkSMPropertyGroup), use "InteractiveHandle" as the "panel_widget" in the
* XML configuration for the proxy. The property group should have properties for
* following functions:
* \li \c WorldPosition: a 3-tuple vtkSMDoubleVectorProperty that will be linked to the
* origin of the interactive plane.
* \li \c Input: (optional) a vtkSMInputProperty that is used to get data
* information for bounds when placing/resetting the widget.
*/
class SMTKPQWIDGETSEXT_EXPORT pqPointPropertyWidget : public pqSMTKInteractivePropertyWidget
{
  Q_OBJECT
  typedef pqSMTKInteractivePropertyWidget Superclass;

public:
  pqPointPropertyWidget(vtkSMProxy* proxy, vtkSMPropertyGroup* smgroup, QWidget* parent = nullptr);
  ~pqPointPropertyWidget() override;

  std::string controlState();

public Q_SLOTS:
  /// Set the state of the controls (to one of: "active", "visible", "inactive").
  void setControlState(const std::string& state);

  /// Set the state of the controls (to one of: 0 (inactive), 1 (visible), 2 (active)).
  void setControlState(int checkState);

  /// Show Qt controls for widget visibility and interactivity.
  void setControlVisibility(bool show);

Q_SIGNALS:
  /// The visibility (bit 0x01) or interactivity (bit 0x02) has changed.
  void controlStateChanged(const std::string& state);

protected Q_SLOTS:
  /**
  * Places the interactive widget using current data source information.
  */
  void placeWidget() override;

private Q_SLOTS:
  void setWorldPosition(double x, double y, double z);

protected:
  int m_state;
  pqPointPickingHelper* m_surfacePickHelper;
  pqPointPickingHelper* m_pointPickHelper;
  QCheckBox* m_control;

private:
  Q_DISABLE_COPY(pqPointPropertyWidget);
};

#endif
