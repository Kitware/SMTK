//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtEmittingLogger - a std::stream that emits its output upon flush
// .SECTION Description
// .SECTION See Also
// qtItem

#ifndef smtk_extension_qtEmittingStringBuffer_h
#define smtk_extension_qtEmittingStringBuffer_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/io/Logger.h"

#include <QObject>

#include <sstream>

namespace smtk
{
namespace extension
{

/**\brief An std::stringbuf that emits when it is updated.
  *
  * smtk::io::Logger does not have any access points for derived classes to
  * inject actions upon the receipt of a log. So, we set our logger to use a
  * std::ostream with a custom std::stringbuf that emits during its sync() call.
  *
  * NOTE: This is not a very performant way to pass log messages, and could
  * become a performance bottleneck.
  */
class SMTKQTEXT_EXPORT qtEmittingStringBuffer
  : public QObject
  , public std::stringbuf
{
  Q_OBJECT

public:
  qtEmittingStringBuffer() = default;

  ~qtEmittingStringBuffer() override { sync(); }

protected:
  int sync() override
  {
    Q_EMIT flush();
    str("");
    return 0;
  }

Q_SIGNALS:
  void flush();
};
} // namespace extension
} // namespace smtk

#endif
