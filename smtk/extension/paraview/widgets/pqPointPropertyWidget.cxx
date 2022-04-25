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
   Module:  pqPointPropertyWidget.cxx

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
#include "pqPointPropertyWidget.h"
#include "ui_pqPointPropertyWidget.h"

#include "smtk/extension/paraview/widgets/pqPointPickingVisibilityHelper.h"

#include "pqActiveObjects.h"
#include "pqPointPickingHelper.h"

#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMPropertyGroup.h"
#include "vtkSMPropertyHelper.h"

#include <QCheckBox>

#include <algorithm>
#include <cctype>

constexpr const char* tooltipInactive =
  "Edit point coordinates manually or click the checkbox to enable 3-d editing.";
constexpr const char* tooltipVisible =
  "Edit point coordinates manually or drag the point in the render view. "
  "Click the checkbox to enable picking.";
#ifdef Q_OS_MAC
constexpr const char* tooltipActive =
  "Edit point coordinates manually or drag the point in the render view. "
  "Type 'P' to pick a point on any surface under the cursor or "
  "'Cmd+P' to snap to the closest point used to define the underlying surface. "
  "Click the checkbox to hide the 3-d widget.";
#else
constexpr const char* tooltipActive =
  "Edit point coordinates manually or drag the point in the render view. "
  "Type 'P' to pick a point on any surface under the cursor or "
  "'Ctrl+P' to snap to the closest point used to define the underlying surface. "
  "Click the checkbox to hide the 3-d widget.";
#endif

pqPointPropertyWidget::pqPointPropertyWidget(
  vtkSMProxy* smproxy,
  vtkSMPropertyGroup* smgroup,
  QWidget* parentObject)
  : Superclass("representations", "HandleWidgetRepresentation", smproxy, smgroup, parentObject)
  , m_state(0)
  , m_surfacePickHelper(nullptr)
  , m_pointPickHelper(nullptr)
{
  Ui::PointPropertyWidget ui;
  ui.setupUi(this);

  if (vtkSMProperty* worldPosition = smgroup->GetProperty("WorldPosition"))
  {
    this->addPropertyLink(
      ui.worldPositionX, "text2", SIGNAL(textChangedAndEditingFinished()), worldPosition, 0);
    this->addPropertyLink(
      ui.worldPositionY, "text2", SIGNAL(textChangedAndEditingFinished()), worldPosition, 1);
    this->addPropertyLink(
      ui.worldPositionZ, "text2", SIGNAL(textChangedAndEditingFinished()), worldPosition, 2);
  }
  else
  {
    qCritical("Missing required property for function 'WorldPosition'");
  }

  m_control = ui.show3DWidget;
  ui.show3DWidget->setCheckState(Qt::Checked);
  // If the user toggles the Qt checkbox, update the 3D widget state:
  this->connect(ui.show3DWidget, SIGNAL(stateChanged(int)), SLOT(setControlState(int)));
  this->setControlState("active");
}

pqPointPropertyWidget::~pqPointPropertyWidget() = default;

std::string pqPointPropertyWidget::controlState()
{
  return m_state == 0 ? "inactive" : (m_state == 1 ? "visible" : "active");
}

void pqPointPropertyWidget::setControlState(const std::string& data)
{
  int startState = m_state;
  std::string state = data;
  std::transform(
    state.begin(), state.end(), state.begin(), [](unsigned char c) { return std::tolower(c); });
  if (state == "active")
  {
    m_state = 3;
    this->setToolTip(tooltipActive);
  }
  else if (state == "visible")
  {
    m_state = 1;
    this->setToolTip(tooltipVisible);
  }
  else if (state == "inactive")
  {
    m_state = 0;
    this->setToolTip(tooltipInactive);
  }

  this->setWidgetVisible(m_state & 0x01);

  if (m_state & 0x02)
  {
    auto* currView = pqActiveObjects::instance().activeView();
    if (!m_surfacePickHelper)
    {
      m_surfacePickHelper = new pqPointPickingHelper(QKeySequence(tr("P")), false, this);
      m_surfacePickHelper->connect(this, SIGNAL(viewChanged(pqView*)), SLOT(setView(pqView*)));
      this->connect(
        m_surfacePickHelper,
        SIGNAL(pick(double, double, double)),
        SLOT(setWorldPosition(double, double, double)));
      pqPointPickingVisibilityHelper<pqPointPickingHelper>{ *this, *m_surfacePickHelper };
      m_surfacePickHelper->setView(currView);
    }

    if (!m_pointPickHelper)
    {
      m_pointPickHelper = new pqPointPickingHelper(QKeySequence(tr("Ctrl+P")), true, this);
      m_pointPickHelper->connect(this, SIGNAL(viewChanged(pqView*)), SLOT(setView(pqView*)));
      this->connect(
        m_pointPickHelper,
        SIGNAL(pick(double, double, double)),
        SLOT(setWorldPosition(double, double, double)));
      pqPointPickingVisibilityHelper<pqPointPickingHelper>{ *this, *m_pointPickHelper };
      m_pointPickHelper->setView(currView);
    }
  }
  else
  {
    if (m_surfacePickHelper)
    {
      m_surfacePickHelper->disconnect(this);
      delete m_surfacePickHelper;
      m_surfacePickHelper = nullptr;
    }

    if (m_pointPickHelper)
    {
      m_pointPickHelper->disconnect(this);
      delete m_pointPickHelper;
      m_pointPickHelper = nullptr;
    }
  }
  if (startState != m_state)
  {
    m_control->setCheckState(
      m_state == 0x00 ? Qt::Unchecked : (m_state == 0x01 ? Qt::PartiallyChecked : Qt::Checked));
    Q_EMIT controlStateChanged(this->controlState());
  }
}

void pqPointPropertyWidget::setControlState(int checkState)
{
  // Note that \a checkState is **not** a proper value for m_state!
  // It is the value provided by a QCheckBox (Qt::CheckState).
  switch (checkState)
  {
    case Qt::Unchecked:
      this->setControlState("inactive");
      break;
    case Qt::PartiallyChecked:
      this->setControlState("visible");
      break;
    case Qt::Checked:
      this->setControlState("active");
      break;
    default:
      std::cerr << "Invalid check state " << checkState << "\n";
      break;
  }
}

void pqPointPropertyWidget::setControlVisibility(bool show)
{
  m_control->setVisible(show);
}

void pqPointPropertyWidget::placeWidget()
{
  // nothing to do.
}

void pqPointPropertyWidget::setWorldPosition(double wx, double wy, double wz)
{
  vtkSMProxy* wdgProxy = this->widgetProxy();
  double o[3] = { wx, wy, wz };
  vtkSMPropertyHelper(wdgProxy, "WorldPosition").Set(o, 3);
  wdgProxy->UpdateVTKObjects();
  Q_EMIT this->changeAvailable();
  this->render();
}
