#include "EdgeSizeEvaluator.hpp"

#include "RefinerTagManager.hpp"
#include "moab/Interface.hpp"

#include <assert.h>

namespace moab {

/// Construct an evaluator.
EdgeSizeEvaluator::EdgeSizeEvaluator()
{
  this->tag_manager = 0;
}

/// Destruction is virtual so subclasses may clean up after refinement.
EdgeSizeEvaluator::~EdgeSizeEvaluator()
{
}

/**\fn bool EdgeSizeEvaluator::evaluate_edge( \
  *         const double* p0, const void* t0, double* p1, void* t1, const double* p2, const void* t2 )
  *\brief Returns true if the edge \a p0 - \a p2 should be subdivided, false otherwise.
  *
  * The arguments \a p0, \a p1, and \a p2 are all pointers to arrays of 6 doubles each
  * while the arguments \a t0, \a t1, and \a t2 are all pointers to arrays of tag data
  * defined at the corresponding point. While the endpoints \a p0 and \a p2 are
  * immutable, the mid-edge point coordinates \a p1 and tag data \a t1 may be altered by
  * evaluate_edge(). Altered values will be ignored if evaluate_edge() returns false.
  * Be careful to ensure that all calls to evaluate_edge() perform identical modifications
  * given identical input values!
  *
  * A list of tags passed in \a t0, \a t1, and \a t2 is stored in the vertex_tags member.
  * The vertex_size member stores the total length of data associated with each pointer (in bytes).
  * Subclasses may access vertex_tags and vertexSize directly; the refiner uses public methods to
  * populate vertex_tags before evaluate_edge() is called.
  */
}

