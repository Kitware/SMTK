#include "DataCoupler.hpp"
#include "moab/Tree.hpp"
#include "moab/TupleList.hpp"
#include "moab/SpatialLocator.hpp"
#include "moab/ElemEvaluator.hpp"
#include "moab/Error.hpp"

#ifdef MOAB_HAVE_MPI
#include "moab/ParallelComm.hpp"
#endif

#include "iostream"
#include <algorithm>
#include <sstream>

#include <stdio.h>
#include "assert.h"

namespace moab {

DataCoupler::DataCoupler(Interface *impl,
                         Range &source_ents,
                         int coupler_id,
                         ParallelComm *pc,
                         bool init_locator,
                         int dim)
  : mbImpl(impl), myPcomm(pc), myId(coupler_id), myDim(dim)
{
  assert(NULL != mbImpl && (myPcomm || !source_ents.empty()));

  // Now initialize the tree
  if (init_locator) {
    myLocator = new SpatialLocator(mbImpl, source_ents);
    myLocator->elem_eval(new ElemEvaluator(mbImpl));

    // Initialize element evaluator with the default for the entity types in source_ents;
    // can be replaced later by application if desired
    if (!source_ents.empty()) {
      Range::pair_iterator pit = source_ents.pair_begin();
      EntityType last_type = MBMAXTYPE;
      for (; pit != source_ents.pair_end(); pit++) {
        EntityType this_type = mbImpl->type_from_handle(pit->first);
        if (last_type == this_type)
          continue;
        ErrorCode rval = myLocator->elem_eval()->set_eval_set(pit->first);
        if (MB_SUCCESS != rval)
          throw(rval);
        last_type = this_type;
      }
    }
  }
  
  if (-1 == dim && !source_ents.empty()) 
    dim = mbImpl->dimension_from_handle(*source_ents.rbegin());
}

/* Destructor
 */
DataCoupler::~DataCoupler()
{
  delete myLocator;
}

ErrorCode DataCoupler::locate_points(Range &targ_ents,
                                     const double rel_iter_tol, const double abs_iter_tol,
                                     const double inside_tol)
{
  targetEnts = targ_ents;

#ifdef MOAB_HAVE_MPI
  if (myPcomm && myPcomm->size() > 1)
    return myLocator->par_locate_points(myPcomm, targ_ents, rel_iter_tol, abs_iter_tol, inside_tol);
#endif

  return myLocator->locate_points(targ_ents, rel_iter_tol, abs_iter_tol, inside_tol);
}

ErrorCode DataCoupler::locate_points(double *xyz, int num_points,
                                     const double rel_iter_tol, const double abs_iter_tol,
                                     const double inside_tol)
{
#ifdef MOAB_HAVE_MPI
  if (myPcomm && myPcomm->size() > 1)
    return myLocator->par_locate_points(myPcomm, xyz, num_points, rel_iter_tol, abs_iter_tol, inside_tol);
#endif

  return myLocator->locate_points(xyz, num_points, rel_iter_tol, abs_iter_tol, inside_tol);
}

ErrorCode DataCoupler::interpolate(/*DataCoupler::Method*/ int method,
                                   const std::string &interp_tag,
                                   double *interp_vals,
                                   std::vector<int> *point_indices,
                                   bool normalize)
{
  // Tag name input, translate to tag handle and pass down the chain

  Tag tag;
  ErrorCode result = mbImpl->tag_get_handle(interp_tag.c_str(), tag);MB_CHK_SET_ERR(result, "Failed to get handle for interpolation tag \"" << interp_tag << "\"");

  return interpolate(method, tag, interp_vals, point_indices, normalize);
}

ErrorCode DataCoupler::interpolate(/*DataCoupler::Method*/ int */* methods */,
                                   Tag *tags,
                                   int *points_per_method,
                                   int num_methods,
                                   double *interp_vals,
                                   std::vector<int> *point_indices,
                                   bool /*normalize*/)
{
  // Lowest-level interpolate function, does actual interpolation using calls to ElemEvaluator

  ErrorCode result = MB_SUCCESS;

  unsigned int pts_total = 0;
  for (int i = 0; i < num_methods; i++)
    pts_total += (points_per_method ? points_per_method[i] : targetEnts.size());

  unsigned int num_indices = (point_indices ? point_indices->size() : targetEnts.size());
  // # points and indices should be identical
  if (pts_total != num_indices)
    return MB_FAILURE;

  // Since each tuple contains one interpolated tag, if we're interpolating multiple tags, every tuple
  // needs to be able to store up to max tag size
  int max_tsize = -1;
  for (int i = 0; i < num_methods; i++) {
    int tmp_tsize;
    result = mbImpl->tag_get_length(tags[i], tmp_tsize);
    if (MB_SUCCESS != result) return MB_FAILURE;
    max_tsize = std::max(max_tsize, tmp_tsize);
  }

  if (myPcomm && myPcomm->size() > 1) {
    // Build up the tuple list to distribute from my target points; assume that
    // all procs use same method/tag input
    TupleList TLob; // TLob structure: (pto_i, ridx_i, lidx_i, meth_tagidx_i)
    TLob.initialize(4, 0, 0, 0, num_indices);
    int tn = 0; // The tuple number we're currently on
    TLob.enableWriteAccess();
    for (int m = 0; m < num_methods; m++) {
      int num_points = (points_per_method ? points_per_method[m] : targetEnts.size());
      for (int j = 0; j < num_points; j++) {
        int idx = (point_indices ? (*point_indices)[j] : j); // The index in my targetEnts for this interpolation point

        // Remote proc/idx from myLocator->parLocTable
        TLob.vi_wr[4*tn]   = myLocator->par_loc_table().vi_rd[2*idx]; // proc
        TLob.vi_wr[4*tn + 1] = myLocator->par_loc_table().vi_rd[2*idx + 1]; // Remote idx

        // Local entity index, tag/method index from my data
        TLob.vi_wr[4*tn + 2] = idx;
        TLob.vi_wr[4*tn + 3] = m;
        TLob.inc_n();
        tn++;
      }
    }

    // Scatter/gather interpolation points
    myPcomm->proc_config().crystal_router()->gs_transfer(1, TLob, 0);

    // Perform interpolation on local source mesh; put results into TLinterp
    TupleList TLinterp; // TLinterp structure: (pto_i, ridx_i, vals[max_tsize]_d)
    TLinterp.initialize(2, 0, 0, max_tsize, TLob.get_n());
    TLinterp.set_n(TLob.get_n()); // Set the size right away
    TLinterp.enableWriteAccess();

    for (unsigned int i = 0; i < TLob.get_n(); i++) {
      int lidx = TLob.vi_rd[4*i + 1]; // Index into myLocator's local table
      // Method and tag indexed with same index
      ///*Method*/ int method = methods[TLob.vi_rd[4*i + 3]];
      Tag tag = tags[TLob.vi_rd[4*i + 3]];
      // Copy proc/remote index from TLob
      TLinterp.vi_wr[2*i] = TLob.vi_rd[4*i];
      TLinterp.vi_wr[2*i + 1] = TLob.vi_rd[4*i + 2];

      // Set up the evaluator for the tag and entity, then interpolate, putting result in TLinterp
      myLocator->elem_eval()->set_tag_handle(tag);
      myLocator->elem_eval()->set_ent_handle(myLocator->loc_table().vul_rd[lidx]);
      result = myLocator->elem_eval()->eval(myLocator->loc_table().vr_rd + 3*lidx, TLinterp.vr_rd + i*max_tsize);
      if (MB_SUCCESS != result)
        return result;
    }

    // Scatter/gather interpolation data
    myPcomm->proc_config().crystal_router()->gs_transfer(1, TLinterp, 0);

    // Copy the interpolated field as a unit
    std::copy(TLinterp.vr_rd, TLinterp.vr_rd + TLinterp.get_n()*max_tsize, interp_vals);
  }
  else {
    // If called serially, interpolate directly from local locations/entities,
    // into either interp_vals or by setting data directly on tags on those entities
    std::vector<double> tmp_vals;
    std::vector<EntityHandle> tmp_ents;
    double *tmp_dbl = interp_vals;
    for (int i = 0; i < num_methods; i++) {
      int num_points = (points_per_method ? points_per_method[i] : targetEnts.size());

      // Interpolated data is tsize long, which is either max size (if data passed back to caller in interp_vals)
      // or tag size (if data will be set on entities, in which case it shouldn't have padding)
      // set sizes here, inside loop over methods, to reduce memory usage
      int tsize = max_tsize, tsize_bytes = 0;
      if (!interp_vals) {
        tmp_vals.resize(num_points*max_tsize);
        tmp_dbl = &tmp_vals[0];
        tmp_ents.resize(num_points);
        result = mbImpl->tag_get_length(tags[i], tsize);
        if (MB_SUCCESS != result)
          return result;
        result = mbImpl->tag_get_bytes(tags[i], tsize_bytes);
        if (MB_SUCCESS != result)
          return result;
      }

      for (int j = 0; j < num_points; j++) {
        int lidx;
        if (point_indices)
          lidx = (*point_indices)[j];
        else
          lidx = j;

        myLocator->elem_eval()->set_tag_handle(tags[i]);
        myLocator->elem_eval()->set_ent_handle(myLocator->loc_table().vul_rd[lidx]);
        if (!interp_vals)
          tmp_ents[j] = targetEnts[lidx]; // Could be performance-sensitive, thus the if test
        result = myLocator->elem_eval()->eval(myLocator->loc_table().vr_rd + 3*lidx, tmp_dbl);
        tmp_dbl += tsize;
        if (MB_SUCCESS != result)
          return result;
      } // for j over points

      if (!interp_vals) {
        // Set tags on tmp_ents; data is already w/o padding, due to tsize setting above
        result = mbImpl->tag_set_data(tags[i], &tmp_ents[0], tmp_ents.size(), &tmp_vals[0]);
        if (MB_SUCCESS != result)
          return result;
      }

    } // for i over methods
  } // if myPcomm

  // Done
  return MB_SUCCESS;
}

} // namespace_moab
