//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include <QPushButton>

#include "InputDialog.h"
#include "ui_InputDialog.h"

// ------------------------------------------------------------------------
InputDialog::InputDialog(QWidget* parent)
  : QDialog(parent)
  , Ui(new Ui::InputDialog)
{
  this->Ui->setupUi(this);
  this->Ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);

  connect(this->Ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(this->Ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this,
    SLOT(acceptOnApply(QAbstractButton*)));
}

// ------------------------------------------------------------------------
InputDialog::~InputDialog() = default;

// ------------------------------------------------------------------------
QWidget* InputDialog::centralWidget()
{
  return Ui->centralWidget;
}

// ------------------------------------------------------------------------
bool InputDialog::validate_impl()
{
  return true;
}

// ------------------------------------------------------------------------
void InputDialog::validate()
{
  const bool isValid = this->validate_impl();
  this->Ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(isValid);
}

// ------------------------------------------------------------------------
QDialogButtonBox* InputDialog::buttonBox()
{
  return this->Ui->buttonBox;
}

// ------------------------------------------------------------------------
void InputDialog::acceptOnApply(QAbstractButton* button)
{
  QDialogButtonBox::ButtonRole role = this->Ui->buttonBox->buttonRole(button);
  if (role == QDialogButtonBox::ApplyRole)
  {
    this->accept();
  }
}
