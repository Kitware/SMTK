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

#include "smtk/extension/qt/qtEmittingStringBuffer.h"

#include "smtk/extension/qt/testing/cxx/qtPrintLog.h"

int UnitTestEmittingStringBuffer(int, char** const)
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
  QObject::connect(&stringbuf, SIGNAL(flush()), &printLog, SLOT(print()));

  // Test the logger.
  smtkErrorMacro(logger, "this is an error no = " << 45 << " ERROR!");
  smtkWarningMacro(logger, "this is a warning no = " << 10.1234 << " WARNING!");
  smtkDebugMacro(logger, "this is a Debug no = " << 1 << " DEBUG!");
  logger.addRecord(smtk::io::Logger::INFO, "Sample Info String\n");

  return 0;
}
