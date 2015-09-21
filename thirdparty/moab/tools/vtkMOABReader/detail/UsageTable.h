#ifndef __smoab_detail_UsageTable_h
#define __smoab_detail_UsageTable_h

#include <set>
#include <vector>

namespace smoab { namespace detail {

namespace internal
{

struct KeyType
{
  moab::EntityHandle Handle;
  int ParentId;

  KeyType(const moab::EntityHandle& h, int p):
    Handle(h),
    ParentId(p)
    {}

  bool operator<(const KeyType& other) const
    {
    return ((this->Handle < other.Handle) ||
           ((this->Handle < other.Handle)&&(this->ParentId<other.ParentId)));
    }
};

class key_inserter
{

protected:
  Range* container;

public:
  //constructor
  explicit key_inserter(Range& x) : container(&x) {}
  key_inserter&
  operator=(const KeyType& value)
  {
    container->insert(value.Handle);
    return *this;
  }

  key_inserter& operator*() { return *this; }
  key_inserter& operator++() { return *this; }
  key_inserter& operator++(int) { return *this; }

  typedef moab::EntityHandle            value_type;
  typedef moab::EntityID                difference_type;
  typedef std::output_iterator_tag      iterator_category;
  typedef moab::EntityHandle*           pointer;
  typedef moab::EntityHandle&           reference;
};

}

//will store a collection of entities that are used only once with a given
//owner id
class UsageTable
{
public:
  void incrementUsage(const std::vector<smoab::EntityHandle>& entities,
                      const std::vector<int>& usageId);

  smoab::Range multipleUsage() const;
  smoab::Range singleUsage() const;

private:
  std::vector<smoab::EntityHandle> MultipleUsageIds;
  std::set<internal::KeyType> SingleUsageIds;
};


//----------------------------------------------------------------------------
void UsageTable::incrementUsage(
                        const std::vector<smoab::EntityHandle>& entities,
                        const std::vector<int>& usageId)
{
  typedef std::vector<smoab::EntityHandle>::const_iterator iterator;
  typedef std::vector<int>::const_iterator uiterator;

  uiterator usage = usageId.begin();
  for(iterator i=entities.begin();
      i != entities.end();
      ++i, ++usage)
    {
    internal::KeyType key(*i,*usage);
    if(this->SingleUsageIds.find(key) != this->SingleUsageIds.end())
      {
      this->SingleUsageIds.erase(key);
      this->MultipleUsageIds.push_back(*i);
      }
    else
      {
      this->SingleUsageIds.insert(key);
      }
    }
}

//----------------------------------------------------------------------------
smoab::Range UsageTable::multipleUsage() const
{
  smoab::Range multiples;
  std::copy(this->MultipleUsageIds.rbegin(),
            this->MultipleUsageIds.rend(),
            moab::range_inserter(multiples));
  return multiples;
}

//----------------------------------------------------------------------------
smoab::Range UsageTable::singleUsage() const
{
  smoab::Range single;
  std::copy(this->SingleUsageIds.rbegin(),
            this->SingleUsageIds.rend(),
            internal::key_inserter(single));
  return single;
}

} }
#endif
