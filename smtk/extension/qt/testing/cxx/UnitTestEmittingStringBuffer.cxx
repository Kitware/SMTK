//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME UnitTestEmittingStringBuffer.h -
// .SECTION Description
// .SECTION See Also

#include "smtk/extension/qt/testing/cxx/UnitTestEmittingStringBuffer.h"
#include "smtk/extension/qt/qtEmittingStringBuffer.h"

#include "smtk/extension/qt/testing/cxx/qtPrintLog.h"

#include <QCoreApplication>
#include <QTime>
#include <QTimer>

namespace
{
void flushEventQueue()
{
  QTime dieTime = QTime::currentTime().addMSecs(1);
  while (QTime::currentTime() < dieTime)
  {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 1);
  }
}
} // namespace

void TestEmittingStringBuffer::run()
{
  // Create an instance of Logger
  smtk::io::Logger logger;

  // Create an instance of our emitting string buffer
  smtk::extension::qtEmittingStringBuffer stringbuf;

  // Create a new std::ostream that uses our string buffer
  std::ostream* ostr = new std::ostream(&stringbuf);

  // Pass the ostream to the logger, and set it to be owned by the logger
  logger.setFlushToStream(ostr, true, false);

  // Create a PrintLogInstance to connect with our EmittingStringBuffer
  qtPrintLog printLog(logger);
  QObject::connect(&stringbuf, SIGNAL(flush()), &printLog, SLOT(print()), Qt::QueuedConnection);

  // Test the logger.
  smtkErrorMacro(logger, "this is an error no = " << 45 << " ERROR!");
  flushEventQueue();
  smtkWarningMacro(logger, "this is a warning no = " << 10.1234 << " WARNING!");
  flushEventQueue();
  smtkDebugMacro(logger, "this is a Debug no = " << 1 << " DEBUG!");
  flushEventQueue();
  logger.addRecord(smtk::io::Logger::Info, "Sample Info String\n");
  flushEventQueue();
  Q_EMIT finished();
}

int UnitTestEmittingStringBuffer(int argc, char** const argv)
{
  QCoreApplication application(argc, argv);

  // Create an instance of our test harness
  TestEmittingStringBuffer* testEmittingStringBuffer = new TestEmittingStringBuffer(&application);

  // Exit when the task signals finished
  QObject::connect(testEmittingStringBuffer, &TestEmittingStringBuffer::finished, [&application]() {
    QTimer::singleShot(0, &application, &QCoreApplication::quit);
  });

  // Run the task from the application event loop
  QTimer::singleShot(0, testEmittingStringBuffer, SLOT(run()));

  return QCoreApplication::exec();

  return 0;
}
