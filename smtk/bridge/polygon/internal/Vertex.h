#ifndef __smtk_bridge_polygon_internal_Vertex_h
#define __smtk_bridge_polygon_internal_Vertex_h

#include "smtk/SharedFromThis.h"
#include "smtk/bridge/polygon/internal/Entity.h"

namespace smtk {
  namespace bridge {
    namespace polygon {
      namespace internal {

/**\brief A base class for all internal vertex storage.
  *
  * Every vertex stores a pointer to its parent and its UUID.
  * This class uses smtkEnableSharedPtr so that all entities may be
  * managed via one pool of shared pointers.
  */
class vertex : public entity
{
public:
  smtkTypeMacro(vertex);
  smtkCreateMacro(vertex);
  smtkEnableSharedFromThis(entity);

protected:
  vertex();
  std::list<edge> m_edges; // CCW list of incident edges
}

#endif // __smtk_bridge_polygon_internal_Vertex_h
