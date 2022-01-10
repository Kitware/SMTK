//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtDoubleClickButton.h"

#include <QLayout>
#include <QMainWindow>

#include <array>

class DoubleClickButtonTester : public QObject
{
  Q_OBJECT;

public:
  DoubleClickButtonTester();

public Q_SLOTS:
  void singleClicked();
  void doubleClicked();

private Q_SLOTS:
  void testSingleAndDoubleClicks();

protected:
  std::array<int, 2> m_counts{ 0, 0 };
  QMainWindow* m_window;
  QWidget* m_widget;
  QLayout* m_layout;
  qtDoubleClickButton* m_button;
};
