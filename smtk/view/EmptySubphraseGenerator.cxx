//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/view/EmptySubphraseGenerator.h"

using namespace smtk::view;

EmptySubphraseGenerator::EmptySubphraseGenerator() = default;

DescriptivePhrases EmptySubphraseGenerator::subphrases(DescriptivePhrase::Ptr src)
{
  (void)src;
  DescriptivePhrases result;
  return result;
}
