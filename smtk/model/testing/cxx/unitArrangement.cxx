//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/Arrangement.h"

#include "smtk/common/testing/cxx/helpers.h"

using namespace smtk::common;
using namespace smtk::model;

int main()
{
  for (int i = 0; i <= KINDS_OF_ARRANGEMENTS; ++i)
    {
    ArrangementKind k = static_cast<ArrangementKind>(i);
    test(k == ArrangementKindFromName(NameForArrangementKind(k)), "Missing arrangement name");
    test(k == ArrangementKindFromAbbreviation(AbbreviationForArrangementKind(k)), "Missing arrangement abbreviation");
    }
  return 0;
}
