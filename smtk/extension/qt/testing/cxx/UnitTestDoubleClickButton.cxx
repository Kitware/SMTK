//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/testing/cxx/UnitTestDoubleClickButton.h"
#include "smtk/common/Environment.h"
#include "smtk/io/Logger.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <QApplication>
#include <QRect>
#include <QVBoxLayout>
#include <QWidget>

#include <QtTest/QtTest>

#include <cassert>
#include <string>

namespace
{
bool interactive = false;
}

DoubleClickButtonTester::DoubleClickButtonTester()
  : m_window(new QMainWindow)
  , m_widget(new QWidget(m_window))
  , m_layout(new QVBoxLayout)
  , m_button(new qtDoubleClickButton("Test"))
{
  m_window->setCentralWidget(m_widget);
  QObject::connect(
    m_button, &qtDoubleClickButton::singleClicked, this, &DoubleClickButtonTester::singleClicked);
  QObject::connect(
    m_button, &qtDoubleClickButton::doubleClicked, this, &DoubleClickButtonTester::doubleClicked);
  m_widget->setLayout(m_layout);
  m_layout->addWidget(m_button);
  m_window->show();
}

void DoubleClickButtonTester::testSingleAndDoubleClicks()
{
  if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive))
  {
    QSKIP("Platform does not support window activation");
  }
  else
  {
    QVERIFY(QTest::qWaitForWindowActive(m_window));
  }

  QTest::mouseClick(m_button, Qt::LeftButton);
  QTest::qWait(qtDoubleClickButton::timeout);
  // Poll for a short time after the expected timeout to catch the click event.
  for (int ii = 0; ii < 10; ++ii)
  {
    if (m_counts[0] > 0)
    {
      break;
    }
    QTest::qWait(50);
  }
  QCOMPARE(m_counts[0], 1);

  // Calling QTest::mouseDClick doesn't work... just call mouseClick twice:
  QTest::mouseClick(m_button, Qt::LeftButton);
  QTest::qWait(qtDoubleClickButton::timeout / 2);
  QTest::mouseClick(m_button, Qt::LeftButton);
  for (int ii = 0; ii < 10; ++ii)
  {
    if (m_counts[1] > 0)
    {
      break;
    }
    QTest::qWait(50);
  }
  QCOMPARE(m_counts[1], 1);
}

void DoubleClickButtonTester::singleClicked()
{
  ++m_counts[0];
  std::cout << "    Clicked\n";
}

void DoubleClickButtonTester::doubleClicked()
{
  ++m_counts[1];
  std::cout << "    Double-clicked\n";
}

int UnitTestDoubleClickButton(int argc, char** const argv)
{
  QApplication app(argc, argv);
  // Passing "-v1" (or any other valid argument) will run the event loop
  // and allow interaction.
  interactive = argc > 1;
  DoubleClickButtonTester tester;

  // Run the event loop after the tester is constructed but before tests
  // so that failing tests do not cause the program to exit before interaction
  // is possible.
  if (!smtk::common::Environment::hasVariable("DASHBOARD_TEST_FROM_CTEST") && interactive)
  {
    QApplication::exec();
    return 1;
  }

  return QTest::qExec(&tester, argc, argv);
}
