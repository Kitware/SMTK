#ifndef __smtk_model_BRepModel_h
#define __smtk_model_BRepModel_h

#include "smtk/util/UUID.h"

#include "smtk/SMTKCoreExports.h" // For SMTKCORE_EXPORT macro.
#include "smtk/model/Item.h"
#include "smtk/model/Cell.h" // For use in specialized template below.

#include <map>

namespace smtk {
  namespace model {

using smtk::util::UUID;

/**\brief A solid model whose entities are referenced individually with instances of T and collectively as sets of type S.
  *
  * Entities are stored as instances of C, regardless of their dimension.
  * The class C must provide a dimension() method.
  *
  * This is templated so we can switch to uint32 values if CGM
  * is unable/unwilling to work with UUIDs.
  */
template<typename UUID, typename UUIDs, typename Cell>
class SMTKCORE_EXPORT BRepModel
{
public:
  typedef BRepModel<UUID,UUIDs,Cell> self_type;
  typedef std::map<UUID,Cell> storage_type;
  typedef typename storage_type::iterator iter_type;

  BRepModel();
  BRepModel(std::map<UUID,Cell>* topology, bool shouldDelete);

  std::map<UUID,Cell>& topology();
  const std::map<UUID,Cell>& topology() const;

  int Dimension(const UUID& ofEntity);
  UUIDs BordantEntities(const UUID& ofEntity, int ofDimension = -2);
  UUIDs BordantEntities(const UUIDs& ofEntities, int ofDimension = -2);
  UUIDs BoundaryEntities(const UUID& ofEntity, int ofDimension = -2);
  UUIDs BoundaryEntities(const UUIDs& ofEntities, int ofDimension = -2);

  UUIDs LowerDimensionalBoundaries(const UUID& ofEntity, int lowerDimension);
  UUIDs HigherDimensionalBordants(const UUID& ofEntity, int higherDimension);
  UUIDs AdjacentEntities(const UUID&ofEntity, int ofDimension);

  UUIDs Entities(int ofDimension);

  iter_type InsertCellOfDimension(int dim);
  iter_type InsertCell(Cell& cell);
  iter_type SetCellOfDimension(const UUID& uid, int dim);
  iter_type SetCell(const UUID& uid, Cell& cell);

  UUID AddCellOfDimension(int dim);
  UUID AddCell(Cell& cell);
  UUID AddCellOfDimensionWithUUID(const UUID& uid, int dim);
  UUID AddCellWithUUID(const UUID& uid, Cell& cell);
};

  } // model namespace
} // smtk namespace

// -----
// Now specialize the template above to use UUIDs as unique identifiers.
#include "smtk/model/BRepModel.txx"
// -----

#endif // __smtk_model_BRepModel_h
