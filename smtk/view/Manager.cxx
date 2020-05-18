//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/view/Manager.h"
#include "smtk/view/QueryFilterSubphraseGenerator.h"
#include "smtk/view/SubphraseGenerator.h"

namespace smtk
{
namespace view
{

Manager::Manager()
  : m_phraseModelFactory(this)
{
}

Manager::~Manager() = default;

} // namespace view
} // namespace smtk
