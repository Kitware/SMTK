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

#include "smtk/resource/Component.h"
#include "smtk/resource/PropertyType.h"
#include "smtk/resource/Resource.h"

#include "smtk/model/EntityTypeBits.h"

namespace smtk
{
namespace view
{

/**\brief An abstract base class for obtaining descriptive phrase information.
  *
  * Subclasses implement methods which indicate whether particular
  * information (title, subtitle, color, visibility, icon) is displayable and
  * potentially editable. If they are, then methods to obtain or set values
  * must also be implemented.
  *
  * Each instance may hold a shared pointer to another instance, which it decorates.
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
    ICON = 0x10,       //!< The icon of the phrase's subject.
    EVERYTHING = 0xff  //!< Every aspect of the phrase content.
  };

  /**\brief Append the given decorator at the tail of this object's chain of decorators.
    *
    * Note that this will force this instance's location to match \a other,
    * and will prefer \a other's location to its own if they are both non-null.
    */
  Ptr appendDecorator(Ptr other)
  {
    if (other)
    {
      auto locn = other->location();
      if (!locn)
      {
        locn = this->location();
        other->setLocation(locn);
      }
      PhraseContent* attr = this;
      attr->m_decorator = other;
      for (; attr->m_decorator; attr = attr->m_decorator.get())
      {
        attr->m_location = locn;
      }
    }
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

  /// Return the content which this instance decorates (or a nullptr).
  Ptr peek() const { return m_decorator; }

  /// Return the original content without any decoration
  Ptr undecoratedContent()
  {
    Ptr content = shared_from_this();
    for (; content->peek(); content = content->peek())
      ;
    return content;
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
  /// Return the persistent object related to this phrase (or nullptr if not well defined).
  ///
  /// This method simply calls relatedComponent() and relatedResource() under the hood, but
  /// that may change in the future.
  virtual smtk::resource::PersistentObjectPtr relatedObject() const
  {
    if (m_decorator)
    {
      auto comp = m_decorator->relatedComponent();
      if (comp)
      {
        return comp;
      }
      auto rsrc = m_decorator->relatedResource();
      if (rsrc)
      {
        return rsrc;
      }
    }
    return nullptr;
  }
  /// Return an operator related to this phrase (or nullptr if not well defined).
  virtual smtk::operation::OperationPtr relatedOperation() const
  {
    return m_decorator ? m_decorator->relatedOperation() : nullptr;
  }

  /// Test for use in derived-class equality operators.
  bool equalTo(const PhraseContent& other) const
  {
    return this->location() == other.location() && (typeid(*this) == typeid(other)) &&
      *m_decorator.get() == *other.m_decorator.get();
  }

  /**\brief Return the location of this content in a DescriptivePhrase hierarchy.
    *
    * The DescriptivePhrase instance which owns this content must be accessible
    * from here so that when content is edited, the proper observers can
    * be triggered in response (to keep a user interface up to date).
    *
    * Note that you could, in theory, obtain a pointer to this instance
    * by calling location()->content() and calling popDecorator()
    * repeatedly (but this will obviously remove the instance from use).
    */
  DescriptivePhrasePtr location() const { return m_location.lock(); }

  /**\brief Set the descriptive phrase that will present this content.
    *
    * Note that this should only be called by appendDecorator() or by
    * subphrase generators as they create the initial content for a phrase.
    */
  bool setLocation(DescriptivePhrasePtr locn)
  {
    if (!locn || m_location.lock() == locn)
    {
      return false;
    }
    m_location = locn;
    return true;
  }

  virtual bool operator==(const PhraseContent& other) const { return this->equalTo(other); }
protected:
  PhraseContent() {}

  Ptr m_decorator;
  WeakDescriptivePhrasePtr m_location;
};
}
}

#endif
