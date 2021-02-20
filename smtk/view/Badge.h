//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_view_Badge_h
#define smtk_view_Badge_h

#include "smtk/view/Configuration.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"

#include <array>

namespace smtk
{
namespace view
{

/**\brief A base class for actions taken on badges.
  *
  * This class contains no information; its subclasses do.
  * When passed to Badge::action(), the badge should attempt
  * to cast this class to the subclasses it can handle.
  * If no subclass can be handled, the badge does not
  * support the given action.
  */
class SMTKCORE_EXPORT BadgeAction
{
public:
  /// When visiting multiple phrases to which a badge action
  /// should apply, this function is supplied by the badge.
  /// Returning true indicates early termination.
  using PhraseVisitor = std::function<bool(DescriptivePhrase*)>;

  /// Some actions may apply to multiple phrases
  /// (e.g., those selected when a key is pressed).
  /// In those cases, subclasses of BadgeAction may provide
  /// a method to visit all phrases to which the action applies.
  virtual void visitRelatedPhrases(PhraseVisitor visitor) const { (void)visitor; }
};

/**\brief The basic badge action: the user clicked on a badge.
  *
  * No additional information is provided.
  * Some frameworks may override visitRelatedPhrases in a subclass
  * to indicate that multiple phrases were selected when the user
  * clicked on the primary phrase passed to Badge::action().
  */
class SMTKCORE_EXPORT BadgeActionToggle : public BadgeAction
{
};

/**\brief A base class for descriptive-phrase badges.
  *
  * A badge is responsible for
  * + determining whether it applies to a descriptive phrase;
  * + providing a tooltip string indicating its purpose to a user;
  * + providing a string of SVG to render (if it applies); and
  * + performing an action when clicked (which may do nothing).
  *
  * A badge may be marked as being a "default."
  * This indicates that user gestures that are not specific to
  * any badge may still apply an badge action; the first
  * _applicable_ badge marked as default should have it action
  * invoked in response to these gestures.
  * An example is how qtReferenceItem uses MembershipBadge:
  * when users click on the title of a phrase (not any badge),
  * it is still desirable for the membership of the phrase to
  * be changed, so the MembershipBadge is marked as default.
  */
class SMTKCORE_EXPORT Badge : smtkEnableSharedPtr(Badge)
{
public:
  smtkTypeMacroBase(Badge);
  Badge()
    : m_isDefault(false)
  {
  }
  Badge(const Badge&) = delete;
  void operator=(const Badge&) = delete;
  virtual ~Badge() {}

  /// Returns true if the badge should appear next to the given phrase:
  virtual bool appliesToPhrase(const DescriptivePhrase*) const { return false; }

  /// Return a tool-tip string for the badge that is relevant to this phrase.
  ///
  /// An empty string (the default) indicates that no tooltip should be shown.
  /// If a non-empty string is returned, it should describe the meaning of the
  /// badge to the user. If the badge changes its SVG based on application state,
  /// the tip should be specific to its current state.
  ///
  /// For example, a badge that controls visibility of a phrase's subject in a render-window
  /// should say either "Click to show <title>" when the phrase's subject is hidden
  /// or "Click to hide <title>" when the phrase's subject is visible.
  virtual std::string tooltip(const DescriptivePhrase*) const { return std::string(); }

  /// Returns a string for rendering the badge icon.
  ///
  /// The string is currently interpreted by qtDescriptivePhraseDelegate::paint()
  /// as scalable vector graphics (SVG).
  ///
  /// The icon may change depending on
  /// (1) the state of the phrase or its parent view,
  /// (2) the background color of the phrase.
  virtual std::string icon(
    const DescriptivePhrase* phrase, const std::array<float, 4>& background) const = 0;

  /// Take an action when the badge is clicked.
  ///
  /// Return true if the action is supported (and was taken) by the badge for the given phrase;
  /// otherwise return false.
  virtual bool action(const DescriptivePhrase*, const BadgeAction&) { return false; }

  /// Return whether this badge be invoked by non-specific user gestures.
  ///
  /// This should only be called when appliesToPhrase() returns true.
  bool isDefault() const { return m_isDefault; }

  /// Set this badge as a default.
  void setIsDefault(bool isDefault) { m_isDefault = isDefault; }

protected:
  /// Should this badge be invoked by non-specific user gestures when it is applicable?
  bool m_isDefault;
};
}
}
#endif
