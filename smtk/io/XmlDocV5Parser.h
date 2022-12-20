//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME XmlDocV5Parser.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_io_XmlDocV5Parser_h
#define __smtk_io_XmlDocV5Parser_h

#include "smtk/io/XmlDocV4Parser.h"

#include <functional>

namespace smtk
{
namespace io
{
class SMTKCORE_EXPORT XmlDocV5Parser : public XmlDocV4Parser
{
public:
  XmlDocV5Parser(smtk::attribute::ResourcePtr resource, smtk::io::Logger& logger);
  ~XmlDocV5Parser() override;

  using XmlDocV4Parser::process;
  void process(
    pugi::xml_node& rootNode,
    std::map<std::string, std::map<std::string, std::string>>& globalItemBlocks) override;
  void processAttribute(pugi::xml_node& attNode) override;
  static bool canParse(pugi::xml_node& node);
  static bool canParse(pugi::xml_document& doc);

protected:
  void processHints(pugi::xml_node& root) override;
};
} // namespace io
} // namespace smtk

#endif /* __smtk_io_XmlDocV5Parser_h */
