//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_view_PhraseModelObserver_h
#define __smtk_view_PhraseModelObserver_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/common/Observers.h"

#include <vector>

namespace smtk
{
namespace view
{

/// Events that can be observed on an smtk::view::PhraseModel.
enum class PhraseModelEvent
{
  ABOUT_TO_INSERT, //!< A phrase or range of phrases is about to be inserted in the parent.
  INSERT_FINISHED, //!< A phrase of range of phrases has been inserted in the parent.
  ABOUT_TO_REMOVE, //!< A phrase or range of phrases is about to be removed from the parent.
  REMOVE_FINISHED, //!< A phrase or range of phrases has been removed from the parent.
  ABOUT_TO_MOVE,   //!< A phrase or range of phrases is being moved from one place to another.
  MOVE_FINISHED,   //!< A phrase or range of phrases has been moved and the update is complete.
  PHRASE_MODIFIED  //!< The given phrase has had its text, color, or some other property modified.
};

/// Events that alter the phrase model trigger callbacks of this type.
typedef std::function<void(
  DescriptivePhrasePtr,
  PhraseModelEvent,
  const std::vector<int>&,
  const std::vector<int>&,
  const std::vector<int>&)>
  PhraseModelObserver;

/// A class for holding PhraseModelObserver functors that observe phrase model events.
typedef smtk::common::Observers<PhraseModelObserver> PhraseModelObservers;
} // namespace view
} // namespace smtk

#endif // __smtk_view_PhraseModelObserver_h
