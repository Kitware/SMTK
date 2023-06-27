//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME XmlV7StringWriter.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_io_XmlV7StringWriter_h
#define __smtk_io_XmlV7StringWriter_h
#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/io/XmlV6StringWriter.h"

#include "smtk/attribute/Resource.h"

#include <functional>

namespace smtk
{
namespace io
{
class SMTKCORE_EXPORT XmlV7StringWriter : public XmlV6StringWriter
{
public:
  XmlV7StringWriter(smtk::attribute::ResourcePtr resource, smtk::io::Logger& logger);
  ~XmlV7StringWriter() override;

protected:
  // Override methods
  // Three virtual methods for writing contents
  std::string className() const override;
  unsigned int fileVersion() const override;

private:
};
} // namespace io
} // namespace smtk

#endif // __smtk_io_XmlV7StringWriter_h
