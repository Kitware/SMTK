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
  qtCollapsibleGroupWidgetInternals() {}
};

qtCollapsibleGroupWidget::qtCollapsibleGroupWidget(QWidget* p)
  : QWidget(p)
{
  m_internals = new qtCollapsibleGroupWidgetInternals;
  m_internals->setupUi(this);
}

qtCollapsibleGroupWidget::~qtCollapsibleGroupWidget()
{
  delete m_internals;
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
