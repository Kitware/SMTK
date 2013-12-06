/*=========================================================================

Copyright (c) 1998-2013 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/
// .NAME Logger.h -
// .SECTION Description
// .SECTION See Also

#include "smtk/util/Logger.h"
#include <iostream>

int main()
{
  smtk::util::Logger logger;
  smtkErrorMacro(logger, "this is an error no = " << 45 << " ERROR!");
  smtkWarningMacro(logger, "this is a warning no = " << 10.1234 << " WARNING!");
  smtkDebugMacro(logger, "this is a Debug no = " << 1 << " DEBUG!");
  logger.addRecord(smtk::util::Logger::INFO, "Sample Info String\n");
  std::size_t i, n = logger.numberOfRecords();
  if (n != 4)
    {
    std::cerr << "Wrong number of records!  Got " << n << " Should be 4!\n";
    return -1;
    }

  smtk::util::Logger::Record r;
  for (i = 0; i < n; i++)
    {
    r = logger.record(i);
    std::cerr << " Record " << i << ": \n\tSeverity = "
              << smtk::util::Logger::severityAsString(r.severity)
              << "\n\tMessage = " << r.message
              << "\tFile = " << r.fileName << "\n\tLine = "
              << r.lineNumber << std::endl;
    }
  return 0;
}
