//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_extension_RedirectOutput_h
#define __smtk_extension_RedirectOutput_h

#include "smtk/PublicPointerDefs.h"

#include "smtk/extension/qt/Exports.h"

namespace smtk
{
namespace io
{
class Logger;
}
}

namespace smtk
{
namespace extension
{
namespace qt
{

//Redirect the output from smtk::io::Logger to Qt's messaging stream.
SMTKQTEXT_EXPORT void RedirectOutputToQt(smtk::io::Logger& log);
}
}
}

#endif
