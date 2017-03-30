//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME Logger.h -
// .SECTION Description
// .SECTION See Also

#include "smtk/io/Logger.h"
#include <iostream>

int main()
{
  smtk::io::Logger logger;
  smtkErrorMacro(logger, "this is an error no = " << 45 << " ERROR!");
  smtkWarningMacro(logger, "this is a warning no = " << 10.1234 << " WARNING!");
  smtkDebugMacro(logger, "this is a Debug no = " << 1 << " DEBUG!");
  logger.addRecord(smtk::io::Logger::INFO, "Sample Info String\n");
  std::size_t i, n = logger.numberOfRecords();
  if (n != 4)
  {
    std::cerr << "Wrong number of records!  Got " << n << " Should be 4!\n";
    return -1;
  }

  smtk::io::Logger::Record r;
  for (i = 0; i < n; i++)
  {
    r = logger.record(static_cast<int>(i));
    std::cerr << " Record " << i
              << ": \n\tSeverity = " << smtk::io::Logger::severityAsString(r.severity)
              << "\n\tMessage = " << r.message << "\tFile = " << r.fileName
              << "\n\tLine = " << r.lineNumber << std::endl;
  }
  return 0;
}
