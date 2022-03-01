//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtOperationAction.h"

#include "smtk/extension/qt/SVGIconEngine.h"
#include "smtk/extension/qt/qtDoubleClickButton.h"
#include "smtk/extension/qt/qtOperationTypeModel.h"

#include "smtk/view/Manager.h"
#include "smtk/view/OperationIcons.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Metadata.h"
#include "smtk/operation/MetadataContainer.h"
#include "smtk/operation/Operation.h"
#include "smtk/operation/SpecificationOps.h"

#include "smtk/attribute/Definition.h"

#include "smtk/io/Logger.h"

#include "smtk/common/Color.h"

#include <QApplication>
#include <QEvent>
#include <QMouseEvent>
#include <QPalette>
#include <QToolButton>

#include <algorithm>
#include <iostream>

using smtk::attribute::utility::EditableParameters;

qtOperationAction::qtOperationAction(
  const std::shared_ptr<smtk::operation::Manager>& operationManager,
  const std::shared_ptr<smtk::view::Manager>& viewManager,
  smtk::operation::Operation::Index typeIndex,
  qtOperationTypeModel* parent)
  : QWidgetAction(parent)
  , m_typeIndex(typeIndex)
{
  (void)operationManager;
  m_model = parent;
  QModelIndex idx = parent->findByTypeIndex(m_typeIndex);
  if (idx.isValid())
  {
    this->setOperationName(
      idx.sibling(idx.row(), static_cast<int>(qtOperationTypeModel::Column::Label))
        .data()
        .toString()
        .toStdString());
    if (viewManager)
    {
      auto baseColor = qApp->palette("QPushButton").base().color();
      std::array<double, 4> secondaryColorF{
        { baseColor.redF(), baseColor.greenF(), baseColor.blueF(), baseColor.alphaF() }
      };
      auto secondaryColor = smtk::common::Color::floatRGBToString(secondaryColorF.data());
      auto& icons = viewManager->operationIcons();
      std::string svg = icons.createIcon(m_typeIndex, secondaryColor);
      QIcon opIcon(new smtk::extension::SVGIconEngine(svg));
      this->setIcon(opIcon);
      // If the operation parameters have a brief description, make a tool-tip.
      QString toolTip =
        idx.sibling(idx.row(), static_cast<int>(qtOperationTypeModel::Column::ToolTip))
          .data()
          .toString();
      if (!toolTip.isEmpty())
      {
        this->setToolTip(toolTip);
      }
    }
    QObject::connect(&m_timer, SIGNAL(timeout()), this, SLOT(parameterTimerElapsed()));
  }
  else
  {
    m_operationName = "Error";
    this->setEnabled(false);
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Invalid operation index " << typeIndex << " (no matching metadata)");
  }
}

qtOperationAction::~qtOperationAction() = default;

void qtOperationAction::setButtonStyle(Qt::ToolButtonStyle style)
{
  m_style = style;
  m_toolButton = (style == Qt::ToolButtonIconOnly);
}

QWidget* qtOperationAction::createWidget(QWidget* parent)
{
  const QIcon& icon = this->icon();
  std::string buttonText = m_operationName;
  if (m_editableParameters == EditableParameters::Mandatory)
  {
    buttonText += "[...]";
  }
  else if (m_editableParameters == EditableParameters::Optional)
  {
    buttonText += "(...)";
  }
  if (m_toolButton)
  {
    auto* w = new QToolButton(parent);
    w->setToolButtonStyle(m_style);
    if (!icon.isNull())
    {
      w->setIcon(icon);
    }
    if (!this->toolTip().isEmpty())
    {
      w->setToolTip(this->toolTip());
    }
    w->setText(buttonText.c_str());
    w->setDefaultAction(this); // Provide a reference back to here.
    w->setObjectName(buttonText.c_str());
    w->setPopupMode(QToolButton::DelayedPopup);
    w->setArrowType(Qt::NoArrow);
    QObject::connect(w, &QToolButton::triggered, this, &qtOperationAction::defaultAction);
    return w;
  }
  else
  {
    auto* w = new qtDoubleClickButton(parent);
    if (!icon.isNull() && m_style != Qt::ToolButtonTextOnly)
    {
      w->setIcon(icon);
      // w->setToolButtonStyle(m_style);
    }
    w->setStyleSheet("text-align: left;"); // We want both icons and buttons left-justified
    if (!this->toolTip().isEmpty())
    {
      w->setToolTip(this->toolTip());
    }
    if (m_style != Qt::ToolButtonIconOnly)
    {
      w->setText(buttonText.c_str());
    }
    else
    {
      // Icon-only buttons should be suitable for toolbars.
      w->setFlat(true);
    }
    w->addAction(this); // Provide a reference back to here.
    w->setObjectName(buttonText.c_str());
    // w->installEventFilter(this);
    QObject::connect(w, &qtDoubleClickButton::clicked, this, &qtOperationAction::editParameters);
    QObject::connect(
      w, &qtDoubleClickButton::doubleClicked, this, &qtOperationAction::acceptDefaults);
    return w;
  }
}

