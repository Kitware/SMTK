//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "InputDialog.h"
#include "ui_InputDialog.h"

// ------------------------------------------------------------------------
InputDialog::InputDialog(QWidget* parent)
  : QDialog(parent)
  , Ui(new Ui::InputDialog)
{
  this->Ui->setupUi(this);

  connect(this->Ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(this->Ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

// ------------------------------------------------------------------------
InputDialog::~InputDialog() = default;

// ------------------------------------------------------------------------
QWidget* InputDialog::centralWidget()
{
  return Ui->centralWidget;
}
