//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/extension/qt/qtLogView.h"

#include <QApplication>
#include <QElapsedTimer>
#include <QPlainTextEdit>
#include <QTemporaryDir>
#include <QtTest/QtTest>

#include <fstream>

namespace
{
class LogView : public smtk::qt::qtLogView
{
public:
  using smtk::qt::qtLogView::qtLogView;

  void reachEndOfFile()
  {
    // Simulate a short read, e.g. when a writer truncates the file during a read.
    m_file.clear();
    m_file.seekg(0, std::ios::end);
    m_file.get();
    test(m_file.eof(), "Expected EOF on the log stream.");
  }
};

bool waitForText(LogView& view, const QString& text)
{
  auto* contents = view.findChild<QPlainTextEdit*>();
  QElapsedTimer elapsed;
  elapsed.start();
  while (elapsed.elapsed() < 2000)
  {
    if (contents->toPlainText().contains(text))
    {
      return true;
    }
    QTest::qWait(20);
  }
  return false;
}
} // namespace

int UnitTestLogView(int argc, char** const argv)
{
  QApplication app(argc, argv);
  QTemporaryDir directory;
  test(directory.isValid(), "Could not create temporary log directory.");
  const std::filesystem::path path(directory.filePath("job.log").toStdString());
  LogView view(path);
  view.readMore(); // The job has not created its log yet.

  // Keep the writer open while the timer reads, as it would be for a running job.
  std::ofstream writer(path, std::ios::binary);
  test(writer.is_open(), "Could not open log for writing.");
  writer << "first line\r\n" << std::flush;
  test(waitForText(view, "first line"), "Initial log contents were not displayed.");
  writer << "second line\r\n" << std::flush;
  test(waitForText(view, "second line"), "The timer did not display appended CRLF text.");
  auto* contents = view.findChild<QPlainTextEdit*>();
  test(!contents->toPlainText().contains(QChar(0)), "A short read inserted NUL characters.");

  view.reachEndOfFile();
  writer << "after EOF\r\n" << std::flush;
  test(waitForText(view, "after EOF"), "The log did not resume updating after EOF.");
  const QString before = contents->toPlainText();
  view.readMore();
  test(contents->toPlainText() == before, "An unchanged log duplicated text.");
  writer.close();

  writer.open(path, std::ios::binary | std::ios::trunc);
  writer << "reset\r\n" << std::flush;
  test(waitForText(view, "reset"), "A truncated log was not reloaded.");
  test(!contents->toPlainText().contains("first line"), "Truncation retained old text.");
  writer << "more\r\n" << std::flush;
  test(waitForText(view, "more"), "The log did not keep updating after truncation.");
  view.done();
  return 0;
}
