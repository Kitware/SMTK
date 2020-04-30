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

/**\brief A base class for descriptive-phrase badges.
  *
  * A badge is responsible for
  * + determining whether it applies to a descriptive phrase;
  * + providing a tooltip string indicating its purpose to a user;
  * + providing a string of SVG to render (if it applies); and
  * + performing an action when clicked (which may do nothing).
  */
class SMTKCORE_EXPORT Badge : smtkEnableSharedPtr(Badge)
{
public:
  smtkTypeMacroBase(Badge);
  Badge() {}
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
  virtual void action(DescriptivePhrase*) const {}
};
}
}
#endif
