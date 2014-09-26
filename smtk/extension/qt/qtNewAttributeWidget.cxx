//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "qtNewAttributeWidget.h"

#include <QComboBox>
#include <QLabel>
#include <QLayout>
#include <QPointer>

using namespace smtk::attribute;

#include "ui_qtNewAttributeWidget.h"
namespace Ui { class qtNewAttributeWidget; }

class qtNewAttributeWidget::PIMPL : public Ui::qtNewAttributeWidget
{
public:
  PIMPL(QWidget* parentW)
  {
  this->BaseWidget = parentW ?
    qobject_cast<QComboBox*>(parentW) : NULL;;
  }
  ~PIMPL() {}
  QPointer<QComboBox> BaseWidget;
};

qtNewAttributeWidget::qtNewAttributeWidget(QWidget* parentW):
  Superclass(parentW->parentWidget(), Qt::Dialog|Qt::FramelessWindowHint)
{
  this->Private = new qtNewAttributeWidget::PIMPL(parentW);
  this->Private->setupUi(this);
  // this->setAttribute(Qt::WA_DeleteOnClose, true);
  this->setFocusPolicy(Qt::StrongFocus);
}

// -------------------------------------------------------------------------
qtNewAttributeWidget::~qtNewAttributeWidget()
{
  delete this->Private;
}

// -------------------------------------------------------------------------
void qtNewAttributeWidget::setBaseWidget(QWidget* widget)
{
  this->Private->BaseWidget = widget ?
    qobject_cast<QComboBox*>(widget) : NULL;;
}

// -------------------------------------------------------------------------
int qtNewAttributeWidget::showWidget(const QString& name,
                                     const QList<QString>& attTypes)
{
  if(!this->Private->BaseWidget)
    {
    return QDialog::Rejected;
    }
  this->Private->comboBoxType->clear();
  this->Private->comboBoxType->addItems(attTypes);
  this->Private->comboBoxType->setCurrentIndex(0);
  this->Private->lineEditName->setText(name);
  QPoint mappedPoint = this->Private->BaseWidget->parentWidget()->childrenRect().topLeft();
  mappedPoint.setX(0);
  mappedPoint = this->Private->BaseWidget->mapToGlobal(mappedPoint);
  mappedPoint = this->mapFromGlobal(mappedPoint);
  this->setGeometry( mappedPoint.x(), mappedPoint.y()-2*this->height(),
                     this->Private->BaseWidget->width(), this->height());
  this->setModal(true);
  return this->exec();
}

// -------------------------------------------------------------------------
QString qtNewAttributeWidget::attributeName() const
{
  return this->Private->lineEditName->text();
}
// -------------------------------------------------------------------------
QString qtNewAttributeWidget::attributeType() const
{
  return this->Private->comboBoxType->currentText();
}
