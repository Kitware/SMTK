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

#ifndef __smtk_io_XmlV3StringWriter_h
#define __smtk_io_XmlV3StringWriter_h
#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/io/XmlV2StringWriter.h"

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
class SMTKCORE_EXPORT XmlV3StringWriter : public XmlV2StringWriter
{
public:
  XmlV3StringWriter(const smtk::attribute::ResourcePtr resource, smtk::io::Logger& logger);
  ~XmlV3StringWriter() override;
  void generateXml() override;

protected:
  // Override methods
  // Three virtual methods for writing contents
  std::string className() const override;
  std::string rootNodeName() const override;
  unsigned int fileVersion() const override;

  void processDefinitionInternal(pugi::xml_node& definition, smtk::attribute::DefinitionPtr def)
    override;
  void processItemDefinitionType(pugi::xml_node& node, smtk::attribute::ItemDefinitionPtr idef)
    override;
  void processItemType(pugi::xml_node& node, smtk::attribute::ItemPtr item) override;

  // New methods
  void processDateTimeDef(pugi::xml_node& node, smtk::attribute::DateTimeItemDefinitionPtr idef);
  void processDateTimeItem(pugi::xml_node& node, smtk::attribute::DateTimeItemPtr item);

  void processReferenceDef(pugi::xml_node& node, smtk::attribute::ReferenceItemDefinitionPtr idef);
  void processReferenceItem(pugi::xml_node& node, smtk::attribute::ReferenceItemPtr item);

  void processResourceDef(pugi::xml_node& node, smtk::attribute::ResourceItemDefinitionPtr idef);
  void processResourceItem(pugi::xml_node& node, smtk::attribute::ResourceItemPtr item);

  void processComponentDef(pugi::xml_node& node, smtk::attribute::ComponentItemDefinitionPtr idef);
  void processComponentItem(pugi::xml_node& node, smtk::attribute::ComponentItemPtr item);

  void processReferenceDefCommon(
    pugi::xml_node& node,
    smtk::attribute::ReferenceItemDefinitionPtr idef,
    const std::string& labelName);

private:
};
} // namespace io
} // namespace smtk

#endif // __smtk_io_XmlV3StringWriter_h
