//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/widgets/pqSMTKAttributeItemWidget.h"

#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/operation/Observer.h"

#include "pqInteractivePropertyWidget.h"

#include "vtkEventQtSlotConnect.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QPointer>

/**\brief State shared by all ParaView-enabled qtItem widgets.
  *
  * ParaView has a standard API (the pqInteractivePropertyWidget)
  * for widgets that have representations in render views.
  * Instances of this class are held by the pqSMTKAttributeItemWidget
  * and used by subclasses to manage properties specific to
  * the type of ParaView widget they expose.
  */
class pqSMTKAttributeItemWidget::Internal
{
public:
  enum class State
  {
    Idle,                  //!< Widget is not being manipulated by attribute system or user.
    UpdatingFromUI,        //!< Widget was manipulated by user.
    UpdatingFromAttribute, //!< Widget was manipulated by attribute system.
    Interacting            //!< Widget is being manipulated by user; do not update item yet.
  };

  Internal(
    smtk::attribute::ItemPtr itm,
    QWidget* p,
    smtk::extension::qtBaseView* bview,
    Qt::Orientation orient)
    : m_orientation(orient)
  {
    (void)itm;
    (void)p;
    (void)bview;
  }

  // state of this item
  QPointer<QGridLayout> m_layout;
  QPointer<QLabel> m_label;
  Qt::Orientation m_orientation;
  pqInteractivePropertyWidget* m_pvwidget;
  // pqDataRepresentation* m_pvrepr;
  vtkNew<vtkEventQtSlotConnect> m_connector;
  OverrideWhen m_overrideWhen{ OverrideWhen::Unset };
  GeometrySource m_geometrySource{ GeometrySource::BestGuess };
  FallbackStrategy m_fallbackStrategy{ FallbackStrategy::Hide };

  // state of children
  QMap<QWidget*, QPair<QLayout*, QWidget*>> m_children;

  smtk::operation::Observers::Key m_opObserver;
  State m_state{ State::Idle };
};
