//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qt_qtDoubleClickButton_h
#define smtk_extension_qt_qtDoubleClickButton_h

#include "smtk/extension/qt/Exports.h"

#include <QPushButton>
#include <QTimer>

#include <string>

/**\brief A subclass of QPushButton that emits single and double clicks.
  *
  * Either a single- or double-click will be emitted, but not both for
  * the same underlying user input.
  */
class SMTKQTEXT_EXPORT qtDoubleClickButton : public QPushButton
{
  Q_OBJECT
public:
  using Superclass = QPushButton;

  qtDoubleClickButton(const QIcon& icon, const QString& text, QWidget* parent = nullptr);
  qtDoubleClickButton(const QString& text, QWidget* parent = nullptr);
  qtDoubleClickButton(QWidget* parent = nullptr);
  ~qtDoubleClickButton() override;

  static constexpr int timeout = 200; // milliseconds

Q_SIGNALS:
  void singleClicked();
  void doubleClicked();

protected Q_SLOTS:
  void startTimer();

protected:
  /// Called by all constructors to configure timer and button connections.
  void configure();

  QTimer m_timer;
};

#endif // smtk_extension_qt_qtDoubleClickButton_h
