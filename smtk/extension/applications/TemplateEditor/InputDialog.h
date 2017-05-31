//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME
// .SECTION
//

#ifndef __InputDialog_h
#define __InputDialog_h

#include <memory>

#include <QDialog>

class QDialogButtonBox;

namespace Ui
{
class InputDialog;
}

class InputDialog : public QDialog
{
  Q_OBJECT

public:
  InputDialog(QWidget* parent = nullptr);
  ~InputDialog();

  QWidget* centralWidget();

protected:
  virtual bool validate_impl() = 0;

  QDialogButtonBox* buttonBox();

protected slots:
  void validate();

private:
  InputDialog(const InputDialog&) = delete;
  void operator=(const InputDialog&) = delete;

  std::unique_ptr<Ui::InputDialog> Ui;
};
#endif //__InputDialog_h
