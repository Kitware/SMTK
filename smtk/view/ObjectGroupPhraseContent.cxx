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
#include "smtk/model/operators/SetProperty.h"

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
  const std::string& title, const std::string& resourceFilter, const std::string& componentFilter)
{
  m_title = title;
  m_resourceFilter = resourceFilter;
  m_componentFilter = componentFilter;
  return shared_from_this();
}

DescriptivePhrasePtr ObjectGroupPhraseContent::createPhrase(const std::string& title,
  const std::string& resourceFilter, const std::string& componentFilter,
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
    std::cout << "ObjectGroupPhraseContent::children " << location->title() << " does not have"
                                                                               "a valid model"
              << std::endl;
    return;
  }

  model->visitSources(
    [this, &container, &location](const smtk::resource::ManagerPtr& rsrcMgr,
      const smtk::operation::ManagerPtr&, const smtk::view::SelectionPtr&) -> bool {
      if (!rsrcMgr)
      {
        return true;
      }
      auto rsrcs = rsrcMgr->find(m_resourceFilter);
      for (auto rsrc : rsrcs)
      {
        if (m_componentFilter.empty())
        {
          auto phr = ResourcePhraseContent::createPhrase(
            rsrc, PhraseContent::ContentType::EVERYTHING, location);
          container.push_back(phr);
        }
        else
        {
          auto comps = rsrc->find(m_componentFilter);
          std::cout << "ObjectGroupPhraseContent: Find " << comps.size() << " Components"
                                                                            " with filter="
                    << m_componentFilter << std::endl;
          for (auto comp : comps)
          {
            auto phr = ComponentPhraseContent::createPhrase(
              comp, PhraseContent::ContentType::EVERYTHING, location);
            container.push_back(phr);
          }
        }
      }
      return true;
    });
  // TODO: sort phrases
}

} // view namespace
} // smtk namespace
