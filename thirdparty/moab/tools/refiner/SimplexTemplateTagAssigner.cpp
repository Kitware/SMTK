#include "SimplexTemplateTagAssigner.hpp"

#include "EdgeSizeEvaluator.hpp"
#include "moab/Interface.hpp"
#include "RefinerTagManager.hpp"
#include "SimplexTemplateRefiner.hpp"

#include <vector>

#include <math.h>

namespace moab {

using namespace std;

/// Construct a template tag assigner.
SimplexTemplateTagAssigner::SimplexTemplateTagAssigner( SimplexTemplateRefiner* r )
{
  this->mesh_refiner = r;
  this->tag_manager = 0;
}

/// Empty destructor for good form.
SimplexTemplateTagAssigner::~SimplexTemplateTagAssigner()
{
}

/**\brief Given endpoint coordinates and tag values plus midpoint coordinates, compute midpoint tag values.
  *
  * Normally, this function will be invoked by the EntityRefiner before evaluate_edge is called.
  * However, if evaluate_edge() changes the parametric coordinates of the midpoint,
  * it should call evaluate_tags_at_midpoint() again to update any tag values;
  * that is why this function is a member of EdgeSizeEvaluator and not EntityRefiner.
  *
  * @param[in] c0 Pointer to endpoint 0 coordinates. The parametric coordinates (3) are followed by world coordinates (3).
  * @param[in] t0 Pointer to endpoint 0 tag values.
  * @param[in] cm Pointer to midpoint coordinates. The parametric coordinates (3) are followed by world coordinates (3).
  * @param[out] tm Pointer to midpoint tag values.
  * @param[in] c1 Pointer to endpoint 1 coordinates. The parametric coordinates (3) are followed by world coordinates (3).
  * @param[in] t1 Pointer to endpoint 1 tag values.
  */
void SimplexTemplateTagAssigner::operator () (
  const double* c0, const void* t0, EntityHandle h0,
  const double* cm, void* tm, 
  const double* c1, const void* t1, EntityHandle h1 )
{
  double c0m_squared = 0.;
  double c01_squared = 0.;
  for ( int i = 0; i < 3; ++i )
    {
    double tmp = cm[i] - c0[i];
    c0m_squared += tmp * tmp;
    tmp = c1[i] - c0[i];
    c01_squared += tmp * tmp;
    }
  double lambda = sqrt( c0m_squared / c01_squared );
  double one_minus_lambda = 1. - lambda;

  DataType data_type;
  int tag_size;
  int num_components;
  int num_tags = this->tag_manager->get_number_of_vertex_tags();
  Tag tag_handle;
  int tag_offset;
  for ( int i = 0; i < num_tags; ++i )
    {
    this->tag_manager->get_input_vertex_tag( i, tag_handle, tag_offset );
    this->tag_manager->get_input_mesh()->tag_get_data_type( tag_handle, data_type );
    this->tag_manager->get_input_mesh()->tag_get_bytes( tag_handle, tag_size );
    
    switch ( data_type )
      {
      case MB_TYPE_DOUBLE:
        {
        num_components = tag_size / sizeof( double );
        double* t0i = (double*) ( (char*)t0 + tag_offset );
        double* tmi = (double*) ( (char*)tm + tag_offset );
        double* t1i = (double*) ( (char*)t1 + tag_offset );
        for ( int j = 0; j < num_components; ++ j )
          tmi[j] = one_minus_lambda * t0i[j] + lambda * t1i[j];
        }
        break;
      default:
        memcpy( (char*)tm + tag_offset, (char*)( h0 < h1 ? t0 : t1 ) + tag_offset, tag_size );
        break;
      }
    }
}

void SimplexTemplateTagAssigner::operator () ( const void* t0,
                                                 const void* t1,
                                                 const void* t2,
                                                 void* tp )
{
  (void)t0;
  (void)t1;
  (void)t2;
  (void)tp;
}

void SimplexTemplateTagAssigner::set_tag_manager( RefinerTagManager* tmgr )
{
  this->tag_manager = tmgr;
}

} // namespace moab
