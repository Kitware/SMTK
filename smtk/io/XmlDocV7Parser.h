//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME XmlDocV7Parser.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_io_XmlDocV7Parser_h
#define __smtk_io_XmlDocV7Parser_h

#include "smtk/io/XmlDocV6Parser.h"

#include <functional>

namespace smtk
{
namespace io
{
class SMTKCORE_EXPORT XmlDocV7Parser : public XmlDocV6Parser
{
public:
  XmlDocV7Parser(smtk::attribute::ResourcePtr resource, smtk::io::Logger& logger);
  ~XmlDocV7Parser() override;

  static bool canParse(pugi::xml_node& node);
  static bool canParse(pugi::xml_document& doc);

protected:
};
} // namespace io
} // namespace smtk

#endif /* __smtk_io_XmlDocV7Parser_h */
