//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME XmlV2StringWriter.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_io_XmlV2StringWriter_h
#define __smtk_io_XmlV2StringWriter_h
#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/io/XmlStringWriter.h" // base

#include "smtk/io/Logger.h"

#include "smtk/attribute/Resource.h"
#include "smtk/model/EntityTypeBits.h"
#include "smtk/view/Configuration.h"

#include <sstream>
#include <string>
#include <vector>

namespace pugi
{
class xml_document;
class xml_node;
}

namespace smtk
{
namespace io
{
class SMTKCORE_EXPORT XmlV2StringWriter : public XmlStringWriter
{
public:
  XmlV2StringWriter(const smtk::attribute::ResourcePtr resource, smtk::io::Logger& logger);
  virtual ~XmlV2StringWriter();
  std::string convertToString(bool no_declaration = false) override;
  virtual std::string getString(std::size_t ith, bool no_declaration = false) override;

  void generateXml() override;

  template <typename Container>
  static std::string concatenate(
    const Container& v, std::string& sep, smtk::io::Logger* logger = nullptr);

protected:
  // Three virtual methods for writing contents
  std::string className() const override;
  std::string rootNodeName() const override;
  unsigned int fileVersion() const override;

  void processAttributeInformation();
  void processViews();
  void processStyles();
  void processModelInfo();

  void processDefinition(smtk::attribute::DefinitionPtr def);
  virtual void processDefinitionInternal(
    pugi::xml_node& definition, smtk::attribute::DefinitionPtr def);
  void processAttribute(pugi::xml_node& attributes, smtk::attribute::AttributePtr att);

  void processItem(pugi::xml_node& node, smtk::attribute::ItemPtr item);
  virtual void processItemAttributes(pugi::xml_node& node, smtk::attribute::ItemPtr item);
  virtual void processItemType(pugi::xml_node& node, smtk::attribute::ItemPtr item);

  void processItemDefinition(pugi::xml_node& node, smtk::attribute::ItemDefinitionPtr idef);
  virtual void processItemDefinitionAttributes(
    pugi::xml_node& node, smtk::attribute::ItemDefinitionPtr idef);
  virtual void processItemDefinitionType(
    pugi::xml_node& node, smtk::attribute::ItemDefinitionPtr idef);

  void processDoubleItem(pugi::xml_node& node, smtk::attribute::DoubleItemPtr item);
  void processDoubleDef(pugi::xml_node& node, smtk::attribute::DoubleItemDefinitionPtr idef);
  void processDirectoryItem(pugi::xml_node& node, smtk::attribute::DirectoryItemPtr item);
  void processDirectoryDef(pugi::xml_node& node, smtk::attribute::DirectoryItemDefinitionPtr idef);
  void processFileItem(pugi::xml_node& node, smtk::attribute::FileItemPtr item);
  void processFileDef(pugi::xml_node& node, smtk::attribute::FileItemDefinitionPtr idef);
  void processFileSystemItem(pugi::xml_node& node, smtk::attribute::FileSystemItemPtr item);
  void processFileSystemDef(
    pugi::xml_node& node, smtk::attribute::FileSystemItemDefinitionPtr idef);
  void processGroupItem(pugi::xml_node& node, smtk::attribute::GroupItemPtr item);
  void processGroupDef(pugi::xml_node& node, smtk::attribute::GroupItemDefinitionPtr idef);
  void processIntItem(pugi::xml_node& node, smtk::attribute::IntItemPtr item);
  void processIntDef(pugi::xml_node& node, smtk::attribute::IntItemDefinitionPtr idef);
  void processStringItem(pugi::xml_node& node, smtk::attribute::StringItemPtr item);
  void processStringDef(pugi::xml_node& node, smtk::attribute::StringItemDefinitionPtr idef);
  void processModelEntityItem(pugi::xml_node& node, smtk::attribute::ModelEntityItemPtr item);
  void processModelEntityDef(
    pugi::xml_node& node, smtk::attribute::ModelEntityItemDefinitionPtr idef);
  void processValueItem(pugi::xml_node& node, smtk::attribute::ValueItemPtr item);
  void processDateTimeDef(pugi::xml_node& node, smtk::attribute::DateTimeItemDefinitionPtr idef);
  void processDateTimeItem(pugi::xml_node& node, smtk::attribute::DateTimeItemPtr item);
  void processValueDef(pugi::xml_node& node, smtk::attribute::ValueItemDefinitionPtr idef);

  virtual void processView(smtk::view::ConfigurationPtr view);
  virtual void processViewComponent(
    const smtk::view::Configuration::Component& comp, pugi::xml_node& node);
  static std::string encodeModelEntityMask(smtk::model::BitFlags m);
  static std::string encodeColor(const double* color);

  pugi::xml_node& topDefinitionsNode() const;
  pugi::xml_node& topRootNode() const;
  pugi::xml_node& topAttributesNode() const;
  pugi::xml_node& topViewsNode() const;

  // Keep pugi headers out of public headers:
  struct Internals;
  Internals* m_internals;

private:
};

template <typename Container>
std::string XmlV2StringWriter::concatenate(
  const Container& container, std::string& sep, smtk::io::Logger* logger)
{
  std::ostringstream token;
  if (container.empty())
    return token.str();

  typename Container::const_iterator it;
  std::set<char> notSep;
  if (sep.empty())
  {
    // Find all the characters that cannot serve as a separator
    // Note that we need to do this even when v.size() == 1
    // because some locales might use our preferred separator
    // in a non-separator-role (e.g., "3.14" is written "3,14"
    // in some locales), so we must ensure that the single value
    // in v does not contain the default separator.
    for (it = container.begin(); it != container.end(); ++it)
    {
      token.precision(17);
      token << *it;
      std::string::const_iterator sit;
      std::string entry = token.str();
      for (sit = entry.begin(); sit != entry.end(); ++sit)
        notSep.insert(*sit);
      token.str(std::string());
      token.clear();
    }
    // Try some preferred separators in order of preference.
    static const char preferredSeps[] = ",;|:#@!.-=_`?+/\\";
    int preferredSepsLen = sizeof(preferredSeps) / sizeof(preferredSeps[0]);
    char finalSep = '\0';
    for (int i = 0; i < preferredSepsLen; ++i)
      if (notSep.find(preferredSeps[i]) == notSep.end())
      {
        finalSep = preferredSeps[i];
        break;
      }
    // OK, desperately try any character at all.
    if (!finalSep)
      for (int i = 1; i < 255; ++i)
        if (notSep.find(preferredSeps[i]) == notSep.end())
        {
          finalSep = preferredSeps[i];
          break;
        }
    if (!finalSep)
    {
      if (logger != nullptr)
      {
        smtkErrorMacro((*logger),
          "Tokens use every single possible character; no separator found. Using comma.");
      }
      finalSep = ',';
    }
    // Return whatever separator we came up with to the caller:
    sep = finalSep;
  }
  // Now accumulate values into an output string.
  token.clear();
  token.precision(17);
  it = container.begin();
  token << *it;
  ++it;
  for (; it != container.end(); ++it)
  {
    token.precision(17);
    token << sep << *it;
  }

  return token.str();
}
}
}

#endif // __smtk_io_XmlV2StringWriter_h
