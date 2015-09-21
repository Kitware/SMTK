/** 
 * \class moab::Coupler
 * \author Tim Tautges
 *
 * \brief This class couples data between meshes.
 *
 * The coupler interpolates solution data at a set of points.  Data
 * being interpolated resides on a source mesh, in a tag.
 * Applications calling this coupler send in entities, usually points
 * or vertices, and receive back the tag value interpolated at those
 * points.  Entities in the source mesh containing those points 
 * do not have to reside on the same processor.
 *
 * To use, an application should:
 * - instantiate this coupler by calling the constructor collectively
 *   on all processors in the communicator
 * - call locate_points, which locates the points to be interpolated and
 *   (optionally) caches the results in this object
 * - call interpolate, which does the interpolation
 *
 * Multiple interpolations can be done after locating the points.
 *
 */
#ifndef COUPLER_HPP
#define COUPLER_HPP

#include "moab/Range.hpp"
#include "moab/Interface.hpp"
#include "moab/CartVect.hpp"
#include "moab/TupleList.hpp"

#include <sstream>

namespace moab {

class ParallelComm;

class AdaptiveKDTree;

class TupleList;

class Coupler
{
public:

  enum Method {CONSTANT, LINEAR_FE, QUADRATIC_FE, SPECTRAL};

  enum IntegType {VOLUME};

    /* Constructor
     * Constructor, which also optionally initializes the coupler
     * \param pc ParallelComm object to be used with this coupler, representing the union
     *    of processors containing source and target meshes
     * \param local_elems Local elements in the source mesh
     * \param coupler_id Id of this coupler, should be the same over all procs
     * \param init_tree If true, initializes kdtree inside the constructor
     */
  Coupler(Interface *impl,
            ParallelComm *pc,
            Range &local_elems,
            int coupler_id,
            bool init_tree = true,
            int max_ent_dim = 3);

    /* Destructor
     */
  virtual ~Coupler();

    /* \brief Locate points on the source mesh
     * This function finds the element/processor/natural coordinates for the
     * source mesh element containing each point, optionally storing the results 
     * on the target mesh processor.  Relative tolerance is compared to bounding 
     * box diagonal length.  Tolerance is compared to [-1,1] parametric extent
     * in the reference element.
     * \param xyz Point locations (interleaved) being located
     * \param num_points Number of points in xyz
     * \param rel_eps Relative tolerance for the non-linear iteration inside a given element
     * \param abs_eps Absolute tolerance for the non-linear iteration inside a given element
     * \param tl Tuple list containing the results, with each tuple
     *           consisting of (p, i), p = proc, i = index on that proc
     * \param store_local If true, stores the tuple list in targetPts
     */
  ErrorCode locate_points(double *xyz, unsigned int num_points,
                          double rel_eps = 0.0, 
                          double abs_eps = 0.0,
                          TupleList *tl = NULL,
                          bool store_local = true);

    /* \brief Locate entities on the source mesh
     * This function finds the element/processor/natural coordinates for the
     * source mesh element containing each entity, optionally storing the results 
     * on the target mesh processor.  Location of each target mesh entity passed in
     * is its centroid (for non-vertices, the avg of its vertex positions).  
     * Relative tolerance is compared to bounding 
     * box diagonal length.  Tolerance is compared to [-1,1] parametric extent
     * in the reference element.
     * \param ents Entities being located
     * \param rel_eps Relative tolerance for the non-linear iteration inside a given element
     * \param abs_eps Absolute tolerance for the non-linear iteration inside a given element
     * \param tl Tuple list containing the results, with each tuple
     *           consisting of (p, i), p = proc, i = index on that proc
     * \param store_local If true, stores the tuple list in targetPts
     *
     */
  ErrorCode locate_points(Range &ents,
                          double rel_eps = 0.0,
                          double abs_eps = 0.0,
                          TupleList *tl = NULL,
                          bool store_local = true);

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
     * \param interp_vals Memory holding interpolated data
     * \param tl Tuple list of points to be interpolated, in format used by targetPts
     *    (see documentation for targetPts below); if NULL, all locations
     *    in targetPts are interpolated
     * \param normalize If true, normalization is done according to method
     */
  ErrorCode interpolate(Coupler::Method method,
                        Tag tag,
                        double *interp_vals,
                        TupleList *tl = NULL,
                        bool normalize = true);

