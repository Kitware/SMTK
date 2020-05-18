//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME XmlV3StringWriter.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_io_XmlV4StringWriter_h
#define __smtk_io_XmlV4StringWriter_h
#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/io/XmlV3StringWriter.h"

#include "smtk/attribute/Resource.h"

#include <functional>

namespace pugi
{
class xml_node;
}

namespace smtk
{
namespace io
{
class SMTKCORE_EXPORT XmlV4StringWriter : public XmlV3StringWriter
{
public:
  XmlV4StringWriter(const smtk::attribute::ResourcePtr resource, smtk::io::Logger& logger);
  virtual ~XmlV4StringWriter();

protected:
  // Override methods
  // Three virtual methods for writing contents
  std::string className() const override;
  unsigned int fileVersion() const override;
  void processDefinitionInternal(pugi::xml_node& definition, smtk::attribute::DefinitionPtr def)
    override;
  void processItemDefinitionAttributes(
    pugi::xml_node& node,
    smtk::attribute::ItemDefinitionPtr idef) override;

private:
};
} // namespace io
} // namespace smtk

#endif // __smtk_io_XmlV3StringWriter_h
