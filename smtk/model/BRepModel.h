#ifndef __smtk_model_BRepModel_h
#define __smtk_model_BRepModel_h

#include "smtk/util/UUID.h"

#include "smtk/SMTKCoreExports.h" // For SMTKCORE_EXPORT macro.
#include "smtk/model/Link.h"

#include <map>

namespace smtk {
  namespace model {

typedef std::map<smtk::util::UUID,Link> UUIDsToLinks;
typedef UUIDsToLinks::iterator UUIDWithLink;

/**\brief A solid model whose entities are referenced individually with instances of T and collectively as sets of type S.
  *
  * Entities are stored as instances of C, regardless of their dimension.
  * The class C must provide a dimension() method.
  *
  * This is templated so we can switch to uint32 values if CGM
  * is unable/unwilling to work with UUIDs.
  */
class SMTKCORE_EXPORT BRepModel
{
public:
  typedef std::map<smtk::util::UUID,Link> storage_type;
  typedef storage_type::iterator iter_type;

  BRepModel();
  BRepModel(std::map<smtk::util::UUID,Link>* topology, bool shouldDelete);
  ~BRepModel();

  std::map<smtk::util::UUID,Link>& topology();
  const std::map<smtk::util::UUID,Link>& topology() const;

  int Type(const smtk::util::UUID& ofEntity);
  int Dimension(const smtk::util::UUID& ofEntity);

  const Link* FindLink(const smtk::util::UUID& uid) const;
  Link* FindLink(const smtk::util::UUID& uid);

  UUIDs BordantEntities(const smtk::util::UUID& ofEntity, int ofDimension = -2);
  UUIDs BordantEntities(const UUIDs& ofEntities, int ofDimension = -2);
  UUIDs BoundaryEntities(const smtk::util::UUID& ofEntity, int ofDimension = -2);
  UUIDs BoundaryEntities(const UUIDs& ofEntities, int ofDimension = -2);

  UUIDs LowerDimensionalBoundaries(const smtk::util::UUID& ofEntity, int lowerDimension);
  UUIDs HigherDimensionalBordants(const smtk::util::UUID& ofEntity, int higherDimension);
  UUIDs AdjacentEntities(const smtk::util::UUID& ofEntity, int ofDimension);

  UUIDs Entities(int ofDimension);

  iter_type InsertLinkOfTypeAndDimension(int entityFlags, int dim);
  iter_type InsertLink(Link& cell);
  iter_type SetLinkOfTypeAndDimension(const smtk::util::UUID& uid, int entityFlags, int dim);
  iter_type SetLink(const smtk::util::UUID& uid, Link& cell);

  smtk::util::UUID AddLinkOfTypeAndDimension(int entityFlags, int dim);
  smtk::util::UUID AddLink(Link& cell);
  smtk::util::UUID AddLinkOfTypeAndDimensionWithUUID(const smtk::util::UUID& uid, int entityFlags, int dim);
  smtk::util::UUID AddLinkWithUUID(const smtk::util::UUID& uid, Link& cell);

  /// Shortcuts for inserting cells with default entity flags.
  //@{
  iter_type InsertCellOfDimension(int dim);
  iter_type SetCellOfDimension(const smtk::util::UUID& uid, int dim);
  smtk::util::UUID AddCellOfDimension(int dim);
  smtk::util::UUID AddCellOfDimensionWithUUID(const smtk::util::UUID& uid, int dim);
  //@}

  void InsertLinkReferences(const UUIDWithLink& c);
  void RemoveLinkReferences(const UUIDWithLink& c);
  void SetDeleteStorage(bool d);
protected:
  UUIDsToLinks* Topology;
  bool DeleteStorage;
};

  } // model namespace
} // smtk namespace

#endif // __smtk_model_BRepModel_h