void qtOperationAction::deleteWidget(QWidget* widget)
{
  delete widget;
}

void qtOperationAction::forceStyle(Qt::ToolButtonStyle buttonStyle, ActionFunctor functor)
{
  Qt::ToolButtonStyle lastStyle = m_style;
  this->setButtonStyle(buttonStyle);
  functor(this);
  this->setButtonStyle(lastStyle);
}

void qtOperationAction::parameterTimerElapsed()
{
  m_timer.stop();
  if (m_editableParameters != EditableParameters::None)
  {
    emit editParameters();
  }
}

void qtOperationAction::defaultAction()
{
  if (m_editableParameters == EditableParameters::None)
  {
    emit acceptDefaults();
    return;
  }
  emit editParameters();
}

bool qtOperationAction::eventFilter(QObject* watched, QEvent* event)
{
  if (watched != this) // must be a QPushButton we created during createWidget()
  {
    if (event->type() == QEvent::MouseButtonPress)
    {
      m_timer.start(longPress);
    }
    else if (event->type() == QEvent::MouseButtonRelease)
    {
      bool stoppedShort = false;
      if (m_timer.isActive())
      {
        m_timer.stop();
        stoppedShort = true;
      }
      auto* ww = dynamic_cast<QWidget*>(watched);
      if (ww)
      {
        /*
        std::cout
          << "  " << ww->objectName().toStdString() << " release."
          << " underMouse: " << (ww->underMouse() ? "T" : "F")
          << "\n";
          */
        auto posn = static_cast<QMouseEvent*>(event)->pos();
        if (!ww->rect().contains(posn))
        {
          return true;
        }
      }
      if (
        m_editableParameters == EditableParameters::None ||
        (m_editableParameters == EditableParameters::Optional && stoppedShort))
      {
        emit acceptDefaults();
      }
      else if (
        !stoppedShort || (m_editableParameters == EditableParameters::Mandatory && stoppedShort))
      {
        emit editParameters();
      }
      return watched->eventFilter(watched, event);
    }
  }
  return QWidgetAction::eventFilter(watched, event);
}

void qtOperationAction::setOperationName(const std::string& label)
{
  // Trim everything to the left of "::" and " - "
  std::size_t colon = label.rfind("::");
  std::size_t dash = label.rfind(" - ");
  std::size_t trim = 0;
  if (colon != std::string::npos)
  {
    trim = colon + 1;
  }
  if (dash != std::string::npos && (dash + 2 > trim))
  {
    trim = dash + 2;
  }
  m_operationName = label.substr(trim);

  // Now search for the space closest to the center of
  // the string and change it into a linebreak.
  std::size_t ss = m_operationName.size();
  std::size_t ml = ss / 2;
  std::size_t mr = ss / 2 + 1;
  for (; ml > 0; --ml)
  {
    if (m_operationName[ml] == ' ')
    {
      break;
    }
  }
  for (; mr < ss; ++mr)
  {
    if (m_operationName[mr] == ' ')
    {
      break;
    }
  }
  std::size_t splitPoint = (ss / 2 - ml < mr - ss / 2) ? ml : mr;
  if (splitPoint != 0 && splitPoint != ss)
  {
    m_operationName[splitPoint] = '\n';
  }
}
