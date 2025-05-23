//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME XmlDocV6Parser.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_io_XmlDocV6Parser_h
#define __smtk_io_XmlDocV6Parser_h

#include "smtk/io/XmlDocV5Parser.h"

#include <functional>

namespace smtk
{
namespace io
{
class SMTKCORE_EXPORT XmlDocV6Parser : public XmlDocV5Parser
{
public:
  XmlDocV6Parser(smtk::attribute::ResourcePtr resource, smtk::io::Logger& logger);
  ~XmlDocV6Parser() override;

  static bool canParse(pugi::xml_node& node);
  static bool canParse(pugi::xml_document& doc);

  using XmlDocV5Parser::process;
  void process(
    pugi::xml_node& rootNode,
    std::map<std::string, std::map<std::string, smtk::io::TemplateInfo>>& globalTemplateMap)
    override;

protected:
  void processCategoryAtts(
    pugi::xml_node& node,
    common::Categories::Expression& catExp,
    common::Categories::CombinationMode& inheritanceMode) override;
  void processCategoryInfoNode(
    pugi::xml_node& node,
    common::Categories::Expression& catExp,
    common::Categories::CombinationMode& inheritanceMode) override;
};
} // namespace io
} // namespace smtk

#endif /* __smtk_io_XmlDocV6Parser_h */
