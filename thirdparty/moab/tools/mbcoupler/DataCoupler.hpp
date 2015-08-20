/** 
 * \class moab::DataCoupler
 *
 * \brief This class couples data between meshes.
 *
 * The coupler interpolates solution data at a set of points.  Data being interpolated resides on a "source"
 * mesh, in a tag or in vertex coords.  Applications calling this coupler send in entities, and receive back 
 * data interpolated at those points.  Entities in the source mesh containing those points do not have to reside 
 * on the same processor.
 *
 * To use, an application should:
 * - instantiate this DataCoupler by calling the constructor collectively on all processors in the communicator
 * - call locate_points, which locates the points to be interpolated and (optionally) caches the results in 
 *   this class and SpatialLocator
 * - call interpolate, which does the interpolation
 *
 * Multiple interpolations (of multiple tags, or element-average vs. true interpolation) can be done after 
 * locating the points.
 *
 * SpatialLocator is used for the spatial location portion of this work.
 *
 * This class is a next-generation implementation of Coupler.
 */
#ifndef DATACOUPLER_HPP
#define DATACOUPLER_HPP

#include "moab/Range.hpp"
#include "moab/Interface.hpp"

#include <sstream>

namespace moab {

class ParallelComm;
class SpatialLocator;

class DataCoupler
{
public:

  enum Method {CONSTANT, LINEAR_FE, QUADRATIC_FE, SPECTRAL};

  enum IntegType {VOLUME};

    /* Constructor
     * Constructor, which also optionally initializes the coupler
     * \param source_ents Elements in the source mesh
     * \param coupler_id Id of this coupler, should be the same over all procs
     * \param pc ParallelComm object to be used with this coupler, representing the union
     *    of processors containing source and target meshes
     * \param init_locator If true, initializes a spatial locator inside the constructor
     * \param dim Dimension of entities to be coupled; if -1, get from source_elems
     */
  DataCoupler(Interface *impl,
              Range &source_ents,
              int coupler_id,
              ParallelComm *pc = NULL,
              bool init_locator = true,
              int dim = -1);

    /* Destructor
     */
  virtual ~DataCoupler();

    /* \brief Locate points on the source mesh
     * This is a pass-through function to SpatialLocator::locate_points
     * \param xyz Point locations (interleaved) being located
     * \param num_points Number of points in xyz
     * \param rel_iter_tol Relative tolerance for non-linear iteration
     * \param abs_iter_tol Relative tolerance for non-linear iteration, usually 10^-10 or so
     * \param inside_tol Tolerance of is_inside evaluation, usually 10^-6 or so
     * \param loc_results Tuple list containing the results; two types of results are possible,
     *      controlled by value of store_local parameter:
     *      store_local = true: each tuple T[j] consists of (p, i), p = proc with src element containing
     *        point j (or 0 if serial), i = index in stored list (in SpatialLocator) on src proc
     *      store_local = false: each tuple T[j] consists of (p, ht, hs, pr[3]), where ht = target mesh entity
     *        handle, hs = source mesh entity containing point (0 if not found), pr = parameters in hs
     * \param store_local If true, stores the located points on SpatialLocator
     */
  ErrorCode locate_points(double *xyz, int num_points,
                          const double rel_iter_tol = 1.0e-10, const double abs_iter_tol = 1.0e-10,
                          const double inside_tol = 1.0e-6);

    /* \brief Locate points on the source mesh
     * This is a pass-through function to SpatialLocator::locate_points
     * \param ents Target entities being located
     * \param rel_iter_tol Relative tolerance for non-linear iteration
     * \param abs_iter_tol Relative tolerance for non-linear iteration, usually 10^-10 or so
     * \param inside_tol Tolerance of is_inside evaluation, usually 10^-6 or so
     * \param loc_results Tuple list containing the results; two types of results are possible,
     *      controlled by value of store_local parameter:
     *      store_local = true: each tuple T[j] consists of (p, i), p = proc with src element containing
     *        point j (or 0 if serial), i = index in stored list (in SpatialLocator) on src proc
     *      store_local = false: each tuple T[j] consists of (p, ht, hs, pr[3]), where ht = target mesh entity
     *        handle, hs = source mesh entity containing point (0 if not found), pr = parameters in hs
     * \param store_local If true, stores the located points on SpatialLocator
     */
  ErrorCode locate_points(Range &ents,
                          const double rel_iter_tol = 1.0e-10, const double abs_iter_tol = 1.0e-10,
                          const double inside_tol = 1.0e-6);

    /* \brief Interpolate data from the source mesh onto points
     * All entities/points or, if tuple_list is input, only those points
     * are interpolated from the source mesh.  Application should
     * allocate enough memory in interp_vals to hold interpolation results.
     * 
     * If normalization is requested, technique used depends on the coupling
     * method.
     *
     * \param method Interpolation/normalization method
     * \param tag Tag on source mesh holding data to be interpolated
     * \param interp_vals Memory holding interpolated data; if NULL, data is written to same tag on target ents
     * \param point_indices If non-NULL, a set of indices of points input to 
     *        locate_points at which to interpolate; if NULL, interpolates at all points
     *        input to locate_points
     * \param normalize If true, normalization is done according to method
     */
  ErrorCode interpolate(/*DataCoupler::Method*/ int method,
                        Tag tag,
                        double *interp_vals = NULL,
                        std::vector<int> *point_indices = NULL,
                        bool normalize = true);