    /* \brief Interpolate data from the source mesh onto points
     * All entities/points or, if tuple_list is input, only those points
     * are interpolated from the source mesh.  Application should
     * allocate enough memory in interp_vals to hold interpolation results.
     * 
     * If normalization is requested, technique used depends on the coupling
     * method.
     *
     * \param methods Interpolation/normalization method
     * \param tag_name Name of tag on source mesh holding data to be interpolated
     * \param interp_vals Memory holding interpolated data
     * \param tl Tuple list of points to be interpolated, in format used by targetPts
     *    (see documentation for targetPts below); if NULL, all locations
     *    in targetPts are interpolated
     * \param normalize If true, normalization is done according to method
     */
  ErrorCode interpolate(Coupler::Method method,
                        const std::string &tag_name,
                        double *interp_vals,
                        TupleList *tl = NULL,
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
     * \param interp_vals Memory holding interpolated data
     * \param tl Tuple list of points to be interpolated; if NULL, all locations
     *       stored in this object are interpolated
     * \param normalize If true, normalization is done according to method
     */
  ErrorCode interpolate(Coupler::Method *methods,
                        const std::string *tag_names,
                        int *points_per_method,
                        int num_methods,
                        double *interp_vals,
                        TupleList *tl = NULL,
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
     * \param interp_vals Memory holding interpolated data
     * \param tl Tuple list of points to be interpolated; if NULL, all locations
     *       stored in this object are interpolated
     * \param normalize If true, normalization is done according to method
     */
  ErrorCode interpolate(Coupler::Method *methods,
                        Tag *tag_names,
                        int *points_per_method,
                        int num_methods,
                        double *interp_vals,
                        TupleList *tl = NULL,
                        bool normalize = true);

    /* \brief Normalize a field over an entire mesh
     * A field existing on the vertices of elements of a mesh is integrated
     * over all elements in the mesh.  The integrated value is normalized 
     * and the normalization factor is saved to a new tag
     * on the mesh entity set.
     * 
     * \param root_set Entity Set representing the entire mesh
     * \param norm_tag Tag containing field data to integrate
     * \param integ_type Type of integration to perform
     * \param num_integ_pts The number of Gaussian integration points to use in each dimension
     */
  ErrorCode normalize_mesh( EntityHandle        root_set,
                            const char          *norm_tag,
                            Coupler::IntegType  integ_type,
                            int                 num_integ_pts);

    /* \brief Normalize a field over subsets of entities
     * A field existing on the vertices of elements of a mesh is integrated
     * over subsets of elements identified by the tags and values.  The integrated 
     * values are normalized and the normalization factor is saved to a new tag
     * on the entity sets which contain the elements of a subset.
     * 
     * \param root_set Entity Set from the mesh from which to select subsets
     * \param norm_tag Tag containing field data to integrate
     * \param tag_names Array of tag names used for selecting element subsets
     * \param num_tags Number of tag names
     * \param tag_values Array of tag values passed as strings; the array will be
     *       the same length as that for tag names however some entries may be 
     *       NULL indicating that tag should be matched for existence and not value
     * \param integ_type Type of integration to perform
     * \param num_integ_pts The number of Gaussian integration points to use in each dimension
     */
  ErrorCode normalize_subset( EntityHandle          root_set,
                              const char            *norm_tag,
                              const char            **tag_names,
                              int                   num_tags,
                              const char            **tag_values,
                              Coupler::IntegType    integ_type,
                              int                   num_integ_pts);

    /* \brief Normalize a field over subsets of entities
     * A field existing on the vertices of elements of a mesh is integrated
     * over subsets of elements identified by the tags and values.  The integrated 
     * values are normalized and the normalization factor is saved to a new tag
     * on the entity sets which contain the elements of a subset.
     * 
     * \param root_set Entity Set from the mesh from which to select subsets
     * \param norm_tag Tag containing field data to integrate
     * \param tag_handles Array of tag handles used for selecting element subsets
     * \param num_tags Number of tag handles
     * \param tag_values Array of tag values passed as strings; the array will be
     *       the same length as that for tag handles however some entries may be 
     *       NULL indicating that tag should be matched for existence and not value
     * \param integ_type Type of integration to perform
     * \param num_integ_pts The number of Gaussian integration points to use in each dimension
     */
  ErrorCode normalize_subset( EntityHandle          root_set,
                              const char            *norm_tag,
                              Tag                   *tag_handles,
                              int                   num_tags,
                              const char            **tag_values,
                              Coupler::IntegType    integ_type,
                              int                   num_integ_pts);

    /* \brief Retrieve groups of entities matching tags and values(if present)
     * Retrieve a vector of vectors of entity handles matching the 
     * tags and values.  The entity set passed is used as the search domain.
     * 
     * \param norm_tag Tag containing field data to integrate
     * \param entity_sets Pointer to vector of vectors of entity set handles
     * \param entity_groups Pointer to vector of vectors of entity handles from each entity set
     * \param integ_type Type of integration to perform
     * \param num_integ_pts The number of Gaussian integration points to use in each dimension
     */
  ErrorCode do_normalization( const char                                 *norm_tag,
                              std::vector< std::vector<EntityHandle> >   &entity_sets,
                              std::vector< std::vector<EntityHandle> >   &entity_groups,
                              Coupler::IntegType                         integ_type,
                              int                                        num_integ_pts);

    /* \brief Retrieve groups of entities matching tags and values(if present)
     * Retrieve a vector of vectors of entity handles matching the 
     * tags and values.  The entity set passed is used as the search domain.
     * 
     * \param root_set Set from which to search for matching entities
     * \param tag_names Array of tag names used to select entities
     * \param tag_values Array of tag values used to select entities
     * \param num_tags Number of tag names
     * \param entity_sets Pointer to vector of vectors of entity set handles found in the search
     * \param entity_groups Pointer to vector of vectors of entity handles from each entity set
     */
  ErrorCode get_matching_entities( EntityHandle                             root_set,
                                   const char                               **tag_names,
                                   const char                               **tag_values,
                                   int                                      num_tags,
                                   std::vector< std::vector<EntityHandle> > *entity_sets,
                                   std::vector< std::vector<EntityHandle> > *entity_groups);

    /* \brief Retrieve groups of entities matching tags and values(if present)
     * Retrieve a vector of vectors of entity handles matching the 
     * tags and values.  The entity set passed is used as the search domain.
     * 
     * \param root_set Set from which to search for matching entities
     * \param tag_handles Array of tag handles used to select entities
     * \param tag_values Array of tag values used to select entities
     * \param num_tags Number of tag handles
     * \param entity_sets Pointer to vector of vectors of entity set handles found in the search
     * \param entity_groups Pointer to vector of vectors of entity handles from each entity set
     */
  ErrorCode get_matching_entities( EntityHandle                             root_set,
                                   Tag                                      *tag_handles,
                                   const char                               **tag_values,
                                   int                                      num_tags,
                                   std::vector< std::vector<EntityHandle> > *entity_sets,
                                   std::vector< std::vector<EntityHandle> > *entity_groups);

    /* \brief Return an array of tuples of tag values for each Entity Set
     * A list of n-tuples will be constructed with 1 n-tuple for each Entity Set.
     * The n-tuple will have an component for each tag given.  It is assumed all
     * of the tags are integer tags.
     * 
     * \param ent_sets Array of Entity Set handles to use for retrieving tag data
     * \param num_sets Number of Entity Sets
     * \param tag_names Array of tag names
     * \param num_tags Number of tag names
     * \param tuples The returned tuple_list structure
     */
  ErrorCode create_tuples( Range         &ent_sets,
                           const char    **tag_names, 
                           unsigned int  num_tags,
                           TupleList     **tuples);

    /* \brief Return an array of tuples of tag values for each Entity Set
     * A list of n-tuples will be constructed with 1 n-tuple for each Entity Set.
     * The n-tuple will have an component for each tag given.  It is assumed all
     * of the tags are integer tags.
     * 
     * \param ent_sets Array of Entity Set handles to use for retrieving tag data
     * \param num_sets Number of Entity Sets
     * \param tag_handles Array of tag handles
     * \param num_tags Number of tag handles
     * \param tuples The returned tuple_list structure
     */
  ErrorCode create_tuples( Range         &ent_sets,
                           Tag           *tag_handles,
                           unsigned int  num_tags,
                           TupleList     **tuples);

    /* \brief Consolidate an array of n-tuples lists into one n-tuple list with no duplicates
     * An array of list of n-tuples are consolidated into a single list of n-tuples
     * with all duplicates removed.  Only integer columns in the tuple_list are assumed to 
     * be used.
     *
     * \param all_tuples Array of tuple_lists to consolidate to one
     * \param num_tuples Number of tuple_lists
     * \param unique_tuples The consolidated tuple_list with no duplicates
     */
  ErrorCode consolidate_tuples( TupleList     **all_tuples,
                                unsigned int  num_tuples,
                                TupleList     **unique_tuples);

    /* \brief Calculate integrated field values for groups of entities
     * An integrated field value, as defined by the field function, 
     * is calculated for each group of entities passed in.
     * 
     * \param groups The vector contains vectors of entity handles, each representing a group
     * \param integ_vals The integrated field values for each group
     * \param norm_tag The tag name of the vertex-based field to be integrated
     * \param num_integ_pts The number of Gaussian integration points to use in each dimension
     * \param integ_type Type of integration to perform
     */
  ErrorCode get_group_integ_vals( std::vector<std::vector<EntityHandle> > &groups,
                                  std::vector<double> &integ_vals, 
                                  const char *norm_tag,
                                  int num_integ_pts,
                                  Coupler::IntegType integ_type);

    /* \brief Apply a normalization factor to group of entities
     * Multiply a normalization factor with the value of norm_tag for each vertex
     * of each entity in a group.  Save the value back to norm_tag on each vertex.
     *
     * \param entity_sets The vector contains vectors of entity set handles, each containing the members of a group
     * \param norm_factors The normalization factors for each group
     * \param norm_tag The tag to be normalized on each group
     * \param integ_type Type of integration to perform
     */
  ErrorCode apply_group_norm_factor( std::vector<std::vector<EntityHandle> > &entity_sets,
                                     std::vector<double> &norm_factors, 
                                     const char *norm_tag,
                                     Coupler::IntegType integ_type);

  /*
   * this method will look at source (and target sets?) sets, and look for the SEM_DIMS tag
   * if it exists, it will trigger a spectral element caching, with the order specified
   */
  ErrorCode initialize_spectral_elements(EntityHandle rootSource, EntityHandle rootTarget,
                                         bool &specSou, bool &specTar);

  /*
   * this method will put in an array, interleaved, the points of interest for coupling
   * with a target mesh (so where do we need to compute the field of interest)
   */
  ErrorCode get_gl_points_on_elements(Range &targ_elems, std::vector<double> &vpos, int &numPointsOfInterest);

    /* Get functions */

  inline Interface *mb_impl() const { return mbImpl; }
  inline AdaptiveKDTree *my_tree() const { return myTree; }
  inline EntityHandle local_root() const { return localRoot; }
  inline const std::vector<double> &all_boxes() const { return allBoxes; }
  inline ParallelComm *my_pc() const { return myPc; }
  inline const Range &target_ents() const { return targetEnts; }
  inline int my_id() const { return myId; }
  inline const Range &my_range() const { return myRange; }
  inline TupleList *mapped_pts() const { return mappedPts; }
  inline int num_its() const { return numIts; }

private:

    // Given a coordinate position, find all entities containing
    // the point and the natural coords in those ents
  ErrorCode nat_param(double xyz[3],
                      std::vector<EntityHandle> &entities,
                      std::vector<CartVect> &nat_coords,
                      double epsilon = 0.0);
  
  ErrorCode interp_field(EntityHandle elem,
                         CartVect nat_coord,
                         Tag tag,
                         double &field);

  ErrorCode constant_interp(EntityHandle elem,
                            Tag tag,
                            double &field);

  ErrorCode test_local_box(double *xyz,
                           int from_proc, int remote_index, int index,
                           bool &point_located,
                           double rel_eps = 0.0,
                           double abs_eps = 0.0,
                           TupleList *tl = NULL);

    /* \brief MOAB instance
     */
  Interface *mbImpl;

    /* \brief Initialize the kdtree, locally and across communicator
     */
  ErrorCode initialize_tree();

    /* \brief Kdtree for local mesh
     */
  AdaptiveKDTree *myTree;

    /* \brief Local root of the kdtree
     */
  EntityHandle localRoot;

    /* \brief Min/max bounding boxes for all proc tree roots
     */
  std::vector<double> allBoxes;

    /* \brief ParallelComm object for this coupler
     */
  ParallelComm *myPc;

    /* \brief Id of this coupler
     */
  int myId;

    /* \brief Range of source elements 
     */
  Range myRange;

    /* \brief Range of target entities
     */
  Range targetEnts;

    /* \brief List of locally mapped tuples
     * Tuples contain the following:
     * n = # mapped points
     * vul[i] = local handle of mapped entity
     * vr[3*i..3*i+2] = natural coordinates in mapped entity
     */
  TupleList *mappedPts;

    /* \brief Tuple list of target points and interpolated data
     * Tuples contain the following:
     * n = # target points
     * vi[3*i]   = remote proc mapping target point
     * vi[3*i+1] = local index of target point
     * vi[3*i+2] = remote index of target point
     */
  TupleList *targetPts;

    /* \brief Number of iterations of tree building before failing
     *
     */
  int numIts;

  // Entity dimension
  int max_dim;

  // A cached spectral element for source and target, separate
  // Assume that their number of GL points (order + 1) does not change
  // If it does change, we need to reinitialize it
  void * _spectralSource;
  void * _spectralTarget;
  moab::Tag _xm1Tag, _ym1Tag, _zm1Tag;
  int _ntot;
};

inline ErrorCode Coupler::interpolate(Coupler::Method method,
                                      Tag tag,
                                      double *interp_vals,
                                      TupleList *tl,
                                      bool normalize)
{
  int num_pts = (tl ? tl->get_n() : targetPts->get_n());
  return interpolate(&method, &tag, &num_pts, 1,
                     interp_vals, tl, normalize);
}

} // namespace moab

#endif
