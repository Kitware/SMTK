#ifndef __smtk_model_Cell_h
#define __smtk_model_Cell_h

#include "smtk/SMTKCoreExports.h" // for SMTKCORE_EXPORT macro

#include <map>
#include <set>
#include <string>
#include <sstream>
#include <vector>

#include "smtk/util/UUID.h"

namespace smtk {
  namespace model {

using smtk::util::UUID;
typedef std::set<UUID> UUIDs;
typedef std::vector<UUID> UUIDArray;
typedef std::vector<UUIDArray> UUIDArrays;

class Cell
{
public:
  Cell();
  Cell(int dimension);

  int dimension() const;

  UUIDArray& relations();
  const UUIDArray& relations() const;

  Cell& appendRelation(const UUID& b);
  Cell& removeRelation(const UUID& b);

protected:
  int Dimension;
  UUIDArray Relations;
private:
};

typedef std::pair<UUID,Cell> UUIDCellPair;

std::string to_json(const UUIDs& uuidSet);
std::string to_json(const Cell& cellRec);
std::string to_json(const UUIDCellPair& entry);

  } // namespace model
} // namespace smtk

#endif // __smtk_model_Cell_h
