#include "smtk/model/ArrangementKind.h"

namespace smtk {
 namespace model {

const char* ArrangementKindName[KINDS_OF_ARRANGEMENTS + 1] = {
  "inclusion",    // INCLUDES
  "cell",         // HAS_CELL
  "shell",        // HAS_SHELL
  "use",          // HAS_USE
  "embedding",    // EMBEDDED_IN
  "subset",       // SUBSET_OF
  "superset",     // SUPERSET_OF
  "instantiates", // INSTANCE_OF
  "instance",     // INSTANCED_BY
  "invalid"       // KINDS_OF_ARRANGEMENTS
};

const char* ArrangementKindAbbr[KINDS_OF_ARRANGEMENTS + 1] = {
  "i", // INCLUDES
  "c", // HAS_CELL
  "s", // HAS_SHELL
  "u", // HAS_USE
  "e", // EMBEDDED_IN
  "b", // SUBSET_OF (suBset)
  "r", // SUPERSET_OF (supeRset)
  "p", // INSTANCE_OF (Prototype)
  "x", // INSTANCED_BY (eXemplar)
  "?"  // KINDS_OF_ARRANGEMENTS
};

ArrangementKind ArrangementKindFromName(const std::string& name)
{
  for (int i = 0; i < KINDS_OF_ARRANGEMENTS; ++i)
    {
    if (name == ArrangementKindName[i])
      {
      return ArrangementKind(i);
      }
    }
  return KINDS_OF_ARRANGEMENTS;
}

std::string NameForArrangementKind(ArrangementKind k)
{
  return ArrangementKindName[k];
}

ArrangementKind ArrangementKindFromAbbreviation(const std::string& abbr)
{
  for (int i = 0; i < KINDS_OF_ARRANGEMENTS; ++i)
    {
    if (abbr == ArrangementKindAbbr[i])
      {
      return ArrangementKind(i);
      }
    }
  return KINDS_OF_ARRANGEMENTS;
}

std::string AbbreviationForArrangementKind(ArrangementKind k)
{
  return ArrangementKindAbbr[k];
}

  } //namespace model
} // namespace smtk
