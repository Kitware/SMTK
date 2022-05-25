//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "qtCollapsibleGroupWidget.h"
#include "ui_qtCollapsibleGroupWidgetInternals.h"
#include <QCheckBox>
#include <QFrame>
using namespace smtk::extension;

class smtk::extension::qtCollapsibleGroupWidgetInternals
  : public Ui::qtCollapsibleGroupWidgetInternals
{
public:
  qtCollapsibleGroupWidgetInternals() = default;
};

qtCollapsibleGroupWidget::qtCollapsibleGroupWidget(
  QWidget* p,
  smtk::view::ConfigurationPtr viewConfig)
  : QWidget(p)
  , m_viewConfig(viewConfig)
{
  m_internals = new qtCollapsibleGroupWidgetInternals;
  m_internals->setupUi(this);
  QString style("QCheckBox::indicator { width:12px; height: 12px;} "
                "QCheckBox::indicator:unchecked {image: url(:/icons/attribute/caret-right.svg);} "
                "QCheckBox::indicator:checked {image: url(:/icons/attribute/caret-down.svg);} ");
  m_internals->VisibilityControl->setStyleSheet(style);
  connect(m_internals->VisibilityControl, &QCheckBox::toggled, [this](bool checked) {
    this->updateViewStateRecord(checked);
  });
}

qtCollapsibleGroupWidget::~qtCollapsibleGroupWidget()
{
  delete m_internals;
}

void qtCollapsibleGroupWidget::updateViewStateRecord(bool state)
{
  if (m_viewConfig)
  {
    std::string stateString = state ? "true" : "false";
    m_viewConfig->details().setAttribute("Open", stateString);
  }
}

QFrame* qtCollapsibleGroupWidget::contents() const
{
  return m_internals->BodyFrame;
}

QLayout* qtCollapsibleGroupWidget::contentsLayout() const
{
  return m_internals->BodyFrame->layout();
}

void qtCollapsibleGroupWidget::setContentsLayout(QLayout* newLayout)
{
  m_internals->BodyFrame->setLayout(newLayout);
}

void qtCollapsibleGroupWidget::setName(const QString& newName)
{
  return m_internals->VisibilityControl->setText(newName);
}

QString qtCollapsibleGroupWidget::name() const
{
  return m_internals->VisibilityControl->text();
}

void qtCollapsibleGroupWidget::open()
{
  return m_internals->VisibilityControl->setChecked(true);
}

void qtCollapsibleGroupWidget::collapse()
{
  return m_internals->VisibilityControl->setChecked(false);
}
