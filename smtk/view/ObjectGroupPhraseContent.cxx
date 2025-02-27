//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/view/ObjectGroupPhraseContent.h"

#include "smtk/common/Visit.h"
#include "smtk/view/ComponentPhraseContent.h"
#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/PhraseModel.h"
#include "smtk/view/ResourcePhraseContent.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"

#include "smtk/operation/Manager.h"

#include "smtk/resource/Resource.h"

namespace smtk
{
namespace view
{

ObjectGroupPhraseContent::ObjectGroupPhraseContent()
  : m_resourceFilter("smtk::resource::Resource")
  , m_componentFilter("*")
  , m_title("everything")
{
}

ObjectGroupPhraseContent::~ObjectGroupPhraseContent() = default;

ObjectGroupPhraseContent::Ptr ObjectGroupPhraseContent::setup(
  const std::string& title,
  const std::string& resourceFilter,
  const std::string& componentFilter)
{
  m_title = title;
  m_resourceFilter = resourceFilter;
  m_componentFilter = componentFilter;
  return shared_from_this();
}

DescriptivePhrasePtr ObjectGroupPhraseContent::createPhrase(
  const std::string& title,
  const std::string& resourceFilter,
  const std::string& componentFilter,
  DescriptivePhrasePtr parent)
{
  DescriptivePhrasePtr result =
    DescriptivePhrase::create()->setup(DescriptivePhraseType::LIST, parent);
  auto content = ObjectGroupPhraseContent::create()->setup(title, resourceFilter, componentFilter);
  result->setContent(content);
  content->setLocation(result);
  return result;
}

void ObjectGroupPhraseContent::children(DescriptivePhrases& container) const
{
  DescriptivePhrasePtr location = m_location.lock();
  if (!location)
  {
    return;
  }

  PhraseModelPtr model = location->phraseModel();
  // Why model is empty
  if (!model)
  {
    std::cout << "ObjectGroupPhraseContent::children " << location->title()
              << " does not have"
                 "a valid model"
              << std::endl;
    return;
  }

  model->visitSources(
    [this, &container, &location](
      const smtk::resource::ManagerPtr& rsrcMgr,
      const smtk::operation::ManagerPtr& /*unused*/,
      const smtk::view::ManagerPtr& /*unused*/,
      const smtk::view::SelectionPtr&
      /*unused*/) -> smtk::common::Visit {
      if (!rsrcMgr)
      {
        return smtk::common::Visit::Continue;
      }
      auto rsrcs = rsrcMgr->find(m_resourceFilter);
      for (const auto& rsrc : rsrcs)
      {
        if (m_componentFilter.empty())
        {
          auto phr = ResourcePhraseContent::createPhrase(
            rsrc, PhraseContent::ContentType::EVERYTHING, location);
          container.push_back(phr);
        }
        else
        {
          auto comps = rsrc->filter(m_componentFilter);
          for (const auto& comp : comps)
          {
            auto phr = ComponentPhraseContent::createPhrase(
              comp, PhraseContent::ContentType::EVERYTHING, location);
            container.push_back(phr);
          }
        }
      }
      return smtk::common::Visit::Continue;
    });
  // TODO: sort phrases
}

bool ObjectGroupPhraseContent::hasChildren() const
{
  DescriptivePhrasePtr location = m_location.lock();
  if (!location)
  {
    return false;
  }

  PhraseModelPtr model = location->phraseModel();
  // Why model is empty
  if (!model)
  {
    return false;
  }

  bool result = false;
  model->visitSources(
    [this, &result](
      const smtk::resource::ManagerPtr& rsrcMgr,
      const smtk::operation::ManagerPtr& /*unused*/,
      const smtk::view::ManagerPtr& /*unused*/,
      const smtk::view::SelectionPtr&
      /*unused*/) -> smtk::common::Visit {
      if (!rsrcMgr)
      {
        return smtk::common::Visit::Continue;
      }
      auto rsrcs = rsrcMgr->find(m_resourceFilter);
      if (m_componentFilter.empty())
      {
        // OK we are looking for resources, did we find any from this source?
        if (rsrcs.empty())
        {
          return smtk::common::Visit::Continue;
        }

        result = true;
        return smtk::common::Visit::Halt;
      }

      for (const auto& rsrc : rsrcs)
      {
        auto comps = rsrc->filter(m_componentFilter);
        // OK we are looking for Components, did we find any?
        if (comps.empty())
        {
          return smtk::common::Visit::Continue;
        }

        result = true;
        return smtk::common::Visit::Halt;
      }
      return smtk::common::Visit::Continue;
    });
  return result;
}

} // namespace view
} // namespace smtk
