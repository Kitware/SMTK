//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/markup/operators/EditComment.h"

#include "smtk/view/NameManager.h"

#include "smtk/markup/Resource.h"
#include "smtk/markup/operators/EditComment_xml.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/markup/json/jsonResource.h"

#include "smtk/operation/MarkGeometry.h"

#include "smtk/resource/json/Helper.h"

#include "smtk/common/StringUtil.h"

namespace smtk
{
namespace markup
{

using namespace smtk::string::literals;

EditComment::Result EditComment::operateInternal()
{
  auto objects = this->parameters()->associations();
  smtk::string::Token mimetype = this->parameters()->findString("mime-type")->value();
  std::string rawText = this->parameters()->findString("text")->value();
  // Trim plain text; leave everything else untouched.
  auto text = mimetype == "text/plain"_token ? smtk::common::StringUtil::trim(rawText) : rawText;
  bool shouldDelete = this->parameters()->findVoid("remove when empty")->isEnabled();

  auto result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  auto modified = result->findComponent("modified");
  if (objects->numberOfValues() == 1 && objects->value()->matchesType("smtk::markup::Comment"))
  {
    // We're editing/deleting an existing comment.
    auto comment = std::dynamic_pointer_cast<smtk::markup::Comment>(objects->value());
    if (text.empty() && shouldDelete)
    {
      auto expunged = result->findComponent("expunged");
      expunged->appendValue(comment);
      // Remove all arcs connected to this component:
      comment->disconnect();
      // Clear the component's geometry:
      smtk::operation::MarkGeometry().erase(comment);
      // Remove the component from the resource (it is now owned by
      // the expunged item, so it is not destroyed yet):
      comment->parentResourceAs<smtk::markup::Resource>()->erase(comment->id());
    }
    else
    {
      comment->setData(text);
      if (mimetype.valid())
      {
        comment->setMimetype(mimetype);
      }
      modified->appendValue(comment);
    }
  }
  else
  {
    // We're creating a new comment.
    auto* resource = std::dynamic_pointer_cast<smtk::resource::Component>(objects->value())
                       ->parentResourceAs<smtk::markup::Resource>();
    auto comment = resource->createNode<smtk::markup::Comment>();
    if (!comment)
    {
      smtkErrorMacro(this->log(), "Could not create comment.");
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }
    if (objects->numberOfValues() == 1)
    {
      comment->setName(objects->value(0)->name() + " comment");
    }
    else if (this->managers()->contains<smtk::view::NameManager::Ptr>())
    {
      auto nameManager = this->managers()->get<smtk::view::NameManager::Ptr>();
      comment->setName(nameManager->nameForObject(*comment));
    }
    else
    {
      comment->setName("comment");
    }
    comment->setData(text);
    if (mimetype.valid())
    {
      comment->setMimetype(mimetype);
    }
    result->findComponent("created")->appendValue(comment);

    // Now attached the comment to the associated objects.
    for (const auto& object : *objects)
    {
      if (auto component = std::dynamic_pointer_cast<smtk::markup::Component>(object))
      {
        comment->outgoing<arcs::LabelsToSubjects>().connect(component);
        modified->appendValue(component);
      }
    }
  }

  return result;
}

const char* EditComment::xmlDescription() const
{
  return EditComment_xml;
}

} // namespace markup
} // namespace smtk
