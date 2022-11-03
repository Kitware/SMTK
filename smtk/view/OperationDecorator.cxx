//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/view/OperationDecorator.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/MetadataContainer.h"

#include "smtk/io/Logger.h"

#include "smtk/Regex.h"

namespace smtk
{
namespace view
{
namespace
{

void configureEntry(OperationDecorator::Entry& entry, const Configuration::Component& entryConfig)
{
  int configIdx = -1;
  if ((configIdx = entryConfig.findChild("Label")) >= 0)
  {
    entry.m_label = entryConfig.child(configIdx).contents();
  }
  if ((configIdx = entryConfig.findChild("ButtonLabel")) >= 0)
  {
    entry.m_buttonLabel = entryConfig.child(configIdx).contents();
  }
  if ((configIdx = entryConfig.findChild("Tooltip")) >= 0)
  {
    entry.m_toolTip = entryConfig.child(configIdx).contents();
  }
}
} // anonymous namespace

OperationDecorator::OperationDecorator(std::initializer_list<Entry> entries)
{
  int precedence = 0; // static_cast<int>(entries.size());
  for (const auto& entry : entries)
  {
    auto result = m_entries.emplace(entry);
    if (result.second)
    {
      result.first->m_precedence = precedence++;
    }
  }
}

OperationDecorator::OperationDecorator(
  const std::shared_ptr<smtk::operation::Manager>& manager,
  const Configuration::Component& config)
{
  if (!manager)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "A null operation manager was provided. "
      "All operations will be omitted.");
    return;
  }
  if (config.name() != "OperationDecorator")
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Bad decorator component name (" << config.name()
                                       << " given, OperationDecorator expected). "
                                          "All operations will be omitted.");
    return;
  }
  const auto& typeNames = manager->metadata().get<smtk::operation::NameTag>();
  for (const auto& entryConfig : config.children())
  {
    if (entryConfig.name() == "Operation")
    {
      std::string opType;
      if (!entryConfig.attribute("Type", opType))
      {
        Entry entryTemplate;
        configureEntry(entryTemplate, entryConfig);
        std::string typeRegex;
        if (!entryConfig.attribute("TypeRegex", typeRegex))
        {
          smtkErrorMacro(
            smtk::io::Logger::instance(),
            "Operation entries must have a Type or TypeRegex attribute.");
        }
        else
        {
          smtk::regex pattern(typeRegex);
          smtk::smatch match;
          for (const auto& metadataEntry : typeNames)
          {
            if (smtk::regex_search(metadataEntry.typeName(), match, pattern))
            {
              Entry entry(entryTemplate);
              entry.m_index = metadataEntry.index();
              this->insert(entry);
            }
          }
        }
      }
      else
      {
        auto opMeta = typeNames.find(opType);
        if (opMeta != typeNames.end())
        {
          Entry entry;
          entry.m_index = opMeta->index();
          configureEntry(entry, entryConfig);
          this->insert(entry);
        }
        else
        {
          smtkWarningMacro(
            smtk::io::Logger::instance(), "Entry has unknown operation type \"" << opType << "\".");
        }
      }
    }
  }
}

void OperationDecorator::dump() const
{
  for (const auto& entry : m_entries)
  {
    std::cout << entry.m_index << " " << entry.m_label << "\n";
  }
}

int OperationDecorator::insert(const Entry& entry)
{
  auto it = m_entries.find(entry);
  if (it != m_entries.end())
  {
    return it->m_precedence;
  }
  int precedence = static_cast<int>(m_entries.size());
  auto result = m_entries.emplace(entry);
  result.first->m_precedence = precedence;
  return precedence;
}

OperationDecorator::Override OperationDecorator::at(smtk::operation::Operation::Index index) const
{
  static thread_local Entry dummy;
  dummy = Entry(index);
  auto it = m_entries.find(dummy);
  if (it == m_entries.end())
  {
    return std::make_pair(false, std::reference_wrapper<const Entry>(dummy));
  }
  return std::make_pair(true, std::reference_wrapper<const Entry>(*it));
}

OperationDecorator::Override OperationDecorator::none()
{
  static thread_local Entry dummy;
  std::pair<bool, std::reference_wrapper<const Entry>> result{ false, dummy };
  return result;
}

} // namespace view
} // namespace smtk
