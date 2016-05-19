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
#include <QCheckBox>
#include <QFrame>
#include "ui_qtCollapsibleGroupWidgetInternals.h"
using namespace smtk::extension;

//----------------------------------------------------------------------------
class smtk::extension::qtCollapsibleGroupWidgetInternals : public Ui::qtCollapsibleGroupWidgetInternals
{
public:
  qtCollapsibleGroupWidgetInternals() {}
};

//----------------------------------------------------------------------------
qtCollapsibleGroupWidget::qtCollapsibleGroupWidget(QWidget* p): QWidget(p)
{
  this->m_internals = new qtCollapsibleGroupWidgetInternals;
  this->m_internals->setupUi(this);
}

//----------------------------------------------------------------------------
qtCollapsibleGroupWidget::~qtCollapsibleGroupWidget()
{
  delete this->m_internals;
}
//----------------------------------------------------------------------------
QFrame *qtCollapsibleGroupWidget::contents() const
{
  return this->m_internals->BodyFrame;
}
//----------------------------------------------------------------------------
QLayout *qtCollapsibleGroupWidget::contentsLayout() const
{
  return this->m_internals->BodyFrame->layout();
}
//----------------------------------------------------------------------------
void qtCollapsibleGroupWidget::setContentsLayout(QLayout *newLayout)
{
  this->m_internals->BodyFrame->setLayout(newLayout);
}
//----------------------------------------------------------------------------
void qtCollapsibleGroupWidget::setName(const QString &newName)
{
  return this->m_internals->VisibilityControl->setText(newName);
}
//----------------------------------------------------------------------------
QString qtCollapsibleGroupWidget::name() const
{
  return this->m_internals->VisibilityControl->text();
}
//----------------------------------------------------------------------------
void qtCollapsibleGroupWidget::open()
{
  return this->m_internals->VisibilityControl->setChecked(true);
}
void qtCollapsibleGroupWidget::collapse()
{
  return this->m_internals->VisibilityControl->setChecked(false);
}