    /* \brief Interpolate data from the source mesh onto points
     * All entities/points or, if tuple_list is input, only those points
     * are interpolated from the source mesh.  Application should
     * allocate enough memory in interp_vals to hold interpolation results.
     * 
     * If normalization is requested, technique used depends on the coupling
     * method.
     *
     * \param method Interpolation/normalization method
     * \param tag_name Tag name on source mesh holding data to be interpolated
     * \param interp_vals Memory holding interpolated data; if NULL, data is written to same tag on target ents
     * \param point_indices If non-NULL, a set of indices of points input to 
     *        locate_points at which to interpolate; if NULL, interpolates at all points
     *        input to locate_points
     * \param normalize If true, normalization is done according to method
     */
  ErrorCode interpolate(/*DataCoupler::Method*/ int method,
                        const std::string &tag_name,
                        double *interp_vals = NULL,
                        std::vector<int> *point_indices = NULL,
                        bool normalize = true);

    /* \brief Interpolate data from multiple tags
     * All entities/points or, if tuple_list is input, only those points
     * are interpolated from the source mesh.  Application should
     * allocate enough memory in interp_vals to hold interpolation results.
     * 
     * In this variant, multiple tags, possibly with multiple interpolation
     * methods, are specified.  Sum of values in points_per_method should be
     * the number of points in tl or, if NULL, targetPts.
     *
     * If normalization is requested, technique used depends on the coupling
     * method.
     *
     * \param methods Vector of Interpolation/normalization methods
     * \param tag_names Names of tag being interpolated for each method
     * \param points_per_method Number of points for each method
     * \param num_methods Length of vectors in previous 3 arguments
     * \param interp_vals Memory holding interpolated data; if NULL, data is written to same tag on target ents
     * \param point_indices If non-NULL, a set of indices of points input to 
     *        locate_points at which to interpolate; if NULL, interpolates at all points
     *        input to locate_points
     * \param normalize If true, normalization is done according to method
     */
  ErrorCode interpolate(/*DataCoupler::Method*/ int *methods,
                        const std::string *tag_names,
                        int *points_per_method,
                        int num_methods,
                        double *interp_vals = NULL,
                        std::vector<int> *point_indices = NULL,
                        bool normalize = true);

    /* \brief Interpolate data from multiple tags
     * All entities/points or, if tuple_list is input, only those points
     * are interpolated from the source mesh.  Application should
     * allocate enough memory in interp_vals to hold interpolation results.
     * 
     * In this variant, multiple tags, possibly with multiple interpolation
     * methods, are specified.  Sum of values in points_per_method should be
     * the number of points in tl or, if NULL, targetPts.
     *
     * If normalization is requested, technique used depends on the coupling
     * method.
     *
     * \param methods Vector of Interpolation/normalization methods
     * \param tag_names Names of tag being interpolated for each method
     * \param points_per_method Number of points for each method
     * \param num_methods Length of vectors in previous 3 arguments
     * \param interp_vals Memory holding interpolated data; if NULL, data is written to same tag on target ents
     * \param point_indices If non-NULL, a set of indices of points input to 
     *        locate_points at which to interpolate; if NULL, interpolates at all points
     *        input to locate_points
     * \param normalize If true, normalization is done according to method
     */
  ErrorCode interpolate(/*DataCoupler::Method*/ int *methods,
                        Tag *tag_names,
                        int *points_per_method,
                        int num_methods,
                        double *interp_vals = NULL,
                        std::vector<int> *point_indices = NULL,
                        bool normalize = true);

    /* Get functions */
  inline SpatialLocator *spatial_locator() { return myLocator; }
  inline int my_id() const { return myId; }
  inline const Range &target_ents() const { return targetEnts; }
  inline Range &target_ents() { return targetEnts; }
  inline int get_dim() const { return myDim; }

private:

    /* \brief MOAB instance
     */
  Interface *mbImpl;

    /* \brief ParallelComm object for this coupler
     */
  ParallelComm *myPcomm;

    /* \brief SpatialLocator for local mesh
     */
  SpatialLocator *myLocator;

    /* \brief Id of this coupler
     */
  int myId;

    /* \brief Range of target entities
     */
  Range targetEnts;

  // Entity dimension
  int myDim;
};

inline ErrorCode DataCoupler::interpolate(/*DataCoupler::Method*/ int method,
                                          Tag tag,
                                          double *interp_vals,
                                          std::vector<int> *point_indices,
                                          bool normalize)
{
  // No point indices input,
  int num_pts = (point_indices ? point_indices->size() : targetEnts.size());
  return interpolate(&method, &tag, &num_pts, 1,
                     interp_vals, point_indices, normalize);
}

} // namespace moab

#endif
