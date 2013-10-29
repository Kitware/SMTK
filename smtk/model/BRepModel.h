#ifndef __smtk_model_BRepModel_h
#define __smtk_model_BRepModel_h

#include "smtk/util/UUID.h"

#include "smtk/SMTKCoreExports.h" // For SMTKCORE_EXPORT macro.
#include "smtk/model/Item.h"
#include "smtk/model/Link.h" // For use in specialized template below.

#include <map>

namespace smtk {
  namespace model {

/**\brief A solid model whose entities are referenced individually with instances of T and collectively as sets of type S.
  *
  * Entities are stored as instances of C, regardless of their dimension.
  * The class C must provide a dimension() method.
  *
  * This is templated so we can switch to uint32 values if CGM
  * is unable/unwilling to work with UUIDs.
  */
template<typename UUID, typename UUIDs, typename Link>
class SMTKCORE_EXPORT BRepModel
{
public:
  typedef BRepModel<UUID,UUIDs,Link> self_type;
  typedef std::map<UUID,Link> storage_type;
  typedef typename storage_type::iterator iter_type;

  BRepModel();
  BRepModel(std::map<UUID,Link>* topology, bool shouldDelete);

  std::map<UUID,Link>& topology();
  const std::map<UUID,Link>& topology() const;

  /**int Type(const UUID& ofEntity); NOT YET IN INITIALIZETION**/
  int Dimension(const UUID& ofEntity);
  UUIDs BordantEntities(const UUID& ofEntity, int ofDimension = -2);
  UUIDs BordantEntities(const UUIDs& ofEntities, int ofDimension = -2);
  UUIDs BoundaryEntities(const UUID& ofEntity, int ofDimension = -2);
  UUIDs BoundaryEntities(const UUIDs& ofEntities, int ofDimension = -2);

  UUIDs LowerDimensionalBoundaries(const UUID& ofEntity, int lowerDimension);
  UUIDs HigherDimensionalBordants(const UUID& ofEntity, int higherDimension);
  UUIDs AdjacentEntities(const UUID&ofEntity, int ofDimension);

  UUIDs Entities(int ofDimension);

  iter_type InsertLinkOfTypeAndDimension(int entityFlags, int dim);
  iter_type InsertLink(Link& cell);
  iter_type SetLinkOfTypeAndDimension(const UUID& uid, int entityFlags, int dim);
  iter_type SetLink(const UUID& uid, Link& cell);

  UUID AddLinkOfTypeAndDimension(int entityFlags, int dim);
  UUID AddLink(Link& cell);
  UUID AddLinkOfTypeAndDimensionWithUUID(const UUID& uid, int entityFlags, int dim);
  UUID AddLinkWithUUID(const UUID& uid, Link& cell);

  /// Shortcuts for inserting cells with default entity flags.
  //@{
  iter_type InsertCellOfDimension(int dim);
  iter_type SetCellOfDimension(const UUID& uid, int dim);
  UUID AddCellOfDimension(int dim);
  UUID AddCellOfDimensionWithUUID(const UUID& uid, int dim);
  //@}
};

  } // model namespace
} // smtk namespace

// -----
// Now specialize the template above to use UUIDs as unique identifiers.
#include "smtk/model/BRepModel.txx"
// -----

#endif // __smtk_model_BRepModel_h
