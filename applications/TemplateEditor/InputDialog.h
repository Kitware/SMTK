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

class QAbstractButton;
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
  ~InputDialog() override;

  QWidget* centralWidget();

  InputDialog(const InputDialog&) = delete;
  InputDialog& operator=(const InputDialog&) = delete;

protected Q_SLOTS:
  void validate();

protected:
  virtual bool validate_impl() = 0;

  QDialogButtonBox* buttonBox();

private Q_SLOTS:
  /**
 * The 'Apply' button is used by default to save changes (and exit the
 * dialog). This slot is used to accomplish that, since this button has
 * an 'ApplyRole' which does not trigger accept automatically.
 */
  void acceptOnApply(QAbstractButton* button);

private:
  std::unique_ptr<Ui::InputDialog> Ui;
};
#endif //__InputDialog_h
