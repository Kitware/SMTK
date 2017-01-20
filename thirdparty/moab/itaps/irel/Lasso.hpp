#ifndef LASSO_HPP
#define LASSO_HPP

#include "irel_export.h"
#include "iRel.h"

#include <set>
#include <vector>
#include <cstring>

class AssocPair;

class IREL_EXPORT Lasso
{
public:
  Lasso() : lastErrorType(iBase_SUCCESS)
  {
    lastErrorDescription[0] = '\0';
  }

  virtual ~Lasso();

    //! find a pair equivalent to these ifaces, passed as pointer to
    //! SIDL interface or interface instance
  AssocPair *find_pair(void *iface0, void *iface1,
                       bool *switched = NULL);

  AssocPair *find_pair(iRel_IfaceType type1, iRel_IfaceType type2,
                       bool *switched = NULL);

  void find_pairs(void *iface, std::vector<AssocPair*> &iface_pairs);

  int insert_pair(AssocPair *this_pair);
  int erase_pair(AssocPair *this_pair);

  inline int set_last_error(int, const char*);

  int lastErrorType;
  char lastErrorDescription[120];
private:
  std::set<AssocPair*> assocPairs;
};

static inline Lasso *lasso_instance(iRel_Instance instance)
{
  return reinterpret_cast<Lasso*>(instance);
}
#define LASSOI lasso_instance(instance)

int Lasso::set_last_error(int code, const char* msg)
{
  std::strncpy( lastErrorDescription, msg, sizeof(lastErrorDescription) );
  lastErrorDescription[sizeof(lastErrorDescription)-1] = '\0';
  return (lastErrorType = static_cast<iBase_ErrorType>(code));
}

#endif // #ifndef LASSO_HPP
