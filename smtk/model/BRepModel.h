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

  int type(const smtk::util::UUID& ofEntity);
  int dimension(const smtk::util::UUID& ofEntity);

  const Link* findLink(const smtk::util::UUID& uid) const;
  Link* findLink(const smtk::util::UUID& uid);

  UUIDs bordantEntities(const smtk::util::UUID& ofEntity, int ofDimension = -2);
  UUIDs bordantEntities(const UUIDs& ofEntities, int ofDimension = -2);
  UUIDs boundaryEntities(const smtk::util::UUID& ofEntity, int ofDimension = -2);
  UUIDs boundaryEntities(const UUIDs& ofEntities, int ofDimension = -2);

  UUIDs lowerDimensionalBoundaries(const smtk::util::UUID& ofEntity, int lowerDimension);
  UUIDs higherDimensionalBordants(const smtk::util::UUID& ofEntity, int higherDimension);
  UUIDs adjacentEntities(const smtk::util::UUID& ofEntity, int ofDimension);

  UUIDs entities(int ofDimension);

  iter_type insertLinkOfTypeAndDimension(int entityFlags, int dim);
  iter_type insertLink(Link& cell);
  iter_type setLinkOfTypeAndDimension(const smtk::util::UUID& uid, int entityFlags, int dim);
  iter_type setLink(const smtk::util::UUID& uid, Link& cell);

  smtk::util::UUID addLinkOfTypeAndDimension(int entityFlags, int dim);
  smtk::util::UUID addLink(Link& cell);
  smtk::util::UUID addLinkOfTypeAndDimensionWithUUID(const smtk::util::UUID& uid, int entityFlags, int dim);
  smtk::util::UUID addLinkWithUUID(const smtk::util::UUID& uid, Link& cell);

  iter_type insertCellOfDimension(int dim);
  iter_type setCellOfDimension(const smtk::util::UUID& uid, int dim);
  smtk::util::UUID addCellOfDimension(int dim);
  smtk::util::UUID addCellOfDimensionWithUUID(const smtk::util::UUID& uid, int dim);

  void insertLinkReferences(const UUIDWithLink& c);
  void removeLinkReferences(const UUIDWithLink& c);
  void setDeleteStorage(bool d);

protected:
  UUIDsToLinks* m_topology;
  bool m_deleteStorage;
};

  } // model namespace
} // smtk namespace

#endif // __smtk_model_BRepModel_h
