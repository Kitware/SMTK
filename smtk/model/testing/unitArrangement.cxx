#include "smtk/model/Arrangement.h"

#include <assert.h>

using namespace smtk::model;

int main()
{
  for (int i = 0; i <= KINDS_OF_ARRANGEMENTS; ++i)
    {
    ArrangementKind k = static_cast<ArrangementKind>(i);
    assert(k == ArrangementKindFromName(NameForArrangementKind(k)));
    assert(k == ArrangementKindFromAbbreviation(AbbreviationForArrangementKind(k)));
    }
  return 0;
}
