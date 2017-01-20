#include "Lasso.hpp"
#include "AssocPair.hpp"

Lasso::~Lasso()
{
  for (std::set<AssocPair*>::iterator i = assocPairs.begin();
       i != assocPairs.end(); ++i)
    delete *i;
}

//! find a pair equivalent to these ifaces, passed as pointer to
//! SIDL interface or interface instance
AssocPair *Lasso::find_pair(void *iface0, void *iface1, bool *switched)
{
  for (std::set<AssocPair*>::iterator i = assocPairs.begin();
       i != assocPairs.end(); ++i) {
    if ((*i)->equivalent(iface0, iface1, switched)) return *i;
  }

  return NULL;
}

//! find a pair with the right types
AssocPair *Lasso::find_pair(iRel_IfaceType type1, iRel_IfaceType type2,
                            bool *switched)
{
  for (std::set<AssocPair*>::iterator i = assocPairs.begin();
       i != assocPairs.end(); ++i) {
    if ((*i)->equivalent(type1, type2, switched)) return *i;
  }

  return NULL;
}

void Lasso::find_pairs(void *iface, std::vector<AssocPair*> &iface_pairs)
{
  for (std::set<AssocPair*>::iterator i = assocPairs.begin();
       i != assocPairs.end(); ++i) {
    if ((*i)->contains(iface)) iface_pairs.push_back(*i);
  }
}

int Lasso::insert_pair(AssocPair *this_pair)
{
  assocPairs.insert(this_pair);
  return iBase_SUCCESS;
}

int Lasso::erase_pair(AssocPair *this_pair)
{
  if (assocPairs.erase(this_pair) == 0)
    return iBase_FAILURE;

  // If the pair was removed, then delete it too
  delete this_pair;
  return iBase_SUCCESS;
}
