#ifndef __smoab_detail_ContinousCellInfo_h
#define __smoab_detail_ContinousCellInfo_h


namespace smoab { namespace detail {

struct ContinousCellInfo
{
  int type;
  int numVerts;
  int numUnusedVerts;
  int numCells;

};

} } //namespace smoab::detail

#endif
