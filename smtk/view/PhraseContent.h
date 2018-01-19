//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_PhraseContent_h
#define smtk_view_PhraseContent_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"

#include "smtk/resource/PropertyType.h"

#include "smtk/model/EntityTypeBits.h"

namespace smtk
{
namespace view
{

/**\brief An abstract base class for obtaining descriptive phrase information.
  *
  * Each object may hold a shared pointer to another instance
  * which it decorates.
  * This can be used to change how some attributes are displayed or edited.
  */
class SMTKCORE_EXPORT PhraseContent : smtkEnableSharedPtr(PhraseContent)
{
public:
  smtkTypeMacroBase(PhraseContent);
  virtual ~PhraseContent() {}

  /// Accepted types of content
  enum ContentType
  {
    TITLE = 0x01,      //!< The phrase's title.
    SUBTITLE = 0x02,   //!< The phrase's subtitle.
    COLOR = 0x04,      //!< A control for displaying/editing the color of the phrase's subject.
    VISIBILITY = 0x08, //!< A control for displaying/editing the visibility of the phrase's subject.
    ICON = 0x10        //!< The icon of the phrase's subject.
  };

  /// Append the given decorator at the tail of this object's chain of decorators.
  Ptr appendDecorator(Ptr other)
  {
    PhraseContent* attr = this;
    for (; attr->m_decorator; attr = attr->m_decorator.get())
    {
      // do nothing
    }
    attr->m_decorator = other;
    return shared_from_this();
  }

  /// Remove the front-most decorator from this object while retaining its children.
  Ptr popDecorator()
  {
    Ptr attr = m_decorator;
    if (attr)
    {
      m_decorator = attr->m_decorator;
      attr->m_decorator = Ptr();
    }
    return attr;
  }

  /// Should \a attr be present in the visual display of the phrase?
  virtual bool displayable(ContentType attr) const
  {
    return m_decorator ? m_decorator->displayable(attr) : false;
  }
  /// Is \a attr editable or fixed (for information/display only)?
  virtual bool editable(ContentType attr) const
  {
    return m_decorator ? m_decorator->editable(attr) : false;
  }

  /// Return a string that reflects the given \a attr value.
  virtual std::string stringValue(ContentType attr) const
  {
    return m_decorator ? m_decorator->stringValue(attr) : std::string();
  }
  /// Return an integer bit-flag that reflects the given \a attr value.
  virtual int flagValue(ContentType attr) const
  {
    return m_decorator ? m_decorator->flagValue(attr) : 0;
  }
  /// Return a color vector that reflects the given \a attr value.
  virtual resource::FloatList colorValue(ContentType attr) const
  {
    return m_decorator ? m_decorator->colorValue(attr) : resource::FloatList({ 0., 0., 0., -1. });
  }

  /// Edit the \a attr value to become the given string (or returns false if no-change/invalid).
  virtual bool editStringValue(ContentType attr, const std::string& val)
  {
    return m_decorator ? m_decorator->editStringValue(attr, val) : false;
  }
  /// Edit the \a attr value to become the given flag (or returns false if no-change/invalid).
  virtual bool editFlagValue(ContentType attr, int val)
  {
    return m_decorator ? m_decorator->editFlagValue(attr, val) : false;
  }
  /// Edit the \a attr value to become the given color (or returns false if no-change/invalid).
  virtual bool editColorValue(ContentType attr, const resource::FloatList& val)
  {
    return m_decorator ? m_decorator->editColorValue(attr, val) : false;
  }

  /// Return the resource related to this phrase (or nullptr if not well defined).
  virtual smtk::resource::ResourcePtr relatedResource() const
  {
    return m_decorator ? m_decorator->relatedResource() : nullptr;
  }
  /// Return the resource component related to this phrase (or nullptr if not well defined).
  virtual smtk::resource::ComponentPtr relatedComponent() const
  {
    return m_decorator ? m_decorator->relatedComponent() : nullptr;
  }
  /// Return an operator related to this phrase (or nullptr if not well defined).
  virtual smtk::operation::OperatorPtr relatedOperator() const
  {
    return m_decorator ? m_decorator->relatedOperator() : nullptr;
  }

  /// Test for use in derived-class equality operators.
  bool equalTo(const PhraseContent& other) const
  {
    return (typeid(*this) == typeid(other)) && *m_decorator.get() == *other.m_decorator.get();
  }

  virtual bool operator==(const PhraseContent& other) const { return this->equalTo(other); }
protected:
  PhraseContent() {}

  Ptr m_decorator;
};
}
}

#endif
