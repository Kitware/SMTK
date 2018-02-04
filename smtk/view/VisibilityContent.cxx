//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/view/VisibilityContent.h"

#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/PhraseModel.h"

#include "smtk/attribute/Attribute.h"

#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"

#include "smtk/resource/Resource.h"

namespace smtk
{
namespace view
{

VisibilityContent::VisibilityContent()
{
}

VisibilityContent::~VisibilityContent()
{
}

VisibilityContent::Ptr VisibilityContent::setup(Delegate delegate)
{
  m_delegate = delegate;
  return shared_from_this();
}

DescriptivePhrasePtr VisibilityContent::decoratePhrase(DescriptivePhrasePtr src, Delegate delegate)
{
  auto decorator = VisibilityContent::create()->setup(delegate);
  decorator->appendDecorator(src->content());
  src->setContent(decorator);
  return src;
}

bool VisibilityContent::displayable(ContentType attr) const
{
  return attr == VISIBILITY ? !!m_delegate(DISPLAYABLE, -1, shared_from_this())
                            : this->Superclass::displayable(attr);
}

bool VisibilityContent::editable(ContentType attr) const
{
  return attr == VISIBILITY ? !!m_delegate(EDITABLE, -1, shared_from_this())
                            : this->Superclass::editable(attr);
}

int VisibilityContent::flagValue(ContentType attr) const
{
  return attr == VISIBILITY ? m_delegate(GET_VALUE, -1, shared_from_this())
                            : this->Superclass::flagValue(attr);
}

bool VisibilityContent::editFlagValue(ContentType attr, int val)
{
  bool didChange = attr == VISIBILITY ? !!m_delegate(SET_VALUE, val, shared_from_this())
                                      : this->Superclass::editFlagValue(attr, val);
  if (didChange)
  {
    auto phr = this->location();
    if (phr)
    {
      auto model = phr->phraseModel();
      if (model)
      {
        auto idx = phr->index();
        const std::vector<int> refs;
        model->trigger(phr, PhraseModelEvent::PHRASE_MODIFIED, idx, idx, refs);
      }
    }
  }
  return didChange;
}

} // view namespace
} // smtk namespace
