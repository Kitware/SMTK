//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME XmlV8StringWriter.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_io_XmlV8StringWriter_h
#define __smtk_io_XmlV8StringWriter_h
#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/io/XmlV7StringWriter.h"

#include "smtk/attribute/Resource.h"

#include <functional>

namespace smtk
{
namespace io
{
class SMTKCORE_EXPORT XmlV8StringWriter : public XmlV7StringWriter
{
public:
  XmlV8StringWriter(smtk::attribute::ResourcePtr resource, smtk::io::Logger& logger);
  ~XmlV8StringWriter() override;

protected:
  // Override methods
  // Three virtual methods for writing contents
  std::string className() const override;
  unsigned int fileVersion() const override;
  void processDefinitionInternal(pugi::xml_node& definition, smtk::attribute::DefinitionPtr def)
    override;

private:
};
} // namespace io
} // namespace smtk

#endif // __smtk_io_XmlV8StringWriter_h
