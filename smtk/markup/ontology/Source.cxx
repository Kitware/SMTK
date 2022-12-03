//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/markup/ontology/Source.h"

#include "smtk/io/Logger.h"

#include <iostream>
#include <map>

namespace smtk
{
namespace markup
{
namespace ontology
{

namespace // anonymous
{
std::map<std::string, Source> g_ontologiesByName;
std::map<std::string, std::string> g_ontologiesUrlToName;
} // anonymous namespace

bool Identifier::operator<(const Identifier& other) const
{
  return this->name < other.name || (this->name == other.name && this->url < other.url);
}

bool Relation::operator<(const Relation& other) const
{
  return url < other.url;
}

const Source& Source::findByName(const std::string& name)
{
  static Source blank;
  auto it = g_ontologiesByName.find(name);
  if (it == g_ontologiesByName.end())
  {
    if (name == "default" && !g_ontologiesByName.empty())
    {
      return g_ontologiesByName.begin()->second;
    }
    return blank;
  }
  return it->second;
}

const Source& Source::findByURL(const std::string& url)
{
  static Source blank;
  auto it = g_ontologiesUrlToName.find(url);
  if (it == g_ontologiesUrlToName.end())
  {
    return blank;
  }
  return Source::findByName(it->second);
}

bool Source::registerSource(const Source& src)
{
  if (
    src.url().empty() || src.name().empty() ||
    g_ontologiesByName.find(src.name()) != g_ontologiesByName.end() ||
    g_ontologiesUrlToName.find(src.url()) != g_ontologiesUrlToName.end())
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(), "Refusing to register a source with no name or url.");
    return false;
  }
  g_ontologiesByName[src.name()] = src;
  g_ontologiesUrlToName[src.url()] = src.name();
  return true;
}

} // namespace ontology
} // namespace markup
} // namespace smtk
