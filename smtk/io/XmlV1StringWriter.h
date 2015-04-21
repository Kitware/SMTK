//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#ifndef __smtk_io_XmlV1StringWriter_h
#define __smtk_io_XmlV1StringWriter_h
#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/io/Logger.h"

#include "smtk/model/EntityTypeBits.h" // for BitFlags

#include "smtk/attribute/Manager.h"

#include <string>
#include <sstream>
#include <vector>

namespace pugi {
class xml_document;
class xml_node;
}

namespace smtk
{
  namespace io
  {

/**\brief Write legacy XML files.
  *
  */
class SMTKCORE_EXPORT XmlV1StringWriter
{
public:
  XmlV1StringWriter(const smtk::attribute::Manager &manager);
  virtual ~XmlV1StringWriter();
  std::string convertToString(smtk::io::Logger &logger,
                              bool no_declaration = false);
  void generateXml(pugi::xml_node& parent_node,
                   smtk::io::Logger &logger,
                   bool createRoot = true);
  const smtk::io::Logger &messageLog() const
  {return this->m_logger;}

  //Control which sections of the attribute manager should be writtern out
  // By Default all sections are processed.  These are advance options!!
  // If val is false then defintions will not be saved
  void includeDefinitions(bool val)
  {this->m_includeDefinitions = val;}

  // If val is false then instances will not be saved
  void includeInstances(bool val)
  {this->m_includeInstances = val;}

  // If val is false then model information will not be saved
  void includeModelInformation(bool val)
  {this->m_includeModelInformation = val;}

  // If val is false then views will not be saved
  void includeViews(bool val)
  {this->m_includeViews = val;}

protected:
  void processAttributeInformation();
  void processViews();
  void processModelInfo();

  void processDefinition(pugi::xml_node &definitions,
                         pugi::xml_node &attributes,
                         smtk::attribute::DefinitionPtr def);
  void processAttribute(pugi::xml_node &attributes,
                        smtk::attribute::AttributePtr att);
  void processItem(pugi::xml_node &node,
                   smtk::attribute::ItemPtr item);
  void processItemDefinition(pugi::xml_node &node,
                             smtk::attribute::ItemDefinitionPtr idef);
  void processRefItem(pugi::xml_node &node,
                               smtk::attribute::RefItemPtr item);
  void processRefDef(pugi::xml_node &node,
                     smtk::attribute::RefItemDefinitionPtr idef);
  void processDoubleItem(pugi::xml_node &node,
                         smtk::attribute::DoubleItemPtr item);
  void processDoubleDef(pugi::xml_node &node,
                        smtk::attribute::DoubleItemDefinitionPtr idef);
  void processDirectoryItem(pugi::xml_node &node,
                            smtk::attribute::DirectoryItemPtr item);
  void processDirectoryDef(pugi::xml_node &node,
                           smtk::attribute::DirectoryItemDefinitionPtr idef);
  void processFileItem(pugi::xml_node &node,
                       smtk::attribute::FileItemPtr item);
  void processFileDef(pugi::xml_node &node,
                      smtk::attribute::FileItemDefinitionPtr idef);
  void processGroupItem(pugi::xml_node &node,
                         smtk::attribute::GroupItemPtr item);
  void processGroupDef(pugi::xml_node &node,
                       smtk::attribute::GroupItemDefinitionPtr idef);
  void processIntItem(pugi::xml_node &node,
                      smtk::attribute::IntItemPtr item);
  void processIntDef(pugi::xml_node &node,
                     smtk::attribute::IntItemDefinitionPtr idef);
  void processStringItem(pugi::xml_node &node,
                         smtk::attribute::StringItemPtr item);
  void processStringDef(pugi::xml_node &node,
                        smtk::attribute::StringItemDefinitionPtr idef);
  void processModelEntityItem(pugi::xml_node &node,
                       smtk::attribute::ModelEntityItemPtr item);
  void processModelEntityDef(pugi::xml_node &node,
                      smtk::attribute::ModelEntityItemDefinitionPtr idef);
  void processValueItem(pugi::xml_node &node,
                         smtk::attribute::ValueItemPtr item);
  void processValueDef(pugi::xml_node &node,
                       smtk::attribute::ValueItemDefinitionPtr idef);

  void processAttributeView(pugi::xml_node &node,
                            smtk::view::AttributePtr v);

  void processInstancedView(pugi::xml_node &node,
                            smtk::view::InstancedPtr v);

  void processModelEntityView(pugi::xml_node &node,
                              smtk::view::ModelEntityPtr v);

  void processSimpleExpressionView(pugi::xml_node &node,
                                   smtk::view::SimpleExpressionPtr v);

  void processGroupView(pugi::xml_node &node,
                        smtk::view::GroupPtr v);

  void processBasicView(pugi::xml_node &node,
                        smtk::view::BasePtr v);

  static std::string encodeModelEntityMask(smtk::model::BitFlags m);
  static std::string encodeColor(const double *color);

  const smtk::attribute::Manager &m_manager;
  bool m_includeDefinitions;
  bool m_includeInstances;
  bool m_includeModelInformation;
  bool m_includeViews;

  // Keep pugi headers out of public headers:
  struct PugiPrivate;
  PugiPrivate *m_pugi;

  smtk::io::Logger  m_logger;
private:

};

  } // namespace io
} // namespace smtk

#endif // __smtk_io_XmlV1StringWriter_h
