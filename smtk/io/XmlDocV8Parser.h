//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME XmlDocV8Parser.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_io_XmlDocV8Parser_h
#define __smtk_io_XmlDocV8Parser_h

#include "smtk/io/XmlDocV7Parser.h"

#include <functional>

namespace smtk
{
namespace io
{
class SMTKCORE_EXPORT XmlDocV8Parser : public XmlDocV7Parser
{
public:
  XmlDocV8Parser(smtk::attribute::ResourcePtr resource, smtk::io::Logger& logger);
  ~XmlDocV8Parser() override;

  static bool canParse(pugi::xml_node& node);
  static bool canParse(pugi::xml_document& doc);

protected:
  smtk::attribute::AttributePtr processAttribute(pugi::xml_node& attNode) override;
  void processCategoryExpressionNode(
    pugi::xml_node& node,
    common::Categories::Expression& catExp,
    common::Categories::CombinationMode& inheritanceMode);
  void processDefinitionAtts(pugi::xml_node& node, smtk::attribute::DefinitionPtr& def) override;
  void processDefinitionChildNode(pugi::xml_node& defNode, smtk::attribute::DefinitionPtr& def)
    override;
  void processItemDefChildNode(pugi::xml_node& node, const smtk::attribute::ItemDefinitionPtr& idef)
    override;
  void processItemDefCategoryExpressionNode(
    pugi::xml_node& node,
    smtk::attribute::ItemDefinitionPtr idef);
  void processDefCategoryExpressionNode(pugi::xml_node& node, smtk::attribute::DefinitionPtr& def);
  smtk::common::UUID getDefinitionID(pugi::xml_node& defNode) override;
};
} // namespace io
} // namespace smtk

#endif /* __smtk_io_XmlDocV8Parser_h */
