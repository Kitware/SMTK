//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_io_ItemDefinitionsHelper_h
#define smtk_io_ItemDefinitionsHelper_h

#include "smtk/io/XmlDocV1Parser.h"

#include "smtk/attribute/ComponentItemDefinition.h"
#include "smtk/attribute/CustomItemDefinition.h"
#include "smtk/attribute/DateTimeItemDefinition.h"
#include "smtk/attribute/DirectoryItemDefinition.h"
#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/ReferenceItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItemDefinition.h"
#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/attribute/ValueItemDefinition.h"
#include "smtk/attribute/VoidItemDefinition.h"

namespace smtk
{
namespace io
{
///\brief A Helper class for parsing XML Nodes that
/// represent Item Definition Sections currently used
/// for Attribute Definitions as well as Value, Group,
/// and Reference Item Definitions

class SMTKCORE_EXPORT ItemDefinitionsHelper
{
public:
  ///\brief Method for parsing Item Definitions represented
  /// by the xml node itemsNode, for Definition-type class
  /// def.
  /// ActiveBlockNames is used when processing Item Block
  /// references in order to prevent infinite recursion.
  /// defName is used for error reporting if a problem should
  /// be encountered and is the name of the def being processed
  /// and attType is its type
  template<typename DefType>
  void processItemDefinitions(
    XmlDocV1Parser* parser,
    pugi::xml_node& itemsNode,
    DefType& def,
    std::set<std::string>& activeBlockNames,
    const std::string& defName,
    const std::string& attType)
  {
    std::string itemName, nodeName;
    attribute::Item::Type itype;
    attribute::ItemDefinitionPtr idef;
    pugi::xml_node node;
    for (node = itemsNode.first_child(); node; node = node.next_sibling())
    {
      pugi::xml_attribute xatt = node.attribute("Name");

      if (!xatt)
      {
        smtkErrorMacro(
          parser->m_logger,
          "Item Definition of " << attType << ": " << defName << " is missing Name attribute");
        continue;
      }

      itemName = xatt.value();
      nodeName = node.name();
      // Are we referencing an Item Definition Block
      if (nodeName == "Block")
      {
        // See if a namespace was specified else assume the global namespace ""
        xatt = node.attribute("Namespace");
        std::string itemNamespace = (xatt) ? xatt.value() : "";

        auto nsit = parser->m_itemDefintionBlocks.find(itemNamespace);
        if (nsit == parser->m_itemDefintionBlocks.end())
        {
          smtkErrorMacro(
            parser->m_logger,
            "Can not find Item Block Namespace: " << itemNamespace << " referenced in " << attType
                                                  << ": " << defName);
          continue;
        }
        auto it = nsit->second.find(itemName);
        if (it == nsit->second.end())
        {
          smtkErrorMacro(
            parser->m_logger,
            "Can not find Item Block Name: " << itemName << "in Namespace: " << itemNamespace
                                             << " referenced in " << attType << ": " << defName);
          continue;
        }
        // Make sure we are not already parsing a block of the same name
        else if (activeBlockNames.find(itemName) != activeBlockNames.end())
        {
          smtkErrorMacro(
            parser->m_logger,
            "Encountered Recursive Loop : " << itemName << " referenced in " << attType << ": "
                                            << defName);
          continue;
        }
        activeBlockNames.emplace(itemName);
        this->processItemDefinitions<DefType>(
          parser, it->second, def, activeBlockNames, defName, attType);
        activeBlockNames.erase(itemName);
        continue;
      }

      itype = attribute::Item::string2Type(nodeName);
      switch (itype)
      {
        case attribute::Item::AttributeRefType:
        {
          auto itemdef =
            def->template addItemDefinition<attribute::ComponentItemDefinition>(itemName);
          if (!itemdef)
          {
            smtkErrorMacro(
              parser->m_logger,
              "Failed to create Ref Item definition Type: " << node.name() << " needed to create "
                                                            << attType << ": " << defName);
            continue;
          }
          parser->processRefDef(node, itemdef);
        }
        break;
        case attribute::Item::ComponentType:
        {
          auto itemdef =
            def->template addItemDefinition<attribute::ComponentItemDefinition>(itemName);
          if (!itemdef)
          {
            smtkErrorMacro(
              parser->m_logger,
              "Failed to create Component Item definition Type: "
                << node.name() << " needed to create " << attType << ": " << defName);
            continue;
          }
          parser->processComponentDef(node, itemdef);
        }
        break;
        case attribute::Item::DateTimeType:
        {
          auto itemdef =
            def->template addItemDefinition<attribute::DateTimeItemDefinition>(itemName);
          if (!itemdef)
          {
            smtkErrorMacro(
              parser->m_logger,
              "Failed to create DateTime Item definition Type: "
                << node.name() << " needed to create " << attType << ": " << defName);
            continue;
          }
          parser->processDateTimeDef(node, itemdef);
        }
        break;
        case attribute::Item::DirectoryType:
        {
          auto itemdef =
            def->template addItemDefinition<attribute::DirectoryItemDefinition>(itemName);
          if (!itemdef)
          {
            smtkErrorMacro(
              parser->m_logger,
              "Failed to create Directory Item definition Type: "
                << node.name() << " needed to create " << attType << ": " << defName);
            continue;
          }
          parser->processDirectoryDef(node, itemdef);
        }
        break;
        case attribute::Item::DoubleType:
        {
          auto itemdef = def->template addItemDefinition<attribute::DoubleItemDefinition>(itemName);
          if (!itemdef)
          {
            smtkErrorMacro(
              parser->m_logger,
              "Failed to create Double Item definition Type: "
                << node.name() << " needed to create " << attType << ": " << defName);
            continue;
          }
          parser->processDoubleDef(node, itemdef);
        }
        break;
        case attribute::Item::FileType:
        {
          auto itemdef = def->template addItemDefinition<attribute::FileItemDefinition>(itemName);
          if (!itemdef)
          {
            smtkErrorMacro(
              parser->m_logger,
              "Failed to create File Item definition Type: " << node.name() << " needed to create "
                                                             << attType << ": " << defName);
            continue;
          }
          parser->processFileDef(node, itemdef);
        }
        break;
        case attribute::Item::GroupType:
        {
          auto itemdef = def->template addItemDefinition<attribute::GroupItemDefinition>(itemName);
          if (!itemdef)
          {
            smtkErrorMacro(
              parser->m_logger,
              "Failed to create Group Item definition Type: " << node.name() << " needed to create "
                                                              << attType << ": " << defName);
            continue;
          }
          parser->processGroupDef(node, itemdef);
        }
        break;
        case attribute::Item::IntType:
        {
          auto itemdef = def->template addItemDefinition<attribute::IntItemDefinition>(itemName);
          if (!itemdef)
          {
            smtkErrorMacro(
              parser->m_logger,
              "Failed to create Int Item definition Type: " << node.name() << " needed to create "
                                                            << attType << ": " << defName);
            continue;
          }
          parser->processIntDef(node, itemdef);
        }
        break;
        case attribute::Item::MeshEntityType:
        {
          auto itemdef =
            def->template addItemDefinition<attribute::ComponentItemDefinition>(itemName);
          if (!itemdef)
          {
            smtkErrorMacro(
              parser->m_logger,
              "Failed to create MeshEntity Item definition Type: "
                << node.name() << " needed to create " << attType << ": " << defName);
            continue;
          }
          parser->processMeshEntityDef(node, itemdef);
        }
        break;
        case attribute::Item::ModelEntityType:
        {
          auto itemdef =
            def->template addItemDefinition<attribute::ComponentItemDefinition>(itemName);
          if (!itemdef)
          {
            smtkErrorMacro(
              parser->m_logger,
              "Failed to create ModelEntity Item definition Type: "
                << node.name() << " needed to create " << attType << ": " << defName);
            continue;
          }
          parser->processModelEntityDef(node, itemdef);
        }
        break;
        case attribute::Item::ReferenceType:
        {
          auto itemdef =
            def->template addItemDefinition<attribute::ReferenceItemDefinition>(itemName);
          if (!itemdef)
          {
            smtkErrorMacro(
              parser->m_logger,
              "Failed to create Reference Item definition Type: "
                << node.name() << " needed to create " << attType << ": " << defName);
            continue;
          }
          parser->processReferenceDef(node, itemdef);
        }
        break;
        case attribute::Item::ResourceType:
        {
          auto itemdef =
            def->template addItemDefinition<attribute::ResourceItemDefinition>(itemName);
          if (!itemdef)
          {
            smtkErrorMacro(
              parser->m_logger,
              "Failed to create Resource Item definition Type: "
                << node.name() << " needed to create " << attType << ": " << defName);
            continue;
          }
          parser->processResourceDef(node, itemdef);
        }
        break;
        case attribute::Item::StringType:
        {
          auto itemdef = def->template addItemDefinition<attribute::StringItemDefinition>(itemName);
          if (!itemdef)
          {
            smtkErrorMacro(
              parser->m_logger,
              "Failed to create String Item definition Type: "
                << node.name() << " needed to create " << attType << ": " << defName);
            continue;
          }
          parser->processStringDef(node, itemdef);
        }
        break;
        case attribute::Item::VoidType:
        {
          auto itemdef = def->template addItemDefinition<attribute::VoidItemDefinition>(itemName);
          if (!itemdef)
          {
            smtkErrorMacro(
              parser->m_logger,
              "Failed to create Void Item definition Type: " << node.name() << " needed to create "
                                                             << attType << ": " << defName);
            continue;
          }
          parser->processItemDef(node, itemdef);
        }
        break;

        default:
          auto typeName = node.attribute("TypeName");
          if (
            typeName &&
            parser->m_resource->customItemDefinitionFactory().contains(typeName.value()))
          {
            idef = parser->m_resource->customItemDefinitionFactory().createFromName(
              typeName.value(), std::string(node.attribute("Name").value()));
            (*static_cast<smtk::attribute::CustomItemBaseDefinition*>(idef.get())) << node;
            def->addItemDefinition(idef);
            parser->processItemDef(node, idef);
          }
          else
          {
            smtkErrorMacro(
              parser->m_logger,
              "Unsupported Item definition Type: " << node.name() << " needed to create " << attType
                                                   << ": " << defName);
          }
      }
    }
  }
};
} // namespace io
} // namespace smtk

#endif /* smtk_io_ItemDefinitionsHelper_h */
