//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/view/NodeGroupPhraseContent.h"

#include "smtk/view/DescriptivePhrase.h"

#include "smtk/resource/Resource.h"

namespace smtk
{
namespace view
{

// only color is mutable
NodeGroupPhraseContent::NodeGroupPhraseContent() = default;

NodeGroupPhraseContent::Ptr NodeGroupPhraseContent::setup(
  DescriptivePhrase::Ptr parent,
  const std::string& childType,
  const std::string& singular,
  const std::string& plural,
  int mutability)
{
  m_childType = childType;
  if (!m_childType.empty() && m_childType[0] != '\'' && m_childType[0] != '/')
  {
    // Quoting a type-name forces an exact type match
    // Slashes around the type-name would allow regular expression matching.
    // One or the other must be present.
    m_childType = "'" + m_childType + "'";
  }

  // The singular and plural forms are used as given or
  // computed by defaultSingularPlural() if empty.
  m_singular = singular;
  m_plural = plural;
  this->defaultSingularPlural();

  this->setLocation(parent);
  m_mutability = mutability;
  return shared_from_this();
}

DescriptivePhrasePtr NodeGroupPhraseContent::createPhrase(
  DescriptivePhrasePtr parent,
  const std::string& childType,
  const std::string& singular,
  const std::string& plural,
  int mutability,
  const DescriptivePhrases& children)
{
  (void)children;

  auto result = DescriptivePhrase::create()->setup(DescriptivePhraseType::COMPONENT_LIST, parent);
  auto content =
    NodeGroupPhraseContent::create()->setup(result, childType, singular, plural, mutability);
  content->setLocation(result);
  result->setContent(content);
  return result;
}

bool NodeGroupPhraseContent::displayable(ContentType attr) const
{
  switch (attr)
  {
    case PhraseContent::TITLE:
    case PhraseContent::SUBTITLE:
      return true;
    case PhraseContent::VISIBILITY:
    case PhraseContent::COLOR:
    case PhraseContent::ICON_LIGHTBG:
    case PhraseContent::ICON_DARKBG:
    default:
      break;
  }
  return false;
}

std::string NodeGroupPhraseContent::stringValue(ContentType attr) const
{
  switch (attr)
  {
    case PhraseContent::TITLE:
      if (m_title.empty())
      {
        m_title = this->generateTitle();
      }
      return m_title;
      break;
    case PhraseContent::SUBTITLE:
      return std::string();
      break;

    // We will not provide strings for these:
    case PhraseContent::COLOR:
    case PhraseContent::VISIBILITY:
    case PhraseContent::ICON_LIGHTBG:
    case PhraseContent::ICON_DARKBG:
    default:
      break;
  }
  return std::string();
}

int NodeGroupPhraseContent::flagValue(ContentType attr) const
{
  switch (attr)
  {
    case PhraseContent::COLOR:
    case PhraseContent::TITLE:
    case PhraseContent::SUBTITLE:
    case PhraseContent::VISIBILITY:
    case PhraseContent::ICON_LIGHTBG:
    case PhraseContent::ICON_DARKBG:
    default:
      break;
  }
  return -1;
}

bool NodeGroupPhraseContent::editStringValue(ContentType attr, const std::string& val)
{
  // This should create and call a "set entity property" operator on the
  // related component's name for attr == TITLE.
  (void)attr;
  (void)val;
  return false;
}

bool NodeGroupPhraseContent::editFlagValue(ContentType attr, int val)
{
  (void)attr;
  (void)val;
  return false;
}

/// The list of entities to be presented.
smtk::resource::ComponentArray NodeGroupPhraseContent::relatedComponents() const
{
  smtk::resource::ComponentArray result;

  auto parent = this->location();
  if (!parent)
  {
    return result;
  }

  auto phrases = parent->subphrases();
  for (const auto& phrase : phrases)
  {
    result.push_back(phrase->relatedComponent());
  }
  return result;
}

void NodeGroupPhraseContent::setMutability(int whatsMutable)
{
  m_mutability = whatsMutable;
}

bool NodeGroupPhraseContent::operator==(const PhraseContent& other) const
{
  auto otherList(static_cast<const NodeGroupPhraseContent&>(other));
  return this->equalTo(other) && (this->location() == otherList.location()) &&
    m_childType == otherList.m_childType && m_singular == otherList.m_singular &&
    m_plural == otherList.m_plural;
}

void NodeGroupPhraseContent::defaultSingularPlural()
{
  if (m_singular.empty())
  {
    auto trimSpot = m_childType.rfind("::");
    if (trimSpot == std::string::npos)
    {
      m_singular = m_childType;
    }
    else
    {
      m_singular = m_childType.substr(trimSpot + 2);
      std::transform(m_singular.begin(), m_singular.end(), m_singular.begin(), [](unsigned char c) {
        return std::tolower(c);
      });
    }
    // Trim quotes or slashes from the child-type search string
    if (!m_singular.empty())
    {
      if (m_singular[0] == '\'' || m_singular[0] == '/')
      {
        m_singular = m_singular.substr(1, std::string::npos);
      }
      auto sl = m_singular.size() - 1;
      if (m_singular[sl] == '\'' || m_singular[sl] == '/')
      {
        m_singular = m_singular.substr(0, sl);
      }
    }
  }
  if (m_plural.empty())
  {
    m_plural = m_singular + "s";
  }
}

std::string NodeGroupPhraseContent::generateTitle() const
{
  auto phrase = this->location();
  if (!phrase)
  {
    return std::string();
  }

  // For now, always use the plural form.
  // Switching between them will require
  // notifying the phrase model the phrase has
  // changed, which is difficult to know.
  return m_plural;
}

} // namespace view
} // namespace smtk
