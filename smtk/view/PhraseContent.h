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
  virtual ~PhraseContent() = default;

  /// Accepted types of content
  enum ContentType
  {
    TITLE = 0x01,          //!< The phrase's title.
    EDITABLE_TITLE = 0x02, //!< The text used to edit the content's title.
    SUBTITLE = 0x04,       //!< The phrase's subtitle.
    COLOR = 0x08,          //!< A control for displaying/editing the color of the phrase's subject.
    VISIBILITY = 0x10, //!< A control for displaying/editing the visibility of the phrase's subject.
    ICON_LIGHTBG = 0x20, //!< The icon of the phrase's subject for light backgrounds.
    ICON_DARKBG = 0x40,  //!< The icon of the phrase's subject for dark backgrounds.
    EVERYTHING = 0xff    //!< Every aspect of the phrase content.
  };

  /// Should \a attr be present in the visual display of the phrase?
  virtual bool displayable(ContentType /*attr*/) const { return false; }
  /// Is \a attr editable or fixed (for information/display only)?
  virtual bool editable(ContentType /*attr*/) const { return false; }

  /// Return a string that reflects the given \a attr value.
  virtual std::string stringValue(ContentType /*attr*/) const { return std::string(); }
  /// Return an integer bit-flag that reflects the given \a attr value.
  virtual int flagValue(ContentType /*attr*/) const { return 0; }

  /// Edit the \a attr value to become the given string (or returns false if no-change/invalid).
  virtual bool editStringValue(ContentType /*attr*/, const std::string& /*val*/) { return false; }
  /// Edit the \a attr value to become the given flag (or returns false if no-change/invalid).
  virtual bool editFlagValue(ContentType /*attr*/, int /*val*/) { return false; }

  /// Return the resource related to this phrase (or nullptr if not well defined).
  virtual smtk::resource::ResourcePtr relatedResource() const { return nullptr; }
  virtual smtk::resource::Resource* relatedRawResource() const
  {
    return this->relatedResource().get();
  }
  /// Return the resource component related to this phrase (or nullptr if not well defined).
  virtual smtk::resource::ComponentPtr relatedComponent() const { return nullptr; }
  virtual smtk::resource::Component* relatedRawComponent() const
  {
    return this->relatedComponent().get();
  }
  /// Return the persistent object related to this phrase (or nullptr if not well defined).
  ///
  /// This method simply calls relatedComponent() and relatedResource() under the hood, but
  /// that may change in the future.
  virtual smtk::resource::PersistentObjectPtr relatedObject() const
  {
    auto comp = this->relatedComponent();
    if (comp)
    {
      return comp;
    }
    auto rsrc = this->relatedResource();
    if (rsrc)
    {
      return rsrc;
    }
    return nullptr;
  }
  /// Return an operator related to this phrase (or nullptr if not well defined).
  virtual smtk::operation::OperationPtr relatedOperation() const { return nullptr; }

  /// Test for use in derived-class equality operators.
  bool equalTo(const PhraseContent& other) const
  {
    return this->location() == other.location() && (typeid(*this) == typeid(other));
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
  PhraseContent() = default;

  WeakDescriptivePhrasePtr m_location;
};
} // namespace view
} // namespace smtk

#endif
