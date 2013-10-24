#include "smtk/model/Cell.h"

#include <algorithm>
#include <set>
#include <map>
#include <vector>

#include <sstream>

//#include <boost/variant.hpp>

using namespace std;

namespace smtk {
  namespace model {

Cell::Cell()
  : Dimension(-1)
{
}

Cell::Cell(int dimension)
  : Dimension(dimension)
{
}

int Cell::dimension() const
{
  return this->Dimension;
}

UUIDArray& Cell::relations()
{
  return this->Relations;
}
const UUIDArray& Cell::relations() const
{
  return this->Relations;
}

Cell& Cell::appendRelation(const UUID& b)
{
  this->Relations.push_back(b);
  return *this;
}

Cell& Cell::removeRelation(const UUID& b)
{
  UUIDArray& arr(this->Relations);
  unsigned size = arr.size();
  unsigned curr;
  for (curr = 0; curr < size; ++curr)
    {
    if (arr[curr] == b)
      {
      arr.erase(arr.begin() + curr);
      --curr;
      --size;
      }
    }
  return *this;
}

std::string to_json(const UUIDs& uuidSet)
{
  std::ostringstream buff;
  buff << "[";
  for (UUIDs::iterator it = uuidSet.begin(); it != uuidSet.end(); ++it)
    {
    if (it != uuidSet.begin())
      {
      buff << ",";
      }
    buff << " \"" << *it << "\"";
    }
  buff << " ]";
  return buff.str();
}

std::string to_json(const Cell& cellRec)
{
  std::ostringstream buff;
  buff
    << "{\"d\":" << cellRec.dimension()
    << ", \"r\":[";
  for (UUIDArray::const_iterator ai = cellRec.relations().begin(); ai != cellRec.relations().end(); ++ai)
    {
    buff << (ai == cellRec.relations().begin() ? "\"" : ",\"") << ai->ToString() << "\"";
    }
  buff << "]}";
  return buff.str();
}

std::string to_json(const UUIDCellPair& entry)
{
  std::ostringstream buff;
  buff << "\"" << entry.first << "\": " << to_json(entry.second);
  return buff.str();
}

  } // namespace model
} //namespace smtk
