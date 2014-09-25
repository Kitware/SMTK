#include "smtk/model/Arrangement.h"

#include "smtk/util/testing/cxx/helpers.h"

using namespace smtk::util;
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
