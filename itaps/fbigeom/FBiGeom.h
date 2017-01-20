#ifndef _ITAPS_FBiGeom
#define _ITAPS_FBiGeom

  /** \mainpage The ITAPS Geometry Interface FBiGeom
   *
   * The ITAPS Geometry Interface FBiGeom provides a common interface for
   * accessing geometry and data associated with a mesh.  Applications written
   * to use this interface can use a variety of implementations, choosing
   * the one that best meets its needs.  They can also use tools written
   * to this interface.
   *
   * \section ITAPS Data Model
   *
   * The ITAPS interfaces use a data model composed of four basic data types:\n
   * \em Entity: basic topological entities in a model, e.g. vertices, 
   * edges, faces, regions. \n
   * \em Entity \em Set: arbitrary grouping of other entities and sets. 
   * Entity sets also support parent/child relations with other sets which
   * are distinct from entities contained in those sets.  Parent/child links
   * can be used to embed graph relationships between sets, e.g. to 
   * represent topological relationships between the sets. \n
   * \em Interface: the object with which model is associated and on which
   * functions in FBiGeom are called. \n
   * \em Tag: application data associated with objects of any of the other 
   * data types.  Each tag has a designated name, size, and data type.
   *
   * \section JTAPS Entity Type
   * Each entity has a specific Entity Type.  The Entity 
   * Type is one of VERTEX, EDGE, FACE, and REGION, and is synonymous with
   * the topological dimension of the entity.  Entity Type is an enumerated
   * type in the iBase_EntityType enumeration.
   *
   * \section KTAPS Entity-, Array-, and Iterator-Based Access
   *
   * The FBiGeom interface provides functions for accessing entities
   * individually, as arrays of entities, or using iterators.  These access
   * methods have different memory versus execution time tradeoffs, 
   * depending on the implementation.
   *
   * \section LTAPS Lists Passed Through Interface
   *
   * Many of the functions in FBiGeom have arguments corresponding to lists of 
   * objects.  In-type arguments for lists consist of a pointer to an array and
   * a list size.  Lists returned from functions are passed in three arguments,
   * a pointer to the array representing the list, and pointers to the
   * allocated and occupied lengths of the array.  These three arguments are 
   * inout-type arguments, because they can be allocated by the application and
   * passed into the interface to hold the results of the function.  Lists
   * which are pre-allocated must be large enough to hold the results of the
   * function; if this is not the case, an error is generated.  Otherwise, the
   * occupied size is changed to the size output from the function.  If a list
   * argument is unallocated (the list pointer points to a NULL value) or if
   * the incoming value of the allocated size is zero, the list storage will be
   * allocated by the implementation.  IN ALL CASES, MEMORY ALLOCATED BY ITAPS
   * INTERFACE IMPLEMENTATIONS IS DONE USING THE C MALLOC FUNCTION, AND CAN BE
   * DE-ALLOCATED USING THE C FREE FUNCTION.
   *
   */

#include "iBase.h"
#include "FBiGeom_protos.h"

#ifdef __cplusplus
extern "C" {
#endif

    /**\brief  Type used to store FBiGeom interface handle
     *
     * Type used to store FBiGeom interface handle
     */
  typedef struct FBiGeom_Instance_Private* FBiGeom_Instance;

    /**\brief  Get a description of the error returned from the last FBiGeom call
     *
     * Get a description of the error returned from the last FBiGeom function
     * \param instance FBiGeom instance handle
     * \param descr Pointer to a character string to be filled with a
     *        description of the error from the last FBiGeom function
     * \param descr_len Length of the character string pointed to by descr
     */
  void FBiGeom_getDescription( FBiGeom_Instance instance,
                             char* descr,
                             int descr_len );

    /**\brief  Get the error type returned from the last FBiGeom function
     *
     * Get the error type returned from the last FBiGeom function.  Value
     * returned is a member of the iBase_ErrorType enumeration.
     * \param instance FBiGeom instance handle
     * \param *error_type Error type returned from last FBiGeom function
     */
  void FBiGeom_getErrorType( FBiGeom_Instance instance,
                           /*out*/ int *error_type );

    /**\brief  Construct a new FBiGeom instance
     *
     * Construct a new FBiGeom instance, using implementation-specific options
     * \param options Pointer to implementation-specific options string
     * \param instance Pointer to FBiGeom instance handle returned from function
     * \param *err Pointer to error type returned from function
     * \param options_len Length of the character string pointed to by options
     */
  void FBiGeom_newGeom( char const* options,
                      FBiGeom_Instance* instance_out,
                      int* err,
                      int options_len );

  /**\brief  Construct a new FBiGeom instance
       *
       * Construct a new FBiGeom instance, using an existing moab iMesh instance
       * and a root set that encapsulates the topological model
       * \param mesh iMesh_Instance
       * \param set  root set for the mesh based geometry
       * \param options Pointer to implementation-specific options string
       * \param instance Pointer to FBiGeom instance handle returned from function
       * \param *err Pointer to error type returned from function
       * \param options_len Length of the character string pointed to by options
       */

/*  void FBiGeom_newGeomFromMesh( iMesh_Instance mesh, iBase_EntitySetHandle set,
                          const char *options, FBiGeom_Instance *geom,
                          int *err, int options_len);*/
    /**\brief  Destroy an FBiGeom instance
     *
     * Destroy an FBiGeom instance
     * \param instance FBiGeom instance to be destroyed
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_dtor( FBiGeom_Instance instance,
                   int* err );


    /**\brief  Load a geom from a file
     *
     * Load a geom from a file.  If entity set is specified, loaded geom
     * is added to that set; specify zero if that is not desired.
     * \param instance FBiGeom instance handle
     * \param entity_set_handle Set to which loaded geom will be added, zero
     *        if not desired
     * \param name File name from which geom is to be loaded
     * \param options Pointer to implementation-specific options string
     * \param *err Pointer to error type returned from function
     * \param name_len Length of the file name character string
     * \param options_len Length of the options character string
     */
  void FBiGeom_load( FBiGeom_Instance instance,
                   char const* name,
                   char const* options,
                   int* err,
                   int name_len,
                   int options_len );

    /**\brief  Save a geom to a file
     *
     * Save a geom to a file.  If entity set is specified, save only the
     * geom contained in that set.
     * \param instance FBiGeom instance handle
     * \param entity_set_handle Entity set being saved
     * \param name File name to which geom is to be saved
     * \param options Pointer to implementation-specific options string
     * \param *err Pointer to error type returned from function
     * \param name_len Length of the file name character string
     * \param options_len Length of the options character string
     */
  void FBiGeom_save( FBiGeom_Instance instance,
                   char const* name,
                   char const* options,
                   int* err,
                   int name_len,
                   int options_len );

    /**\brief  Get handle of the root set for this instance
     *
     * Get handle of the root set for this instance.  All geom in
     * this instance can be accessed from this set.
     * \param instance FBiGeom instance handle
     * \param root_set Pointer to set handle returned from function
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getRootSet( FBiGeom_Instance instance,
                         iBase_EntitySetHandle* root_set,
                         int* err );

    /**\brief Get the bounding box of the entire model
     *
     * Get the bounding box of the entire model
     * \param instance FBiGeom instance handle
     * \param min_x Minimum coordinate of bounding box
     * \param min_y Minimum coordinate of bounding box
     * \param min_z Minimum coordinate of bounding box
     * \param max_x Maximum coordinate of bounding box
     * \param max_y Maximum coordinate of bounding box
     * \param max_z Maximum coordinate of bounding box
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getBoundBox( FBiGeom_Instance instance,
                          double* min_x,
                          double* min_y,
                          double* min_z,
                          double* max_x,
                          double* max_y,
                          double* max_z,
                          int* err );

    /**\brief  Get entities of specific type in set or instance
     *
     * Get entities of specific type in set or instance.  All entities are
     * requested by specifying iBase_ALL_TYPES.  Specified type must be a value
     * in the iBase_EntityType enumeration.
     * \param instance FBiGeom instance handle
     * \param entity_set_handle Entity set being queried
     * \param entity_type Type of entities being requested
     * \param entity_topology Topology of entities being requested
     * \param *entity_handles Pointer to array of entity handles returned 
     *        from function
     * \param *entity_handles_allocated Pointer to allocated size of 
     *        entity_handles array
     * \param *entity_handles_size Pointer to occupied size of entity_handles
     *        array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getEntities( FBiGeom_Instance instance,
                          iBase_EntitySetHandle set_handle,
                          int entity_type,
                          iBase_EntityHandle** entity_handles,
                          int* entity_handles_allococated,
                          int* entity_handles_size,
                          int* err );

    /**\brief  Get the number of entities with the specified type in the
     *         instance or set
     *
     * Get the number of entities with the specified type in the instance 
     * or set.  If entity set handle is zero, return information for instance,
     * otherwise for set.  Value of entity type must be from the
     * iBase_EntityType enumeration.  If iBase_ALL_TYPES is specified, total
     * number of entities (excluding entity sets) is returned.
     * \param instance FBiGeom instance handle
     * \param entity_set_handle Entity set being queried
     * \param entity_type Type of entity requested
     * \param num_type Pointer to number of entities, returned from function
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getNumOfType( FBiGeom_Instance instance,
                           iBase_EntitySetHandle set_handle,
                           int entity_type,
                           int* num_out,
                           int* err );

    /**\brief  Get the entity type for the specified entity
     *
     * Get the entity type for the specified entity.  Type returned is a value
     * in the iBase_EntityType enumeration.
     * \param instance FBiGeom instance handle
     * \param entity_handle entity handle being queried
     * \param *type Pointer to location at which to store the returned type
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getEntType( FBiGeom_Instance instance,
                         iBase_EntityHandle entity_handle,
                         int* type,
                         int* err );

    /**\brief  Get the entity type for the specified entities
     *
     * Get the entity type for the specified entities.  Types returned are
     * values in the iBase_EntityType enumeration.
     * \param instance FBiGeom instance handle
     * \param entity_handles Array of entity handles being queried
     * \param entity_handles_size Number of entities in entity_handles array
     * \param *type Pointer to array of types returned from function
     * \param *type_allocated Pointer to allocated size of type array
     * \param *type_size Pointer to occupied size of type array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getArrType( FBiGeom_Instance instance,
                         iBase_EntityHandle const* entity_handles,
                         int entity_handles_size,
                         int** type,
                         int* type_allocated,
                         int* type_size,
                         int* err );

    /**\brief  Get entities of specified type adjacent to an entity
     *
     * Get entities of specified type adjacent to an entity.  Specified type
     * must be value in the iBase_EntityType enumeration.
     * \param instance FBiGeom instance handle
     * \param entity_handle Entity handle being queried
     * \param entity_type_requested Type of adjacent entities requested
     * \param *adj_entity_handles Pointer to array of adjacent entities
     *        returned from function
     * \param *adj_entity_handles_allocated Pointer to allocated size of 
     *        adj_entity_handles array
     * \param *adj_entity_handles_size Pointer to occupied size of 
     *        adj_entity_handles array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getEntAdj( FBiGeom_Instance instance,
                        iBase_EntityHandle entity_handle,
                        int to_dimension,
                        iBase_EntityHandle** adj_entities,
                        int* adj_entities_allocated,
                        int* adj_entities_size,
                        int* err );

    /**\brief  Get entities of specified type adjacent to entities
     *
     * Get entities of specified type adjacent to entities.  Specified type
     * must be value in the iBase_EntityType enumeration.  \em offset(i) is
     * index of first entity in adjacentEntityHandles array adjacent to 
     * entity_handles[i].
     * \param instance FBiGeom instance handle
     * \param entity_handles Array of entity handles being queried
     * \param entity_handles_size Number of entities in entity_handles array
     * \param entity_type_requested Type of adjacent entities requested
     * \param *adjacentEntityHandles Pointer to array of adjacentEntityHandles 
     *        returned from function
     * \param *adjacentEntityHandles_allocated Pointer to allocated size of 
     *        adjacentEntityHandles array
     * \param *adj_entity_handles_size Pointer to occupied size of 
     *        adjacentEntityHandles array
     * \param *offset Pointer to array of offsets returned from function
     * \param *offset_allocated Pointer to allocated size of offset array
     * \param *offset_size Pointer to occupied size of offset array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getArrAdj( FBiGeom_Instance instance,
                        iBase_EntityHandle const* entity_handles,
                        int entity_handles_size,
                        int requested_entity_type,
                        iBase_EntityHandle** adj_entity_handles,
                        int* adj_entity_handles_allocated,
                        int* adj_entity_handles_size,
                        int** offset,
                        int* offset_allocated,
                        int* offset_size,
                        int* err );

    /**\brief  Get "2nd order" adjacencies to an entity
     *
     * Get "2nd order" adjacencies to an entity, that is, from an entity,
     * through other entities of a specified "bridge" dimension, to other
     * entities of another specified "to" dimension.
     * \param instance FBiGeom instance handle
     * \param entity_handle Entity from which adjacencies are requested
     * \param bridge_dimension Bridge dimension for 2nd order adjacencies
     * \param to_dimension Dimension of adjacent entities returned
     * \param adjacent_entities Adjacent entities
     * \param adjacent_entities_allocated Allocated size of returned array
     * \param adjacent_entities_size Occupied size of returned array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getEnt2ndAdj( FBiGeom_Instance instance,
                           iBase_EntityHandle entity_handle,
                           int bridge_dimension,
                           int to_dimension,
                           iBase_EntityHandle** adjacent_entities,
                           int* adjacent_entities_allocated,
                           int* adjacent_entities_size,
                           int* err );

    /**\brief  Get "2nd order" adjacencies to an array of entities
     *
     * Get "2nd order" adjacencies to an array of entities, that is, from each
     * entity, through other entities of a specified "bridge" dimension, to
     * other entities of another specified "to" dimension.
     * \param instance FBiGeom instance handle
     * \param entity_handles Entities from which adjacencies are requested
     * \param entity_handles_size Number of entities whose adjacencies are
     *        requested
     * \param bridge_dimension Bridge dimension for 2nd order adjacencies
     * \param to_dimension Dimension of adjacent entities returned
     * \param adj_entity_handles Adjacent entities
     * \param adj_entity_handles_allocated Allocated size of returned array
     * \param adj_entity_handles_size Occupied size of returned array
     * \param offset Offset[i] is offset into adj_entity_handles of 2nd order 
     *        adjacencies of ith entity in entity_handles
     * \param offset_allocated Allocated size of offset array
     * \param offset_size Occupied size of offset array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getArr2ndAdj( FBiGeom_Instance instance,
                           iBase_EntityHandle const* entity_handles,
                           int entity_handles_size,
                           int order_adjacent_key,
                           int requested_entity_type,
                           iBase_EntityHandle** adj_entity_handles,
                           int* adj_entity_handles_allocated,
                           int* adj_entity_handles_size,
                           int** offset,
                           int* offset_allocated,
                           int* offset_size,
                           int* err );

    /**\brief  Return whether two entities are adjacent
     *
     * Return whether two entities are adjacent.
     * \param instance FBiGeom instance handle
     * \param entity_handle1 First entity queried
     * \param entity_handle2 Second entity queried
     * \param are_adjacent If returned non-zero, entities are adjacent,
     *        otherwise they are not
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_isEntAdj( FBiGeom_Instance instance,
                       iBase_EntityHandle entity_handle1,
                       iBase_EntityHandle entity_handle2,
                       int* are_adjacent,
                       int* err );

    /**\brief  Return whether entity pairs are adjacent
     *
     * Return whether entity pairs are adjacent, i.e. if entity_handles_1[i] is
     * adjacent to entity_handles_2[i].  This function requires
     * entity_handles_1_size and entity_handles_2_size to be equal.
     * \param instance FBiGeom instance handle
     * \param entity_handles_1 First array of entities
     * \param entity_handles_1_size Number of entities in first array
     * \param entity_handles_2 Second array of entities
     * \param entity_handles_2_size Number of entities in second array
     * \param is_adjacent_info Array of flags returned from function
     * \param is_adjacent_info_allocated Allocated size of flags array
     * \param is_adjacent_info_size Occupied size of flags array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_isArrAdj( FBiGeom_Instance instance,
                       iBase_EntityHandle const* entity_handles_1,
                       int entity_handles_1_size,
                       iBase_EntityHandle const* entity_handles_2,
                       int entity_handles_2_size,
                       int** is_adjacent_info,
                       int* is_adjacent_info_allocated,
                       int* is_adjacent_info_size,
                       int* err );

    /**\brief  Return the topology level of the geometry
     *
     * Return the topology level of the geometry as an integer, where 0 = basic
     * entities only, 1 = manifold entities, 2 = non-manifold entities.
     * \param instance FBiGeom instance handle
     * \param topo_level_out The topology level
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getTopoLevel( FBiGeom_Instance instance,
                           int* topo_level_out,
                           int* err );

    /**\brief  Get closest point to an entity
     *
     * Get closest point to a specified position on an entity
     * \param instance FBiGeom instance handle
     * \param entity_handle Entity being queried
     * \param near_x Coordinates of starting point
     * \param near_y Coordinates of starting point
     * \param near_z Coordinates of starting point
     * \param on_x Closest point on entity
     * \param on_y Closest point on entity
     * \param on_z Closest point on entity
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getEntClosestPt( FBiGeom_Instance instance,
                              iBase_EntityHandle entity_handle,
                              double near_x, 
                              double near_y,
                              double near_z,
                              double* on_x,
                              double* on_y,
                              double* on_z,
                              int* err );

    /**\brief  Get closest point for an array of entities and points
     * For surfaces, closest point could be on the void space inside it.
     * Get closest point for an array of entities and points.  If either the
     * number of entities or number of coordinate triples is unity, then all
     * points or entities are queried for that entity or point, respectively,
     * otherwise each point corresponds to each entity.  storage_order should be
     * a value in the iBase_StorageOrder enum.
     * \param instance FBiGeom instance handle
     * \param entity_handles Entity(ies) being queried
     * \param entity_handles_size Number of entities being queried
     * \param storage_order Storage order of input points
     * \param near_coordinates Coordinates of starting point(s)
     * \param near_coordinates_size Number of values in near_coordinates array
     * \param on_coordinates Coordinates of closest points
     * \param on_coordinates_allocated Allocated size of closest point array
     * \param on_coordinates_size Occupied size of closest point array
     * \param *err Pointer to error type returned from function
     */

  void FBiGeom_getEntClosestPtTrimmed( FBiGeom_Instance instance,
                                     iBase_EntityHandle entity_handle,
                                     double near_x,
                                     double near_y,
                                     double near_z,
                                     double* on_x,
                                     double* on_y,
                                     double* on_z,
                                     int* err );

    /**\brief  Get closest point for an array of entities and points
     * For surfaces, it made sure the closest point in on surface.
     * Get closest point for an array of entities and points.  If either the
     * number of entities or number of coordinate triples is unity, then all
     * points or entities are queried for that entity or point, respectively,
     * otherwise each point corresponds to each entity.  storage_order should be
     * a value in the iBase_StorageOrder enum.
     * \param instance FBiGeom instance handle
     * \param entity_handles Entity(ies) being queried
     * \param entity_handles_size Number of entities being queried
     * \param storage_order Storage order of input points
     * \param near_coordinates Coordinates of starting point(s)
     * \param near_coordinates_size Number of values in near_coordinates array
     * \param on_coordinates Coordinates of closest points
     * \param on_coordinates_allocated Allocated size of closest point array
     * \param on_coordinates_size Occupied size of closest point array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getArrClosestPt( FBiGeom_Instance instance,
                              iBase_EntityHandle const* entity_handles,
                              int entity_handles_size,
                              int storage_order,
                              double const* near_coordinates,
                              int near_coordinates_size,
                              double** on_coordinates,
                              int* on_coordinates_allocated,
                              int* on_coordinates_size,
                              int* err );

    /**\brief  Get the normal vector on an entity at the given position
     * Get the normal vector on an entity at the given position.
     * \param instance FBiGeom instance handle
     * \param entity_handle Entity being queried
     * \param x Coordinates of starting point
     * \param y Coordinates of starting point
     * \param z Coordinates of starting point
     * \param nrml_i Normal vector at starting point
     * \param nrml_j Normal vector at starting point
     * \param nrml_k Normal vector at starting point
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getEntNrmlXYZ( FBiGeom_Instance instance,
                            iBase_EntityHandle entity_handle,
                            double x,
                            double y,
                            double z,
                            double* nrml_i,
                            double* nrml_j,
                            double* nrml_k,
                            int* err );

    /**\brief  Get the normal vector on an entity(ies) at given position(s)
     *
     * Get the normal vector on an entity(ies) at given position(s).  If either
     * the number of entities or number of coordinate triples is unity, then all
     * points or entities are queried for that entity or point, respectively,
     * otherwise each point corresponds to each entity.  storage_order should be
     * a value in the iBase_StorageOrder enum.
     * \param instance FBiGeom instance handle
     * \param entity_handles Entity(ies) being queried
     * \param entity_handles_size Number of entities being queried
     * \param storage_order Storage order of coordinates
     * \param coordinates Starting coordinates
     * \param coordinates_size Number of values in coordinates array
     * \param normals Normal coordinates
     * \param normals_allocated Allocated size of normals array
     * \param normals_size Occupied size of normals array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getArrNrmlXYZ( FBiGeom_Instance instance,
                            iBase_EntityHandle const* entity_handles,
                            int entity_handles_size,
                            int storage_order,
                            double const* coordinates,
                            int coordinates_size,
                            double** normals,
                            int* normals_allocated,
                            int* normals_size,
                            int* err );

    /**\brief  Get the normal vector AND closest point on an entity at given
     *         position
     *
     * Get the normal vector AND closest point on an entity at a given position.
     * \param entity_handle Entity being queried
     * \param instance FBiGeom instance handle
     * \param x Starting coordinates
     * \param y Starting coordinates
     * \param z Starting coordinates
     * \param pt_x Closest point
     * \param pt_y Closest point
     * \param pt_z Closest point
     * \param nrml_i Normal at closest point
     * \param nrml_j Normal at closest point
     * \param nrml_k Normal at closest point
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getEntNrmlPlXYZ( FBiGeom_Instance instance,
                              iBase_EntityHandle entity_handle,
                              double x,
                              double y,
                              double z,
                              double* pt_x,
                              double* pt_y,
                              double* pt_z,
                              double* nrml_i,
                              double* nrml_j,
                              double* nrml_k,
                              int* err );

    /**\brief Get the normal vector AND closest point on an entity(ies) at
     *        given position(s)
     *
     * Get the normal vector AND closest point on an entity(ies) at given
     * position(s).  If either the number of entities or number of coordinate
     * triples is unity, then all points or entities are queried for that entity
     * or point, respectively, otherwise each point corresponds to each entity.
     * storage_order should be a value in the iBase_StorageOrder enum.
     * \param instance FBiGeom instance handle
     * \param entity_handles Entity(ies) being queried
     * \param entity_handles_size Number of entity(ies) being queried
     * \param storage_order Storage order in near_coordinates array
     * \param near_coordinates Starting coordinates
     * \param near_coordinates_size Number of values in near_coordinates array
     * \param on_coordinates Closest point array
     * \param on_coordinates_allocated Allocated size of closest point array
     * \param on_coordinates_size Occupied size of closest point array
     * \param normals Normal array
     * \param normals_allocated Allocated size of normal array
     * \param normals_size Occupied size of normal array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getArrNrmlPlXYZ( FBiGeom_Instance instance,
                              iBase_EntityHandle const* entity_handles,
                              int entity_handles_size,
                              int storage_order,
                              double const* near_coordinates,
                              int near_coordinates_size,
                              double** on_coordinates,
                              int* on_coordinates_allocated,
                              int* on_coordinates_size,
                              double** normals,
                              int* normals_allocated,
                              int* normals_size,
                              int* err );

    /**\brief  Get the tangent vector on an entity at given position
     *
     * Get the tangent vector on an entity at a given position.
     * \param instance FBiGeom instance handle
     * \param entity_handle Entity being queried
     * \param x Starting coordinates
     * \param y Starting coordinates
     * \param z Starting coordinates
     * \param tgnt_i Tangent at closest point
     * \param tgnt_j Tangent at closest point
     * \param tgnt_k Tangent at closest point
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getEntTgntXYZ( FBiGeom_Instance instance,
                            iBase_EntityHandle entity_handle,
                            double x,
                            double y,
                            double z,
                            double* tgnt_i,
                            double* tgnt_j,
                            double* tgnt_k,
                            int* err );

    /**\brief  Get the tangent vector on an entity(ies) at given position(s)
     *
     * Get the tangent vector on an entity(ies) at given position(s).  If either
     * the number of entities or number of coordinate triples is unity, then all
     * points or entities are queried for that entity or point, respectively,
     * otherwise each point corresponds to each entity.  storage_order should be
     * a value in the iBase_StorageOrder enum
     * \param instance FBiGeom instance handle
     * \param entity_handles Entity(ies) being queried
     * \param entity_handles_size Number of entities being queried
     * \param storage_order Storage order of coordinates
     * \param coordinates Starting coordinates
     * \param coordinates_size Number of values in coordinates array
     * \param tangents Tangent coordinates
     * \param tangents_allocated Allocated size of tangents array
     * \param tangents_size Occupied size of tangents array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getArrTgntXYZ( FBiGeom_Instance instance,
                            iBase_EntityHandle const* entity_handles,
                            int entity_handles_size,
                            int storage_order,
                            double const* coordinates,
                            int coordinates_size,
                            double** tangents,
                            int* tangents_allocated,
                            int* tangents_size,
                            int* err );

    /**\brief  Get the two principle curvature vectors for a face at a point
     *
     * Get the two principle curvature vectors for a face at a point.
     * Magnitudes of vectors are curvature, directions are directions of
     * principal curvatures.
     * \param instance FBiGeom instance handle
     * \param face_handle Face being queried
     * \param x Position being queried
     * \param y Position being queried
     * \param z Position being queried
     * \param cvtr1_i Maximum curvature vector
     * \param cvtr1_j Maximum curvature vector
     * \param cvtr1_k Maximum curvature vector
     * \param cvtr2_i Minimum curvature vector
     * \param cvtr2_j Minimum curvature vector
     * \param cvtr2_k Minimum curvature vector
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getFcCvtrXYZ( FBiGeom_Instance instance,
                           iBase_EntityHandle face_handle,
                           double x,
                           double y,
                           double z,
                           double* cvtr1_i,
                           double* cvtr1_j,
                           double* cvtr1_k,
                           double* cvtr2_i,
                           double* cvtr2_j,
                           double* cvtr2_k,
                           int* err );

    /**\brief  Get the principle curvature vector for an edge at a point
     *
     * Get the principle curvature vector for an edge at a point.  Magnitude of
     * vector is the curvature, direction is direction of principal curvature.
     * \param instance FBiGeom instance handle
     * \param edge_handle Edge being queried
     * \param x Position being queried
     * \param y Position being queried
     * \param z Position being queried
     * \param cvtr_i Maximum curvature vector
     * \param cvtr_j Maximum curvature vector
     * \param cvtr_k Maximum curvature vector
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getEgCvtrXYZ( FBiGeom_Instance instance,
                           iBase_EntityHandle edge_handle,
                           double x,
                           double y,
                           double z,
                           double* cvtr_i,
                           double* cvtr_j,
                           double* cvtr_k,
                           int* err );

    /**\brief  Get the curvature(s) on an entity(ies) at given position(s)
     *
     * Get the curvature(s) on an entity(ies) at given position(s).  If either
     * the number of entities or number of coordinate triples is unity, then all
     * points or entities are queried for that entity or point, respectively,
     * otherwise each point corresponds to each entity.  storage_order should be
     * a value in the iBase_StorageOrder enum.
     * \param instance FBiGeom instance handle
     * \param entity_handles Entity(ies) being queried
     * \param entity_handles_size Number of entities being queried
     * \param storage_order Storage order of coordinates
     * \param coords Starting coordinates
     * \param coords_size Number of values in coordinates array
     * \param cvtr_1 First principal curvatures
     * \param cvtr_1_allocated Allocated size of first curvature array
     * \param cvtr_1_size Occupied size of first curvature array
     * \param cvtr_2 Second principal curvatures
     * \param cvtr_2_allocated Allocated size of second curvature array
     * \param cvtr_2_size Occupied size of second curvature array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getEntArrCvtrXYZ( FBiGeom_Instance instance,
                               iBase_EntityHandle const* entity_handles,
                               int entity_handles_size,
                               int storage_order,
                               double const* coords,
                               int coords_size,
                               double** cvtr_1,
                               int* cvtr_1_allocated,
                               int* cvtr_1_size,
                               double** cvtr_2,
                               int* cvtr_2_allocated,
                               int* cvtr_2_size,
                               int* err );

    /**\brief  Get closest point, tangent, and curvature of edge
     *
     * Get closest point, tangent, and curvature of edge.
     * \param instance FBiGeom instance handle
     * \param edge_handle Edge being queried
     * \param x Point at which entity is being queried
     * \param y Point at which entity is being queried
     * \param z Point at which entity is being queried
     * \param on_x Closest point at point being queried
     * \param on_y Closest point at point being queried
     * \param on_z Closest point at point being queried
     * \param tgnt_i Tangent at point being queried
     * \param tgnt_j Tangent at point being queried
     * \param tgnt_k Tangent at point being queried
     * \param cvtr_i Curvature at point being queried
     * \param cvtr_j Curvature at point being queried
     * \param cvtr_k Curvature at point being queried
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getEgEvalXYZ( FBiGeom_Instance instance,
                           iBase_EntityHandle edge_handle,
                           double x,
                           double y,
                           double z,
                           double* on_x,
                           double* on_y,
                           double* on_z,
                           double* tgnt_i,
                           double* tgnt_j,
                           double* tgnt_k,
                           double* cvtr_i,
                           double* cvtr_j,
                           double* cvtr_k,
                           int* err );

    /**\brief  Get closest point, tangent, and curvature of face
     *
     * Get closest point, tangent, and curvature of face.  If any of input
     * coordinate pointers are NULL, that value is not returned.
     * \param instance FBiGeom instance handle
     * \param face_handle Face being queried
     * \param x Point at which entity is being queried
     * \param y Point at which entity is being queried
     * \param z Point at which entity is being queried
     * \param on_x Closest point at point being queried
     * \param on_y Closest point at point being queried
     * \param on_z Closest point at point being queried
     * \param nrml_i Normal at point being queried
     * \param nrml_j Normal at point being queried
     * \param nrml_k Normal at point being queried
     * \param cvtr1_i First principal curvature at point being queried
     * \param cvtr1_j First principal curvature at point being queried
     * \param cvtr1_k First principal curvature at point being queried
     * \param cvtr2_i Second principal curvature at point being queried
     * \param cvtr2_j Second principal curvature at point being queried
     * \param cvtr2_k Second principal curvature at point being queried
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getFcEvalXYZ( FBiGeom_Instance instance,
                           iBase_EntityHandle face_handle,
                           double x,
                           double y,
                           double z,
                           double* on_x,
                           double* on_y,
                           double* on_z,
                           double* nrml_i,
                           double* nrml_j,
                           double* nrml_k,
                           double* cvtr1_i,
                           double* cvtr1_j,
                           double* cvtr1_k,
                           double* cvtr2_i,
                           double* cvtr2_j,
                           double* cvtr2_k,
                           int* err );

    /**\brief  Get the closest point(s), tangent(s), and curvature(s) on an
     *         entity(ies) at given position(s)
     *
     * Get the closest point(s), tangent(s), and curvature(s) on an entity(ies)
     * at given position(s).  If either the number of entities or number of
     * coordinate triples is unity, then all points or entities are queried for
     * that entity or point, respectively, otherwise each point corresponds to
     * each entity.  storage_order should be a value in the iBase_StorageOrder
     * enum.
     * \param instance FBiGeom instance handle
     * \param edge_handles Edge(s) being queried
     * \param edge_handles_size Number of edges being queried
     * \param storage_order Storage order of coordinates
     * \param coords Starting coordinates
     * \param coords_size Number of values in coordinates array
     * \param on_coords Closest point array
     * \param on_coords_allocated Allocated size of closest point array
     * \param on_coords_size Occupied size of closest point array
     * \param tangent Tangent array
     * \param tangent_allocated Allocated size of tangent array
     * \param tangent_size Occupied size of tangent array
     * \param cvtr First principal curvatures
     * \param cvtr_allocated Allocated size of first curvature array
     * \param cvtr_size Occupied size of first curvature array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getArrEgEvalXYZ( FBiGeom_Instance instance,
                              iBase_EntityHandle const* edge_handles,
                              int edge_handles_size,
                              int storage_order,
                              double const* coords,
                              int coords_size,
                              double** on_coords,
                              int* on_coords_allocated,
                              int* on_coords_size,
                              double** tangent,
                              int* tangent_allocated,
                              int* tangent_size,
                              double** cvtr,
                              int* cvtr_allocated,
                              int* cvtr_size,
                              int* err );

    /**\brief  Get the closest point(s), tangent(s), and curvature(s) on an
     *         entity(ies) at given position(s)
     *
     * Get the closest point(s), tangent(s), and curvature(s) on an entity(ies)
     * at given position(s).  If either the number of entities or number of
     * coordinate triples is unity, then all points or entities are queried for
     * that entity or point, respectively, otherwise each point corresponds to
     * each entity.  storage_order should be a value in the iBase_StorageOrder
     * enum.
     * \param instance FBiGeom instance handle
     * \param edge_handles Edge(s) being queried
     * \param edge_handles_size Number of edges being queried
     * \param storage_order Storage order of coordinates
     * \param coords Starting coordinates
     * \param coords_size Number of values in coordinates array
     * \param on_coords Closest point array
     * \param on_coords_allocated Allocated size of closest point array
     * \param on_coords_size Occupied size of closest point array
     * \param normal Normal array
     * \param normal_allocated Allocated size of normal array
     * \param normal_size Occupied size of normal array
     * \param cvtr_1 First principal curvatures
     * \param cvtr_1_allocated Allocated size of first curvature array
     * \param cvtr_1_size Occupied size of first curvature array
     * \param cvtr_2 Second principal curvatures
     * \param cvtr_2_allocated Allocated size of second curvature array
     * \param cvtr_2_size Occupied size of second curvature array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getArrFcEvalXYZ( FBiGeom_Instance instance,
                              iBase_EntityHandle const* face_handles,
                              int face_handles_size,
                              int storage_order,
                              double const* coords,
                              int coords_size,
                              double** on_coords,
                              int* on_coords_allocated,
                              int* on_coords_size,
                              double** normal,
                              int* normal_allocated,
                              int* normal_size,
                              double** cvtr1,
                              int* cvtr1_allocated,
                              int* cvtr1_size,
                              double** cvtr2,
                              int* cvtr2_allocated,
                              int* cvtr2_size,
                              int* err );

    /**\brief  Get the bounding box of the specified entity
     *
     * Get the bounding box of the specified entity
     * \param instance FBiGeom instance handle
     * \param entity_handle Entity being queried
     * \param min_x Minimum coordinate of bounding box 
     * \param min_y Minimum coordinate of bounding box 
     * \param min_z Minimum coordinate of bounding box 
     * \param max_x Maximum coordinate of bounding box 
     * \param max_y Maximum coordinate of bounding box 
     * \param max_z Maximum coordinate of bounding box 
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getEntBoundBox( FBiGeom_Instance instance,
                             iBase_EntityHandle entity_handle,
                             double* min_x,
                             double* min_y,
                             double* min_z,
                             double* max_x,
                             double* max_y,
                             double* max_z,
                             int* err );

    /**\brief  Get the bounding box of the specified entities
     *
     * Get the bounding box of the specified entities.  Storage order passed in
     * should be a member of iBase_StorageOrder enum.
     * \param instance FBiGeom instance handle
     * \param entity_handles Entity handles being queried
     * \param enttiy_handles_size Number of entities being queried
     * \param storage_order Storage order of coordinates passed back
     * \param min_corner Minimum coordinates of bounding boxes
     * \param min_corner_allocated Allocated size of minimum coordinates array
     * \param min_corner_size Occupied size of minimum coordinates array
     * \param max_corner Maximum coordinates of bounding boxes
     * \param max_corner_allocated Allocated size of maximum coordinates array
     * \param max_corner_size Occupied size of maximum coordinates array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getArrBoundBox( FBiGeom_Instance instance,
                             iBase_EntityHandle const* entity_handles,
                             int entity_handles_size,
                             int storage_order,
                             double** min_corner,
                             int* min_corner_allocated,
                             int* min_corner_size,
                             double** max_corner,
                             int* max_corner_allocated,
                             int* max_corner_size,
                             int* err );

    /**\brief  Get coordinates of specified vertex
     *
     * Get coordinates of specified vertex.
     * \param instance FBiGeom instance handle
     * \param vertex_handle Geom vertex being queried
     * \param *x Pointer to x coordinate returned from function
     * \param *y Pointer to y coordinate returned from function
     * \param *z Pointer to z coordinate returned from function
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getVtxCoord( FBiGeom_Instance instance,
                          iBase_EntityHandle vertex_handle,
                          double* x,
                          double* y,
                          double* z,
                          int* err );

    /**\brief  Get coordinates of specified vertices
     *
     * Get coordinates of specified vertices.  If storage order is passed in
     * with a value other than iBase_UNDETERMINED, coordinates are returned
     * in the specified storage order, otherwise storage order is that native
     * to the implementation.  Storage order of returned coordinates is also
     * returned.
     * \param instance FBiGeom instance handle
     * \param vertex_handles Array of geom vertex handles whose coordinates are
     *        being requested
     * \param vertex_handles_size Number of vertices in vertex_handles array
     * \param storage_order Storage order requested for coordinate data
     * \param *coords Pointer to array of coordinates returned from function
     * \param *coords_allocated Pointer to allocated size of coords array
     * \param *coords_size Pointer to occupied size of coords array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getVtxArrCoords( FBiGeom_Instance instance,
                              iBase_EntityHandle const* entity_handles,
                              int entity_handles_size,
                              int storage_order,
                              double** coordinates,
                              int* coordinates_allocated,
                              int* coordinates_size,
                              int* err );

    /**\brief  Intersect a ray with the model
     *
     * Intersect a ray with the model.  Storage orders passed in should be a
     * member of the iBase_StorageOrder enumeration.
     * \param instance FBiGeom instance handle
     * \param x Point from which ray is fired
     * \param y Point from which ray is fired
     * \param z Point from which ray is fired
     * \param dir_x Direction in which ray is fired
     * \param dir_y Direction in which ray is fired
     * \param dir_z Direction in which ray is fired
     * \param intersect_entity_handles Entities intersected by ray
     * \param intersect_entity_handles_allocated Allocated size of
     *        intersections array
     * \param intersect_entity_hangles_size Occupied size of intersections array
     * \param storage_order Storage order of coordinates passed back
     * \param intersect_coords Coordinates of intersections
     * \param intersect_coords_allocated Allocated size of coordinates array
     * \param intersect_coords_size Occupied size of coordinates array
     * \param param_coords Distances along ray of intersections
     * \param param_coords_allocated Allocated size of param_coords array
     * \param param_coords_size Occupied size of param_coords array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getPntRayIntsct( FBiGeom_Instance instance,
                              double x,
                              double y,
                              double z,
                              double dir_x,
                              double dir_y,
                              double dir_z,
                              iBase_EntityHandle** intersect_entity_handles,
                              int* intersect_entity_handles_allocated,
                              int* intersect_entity_hangles_size,
                              int storage_order,
                              double** intersect_coords,
                              int* intersect_coords_allocated,
                              int* intersect_coords_size,
                              double** param_coords,
                              int* param_coords_allocated,
                              int* param_coords_size,
                              int* err );

    /**\brief  Intersect an array of rays with the model
     *
     * Intersect an array of rays with the model.  Storage order passed in is
     * a member of the iBase_StorageOrder enumeration.
     * \param instance FBiGeom instance handle
     * \param storage_order Storage order of input coordinates
     * \param coords Points from which rays are fired
     * \param coords_size Number of points from which rays are fired
     * \param directions Directions in which rays are fired
     * \param directions_size Number of coordinates in directions array
     * \param intersect_entity_handles Entities intersected by ray
     * \param intersect_entity_handles_allocated Allocated size of intersections
     *        array
     * \param intersect_entity_hangles_size Occupied size of intersections array
     * \param offset Offset[i] is offset into intersect_entity_handles of ith
     *        ray
     * \param offset_allocated Allocated size of offset array
     * \param offset_size Occupied size of offset array
     * \param storage_order Storage order of coordinates passed back
     * \param intersect_coords Coordinates of intersections
     * \param intersect_coords_allocated Allocated size of coordinates array
     * \param intersect_coords_size Occupied size of coordinates array
     * \param param_coords Distances along ray of intersections
     * \param param_coords_allocated Allocated size of param_coords array
     * \param param_coords_size Occupied size of param_coords array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getPntArrRayIntsct( FBiGeom_Instance instance,
                                 int storage_order,
                                 const double* coords,
                                 int coords_size,
                                 const double* directions,
                                 int directions_size,
                                 iBase_EntityHandle** intersect_entity_handles,
                                 int* intersect_entity_handles_allocated,
                                 int* intersect_entity_hangles_size,
                                 int** offset,
                                 int* offset_allocated,
                                 int* offset_size,
                                 double** intersect_coords,
                                 int* intersect_coords_allocated,
                                 int* intersect_coords_size,
                                 double** param_coords,
                                 int* param_coords_allocated,
                                 int* param_coords_size,
                                 int* err );

    /**\brief  Get the entity on which a point is located
     *
     * Get the entity on which a point is located
     * \param instance FBiGeom instance handle
     * \param x Point being queried
     * \param y Point being queried
     * \param z Point being queried
     * \param entity_handle Entity on which point is located
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getPntClsf( FBiGeom_Instance instance,
                         double x,
                         double y,
                         double z,
                         iBase_EntityHandle* entity_handle,
                         int* err );

    /**\brief  Get the entities on which points are located
     *
     * Get the entities on which points are located.  Storage orders should be
     * members of the iBase_StorageOrder enumeration.
     * \param instance FBiGeom instance handle
     * \param storage_order Storage order of coordinates in coords
     * \param coords Points being queried
     * \param coords_size Number of entries in coords array
     * \param entity_handles Entities on which points are located
     * \param entity_handles_allocated Allocated size of entity_handles array
     * \param entity_handles_size Occupied size of entity_handles array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getPntArrClsf( FBiGeom_Instance instance,
                            int storage_order,
                            double const* coords,
                            int coords_size,
                            iBase_EntityHandle** entity_handles,
                            int* entity_handles_allocated,
                            int* entity_handles_size,
                            int* err );

    /**\brief  Get the sense of a face with respect to a region
     *
     * Get the sense of a face with respect to a region.  Sense returned is -1,
     * 0, or 1, representing "reversed", "both", or "forward".  "both" sense
     * indicates that face bounds the region once with each sense.
     * \param instance FBiGeom instance handle
     * \param face Face being queried
     * \param region Region being queried
     * \param sense_out Sense of face with respect to region
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getEntNrmlSense( FBiGeom_Instance instance,
                              iBase_EntityHandle face,
                              iBase_EntityHandle region,
                              int* sense_out,
                              int* err );

    /**\brief  Get the senses of an array of faces with respect to an array of
     *         regions
     *
     * Get the senses of an array of faces with respect to an array of regions.
     * Sense returned is -1, 0, or 1, representing "reversed", "both", or
     * "forward".  "both" sense indicates that face bounds the region once with
     * each sense.
     * \param instance FBiGeom instance handle
     * \param face_handles Faces being queried
     * \param face_handles_size Size of face handles array
     * \param region_handles Regions being queried
     * \param region_handles_size Size of region handles array
     * \param sense Senses of faces with respect to regions
     * \param sense_allocated Allocated size of senses array
     * \param sense_size Occupied size of senses array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getArrNrmlSense( FBiGeom_Instance instance,
                              iBase_EntityHandle const* face_handles,
                              int face_handles_size,
                              iBase_EntityHandle const* region_handles,
                              int region_handles_size,
                              int** sense,
                              int* sense_allocated,
                              int* sense_size,
                              int* err );

    /**\brief  Get the sense of an edge with respect to a face
     *
     * Get the sense of an edge with respect to a face.  Sense returned is -1,
     * 0, or 1, representing "reversed", "both", or "forward".  "both" sense
     * indicates that edge bounds the face once with each sense.
     * \param instance FBiGeom instance handle
     * \param edge Edge being queried
     * \param face Face being queried
     * \param sense_out Sense of edge with respect to face
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getEgFcSense( FBiGeom_Instance,
                           iBase_EntityHandle edge,
                           iBase_EntityHandle face,
                           int* sense_out,
                           int* err );

    /**\brief  Get the senses of an array of edges with respect to an array of
     *         faces
     *
     * Get the senses of an array of edges with respect to an array of faces.
     * Sense returned is -1, 0, or 1, representing "reversed", "both", or
     * "forward".  "both" sense indicates that edge bounds the face once with
     * each sense.
     * \param instance FBiGeom instance handle
     * \param edge_handles Edges being queried
     * \param edge_handles_size Size of edge handles array
     * \param face_handles Faces being queried
     * \param face_handles_size Size of face handles array
     * \param sense Senses of faces with respect to regions
     * \param sense_allocated Allocated size of senses array
     * \param sense_size Occupied size of senses array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getEgFcArrSense( FBiGeom_Instance instance,
                              iBase_EntityHandle const* edge_handles,
                              int edge_handles_size,
                              iBase_EntityHandle const* face_handles,
                              int face_handles_size,
                              int** sense,
                              int* sense_allocated,
                              int* sense_size,
                              int* err );

    /**\brief  Get the sense of a vertex pair with respect to an edge
     *
     * Get the sense of a vertex pair with respect to an edge.  Sense returned
     * is -1, 0, or 1, representing "reversed", "both", or "forward".  "both"
     * sense indicates that vertices are identical and that vertex bounds both
     * sides of the edge.
     * \param instance FBiGeom instance handle
     * \param edge Edge being queried
     * \param vertex1 First vertex being queried
     * \param vertex2 Second vertex being queried
     * \param sense_out Sense of vertex pair with respect to edge
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getEgVtxSense( FBiGeom_Instance instance,
                            iBase_EntityHandle edge,
                            iBase_EntityHandle vertex1,
                            iBase_EntityHandle vertex2,
                            int* sense_out,
                            int* err );

    /**\brief  Get the senses of vertex pair with respect to a edges
     *
     * Get the senses of vertex pairs with respect to edges.  Sense returned is
     * -1, 0, or 1, representing "reversed", "both", or "forward".  "both" sense
     * indicates that both vertices in pair are identical and that vertex bounds
     * both sides of the edge.
     * \param instance FBiGeom instance handle
     * \param edge_handles Edges being queried
     * \param edge_handles_size Number of edges being queried
     * \param vertex_handles_1 First vertex being queried
     * \param vertex_handles_1_size Number of vertices in vertices array
     * \param vertex_handles_2 Second vertex being queried
     * \param vertex_handles_2_size Number of vertices in vertices array
     * \param sense Sense of vertex pair with respect to edge
     * \param sense_allocated Allocated size of sense array
     * \param sense_size Occupied size of sense array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getEgVtxArrSense( FBiGeom_Instance instance,
                               iBase_EntityHandle const* edge_handles,
                               int edge_handles_size,
                               iBase_EntityHandle const* vertex_handles_1,
                               int veretx_handles_1_size,
                               iBase_EntityHandle const* vertex_handles_2,
                               int vertex_handles_2_size,
                               int** sense,
                               int* sense_allocated,
                               int* sense_size,
                               int* err );

    /**\brief  Return the measure (length, area, or volume) of entities
     *
     * Return the measure (length, area, or volume) of entities
     * \param instance FBiGeom instance handle
     * \param entity_handles Array of entities being queried
     * \param entity_handles_size Number of entities in entity array
     * \param measures Measures of entities being queried
     * \param measures_allocated Allocated size of measures array
     * \param measures_size Occupied size of measures array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_measure( FBiGeom_Instance instance,
                      iBase_EntityHandle const* entity_handles,
                      int entity_handles_size,
                      double** measures,
                      int* measures_allocated,
                      int* measures_size,
                      int* err );

    /**\brief  Get the geometric type of a face
     *
     * Get the geometric type of a face.  Specific types depend on
     * implementation.
     * \param instance FBiGeom instance handle
     * \param face_handle Face being queried
     * \param face_type Face type
     * \param face_type_length Length of face type string
     */
  void FBiGeom_getFaceType( FBiGeom_Instance instance,
                          iBase_EntityHandle face_handle,
                          char* face_type,
                          int* err,
                          int* face_type_length);

    /**\brief  Return whether interface has information about parameterization
     *
     * Return whether an interface has information about parameterization (!=0)
     * or not (0)
     * \param instance FBiGeom instance handle
     * \param is_parametric If non-zero, interface has information about
     *        parameterization
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getParametric( FBiGeom_Instance instance,
                            int* is_parametric,
                            int* err );

    /**\brief  Return whether an entity has a parameterization
     *
     * Return whether an entity has a parameterization (=1) or not (=0)
     * \param instance FBiGeom instance handle
     * \param entity_handle Entity being queried
     * \param is_parametric Entity has a parameterization (=1) or not (=0)
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_isEntParametric( FBiGeom_Instance instance,
                              iBase_EntityHandle entity_handle,
                              int* parametric,
                              int* err );

    /**\brief  Return whether entities have parameterizations
     *
     * Return whether entities have parameterizations (=1) or not (=0)
     * \param instance FBiGeom instance handle
     * \param entity_handles Entities being queried
     * \param entity_handles_size Number of entities being queried
     * \param is_parametric entity_handles[i] has a parameterization (=1) or
     *        not (=0)
     * \param is_parametric_allocated Allocated size of is_parametric array
     * \param is_parametric_size Occupied size of is_parametric array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_isArrParametric( FBiGeom_Instance instance,
                              iBase_EntityHandle const* entity_handles,
                              int entity_handles_size,
                              int** is_parametric,
                              int* is_parametric_allocated,
                              int* is_parametric_size,
                              int* err );

    /**\brief  Return coordinate position at specified parametric position on
     *         entity
     *
     * Return coordinate position at specified parametric position on entity.
     * \param instance FBiGeom instance handle
     * \param entity_handle Entity being queried
     * \param u Parametric coordinate being queried
     * \param v Parametric coordinate being queried
     * \param x Spatial coordinate at parametric position being queried
     * \param y Spatial coordinate at parametric position being queried
     * \param z Spatial coordinate at parametric position being queried
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getEntUVtoXYZ( FBiGeom_Instance instance,
                            iBase_EntityHandle entity_handle,
                            double u,
                            double v,
                            double* x,
                            double* y,
                            double* z,
                            int* err );

    /**\brief  Return coordinate positions at specified parametric position(s)
     *         on entity(ies)
     *
     * Return coordinate positions at specified parametric position(s) on
     * entity(ies).  If either the number of entities or number of parametric
     * coordinate pairs is unity, then all points or entities are queried for
     * that entity or point, respectively, otherwise each point corresponds to
     * each entity.  storage_order should be a value in the iBase_StorageOrder
     * enum.
     * \param instance FBiGeom instance handle
     * \param entity_handles Entities being queried
     * \param entity_handles_size Number of entities being queried
     * \param storage_order Storage order of uv coordinates input and xyz
     *        coordinate output
     * \param uv Coordinates being queried
     * \param uv_size Number of coordinates in array
     * \param coordinates Coordinates of parametric positions
     * \param coordinates_allocated Allocated size of coordinates array
     * \param coordinates_size Occupied size of coordinates array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getArrUVtoXYZ( FBiGeom_Instance instance,
                            iBase_EntityHandle const* entity_handles,
                            int entity_handles_size,
                            int storage_order,
                            double const* uv,
                            int uv_size,
                            double** coordinates,
                            int* coordinates_allocated,
                            int* coordinates_size,
                            int* err );

    /**\brief  Return coordinate position at specified parametric position on
     *         entity
     *
     * Return coordinate position at specified parametric position on entity.
     * \param instance FBiGeom instance handle
     * \param entity_handle Entity being queried
     * \param u Parametric coordinate being queried
     * \param x Spatial coordinate at parametric position being queried
     * \param y Spatial coordinate at parametric position being queried
     * \param z Spatial coordinate at parametric position being queried
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getEntUtoXYZ( FBiGeom_Instance instance,
                           iBase_EntityHandle entity_handle,
                           double u,
                           double* x, 
                           double* y,
                           double* z,
                           int* err );

    /**\brief  Return coordinate positions at specified parametric position(s)
     *         on entity(ies)
     *
     * Return coordinate positions at specified parametric position(s) on
     * entity(ies). If either the number of entities or number of parametric
     * coordinate pairs is unity, then all points or entities are queried for
     * that entity or point, respectively, otherwise each point corresponds to
     * each entity.  storage_order should be a value in the iBase_StorageOrder
     * enum.
     * \param instance FBiGeom instance handle
     * \param entity_handles Entities being queried
     * \param entity_handles_size Number of entities being queried
     * \param storage_order Storage order of resulting coordinates
     * \param uv Coordinates being queried
     * \param uv_size Number of coordinates in array
     * \param coordinates Coordinates of parametric positions
     * \param coordinates_allocated Allocated size of coordinates array
     * \param coordinates_size Occupied size of coordinates array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getArrUtoXYZ( FBiGeom_Instance instance,
                           iBase_EntityHandle const* entity_handles,
                           int entity_handles_size,
                           double const* u,
                           int u_size,
                           int storage_order,
                           double** on_coords,
                           int* on_coords_allocated,
                           int* on_coords_size,
                           int* err );

    /**\brief  Return parametric position at specified spatial position on
     *         entity
     *
     * Return parametric position at specified spatial position on entity
     * \param instance FBiGeom instance handle
     * \param entity_handle Entity being queried
     * \param x Spatial coordinate being queried
     * \param y Spatial coordinate being queried
     * \param z Spatial coordinate being queried
     * \param u Parametric coordinate at spatial position being queried
     * \param v Parametric coordinate at spatial position being queried
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getEntXYZtoUV( FBiGeom_Instance instance,
                            iBase_EntityHandle entity_handle,
                            double x,
                            double y,
                            double z,
                            double* u,
                            double* v, 
                            int* err );

    /**\brief  Return parametric position at specified spatial position on
     *         entity
     *
     * Return parametric position at specified spatial position on entity
     * \param instance FBiGeom instance handle
     * \param entity_handle Entity being queried
     * \param x Spatial coordinate being queried
     * \param y Spatial coordinate being queried
     * \param z Spatial coordinate being queried
     * \param u Parametric coordinate at spatial position being queried
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getEntXYZtoU( FBiGeom_Instance instance,
                           iBase_EntityHandle entity_handle,
                           double x,
                           double y,
                           double z,
                           double* u,
                           int* err );

    /**\brief  Return parametric positions at specified spatial position(s) on
     *         entity(ies)
     * Return parametric positions at specified spatial position(s) on
     * entity(ies).  If either the number of entities or number of spatial
     * coordinate triples is unity, then all points or entities are queried for
     * that entity or point, respectively, otherwise each point corresponds to
     * each entity.  storage_order should be a value in the iBase_StorageOrder
     * enum.
     * \param instance FBiGeom instance handle
     * \param entity_handles Entities being queried
     * \param entity_handles_size Number of entities being queried
     * \param storage_order Storage order of spatial coordinates input
     * \param coordinates Coordinates being queried
     * \param coordinates_size Number of coordinates in array
     * \param uv Coordinates of parametric positions
     * \param uv_allocated Allocated size of coordinates array
     * \param uv_size Occupied size of coordinates array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getArrXYZtoUV( FBiGeom_Instance instance,
                            iBase_EntityHandle const* entity_handles,
                            int entity_handles_size,
                            int storage_order,
                            double const* coordinates,
                            int coordinates_size,
                            double** uv,
                            int* uv_allocated,
                            int* uv_size,
                            int* err );

    /**\brief  Return spatial positions at specified parametric position(s) on
     *         entity(ies)
     *
     * Return spatial positions at specified parametric position(s) on
     * entity(ies). If either the number of entities or number of spatial
     * coordinate triples is unity, then all points or entities are queried for
     * that entity or point, respectively, otherwise each point corresponds to
     * each entity.  storage_order should be a value in the iBase_StorageOrder
     * enum.
     * \param instance FBiGeom instance handle
     * \param entity_handles Entities being queried
     * \param entity_handles_size Number of entities being queried
     * \param storage_order Storage order of spatial coordinates input
     * \param coordinates Coordinates being queried
     * \param coordinates_size Number of coordinates in array
     * \param u Coordinates of parametric positions
     * \param u_allocated Allocated size of coordinates array
     * \param u_size Occupied size of coordinates array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getArrXYZtoU( FBiGeom_Instance instance,
                           iBase_EntityHandle const* entity_handles,
                           int entity_handles_size,
                           int storage_order,
                           double const* coordinates,
                           int coordinates_size,
                           double** u,
                           int* u_allocated,
                           int* u_size,
                           int* err );

    /**\brief  Return parametric position at specified spatial position on
     *         entity, based on parametric position hint
     *
     * Return parametric position at specified spatial position on entity,
     * based on parametric position hint.  For this function, u and v are input
     * with parameters from which to start search. Typically this will reduce
     * the search time for new parametric coordinates.
     * \param instance FBiGeom instance handle
     * \param entity_handle Entity being queried
     * \param x Spatial coordinate being queried
     * \param y Spatial coordinate being queried
     * \param z Spatial coordinate being queried
     * \param u Parametric coordinate at spatial position being queried
     * \param v Parametric coordinate at spatial position being queried
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getEntXYZtoUVHint( FBiGeom_Instance instance,
                                iBase_EntityHandle entity_handle,
                                double x,
                                double y,
                                double z,
                                double* u,
                                double* v,
                                int* err );

    /**\brief  Return parametric positions at specified spatial position(s) on
     *         entity(ies), based on parametric position hints
     * Return parametric positions at specified spatial position(s) on
     * entity(ies), based on parametric position hints.  If either the number of
     * entities or number of spatial coordinate triples is unity, then all
     * points or entities are queried for that entity or point, respectively,
     * otherwise each point corresponds to each entity.  storage_order should be
     * a value in the iBase_StorageOrder enum.
     * \param instance FBiGeom instance handle
     * \param entity_handles Entities being queried
     * \param entity_handles_size Number of entities being queried
     * \param storage_order Storage order of spatial coordinates input
     * \param coordinates Coordinates being queried
     * \param coordinates_size Number of coordinates in array
     * \param uv Coordinates of parametric positions
     * \param uv_allocated Allocated size of coordinates array
     * \param uv_size Occupied size of coordinates array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getArrXYZtoUVHint( FBiGeom_Instance instance,
                                iBase_EntityHandle const* entity_handles,
                                int entity_handles_size,
                                int storage_order,
                                double const* coords,
                                int coords_size,
                                double** uv,
                                int* uv_allocated,
                                int* uv_size,
                                int* err );

    /**\brief  Get parametric range of entity
     *
     * Get parametric range of entity
     * \param instance FBiGeom instance handle
     * \param entity_handle Entity being queried
     * \param u_min Minimum parametric coordinate for entity
     * \param v_min Minimum parametric coordinate for entity
     * \param u_max Maximum parametric coordinate for entity
     * \param v_max Maximum parametric coordinate for entity
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getEntUVRange( FBiGeom_Instance instance,
                            iBase_EntityHandle entity_handle,
                            double* u_min,
                            double* v_min,
                            double* u_max,
                            double* v_max,
                            int* err );

    /**\brief  Get parametric range of entity
     *
     * Get parametric range of entity
     * \param instance FBiGeom instance handle
     * \param entity_handle Entity being queried
     * \param u_min Minimum parametric coordinate for entity
     * \param u_max Maximum parametric coordinate for entity
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getEntURange( FBiGeom_Instance instance,
                           iBase_EntityHandle entity_handle,
                           double* u_min,
                           double* u_max,
                           int* err );

    /**\brief  Get parametric range of entities
     *
     * Get parametric range of entities
     * \param instance FBiGeom instance handle
     * \param entity_handles Entities being queried
     * \param entity_handles_size Number of entities being queried
     * \param storage_order Storage order of parametric coordinates being
     *        returned
     * \param uv_min Minimum parametric coordinate for entities
     * \param uv_min_allocated Allocated size of minimum parametric coordinate
     *        array
     * \param uv_min_size Occupied size of minimum parametric coordinate array
     * \param uv_max Maximum parametric coordinate for entities
     * \param uv_max_allocated Allocated size of maximum parametric coordinate
     *        array
     * \param uv_max_size Occupied size of maximum parametric coordinate array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getArrUVRange( FBiGeom_Instance instance,
                            iBase_EntityHandle const* entity_handles,
                            int entity_handles_size,
                            int storage_order,
                            double** uv_min,
                            int* uv_min_allocated,
                            int* uv_min_size,
                            double** uv_max,
                            int* uv_max_allocated,
                            int* uv_max_size,
                            int* err );

    /**\brief  Get parametric range of entities
     *
     * Get parametric range of entities
     * \param instance FBiGeom instance handle
     * \param entity_handles Entities being queried
     * \param entity_handles_size Number of entities being queried
     * \param storage_order Storage order of parametric coordinates being
     *        returned
     * \param u_min Minimum parametric coordinate for entities
     * \param u_min_allocated Allocated size of minimum parametric coordinate
     *        array
     * \param u_min_size Occupied size of minimum parametric coordinate array
     * \param u_max Maximum parametric coordinate for entities
     * \param u_max_allocated Allocated size of maximum parametric coordinate
     *        array
     * \param u_max_size Occupied size of maximum parametric coordinate array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getArrURange( FBiGeom_Instance instance,
                           iBase_EntityHandle const* entity_handles,
                           int entity_handles_size,
                           double** u_min,
                           int* u_min_allocated,
                           int* u_min_size,
                           double** u_max,
                           int* u_max_allocated,
                           int* u_max_size,
                           int* err );

    /**\brief  Return the face parametric coordinates for a parametric position
     *         on a bounding edge
     *
     * Return the face parametric coordinates for a parametric position on a
     * bounding edge
     * \param instance FBiGeom instance handle
     * \param edge_handle Edge being queried
     * \param face_handle Face being queried
     * \param in_u Parametric position on edge
     * \param u Corresponding parametric position on face
     * \param v Corresponding parametric position on face
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getEntUtoUV( FBiGeom_Instance instance,
                          iBase_EntityHandle edge_handle,
                          iBase_EntityHandle face_handle,
                          double in_u,
                          double* u,
                          double* v,
                          int* err );

    /**\brief  Return parametric coordinates on face of vertex position
     *
     * Return parametric coordinates on face of vertex position
     * \param instance FBiGeom instance handle
     * \param vertex_handle Vertex being queried
     * \param face_handle Face being queried
     * \param u Corresponding parametric position on face
     * \param v Corresponding parametric position on face
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getVtxToUV( FBiGeom_Instance instance,
                         iBase_EntityHandle vertex_handle,
                         iBase_EntityHandle face_handle,
                         double* u,
                         double* v,
                         int* err );

    /**\brief  Return parametric coordinates on edge of vertex position
     *
     * Return parametric coordinates on edge of vertex position
     * \param instance FBiGeom instance handle
     * \param vertex_handle Vertex being queried
     * \param edge_handle Edge being queried
     * \param u Corresponding parametric position on face
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getVtxToU( FBiGeom_Instance instance,
                        iBase_EntityHandle vertex_handle,
                        iBase_EntityHandle edge_handle,
                        double* u,
                        int* err );

    /**\brief  Return the face parametric coordinates for a parametric position
     *         on bounding edges
     *
     * Return the face parametric coordinates for a parametric position on
     * bounding edges
     * \param instance FBiGeom instance handle
     * \param edge_handles Edges being queried
     * \param edge_handles_size Number of edges being queried
     * \param face_handles Faces being queried
     * \param face_handles_size Number of faces being queried
     * \param u_in Parametric positions on edges
     * \param u_in_size Number of parametric positions on edges
     * \param storage_order Storage order of coordinates returned
     * \param uv Corresponding parametric positions on faces
     * \param uv_allocated Allocated size of parameter array
     * \param uv_size Occupied size of parameter array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getArrUtoUV( FBiGeom_Instance instance,
                          iBase_EntityHandle const* edge_handles,
                          int edge_handles_size,
                          iBase_EntityHandle const* face_handles,
                          int face_handles_size,
                          double const* u_in,
                          int u_in_size,
                          int storage_order,
                          double** uv,
                          int* uv_allocated,
                          int* uv_size,
                          int* err );

    /**\brief  Return parametric coordinates on faces of vertex positions
     *
     * Return parametric coordinates on faces of vertex positions
     * \param instance FBiGeom instance handle
     * \param vertex_handles Vertices being queried
     * \param vertex_handles_size Number of vertices being queried
     * \param face_handles Faces being queried
     * \param face_handles_size Number of faces being queried
     * \param storage_order Storage order of coordinates returned
     * \param uv Corresponding parametric positions on faces
     * \param uv_allocated Allocated size of positions array
     * \param uv_size Occupied size of positions array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getVtxArrToUV( FBiGeom_Instance instance,
                            iBase_EntityHandle const* vertex_handles,
                            int vertex_handles_size,
                            iBase_EntityHandle const* face_handles,
                            int face_handles_size,
                            int storage_order,
                            double** uv,
                            int* uv_allocated,
                            int* uv_size,
                            int* err );

    /**\brief  Return parametric coordinates on edges of vertex positions
     *
     * Return parametric coordinates on edges of vertex positions
     * \param instance FBiGeom instance handle
     * \param vertex_handles Vertices being queried
     * \param vertex_handles_size Number of vertices being queried
     * \param edge_handles Edges being queried
     * \param edge_handles_size Number of edges being queried
     * \param u Corresponding parametric positions on faces
     * \param u_allocated Allocated size of positions array
     * \param u_size Occupied size of positions array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getVtxArrToU( FBiGeom_Instance instance,
                           iBase_EntityHandle const* vertex_handles,
                           int vertex_handles_size,
                           iBase_EntityHandle const* edge_handles,
                           int edge_handles_size,
                           double** u,
                           int* u_allocated,
                           int* u_size,
                           int* err );

    /**\brief  Return the normal at a specified parametric position
     *
     * Return the normal at a specified parametric position
     * \param instance FBiGeom instance handle
     * \param entity_handle Entity being queried
     * \param u Parametric position being queried
     * \param v Parametric position being queried
     * \param nrml_i Normal at specified position
     * \param nrml_j Normal at specified position
     * \param nrml_k Normal at specified position
     * \param *err Pointer to error type returned from functino
     */
  void FBiGeom_getEntNrmlUV( FBiGeom_Instance instance,
                           iBase_EntityHandle entity_handle,
                           double u,
                           double v,
                           double* nrml_i,
                           double* nrml_j,
                           double* nrml_k,
                           int* err );

    /**\brief  Return the normals at specified parametric positions
     *
     * Return the normals at specified parametric positions.  If either the
     * number of entities or number of spatial coordinate pairs is unity, then
     * all points or entities are queried for that entity or point,
     * respectively, otherwise each point corresponds to each entity.
     * storage_order should be a value in the iBase_StorageOrder enum.
     * \param instance FBiGeom instance handle
     * \param face_handle Faces being queried
     * \param face_handles_size Number of faces being queried
     * \param storage_order Storage order of coordinates input and output
     * \param parameters Parametric coordinates being queried
     * \param parameters_size Number of coordinates in array
     * \param normals Coordinates of normals at specified positions
     * \param normals_allocated Allocated size of normals array
     * \param normals_size Occupied size of normals array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getArrNrmlUV( FBiGeom_Instance instance,
                           iBase_EntityHandle const* face_handles,
                           int face_handles_size,
                           int storage_order,
                           double const* parameters,
                           int parameters_size,
                           double** normals,
                           int* normals_allocated,
                           int* normals_size,
                           int* err );

    /**\brief  Return the tangent at a specified parametric position
     *
     * Return the tangent at a specified parametric position
     * \param instance FBiGeom instance handle
     * \param entity_handle Entity being queried
     * \param u Parametric position being queried
     * \param tgnt_i Tangent at specified position
     * \param tgnt_j Tangent at specified position
     * \param tgnt_k Tangent at specified position
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getEntTgntU( FBiGeom_Instance instance,
                          iBase_EntityHandle entity_handle,
                          double u,
                          double* tgnt_i,
                          double* tgnt_j,
                          double* tgnt_k,
                          int* err );

    /**\brief  Return the tangents at specified parametric positions
     *
     * Return the tangents at specified parametric positions.  If either the
     * number of entities or number of spatial coordinates is unity, then all
     * points or entities are queried for that entity or point, respectively,
     * otherwise each point corresponds to each entity.  storage_order should be
     * a value in the iBase_StorageOrder enum.
     * \param instance FBiGeom instance handle
     * \param edge_handle Edges being queried
     * \param edge_handles_size Number of faces being queried
     * \param storage_order Storage order of coordinates output
     * \param parameters Parametric coordinates being queried
     * \param parameters_size Number of coordinates in array
     * \param tangents Coordinates of tangents at specified positions
     * \param tangents_allocated Allocated size of tangents array
     * \param tangents_size Occupied size of tangents array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getArrTgntU( FBiGeom_Instance instance,
                          iBase_EntityHandle const* edge_handles,
                          int edge_handles_size,
                          int storage_order,
                          double const* parameters,
                          int parameters_size,
                          double** tangents,
                          int* tangents_allocated,
                          int* tangents_size,
                          int* err );

    /**\brief  Get the first derivative of a face at specified parametric
     *         position
     *
     * Get the first derivative of a face at specified parametric position.
     * \param instance FBiGeom instance handle
     * \param entity_handle Entity being queried
     * \param u Parametric position being queried
     * \param v Parametric position being queried
     * \param dvrt_u Pointer to coordinates of derivative with respect to u at
     *        specified position returned from function
     * \param dvrt_u_allocated Allocated size of dvrt_u array
     * \param dvrt_u_size Occupied size of dvrt_u array
     * \param dvrt_v Pointer to coordinates of derivative with respect to v at
     *        specified position returned from function
     * \param dvrt_v_allocated Allocated size of dvrt_v array
     * \param dvrt_v_size Occupied size of dvrt_v array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getEnt1stDrvt( FBiGeom_Instance instance,
                            iBase_EntityHandle entity_handle,
                            double u,
                            double v,
                            double** drvt_u,
                            int* drvt_u_allocated,
                            int* drvt_u_size,
                            double** drvt_v,
                            int* dvrt_v_allocated,
                            int* dvrt_v_size,
                            int* err );

    /**\brief  Get the first derivatives of faces at specified parametric
     *         positions
     *
     * Get the first derivatives of faces at specified parametric positions.
     * storage_order should be a value in the iBase_StorageOrder enum.
     * \param instance FBiGeom instance handle
     * \param entity_handles Array of entity handles being queried
     * \param entity_handles_size Number of entities in entity_handles array
     * \param storage_order Storage order of coordinates input and output
     * \param uv Parametric coordinates being queried
     * \param uv_size Number of coordinates in array
     * \param dvrt_u Pointer to array of coordinates of derivative with respect
     *        to u at specified position returned from function
     * \param dvrt_u_allocated Allocated size of dvrt_u array
     * \param dvrt_u_size Occupied size of dvrt_u array
     * \param u_offset Pointer to array of offsets for dvrt_u returned from
     *        function
     * \param u_offset_allocated Pointer to allocated size of u_offset array
     * \param u_offset_size Pointer to occupied size of u_offset array
     * \param dvrt_v Pointer to array of coordinates of derivative with respect
     *        to v at specified position returned from function
     * \param dvrt_v_allocated Allocated size of dvrt_v array
     * \param dvrt_v_size Occupied size of dvrt_v array
     * \param v_offset Pointer to array of offsets for dvrt_v returned from
     *        function
     * \param v_offset_allocated Pointer to allocated size of v_offset array
     * \param v_offset_size Pointer to occupied size of v_offset array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getArr1stDrvt( FBiGeom_Instance instance,
                            iBase_EntityHandle const* entity_handles,
                            int entity_handles_size,
                            int storage_order,
                            double const* uv,
                            int uv_size,
                            double** dvrt_u,
                            int* dvrt_u_allocated,
                            int* dvrt_u_size,
                            int** u_offset,
                            int* u_offset_allocated,
                            int* u_offset_size,
                            double** dvrt_v,
                            int* dvrt_v_allocated,
                            int* dvrt_v_size,
                            int** v_offset,
                            int* v_offset_allocated,
                            int* v_offset_size,
                            int* err );

    /**\brief  Get the second derivative of a face at specified parametric
     *         position
     *
     * Get the second derivative of a face at specified parametric position.
     * \param instance FBiGeom instance handle
     * \param entity_handle Entity being queried
     * \param u Parametric position being queried
     * \param v Parametric position being queried
     * \param dvrt_uu Pointer to coordinates of derivative with respect to u at
     *        specified position returned from function
     * \param dvrt_uu_allocated Allocated size of dvrt_uu array
     * \param dvrt_uu_size Occupied size of dvrt_uu array
     * \param dvrt_vv Pointer to coordinates of derivative with respect to v at
     *        specified position returned from function
     * \param dvrt_vv_allocated Allocated size of dvrt_vv array
     * \param dvrt_vv_size Occupied size of dvrt_vv array
     * \param dvrt_uv Pointer to coordinates of derivative with respect to u and
     *        v at specified position returned from function
     * \param dvrt_uv_allocated Allocated size of dvrt_uv array
     * \param dvrt_uv_size Occupied size of dvrt_uv array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getEnt2ndDrvt( FBiGeom_Instance instance,
                            iBase_EntityHandle entity_handle,
                            double u,
                            double v,
                            double** drvt_uu,
                            int* drvt_uu_allocated,
                            int* drvt_uu_size,
                            double** drvt_vv,
                            int* dvrt_vv_allocated,
                            int* dvrt_vv_size,
                            double** drvt_uv,
                            int* dvrt_uv_allocated,
                            int* dvrt_uv_size,
                            int* err );

    /**\brief  Get the second derivatives of faces at specified parametric
     *         positions
     *
     * Get the second derivatives of faces at specified parametric positions.
     * storage_order should be a value in the iBase_StorageOrder enum.
     * \param instance FBiGeom instance handle
     * \param entity_handles Array of entity handles being queried
     * \param entity_handles_size Number of entities in entity_handles array
     * \param storage_order Storage order of coordinates input and output
     * \param uv Parametric coordinates being queried
     * \param uv_size Number of coordinates in array
     * \param dvrt_uu Pointer to array of coordinates of derivative with respect
     *        to u at specified position returned from function
     * \param dvrt_uu_allocated Allocated size of dvrt_uu array
     * \param dvrt_uu_size Occupied size of dvrt_uu array
     * \param uu_offset Pointer to array of offsets for dvrt_uu returned from
     *        function
     * \param uu_offset_allocated Pointer to allocated size of uu_offset array
     * \param uu_offset_size Pointer to occupied size of uu_offset array
     * \param dvrt_vv Pointer to array of coordinates of derivative with respect
     *        to v at specified position returned from function
     * \param dvrt_vv_allocated Allocated size of dvrt_vv array
     * \param dvrt_vv_size Occupied size of dvrt_vv array
     * \param vv_offset Pointer to array of offsets for dvrt_vv returned from
     *        function
     * \param vv_offset_allocated Pointer to allocated size of vv_offset array
     * \param vv_offset_size Pointer to occupied size of vv_offset array
     * \param dvrt_uv Pointer to array of coordinates of derivative with respect
     *        to u and v at specified position returned from function
     * \param dvrt_uv_allocated Allocated size of dvrt_uv array
     * \param dvrt_uv_size Occupied size of dvrt_uv array
     * \param uv_offset Pointer to array of offsets for dvrt_uv returned from
     *        function
     * \param uv_offset_allocated Pointer to allocated size of uv_offset array
     * \param uv_offset_size Pointer to occupied size of uv_offset array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getArr2ndDrvt( FBiGeom_Instance instance,
                            iBase_EntityHandle const* entity_handles,
                            int entity_handles_size,
                            int storage_order,
                            double const* uv,
                            int uv_size,
                            double** dvtr_uu,
                            int* dvrt_uu_allocated,
                            int* dvrt_uu_size,
                            int** uu_offset,
                            int* uu_offset_allocated,
                            int* uu_offset_size,
                            double** dvtr_vv,
                            int* dvrt_vv_allocated,
                            int* dvrt_vv_size,
                            int** vv_offset,
                            int* vv_offset_allocated,
                            int* vv_offset_size,
                            double** dvrt_uv,
                            int* dvrt_uv_allocated,
                            int* dvrt_uv_size,
                            int** uv_offset,
                            int* uv_offset_allocated,
                            int* uv_offset_size,
                            int* err );

    /**\brief  Get the two principle curvature vectors for a face at a
     *         parametric position
     *
     * Get the two principle curvature vectors for a face at a parametric
     * position.  Magnitudes of vectors are curvature, directions are
     * directions of principal curvatures.   
     * \param instance FBiGeom instance handle
     * \param face_handle Face being queried
     * \param u Parametric position being queried
     * \param v Parametric position being queried
     * \param cvtr1_i Maximum curvature vector
     * \param cvtr1_j Maximum curvature vector
     * \param cvtr1_k Maximum curvature vector
     * \param cvtr2_i Minimum curvature vector
     * \param cvtr2_j Minimum curvature vector
     * \param cvtr2_k Minimum curvature vector
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getFcCvtrUV( FBiGeom_Instance instance,
                          iBase_EntityHandle face_handle,
                          double u,
                          double v,
                          double* cvtr1_i,
                          double* cvtr1_j,
                          double* cvtr1_k,
                          double* cvtr2_i,
                          double* cvtr2_j,
                          double* cvtr2_k,
                          int* err );

    /**\brief  Get the curvature(s) on face(s) at given parametric position(s)
     *
     * Get the curvature(s) on face(s) at given parametric position(s).  If
     * either the number of faces or number of coordinate pairs is unity, then
     * all points or entities are queried for that entity or point,
     * respectively, otherwise each point corresponds to each entity.
     * storage_order should be a value in the iBase_StorageOrder enum.
     * \param instance FBiGeom instance handle
     * \param face_handles Face(s) being queried
     * \param face_handles_size Number of entities being queried
     * \param storage_order Storage order of uv coordinates
     * \param uv Starting parametric coordinates
     * \param uv_size Number of values in uv array
     * \param cvtr_1 First principal curvatures
     * \param cvtr_1_allocated Allocated size of first curvature array
     * \param cvtr_1_size Occupied size of first curvature array
     * \param cvtr_2 Second principal curvatures
     * \param cvtr_2_allocated Allocated size of second curvature array
     * \param cvtr_2_size Occupied size of second curvature array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getFcArrCvtrUV( FBiGeom_Instance instance,
                             iBase_EntityHandle const* face_handles,
                             int face_handles_size,
                             int storage_order,
                             double const* uv,
                             int uv_size,
                             double** cvtr_1,
                             int* cvtr_1_allocated,
                             int* cvtr_1_size,
                             double** cvtr_2,
                             int* cvtr_2_allocated,
                             int* cvtr_2_size,
                             int* err );

    /**\brief  Return whether an entity is periodic
     *
     * Return whether an entity is periodic (=1) or not (=0) in the u and v
     * directions.
     * \param instance FBiGeom instance handle
     * \param entity_handle Entity being queried
     * \param in_u Entity is periodic in u direction (=1) or not (=0)
     * \param in_v Entity is periodic in v direction (=1) or not (=0)
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_isEntPeriodic( FBiGeom_Instance instance,
                            iBase_EntityHandle entity_handle,
                            int* in_u,
                            int* in_v,
                            int* err );

    /**\brief  Return whether entities are periodic
     *
     * Return whether entities are periodic (=1) or not (=0) in the u and v
     * directions.
     * \param instance FBiGeom instance handle
     * \param entity_handles Entities being queried
     * \param entity_handles_size Number of entities being queried
     * \param in_uv Array of pairs of integers representing whether
     *        entity_handles[i] is periodic (=1) or not (=0) in u and v
     *        directions
     * \param in_uv_allocated Allocated size of in_uv array
     * \param in_uv_size Occupied size of in_uv array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_isArrPeriodic( FBiGeom_Instance instance,
                            iBase_EntityHandle const* entity_handles,
                            int entity_handles_size,
                            int** in_uv,
                            int* in_uv_allocated,
                            int* in_uv_size,
                            int* err );

    /**\brief  Return whether a face is degenerate
     *
     * Return whether a face is degenerate (=1) or not (=0).
     * \param instance FBiGeom instance handle
     * \param face_handle Face being queried
     * \param is_degenerate Face is degenerate (=1) or not (=0)
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_isFcDegenerate( FBiGeom_Instance instance,
                             iBase_EntityHandle face_handle,
                             int* is_degenerate,
                             int* err );

    /**\brief  Return whether faces are degenerate
     *
     * Return whether faces are degenerate (=1) or not (=0).
     * \param instance FBiGeom instance handle
     * \param face_handles Faces being queried
     * \param face_handles_size Number of faces being queried
     * \param degenerate face_handles[i] is degenerate (=1) or not (=0)
     * \param degenerate_allocated Allocated size of degenerate array
     * \param degenerate_size Occupied size of degenerate array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_isFcArrDegenerate( FBiGeom_Instance instance,
                                iBase_EntityHandle const* face_handles,
                                int face_handles_size,
                                int** degenerate,
                                int* degenerate_allocated,
                                int* degenerate_size,
                                int* err );

    /**\brief  Get the tolerance of the instance
     *
     * Get the tolerance at the modeler level.  type is an integer representing
     * the type of the tolerance, where 0 = no tolerance information,
     * 1 = modeler-level tolerance, 2 = entity-level tolerances.  If type is 1,
     * tolerance returns the modeler-level tolerance.  If type is 2, use
     * FBiGeom_getEntTolerance to query the tolerance on a per-entity basis.
     * \param instance FBiGeom instance handle
     * \param type Type of tolerance used by the modeler
     * \param tolerance Modeler-level tolerance, if any
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getTolerance( FBiGeom_Instance instance,
                           int* type,
                           double* tolerance,
                           int* err );

    /**\brief  Get the tolerance of the specified entity
     *
     * Get the tolerance of the specified entity.
     * \param instance FBiGeom instance handle
     * \param entity_handle Entity handle being queried
     * \param tolerance Pointer to tolerance returned from function
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getEntTolerance( FBiGeom_Instance instance,
                              iBase_EntityHandle entity_handle,
                              double* tolerance,
                              int* err );

    /**\brief  Get the tolerances of the specified entities
     *
     * Get the tolerances of the specified entities.
     * \param instance FBiGeom instance handle
     * \param entity_handles Array of entity handles being queried
     * \param entity_handles_size Number of entities in entity_handles array
     * \param tolerance Pointer to array of tolerances returned from function
     * \param tolerance_allocated Pointer to allocated size of tolerance array
     * \param tololerance_size Pointer to occupied size of tolerance array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getArrTolerance( FBiGeom_Instance instance,
                              iBase_EntityHandle const* entity_handles,
                              int entity_handles_size,
                              double** tolerances,
                              int* tolerances_allocated,
                              int* tolerances_size,
                              int* err );

    /**\brief  Initialize an iterator over specified entity type
     *
     * Initialize an iterator over specified entity type for a specified set or
     * instance.  Iterator returned can be used as input to functions returning
     * the entity for the iterator.  If all entities of a specified type are to
     * be iterated, specify iBase_ALL_TYPES.  Specified type must be a value in
     * the iBase_EntityType enumeration.
     * \param instance FBiGeom instance handle
     * \param entity_set_handle Entity set being iterated
     * \param requested_entity_type Type of entity to iterate
     * \param entity_iterator Pointer to iterator returned from function
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_initEntIter( FBiGeom_Instance instance,
                          iBase_EntitySetHandle entity_set_handle,
                          int requested_entity_type,
                          iBase_EntityIterator* entity_iterator,
                          int* err );

    /**\brief  Initialize an array iterator over specified entity type and size
     *
     * Initialize an array iterator over specified entity type and size for a
     * specified set or instance.  Iterator returned can be used as input to
     * functions returning entities for the iterator.  If all entities of a
     * specified type are to be iterated, specify iBase_ALL_TYPES. Specified
     * type must be a value in the iBase_EntityType enumerations.
     * \param instance FBiGeom instance handle
     * \param entity_set_handle Entity set being iterated
     * \param requested_entity_type Type of entity to iterate
     * \param requested_array_size Size of chunks of handles returned for each
     *        value of the iterator
     * \param entArr_iterator Pointer to iterator returned from function
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_initEntArrIter( FBiGeom_Instance instance,
                             iBase_EntitySetHandle entity_set_handle,
                             int requested_entity_type,
                             int requested_array_size,
                             iBase_EntityArrIterator* entArr_iterator,
                             int* err );

    /**\brief  Get entity corresponding to an iterator and increment iterator
     *
     * Get the entity corresponding to an array iterator, and increment the 
     * iterator.  Also return whether the next value of the iterator has
     * an entity (if non-zero, next iterator value is the end of the
     * iteration).
     * \param instance FBiGeom instance handle
     * \param entity_iterator Iterator being queried
     * \param entity_handle Pointer to an entity handle corresponding to the
     *        current value of iterator
     * \param has_data Pointer to a flag indicating if the value returned
     *        in entity_handle is valid. A non-zero value indicates the value
     *        is valid. A zero value indicates the value is NOT valid.
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getNextEntIter( FBiGeom_Instance instance,
                             iBase_EntityIterator entity_iterator,
                             iBase_EntityHandle* entity_handle,
                             int* has_data,
                             int* err );

    /**\brief  Get entities contained in array iterator and increment iterator
     *
     * Get the entities contained in an array iterator, and increment the 
     * iterator.  Also return whether the next value of the iterator has
     * any entities (if non-zero, next iterator value is the end of the
     * iteration).
     * \param instance FBiGeom instance handle
     * \param entArr_iterator Iterator being queried
     * \param *entity_handles Pointer to array of entity handles contained in
     *        current value of iterator
     * \param *entity_handles_allocated Pointer to allocated size of 
     *        entity_handles array
     * \param *entity_handles_size Pointer to occupied size of entity_handles 
     *        array
     * \param has_data Pointer to a flag indicating if the value(s) returned
     *        in entity_handles are valid. A non-zero value indicates the 
     *        value(s) are valid. A zero value indicates the value(s) are NOT
     *        valid.
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getNextEntArrIter( FBiGeom_Instance instance,
                                iBase_EntityArrIterator entArr_iterator,
                                iBase_EntityHandle** entity_handles,
                                int* entity_handles_allocated,
                                int* entity_handles_size,
                                int* has_data,
                                int* err );

    /**\brief  Reset the iterator
     *
     * Reset the iterator
     * \param instance FBiGeom instance handle
     * \param entity_iterator Iterator to reset
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_resetEntIter( FBiGeom_Instance instance,
                           iBase_EntityIterator entity_iterator,
                           int* err );

    /**\brief  Reset the array iterator
     *
     * Reset the array iterator
     * \param instance FBiGeom instance handle
     * \param entArr_iterator Iterator to reset
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_resetEntArrIter( FBiGeom_Instance instance,
                              iBase_EntityArrIterator entArr_iterator,
                              int* err );

    /**\brief  Destroy the specified iterator
     *
     * Destroy the specified iterator
     * \param instance FBiGeom instance handle
     * \param entity_iterator Iterator which gets destroyed
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_endEntIter( FBiGeom_Instance instance,
                         iBase_EntityIterator entity_iterator,
                         int* err );

    /**\brief  Destroy the specified array iterator
     *
     * Destroy the specified array iterator
     * \param instance FBiGeom instance handle
     * \param entArr_iterator Iterator which gets destroyed
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_endEntArrIter( FBiGeom_Instance instance,
                            iBase_EntityArrIterator entArr_iterator,
                            int* err );

    /**\brief  Make a copy of the specified entity
     *
     * Make a copy of the specified entity
     * \param instance FBiGeom instance handle
     * \param source entity to be copied
     * \param copy the newly-created entity
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_copyEnt( FBiGeom_Instance instance,
                      iBase_EntityHandle source,
                      iBase_EntityHandle* copy,
                      int* err );

    /**\brief  Sweep (extrude) an entity about an axis
     *
     * Sweep (extrude) an entity by the given angle about the given axis.
     *
     * \param instance FBiGeom instance handle
     * \param geom_entity the entity to rotate
     * \param angle the rotational angle, in degrees
     * \param axis_x x coordinate of the axis
     * \param axis_y y coordinate of the axis
     * \param axis_z z coordinate of the axis
     * \param geom_entity2 Pointer to new entity handle returned from function
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_sweepEntAboutAxis( FBiGeom_Instance instance,
                                iBase_EntityHandle geom_entity,
                                double angle,
                                double axis_x,
                                double axis_y,
                                double axis_z,
                                iBase_EntityHandle* geom_entity2,
                                int* err );

    /**\brief  Delete all entities and sets
     *
     * Delete all entities and sets
     * \param instance FBiGeom instance handle
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_deleteAll( FBiGeom_Instance instance,
                        int* err );

    /**\brief  Delete specified entity
     *
     * Delete specified entity
     * \param instance FBiGeom instance handle
     * \param entity_handle Entity to be deleted
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_deleteEnt( FBiGeom_Instance instance,
                        iBase_EntityHandle entity_handle,
                        int* err );

    /**\brief Create a sphere
     *
     * Create a sphere of the specified radius centered on the origin.
     * \param instance FBiGeom instance handle
     * \param radius radius of the sphere
     * \param geom_entity Pointer to new entity handle returned from function
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_createSphere( FBiGeom_Instance instance,
                           double radius,
                           iBase_EntityHandle* geom_entity,
                           int* err );

    /**\brief  Create a prism
     *
     * Create a prism parallel to the z-axis and centered at the origin (so
     * that its z-coordinate extents are +height/2 and -height/2).
     * \param instance FBiGeom instance handle
     * \param height height of new prism
     * \param n_sides number of sides of new prism
     * \param major_rad major radius of new prism
     * \param minor_rad minor radius of new prism
     * \param geom_entity Pointer to new entity handle returned from function
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_createPrism( FBiGeom_Instance instance,
                          double height,
                          int n_sides,
                          double major_rad,
                          double minor_rad,
                          iBase_EntityHandle* geom_entity,
                          int* err );

    /**\brief  Create an axis-oriented box 
     *
     * Create an axis-oriented box of the given dimensions, centered at the
     * origin.
     * \param instance FBiGeom instance handle
     * \param x x dimension of new box
     * \param y y dimension of new box
     * \param z z dimension of new box
     * \param geom_entity Pointer to new entity handle returned from function
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_createBrick( FBiGeom_Instance instance,
                          double x,
                          double y,
                          double z,
                          iBase_EntityHandle* geom_entity,
                          int* err );

    /**\brief  Create a cylinder
     *
     * Create a cylinder parallel to the z-axis and centered at the origin (so
     * that its z-coordinate extents are +height/2 and -height/2).
     * \param instance FBiGeom instance handle
     * \param height The height of the cylinder.
     * \param major_rad The x-axis radius
     * \param minor_rad The y-axis radius. If minor_rad is 0, the cylinder will 
     *        be circular (as if minor_rad == major_rad).
     * \param geom_entity Pointer to new entity handle returned from function
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_createCylinder( FBiGeom_Instance instance,
                             double height,
                             double major_rad,
                             double minor_rad,
                             iBase_EntityHandle* geom_entity,
                             int* err );

    /**\brief  Create a cone or tapered cylinder
     *
     * Create a cone parallel to the z-axis and centered at the origin (so that
     * its z-coordinate extents are +height/2 and -height/2). The 'base' of the
     * cylinder is at z = -height/2, and the top is at +height/2.
     * \param instance FBiGeom instance handle
     * \param height The height of the cone.
     * \param major_rad_base The x-axis radius at the base of the cylinder 
     * \param minor_rad_base The y-axis radius at the base.  If minor_rad_base
     *        is 0, the cylinder will be circular (as if minor_rad_base ==
     *        major_rad_base)
     * \param rad_top The x-axis radius at the top of the cone.  The y-axis
     *        radius at the top of the cone will be inferred to keep the aspect 
     *        ratio of the top of the cone the same as the bottom. If rad_top is
     *        0, the cone terminates at a point.
     * \param geom_entity Pointer to new entity handle returned from function
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_createCone( FBiGeom_Instance instance,
                         double height,
                         double major_rad_base,
                         double minor_rad_base,
                         double rad_top,
                         iBase_EntityHandle* geom_entity,
                         int* err );

    /**\brief  Create a torus
     *
     * Create a torus centered on the origin and encircling the z-axis.
     * \param instance FBiGeom instance handle
     * \param major_rad The distance from the origin to the center of the
     *        torus's circular cross-section.
     * \param minor_rad The radius of the cross-section.
     * \param geom_entity Pointer to new entity handle returned from function
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_createTorus( FBiGeom_Instance instance,
                          double major_rad,
                          double minor_rad,
                          iBase_EntityHandle* geom_entity,
                          int* err );

    /**\brief  Move an entity by the given vector
     *
     * Move an entity by translating it along the given vector.
     * \param instance FBiGeom instance handle
     * \param geom_entity the entity to move
     * \param x x coordinate of the vector
     * \param y y coordinate of the vector
     * \param z z coordinate of the vector
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_moveEnt( FBiGeom_Instance instance,
                      iBase_EntityHandle geom_entity,
                      double x,
                      double y,
                      double z,
                      int* err );

    /**\brief  Rotate an entity about an axis
     *
     * Rotate an entity by the given angle about the given axis.
     * \param instance FBiGeom instance handle
     * \param geom_entity the entity to rotate
     * \param angle the rotational angle, in degrees
     * \param axis_x x coordinate of the axis
     * \param axis_y y coordinate of the axis
     * \param axis_z z coordinate of the axis
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_rotateEnt( FBiGeom_Instance instance,
                        iBase_EntityHandle geom_entity,
                        double angle,
                        double axis_x,
                        double axis_y,
                        double axis_z,
                        int* err );

   /**\brief  Reflect an entity across a plane
     *
     * Reflect an entity across the given plane
     * \param instance FBiGeom instance handle
     * \param geom_entity the entity to reflect, 
     * \param point_x  x coordinate of the point that the reflecting plane goes though
     * \param point_y  y coordinate of the point that the reflecting plane goes though
     * \param point_z  z coordinate of the point that the reflecting plane goes though
     * \param plane_normal_x x coordinate of the plane's normal
     * \param plane_normal_y y coordinate of the plane's normal
     * \param plane_normal_z z coordinate of the plane's normal
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_reflectEnt( FBiGeom_Instance instance,
                           iBase_EntityHandle geom_entity,
                           double x, 
                           double y, 
                           double z, 
                           double plane_normal_x, 
                           double plane_normal_y,
                           double plane_normal_z,
                           int* err );

   /**\brief  Scale an entity in the x, y, and z directions
     *
     * Scale an entity in the x, y, and z directions.
     * \param instance FBiGeom instance handle
     * \param geom_entity the entity to scale, 
     * \param point_x  x coordinate of the scaling center
     * \param point_y  y coordinate of the scaling center
     * \param point_z  z coordinate of the scaling center
     * \param scale_x factor to scale by in the x direction
     * \param scale_y factor to scale by in the y direction
     * \param scale_z factor to scale by in the z direction
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_scaleEnt( FBiGeom_Instance instance,
                         iBase_EntityHandle geom_entity,
                         double x, 
                         double y, 
                         double z, 
                         double scale_x,
                         double scale_y, 
                         double scale_z, 
                         int* err );

    /**\brief  Geometrically unite entities
     *
     * Geometrically unite the specified entities.
     * \param instance FBiGeom instance handle
     * \param geom_entities Array of entity handles being united
     * \param geom_entities_size Number of entities in geom_entities array
     * \param geom_entity Pointer to new entity handle returned from function
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_uniteEnts( FBiGeom_Instance instace,
                        iBase_EntityHandle const* geom_entities,
                        int geom_entities_size,
                        iBase_EntityHandle* geom_entity,
                        int* err );

    /**\brief  Geometrically subtract one entity from another
     *
     * Geometrically subtract the entity tool from the entity blank.
     * \param instance FBiGeom instance handle
     * \param blank The entity to subtract from
     * \param tool The entity to subtract
     * \param geom_entity Pointer to new entity handle returned from function
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_subtractEnts( FBiGeom_Instance instance,
                           iBase_EntityHandle blank,
                           iBase_EntityHandle tool,
                           iBase_EntityHandle* geom_entity,
                           int* err );

    /**\brief  Geometrically intersect a pair of entities
     *
     * Geometrically intersect a pair of entities.
     * \param instance FBiGeom instance handle
     * \param entity1 The entity to intersect
     * \param entity2 The entity to intersect
     * \param geom_entity Pointer to new entity handle returned from function
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_intersectEnts( FBiGeom_Instance instance,
                            iBase_EntityHandle entity2,
                            iBase_EntityHandle entity1,
                            iBase_EntityHandle* geom_entity,
                            int* err );

    /**\brief  Section (cut) a region with a plane
     *
     * Section (cut) a region with a plane, retaining one of the pieces and
     * discarding the other.
     * \param instance FBiGeom instance handle
     * \param geom_entity The entity to section
     * \param plane_normal_x x coordinate of the plane's normal
     * \param plane_normal_y y coordinate of the plane's normal
     * \param plane_normal_z z coordinate of the plane's normal
     * \param offset Distance of the plane from the origin
     * \param reverse Keep the piece on the normal's side (=0) or not (=1)
     * \param geom_entity2 Pointer to new entity handle returned from function
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_sectionEnt( FBiGeom_Instance instance,
                         iBase_EntityHandle geom_entity,
                         double plane_normal_x,
                         double plane_normal_y,
                         double plane_normal_z,
                         double offset,
                         int reverse,
                         iBase_EntityHandle* geom_entity2,
                         int* err );

    /**\brief  Imprint entities
     *
     * Imprint entities by merging coincident surfaces.
     * \param instance FBiGeom instance handle
     * \param geom_entities Array of entity handles being imprinted
     * \param geom_entities_size Number of entities in geom_entities array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_imprintEnts( FBiGeom_Instance instance,
                          iBase_EntityHandle const* geom_entities,
                          int geom_entities_size,
                          int* err );

    /**\brief  Merge ents
     *
     * Merge entities of corresponding topology/geometry within the specified
     * tolerance.
     * \param instance FBiGeom instance handle
     * \param geom_entities Array of entity handles being imprinted
     * \param geom_entities_size Number of entities in geom_entities array
     * \param tolerance Tolerance within which entities are considered the same
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_mergeEnts( FBiGeom_Instance instance,
                        iBase_EntityHandle const* geom_entities,
                        int geom_entities_size,
                        double tolerance,
                        int* err );

    /**\brief  Create an entity set
     *
     * Create an entity set, either ordered (isList=1) or unordered 
     * (isList=0).  Unordered entity sets can contain a given entity or 
     * set only once.
     * \param instance FBiGeom instance handle
     * \param isList If non-zero, an ordered list is created, otherwise an
     *        unordered set is created.
     * \param entity_set_created Entity set created by function
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_createEntSet( FBiGeom_Instance instance,
                           int isList,
                           iBase_EntitySetHandle* entity_set_created, 
                           int *err );

    /**\brief  Destroy an entity set
     *
     * Destroy an entity set
     * \param instance FBiGeom instance handle
     * \param entity_set Entity set to be destroyed
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_destroyEntSet( FBiGeom_Instance instance,
                            iBase_EntitySetHandle entity_set, 
                            int *err );

    /**\brief  Return whether a specified set is ordered or unordered
     *
     * Return whether a specified set is ordered (*is_list=1) or 
     * unordered (*is_list=0)
     * \param instance FBiGeom instance handle
     * \param entity_set Entity set being queried
     * \param is_list Pointer to flag returned from function
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_isList( FBiGeom_Instance instance,
                     iBase_EntitySetHandle entity_set,
                     int *is_list, 
                     int *err );

    /**\brief  Get the number of entity sets contained in a set or interface
     *
     * Get the number of entity sets contained in a set or interface.  If
     * a set is input which is not the root set, num_hops indicates the 
     * maximum number of contained sets from entity_set_handle to one of the
     * contained sets, not inclusive of the contained set.
     * \param instance FBiGeom instance handle
     * \param entity_set_handle Entity set being queried
     * \param num_hops Maximum hops from entity_set_handle to contained set,
     *        not inclusive of the contained set
     * \param num_sets Pointer to the number of sets returned from function
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getNumEntSets( FBiGeom_Instance instance,
                            iBase_EntitySetHandle entity_set_handle,
                            int num_hops,
                            int *num_sets, 
                            int *err );

    /**\brief  Get the entity sets contained in a set or interface
     *
     * Get the entity sets contained in a set or interface.  If
     * a set is input which is not the root set, num_hops indicates the 
     * maximum number of contained sets from entity_set_handle to one of the
     * contained sets, not inclusive of the contained set.
     * \param instance FBiGeom instance handle
     * \param entity_set_handle Entity set being queried
     * \param num_hops Maximum hops from entity_set_handle to contained set,
     *        not inclusive of the contained set
     * \param *contained_set_handles Pointer to array of set handles returned
     *        from function
     * \param contained_set_handles_allocated Pointer to allocated length of
     *        contained_set_handles array
     * \param contained_set_handles_size Pointer to occupied length of
     *        contained_set_handles array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getEntSets( FBiGeom_Instance instance,
                         iBase_EntitySetHandle entity_set_handle,
                         int num_hops,
                         iBase_EntitySetHandle** contained_set_handles,
                         int* contained_set_handles_allocated,
                         int* contained_set_handles_size, 
                         int *err );

    /**\brief  Add an entity to a set
     *
     * Add an entity to a set
     * \param instance FBiGeom instance handle
     * \param entity_handle The entity being added
     * \param entity_set Pointer to the set being added to
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_addEntToSet( FBiGeom_Instance instance,
                          iBase_EntityHandle entity_handle,
                          iBase_EntitySetHandle entity_set, 
                          int *err );

    /**\brief  Remove an entity from a set
     *
     * Remove an entity from a set
     *
     * \param instance FBiGeom instance handle
     * \param entity_handle The entity being removed
     * \param entity_set Pointer to the set being removed from
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_rmvEntFromSet( FBiGeom_Instance instance,
                            iBase_EntityHandle entity_handle,
                            iBase_EntitySetHandle entity_set, 
                            int *err );

    /**\brief  Add an array of entities to a set
     *
     * Add an array of entities to a set
     * \param instance FBiGeom instance handle
     * \param entity_handles Array of entities being added
     * \param entity_handles_size Number of entities in entity_handles array
     * \param entity_set Pointer to the set being added to
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_addEntArrToSet( FBiGeom_Instance instance,
                             const iBase_EntityHandle* entity_handles,
                             int entity_handles_size,
                             iBase_EntitySetHandle entity_set, 
                             int *err );

    /**\brief  Remove an array of entities from a set
     *
     * Remove an array of entities from a set
     * \param instance FBiGeom instance handle
     * \param entity_handles Array of entities being remove
     * \param entity_handles_size Number of entities in entity_handles array
     * \param entity_set Pointer to the set being removed from
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_rmvEntArrFromSet( FBiGeom_Instance instance,
                               const iBase_EntityHandle* entity_handles,
                               int entity_handles_size,
                               iBase_EntitySetHandle entity_set,
                               int *err );

    /**\brief  Add an entity set to a set
     *
     * Add an entity set to a set
     * \param instance FBiGeom instance handle
     * \param entity_set_to_add The entity set being added
     * \param entity_set_handle Pointer to the set being added to
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_addEntSet( FBiGeom_Instance instance,
                        iBase_EntitySetHandle entity_set_to_add,
                        iBase_EntitySetHandle entity_set_handle, 
                        int *err);

    /**\brief  Remove an entity set from a set
     *
     * Remove an entity set from a set
     * \param instance FBiGeom instance handle
     * \param entity_set_to_remove The entity set being removed
     * \param entity_set_handle Pointer to the set being removed from
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_rmvEntSet( FBiGeom_Instance instance,
                        iBase_EntitySetHandle entity_set_to_remove,
                        iBase_EntitySetHandle entity_set_handle, 
                        int *err );

    /**\brief  Return whether an entity is contained in another set
     *
     * Return whether an entity is contained (*is_contained=1) or not 
     * contained (*is_contained=0) in another set
     * \param instance FBiGeom instance handle
     * \param containing_entity_set Entity set being queried
     * \param contained_entity Entity potentially contained in 
     *        containing_entity_set
     * \param is_contained Pointer to flag returned from function
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_isEntContained( FBiGeom_Instance instance,
                             iBase_EntitySetHandle containing_entity_set,
                             iBase_EntityHandle contained_entity,
                             int *is_contained, 
                             int *err );

    /**\brief  Return whether entities are contained in a set
     *
     * Return whether each entity is contained in the set.
     * \param instance iMesh instance handle
     * \param containing_entity_set Entity set being queried
     * \param entity_handles List of entities for which to check containment.
     * \param is_contained One value for each input entity, 1 if contained
     *          in set, zero otherwise.
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_isEntArrContained( FBiGeom_Instance instance,
                                /*in*/ iBase_EntitySetHandle containing_set,
                                /*in*/ const iBase_EntityHandle* entity_handles,
                                /*in*/ int num_entity_handles,
                                /*inout*/ int** is_contained,
                                /*inout*/ int* is_contained_allocated,
                                /*out*/ int* is_contained_size,
                                /*out*/ int* err );

    /**\brief  Return whether an entity set is contained in another set
     *
     * Return whether a set is contained (*is_contained=1) or not contained
     * (*is_contained=0) in another set
     * \param instance FBiGeom instance handle
     * \param containing_entity_set Entity set being queried
     * \param contained_entity_set Entity set potentially contained in 
     *        containing_entity_set
     * \param is_contained Pointer to flag returned from function
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_isEntSetContained( FBiGeom_Instance instance,
                                iBase_EntitySetHandle containing_entity_set,
                                iBase_EntitySetHandle contained_entity_set,
                                int *is_contained, 
                                int *err );

    /**\brief  Add parent/child links between two sets
     *
     * Add parent/child links between two sets.  Makes parent point to child
     * and child point to parent.
     * \param instance FBiGeom instance handle
     * \param parent_entity_set Pointer to parent set
     * \param child_entity_set Pointer to child set
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_addPrntChld( FBiGeom_Instance instance,
                          iBase_EntitySetHandle parent_entity_set,
                          iBase_EntitySetHandle child_entity_set, 
                          int *err );

    /**\brief  Remove parent/child links between two sets
     *
     * Remove parent/child links between two sets.
     * \param instance FBiGeom instance handle
     * \param parent_entity_set Pointer to parent set
     * \param child_entity_set Pointer to child set
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_rmvPrntChld( FBiGeom_Instance instance,
                          iBase_EntitySetHandle parent_entity_set,
                          iBase_EntitySetHandle child_entity_set, 
                          int *err );

    /**\brief  Return whether two sets are related by parent/child links
     *
     * Return whether two sets are related (*is_child=1) or not (*is_child=0)
     * by parent/child links
     * \param instance FBiGeom instance handle
     * \param parent_entity_set Pointer to parent set
     * \param child_entity_set Pointer to child set
     * \param is_child Pointer to flag returned from function
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_isChildOf( FBiGeom_Instance instance,
                        iBase_EntitySetHandle parent_entity_set,
                        iBase_EntitySetHandle child_entity_set,
                        int *is_child, 
                        int *err );

    /**\brief  Get the number of child sets linked from a specified set
     *
     * Get the number of child sets linked from a specified set.  If num_hops
     * is not -1, this represents the maximum hops from entity_set to any
     * child in the count.
     * \param instance FBiGeom instance handle
     * \param entity_set Entity set being queried
     * \param num_hops Maximum hops from entity_set_handle to child set,
     *        not inclusive of the child set
     * \param num_child Pointer to number of children returned from function
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getNumChld( FBiGeom_Instance instance,
                         iBase_EntitySetHandle entity_set,
                         int num_hops,
                         int *num_child, 
                         int *err );

    /**\brief  Get the number of parent sets linked from a specified set
     *
     * Get the number of parent sets linked from a specified set.  If num_hops
     * is not -1, this represents the maximum hops from entity_set to any
     * parent in the count.
     * \param instance FBiGeom instance handle
     * \param entity_set Entity set being queried
     * \param num_hops Maximum hops from entity_set_handle to parent set,
     *        not inclusive of the parent set
     * \param num_parent Pointer to number of parents returned from function
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getNumPrnt( FBiGeom_Instance instance,
                         iBase_EntitySetHandle entity_set,
                         int num_hops,
                         int *num_parent, 
                         int *err );

    /**\brief  Get the child sets linked from a specified set
     *
     * Get the child sets linked from a specified set.  If num_hops
     * is not -1, this represents the maximum hops from entity_set to any
     * child.
     * \param instance FBiGeom instance handle
     * \param from_entity_set Entity set being queried
     * \param num_hops Maximum hops from entity_set_handle to child set,
     *        not inclusive of the child set
     * \param *entity_set_handles Pointer to array of child sets
     *        returned from function
     * \param *entity_set_handles_allocated Pointer to allocated size of 
     *        entity_set_handles array
     * \param *entity_set_handles_size Pointer to occupied size of 
     *        entity_set_handles array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getChldn( FBiGeom_Instance instance,
                       iBase_EntitySetHandle from_entity_set,
                       int num_hops,
                       iBase_EntitySetHandle** entity_set_handles,
                       int* entity_set_handles_allocated,
                       int* entity_set_handles_size, 
                       int *err );

    /**\brief  Get the parent sets linked from a specified set
     *
     * Get the parent sets linked from a specified set.  If num_hops
     * is not -1, this represents the maximum hops from entity_set to any
     * parent.
     * \param instance FBiGeom instance handle
     * \param from_entity_set Entity set being queried
     * \param num_hops Maximum hops from entity_set_handle to parent set,
     *        not inclusive of the parent set
     * \param *entity_set_handles Pointer to array of parent sets
     *        returned from function
     * \param *entity_set_handles_allocated Pointer to allocated size of 
     *        entity_set_handles array
     * \param *entity_set_handles_size Pointer to occupied size of 
     *        entity_set_handles array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getPrnts( FBiGeom_Instance instance,
                       iBase_EntitySetHandle from_entity_set,
                       int num_hops,
                       iBase_EntitySetHandle** entity_set_handles,
                       int* entity_set_handles_allocated,
                       int* entity_set_handles_size, 
                       int *err );

    /**\brief  Create a tag with specified name, size, and type
     *
     * Create a tag with specified name, size, and type.  Tag size is in
     * units of size of tag_type data types.  Value input for tag type must be 
     * value in iBase_TagType enumeration.
     * \param instance FBiGeom instance handle
     * \param tag_name Character string indicating tag name
     * \param tag_size Size of each tag value, in units of number of tag_type 
     *        entities
     * \param tag_type Data type for data stored in this tag
     * \param tag_handle Pointer to tag handle returned from function
     * \param *err Pointer to error type returned from function
     * \param tag_name_len Length of tag name string
     */
  void FBiGeom_createTag( FBiGeom_Instance instance,
                        const char* tag_name,
                        int tag_size,
                        int tag_type,
                        iBase_TagHandle* tag_handle, 
                        int *err,
                        int tag_name_len );


    /**\brief  Destroy a tag
     *
     * Destroy a tag.  If forced is non-zero and entities still have values
     * set for this tag, tag is deleted anyway and those values disappear,
     * otherwise tag is not deleted.
     * \param instance FBiGeom instance handle
     * \param tag_handle Handle of tag to be deleted
     * \param forced If non-zero, delete the tag even if entities have values
     *        set for that tag
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_destroyTag( FBiGeom_Instance instance,
                         iBase_TagHandle tag_handle,
                         int forced, 
                         int *err);

    /**\brief  Get the name for a given tag handle
     *
     * Get the name for a given tag handle
     * \param instance FBiGeom instance handle
     * \param tag_handle Tag handle being queried
     * \param name Pointer to character string to store name returned from 
     *        function
     * \param *err Pointer to error type returned from function
     * \param name_len Length of character string input to function
     */
  void FBiGeom_getTagName( FBiGeom_Instance instance,
                         iBase_TagHandle tag_handle,
                         char *name, 
                         int* err,
                         int name_len );

    /**\brief  Get size of a tag in units of numbers of tag data type
     *
     * Get size of a tag in units of numbers of tag data type
     * \param instance FBiGeom instance handle
     * \param tag_handle Handle of tag being queried
     * \param tag_size Pointer to tag size returned from function
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getTagSizeValues( FBiGeom_Instance instance,
                               iBase_TagHandle tag_handle,
                               int *tag_size, 
                               int *err );

    /**\brief  Get size of a tag in units of bytes
     *
     * Get size of a tag in units of bytes
     * \param instance FBiGeom instance handle
     * \param tag_handle Handle of tag being queried
     * \param tag_size Pointer to tag size returned from function
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getTagSizeBytes( FBiGeom_Instance instance,
                              iBase_TagHandle tag_handle,
                              int *tag_size, 
                              int *err );

    /**\brief  Get a the handle of an existing tag with the specified name
     *
     * Get a the handle of an existing tag with the specified name
     * \param instance FBiGeom instance handle
     * \param tag_name Name of tag being queried
     * \param tag_handle Pointer to tag handle returned from function
     * \param *err Pointer to error type returned from function
     * \param tag_name_len Length of tag name string
     */
  void FBiGeom_getTagHandle( FBiGeom_Instance instance,
                           const char* tag_name,
                           iBase_TagHandle *tag_handle, 
                           int *err,
                           int tag_name_len );

    /**\brief  Get the data type of the specified tag handle
     *
     * Get the data type of the specified tag handle.  Tag type is a value in
     * the iBase_TagType enumeration.
     * \param instance FBiGeom instance handle
     * \param tag_handle Handle for the tag being queried
     * \param tag_type Pointer to tag type returned from function
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getTagType( FBiGeom_Instance instance,
                         iBase_TagHandle tag_handle,
                         int *tag_type, 
                         int *err );

    /**\brief  Set a tag value of arbitrary type on an entity set
     *
     * Set a tag value of arbitrary type on an entity set. The tag data
     * is passed as void*. tag_value_size specifies the size of the memory
     * pointed to by tag_value in terms of bytes. Applications are free to
     * use this function to set data of any type, not just iBase_BYTES.
     * However, in all cases, the size specified by tag_value_size is
     * always in terms of bytes.
     *
     * \param instance FBiGeom instance handle
     * \param entity_set_handle Entity set on which tag is being set
     * \param tag_handle Tag being set on an entity set
     * \param tag_value Pointer to tag data being set on entity set
     * \param tag_value_size Size in bytes of tag data
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_setEntSetData( FBiGeom_Instance instance,
                            iBase_EntitySetHandle entity_set_handle,
                            const iBase_TagHandle tag_handle,
                            const void* tag_value,
                            const int tag_value_size,
                            int *err );

    /**\brief  Set a tag value of integer type on an entity set
     *
     * Set a tag value of integer type on an entity set.
     * \param instance FBiGeom instance handle
     * \param entity_set Entity set on which tag is being set
     * \param tag_handle Tag being set on an entity set
     * \param tag_value Tag value being set on entity set
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_setEntSetIntData( FBiGeom_Instance instance,
                               iBase_EntitySetHandle entity_set,
                               iBase_TagHandle tag_handle,
                               int tag_value, 
                               int *err );

    /**\brief  Set a tag value of double type on an entity set
     *
     * Set a tag value of double type on an entity set.
     * \param instance FBiGeom instance handle
     * \param entity_set Entity set on which tag is being set
     * \param tag_handle Tag being set on an entity set
     * \param tag_value Tag value being set on entity set
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_setEntSetDblData( FBiGeom_Instance instance,
                               iBase_EntitySetHandle entity_set,
                               iBase_TagHandle tag_handle,
                               double tag_value, 
                               int *err );

    /**\brief  Set a tag value of entity handle type on an entity set
     *
     * Set a tag value of entity handle type on an entity set.
     * \param instance FBiGeom instance handle
     * \param entity_set Entity set on which tag is being set
     * \param tag_handle Tag being set on an entity set
     * \param tag_value Tag value being set on entity set
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_setEntSetEHData( FBiGeom_Instance instance,
                              iBase_EntitySetHandle entity_set,
                              iBase_TagHandle tag_handle,
                              iBase_EntityHandle tag_value, 
                              int *err );

    /**\brief  Set a tag value of entity set handle type on an entity set
     *
     * Set a tag value of entity set handle type on an entity set.
     * \param instance FBiGeom instance handle
     * \param entity_set Entity set on which tag is being set
     * \param tag_handle Tag being set on an entity set
     * \param tag_value Tag value being set on entity set
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_setEntSetESHData( FBiGeom_Instance instance,
                               iBase_EntitySetHandle entity_set,
                               iBase_TagHandle tag_handle,
                               iBase_EntitySetHandle tag_value, 
                               int *err );

    /**\brief  Get the value of a tag of arbitrary type on an entity set
     *
     * Get the value of a tag of arbitrary type on an entity set.  Tag data 
     * is returned back as void*. tag_value_size specifies the size of the
     * memory pointed to by tag_value in terms of bytes. Applications may
     * use this function to get data of any type, not just iBase_BYTES.
     * However because this function supports data of arbitrary type,
     * in all cases the size specified by tag_value_size is always in terms
     * of bytes.
     *
     * \param instance FBiGeom instance handle
     * \param entity_set_handle Entity set on which tag is being set
     * \param tag_handle Tag being set on an entity set
     * \param *tag_value Pointer to tag data array being queried
     * \param *tag_value_allocated Pointer to tag data array allocated size
     * \param *tag_value_size Pointer to occupied size in bytes of tag data
     *        array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getEntSetData( FBiGeom_Instance instance,
                            iBase_EntitySetHandle entity_set_handle,
                            iBase_TagHandle tag_handle,
                            void** tag_value,
                            int* tag_value_allocated,
                            int* tag_value_size, 
                            int *err );

    /**\brief  Get the value of a tag of integer type on an entity set
     *
     * Get the value of a tag of integer type on an entity set.
     * \param instance FBiGeom instance handle
     * \param entity_set Entity set on which tag is being set
     * \param tag_handle Tag being set on an entity set
     * \param *out_data Pointer to tag value returned from function
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getEntSetIntData( FBiGeom_Instance instance,
                               iBase_EntitySetHandle entity_set,
                               iBase_TagHandle tag_handle,
                               int *out_data, 
                               int *err );

    /**\brief  Get the value of a tag of double type on an entity set
     *
     * Get the value of a tag of double type on an entity set.
     * \param instance FBiGeom instance handle
     * \param entity_set Entity set on which tag is being set
     * \param tag_handle Tag being set on an entity set
     * \param *out_data Pointer to tag value returned from function
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getEntSetDblData( FBiGeom_Instance instance,
                               iBase_EntitySetHandle entity_set,
                               iBase_TagHandle tag_handle,
                               double *out_data, 
                               int *err );

    /**\brief  Get the value of a tag of entity handle type on an entity set
     *
     * Get the value of a tag of entity handle type on an entity set.
     * \param instance FBiGeom instance handle
     * \param entity_set Entity set on which tag is being set
     * \param tag_handle Tag being set on an entity set
     * \param *out_data Pointer to tag value returned from function
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getEntSetEHData( FBiGeom_Instance instance,
                              iBase_EntitySetHandle entity_set,
                              iBase_TagHandle tag_handle,
                              iBase_EntityHandle *out_data, 
                              int *err );

    /**\brief  Get the value of a tag of entity set handle type on an entity set
     *
     * Get the value of a tag of entity set handle type on an entity set.
     * \param instance FBiGeom instance handle
     * \param entity_set Entity set on which tag is being set
     * \param tag_handle Tag being set on an entity set
     * \param *out_data Pointer to tag value returned from function
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getEntSetESHData( FBiGeom_Instance instance,
                              iBase_EntitySetHandle entity_set,
                              iBase_TagHandle tag_handle,
                              iBase_EntitySetHandle *out_data, 
                              int *err );

    /**\brief  Get all the tags associated with a specified entity set
     *
     * Get all the tags associated with a specified entity set
     * \param instance FBiGeom instance handle
     * \param entity_set_handle Entity being queried
     * \param *tag_handles Pointer to array of tag_handles returned from 
     *        function
     * \param *tag_handles_allocated Pointer to allocated size of tag_handles 
     *        array
     * \param *tag_handles_size Pointer to occupied size of tag_handles array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getAllEntSetTags( FBiGeom_Instance instance,
                               iBase_EntitySetHandle entity_set_handle,
                               iBase_TagHandle** tag_handles,
                               int* tag_handles_allocated,
                               int* tag_handles_size, 
                               int *err );

    /**\brief  Remove a tag value from an entity set
     *
     * Remove a tag value from an entity set
     * \param instance FBiGeom instance handle
     * \param entity_set_handle Entity set from which tag is being removed
     * \param tag_handle Tag handle of tag being removed
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_rmvEntSetTag( FBiGeom_Instance instance,
                           iBase_EntitySetHandle entity_set_handle,
                           iBase_TagHandle tag_handle, 
                           int *err );

    /**\brief  Get tag values of arbitrary type for an array of entities
     *
     * Get tag values of arbitrary type for an array of entities.  Tag data 
     * is returned as void*. tag_values_size specifies the size of the
     * memory pointed to by tag_values in terms of bytes. Applications may
     * use this function to get data of any type, not just iBase_BYTES.
     * However, because this function supports data of arbitrary type, in
     * all cases the size specified by tag_values_size always in terms of
     * bytes.
     *
     * \param instance FBiGeom instance handle
     * \param entity_handles Entity array on which tag is being set
     * \param entity_handles_size Number of entities in array
     * \param tag_handle Tag being set on an entity
     * \param *tag_values Pointer to tag data array being returned from 
     *        function. Note that the implicit INTERLEAVED storage
     *        order rule applies (see section ITAPS Storage Orders)
     * \param tag_values_allocated Pointer to allocated size of tag data array
     * \param tag_values_size Pointer to occupied size in bytes of tag data
     *        array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getArrData( FBiGeom_Instance instance,
                         const iBase_EntityHandle* entity_handles,
                         int entity_handles_size,
                         iBase_TagHandle tag_handle,
                         void** tag_values,
                         int* tag_values_allocated,
                         int* tag_values_size, 
                         int *err );

    /**\brief  Get tag values of integer type for an array of entities
     *
     * Get tag values of integer type for an array of entities.
     * \param instance FBiGeom instance handle
     * \param entity_handles Entity array on which tag is being set
     * \param entity_handles_size Number of entities in array
     * \param tag_handle Tag being set on an entity
     * \param *tag_values Pointer to tag data array being returned from 
     *        function
     * \param tag_values_allocated Pointer to allocated size of tag data array
     * \param tag_values_size Pointer to occupied size of tag data array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getIntArrData( FBiGeom_Instance instance,
                            const iBase_EntityHandle* entity_handles,
                            int entity_handles_size,
                            iBase_TagHandle tag_handle,
                            int** tag_values,
                            int* tag_values_allocated,
                            int* tag_values_size, 
                            int *err );

    /**\brief  Get tag values of double type for an array of entities
     *
     * Get tag values of double type for an array of entities.
     * \param instance FBiGeom instance handle
     * \param entity_handles Entity array on which tag is being set
     * \param entity_handles_size Number of entities in array
     * \param tag_handle Tag being set on an entity
     * \param *tag_values Pointer to tag data array being returned from 
     *        function
     * \param tag_values_allocated Pointer to allocated size of tag data array
     * \param tag_values_size Pointer to occupied size of tag data array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getDblArrData( FBiGeom_Instance instance,
                            const iBase_EntityHandle* entity_handles,
                            int entity_handles_size,
                            iBase_TagHandle tag_handle,
                            double** tag_values,
                            int* tag_values_allocated,
                            int* tag_values_size, 
                            int *err );

    /**\brief  Get tag values of entity handle type for an array of entities
     *
     * Get tag values of entity handle type for an array of entities.
     * \param instance FBiGeom instance handle
     * \param entity_handles Entity array on which tag is being set
     * \param entity_handles_size Number of entities in array
     * \param tag_handle Tag being set on an entity
     * \param *tag_value Pointer to tag data array being returned from 
     *        function
     * \param tag_value_allocated Pointer to allocated size of tag data array
     * \param tag_value_size Pointer to occupied size of tag data array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getEHArrData( FBiGeom_Instance instance,
                           const iBase_EntityHandle* entity_handles,
                           int entity_handles_size,
                           iBase_TagHandle tag_handle,
                           iBase_EntityHandle** tag_value,
                           int* tag_value_allocated,
                           int* tag_value_size, 
                           int *err );

    /**\brief  Get tag values of entity set handle type for an array of entities
     *
     * Get tag values of entity set handle type for an array of entities.
     * \param instance FBiGeom instance handle
     * \param entity_handles Entity array on which tag is being set
     * \param entity_handles_size Number of entities in array
     * \param tag_handle Tag being set on an entity
     * \param *tag_value Pointer to tag data array being returned from 
     *        function
     * \param tag_value_allocated Pointer to allocated size of tag data array
     * \param tag_value_size Pointer to occupied size of tag data array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getESHArrData( FBiGeom_Instance instance,
                            const iBase_EntityHandle* entity_handles,
                            int entity_handles_size,
                            iBase_TagHandle tag_handle,
                            iBase_EntitySetHandle** tag_value,
                            int* tag_value_allocated,
                            int* tag_value_size, 
                            int *err );


    /**\brief  Set tag values of arbitrary type on an array of entities
     *
     * Set tag values of arbitrary type on an array of entities.  Tag data
     * is passed as void*. tag_values_size specifies the size of the
     * memory pointed to by tag_values in terms of bytes. Applications may
     * use this function to set data of any type, not just iBase_BYTES.
     * However, because this function supports data of arbitrary type, in all
     * cases the size specified by tag_values_size is always in terms of
     * bytes.
     *
     * \param instance FBiGeom instance handle
     * \param entity_handles Entity array on which tag is being set
     * \param entity_handles_size Number of entities in array
     * \param tag_handle Tag being set on an entity
     * \param tag_values Pointer to tag data being set on entity. Note that
     *        the implicit INTERLEAVED storage order rule applies (see section
     *        ITAPS Storage Orders)
     * \param tag_values_size Size in bytes of tag data
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_setArrData( FBiGeom_Instance instance,
                         const iBase_EntityHandle* entity_handles,
                         int entity_handles_size,
                         iBase_TagHandle tag_handle,
                         const void* tag_values,
                         int tag_values_size, 
                         int *err );

    /**\brief  Set tag values of integer type on an array of entities
     *
     * Set tag values of integer type on an array of entities.
     * \param instance FBiGeom instance handle
     * \param entity_handles Entity array on which tag is being set
     * \param entity_handles_size Number of entities in array
     * \param tag_handle Tag being set on an entity
     * \param tag_values Pointer to tag data being set on entities
     * \param tag_values_size Size in total number of integers of tag data
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_setIntArrData( FBiGeom_Instance instance,
                            const iBase_EntityHandle* entity_handles,
                            int entity_handles_size,
                            iBase_TagHandle tag_handle,
                            const int* tag_values,
                            int tag_values_size, 
                            int *err );

    /**\brief  Set tag values of double type on an array of entities
     *
     * Set tag values of double type on an array of entities.
     * \param instance FBiGeom instance handle
     * \param entity_handles Entity array on which tag is being set
     * \param entity_handles_size Number of entities in array
     * \param tag_handle Tag being set on an entity
     * \param tag_values Pointer to tag data being set on entities
     * \param tag_values_size Size in total number of doubles of tag data
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_setDblArrData( FBiGeom_Instance instance,
                            const iBase_EntityHandle* entity_handles,
                            int entity_handles_size,
                            iBase_TagHandle tag_handle,
                            const double* tag_values,
                            const int tag_values_size, 
                            int *err );

    /**\brief  Set tag values of entity handle type on an array of entities
     *
     * Set tag values of entity handle type on an array of entities.
     * \param instance FBiGeom instance handle
     * \param entity_handles Entity array on which tag is being set
     * \param entity_handles_size Number of entities in array
     * \param tag_handle Tag being set on an entity
     * \param tag_values Pointer to tag data being set on entities
     * \param tag_values_size Size in total number of entity handles of tag 
     *        data
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_setEHArrData( FBiGeom_Instance instance,
                           const iBase_EntityHandle* entity_handles,
                           int entity_handles_size,
                           iBase_TagHandle tag_handle,
                           const iBase_EntityHandle* tag_values,
                           int tag_values_size, 
                           int *err );

    /**\brief  Set tag values of entity set handle type on an array of entities
     *
     * Set tag values of entity set handle type on an array of entities.
     * \param instance FBiGeom instance handle
     * \param entity_handles Entity array on which tag is being set
     * \param entity_handles_size Number of entities in array
     * \param tag_handle Tag being set on an entity
     * \param tag_values Pointer to tag data being set on entities
     * \param tag_values_size Size in total number of entity handles of tag 
     *        data
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_setESHArrData( FBiGeom_Instance instance,
                            const iBase_EntityHandle* entity_handles,
                            int entity_handles_size,
                            iBase_TagHandle tag_handle,
                            const iBase_EntitySetHandle* tag_values,
                            int tag_values_size, 
                            int *err );

    /**\brief  Remove a tag value from an array of entities
     *
     * Remove a tag value from an array of entities
     * \param instance FBiGeom instance handle
     * \param entity_handles Entity from which tag is being removed
     * \param entity_handles_size Number of entities in entity array
     * \param tag_handle Tag handle of tag being removed
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_rmvArrTag( FBiGeom_Instance instance,
                        const iBase_EntityHandle* entity_handles,
                        int entity_handles_size,
                        iBase_TagHandle tag_handle, 
                        int *err );

    /**\brief  Get the value of a tag of arbitrary type on an entity
     *
     * Get the value of a tag of arbitrary type on an entity.  Tag data 
     * is passed back as void*. tag_value_size specifies the size of the
     * memory pointed to by tag_value in terms of bytes. Applications may
     * use this function to get data of any type, not just iBase_BYTES.
     * However, because this function supports arbitrary type, in all
     * cases the size specified by tag_value_size is always in terms of
     * bytes.
     *
     * \param instance FBiGeom instance handle
     * \param entity_handle Entity on which tag is being set
     * \param tag_handle Tag being set on an entity
     * \param *tag_value Pointer to tag data array being queried
     * \param *tag_value_allocated Pointer to tag data array allocated size
     * \param *tag_value_size Pointer to occupied size in bytes of tag data
     *        array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getData( FBiGeom_Instance instance,
                      iBase_EntityHandle entity_handle,
                      iBase_TagHandle tag_handle,
                      void** tag_value,
                      int *tag_value_allocated,
                      int *tag_value_size, 
                      int *err );

    /**\brief  Get the value of a tag of integer type on an entity
     *
     * Get the value of a tag of integer type on an entity.
     * \param instance FBiGeom instance handle
     * \param entity_handle Entity on which tag is being set
     * \param tag_handle Tag being set on an entity
     * \param *out_data Pointer to tag value returned from function
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getIntData( FBiGeom_Instance instance,
                         iBase_EntityHandle entity_handle,
                         iBase_TagHandle tag_handle,
                         int *out_data, 
                         int *err );

    /**\brief  Get the value of a tag of double type on an entity
     *
     * Get the value of a tag of double type on an entity.
     * \param instance FBiGeom instance handle
     * \param entity_handle Entity on which tag is being set
     * \param tag_handle Tag being set on an entity
     * \param *out_data Pointer to tag value returned from function
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getDblData( FBiGeom_Instance instance,
                         /*in*/ const iBase_EntityHandle entity_handle,
                         /*in*/ const iBase_TagHandle tag_handle,
                         double *out_data,
                         int *err );

    /**\brief  Get the value of a tag of entity handle type on an entity
     *
     * Get the value of a tag of entity handle type on an entity.
     * \param instance FBiGeom instance handle
     * \param entity_handle Entity on which tag is being set
     * \param tag_handle Tag being set on an entity
     * \param *out_data Pointer to tag value returned from function
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getEHData( FBiGeom_Instance instance,
                        iBase_EntityHandle entity_handle,
                        iBase_TagHandle tag_handle,
                        iBase_EntityHandle *out_data, 
                        int *err );

    /**\brief  Get the value of a tag of entity set handle type on an entity
     *
     * Get the value of a tag of entity set handle type on an entity.
     * \param instance FBiGeom instance handle
     * \param entity_handle Entity on which tag is being set
     * \param tag_handle Tag being set on an entity
     * \param *out_data Pointer to tag value returned from function
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getESHData( FBiGeom_Instance instance,
                         iBase_EntityHandle entity_handle,
                         iBase_TagHandle tag_handle,
                         iBase_EntitySetHandle *out_data, 
                         int *err );

    /**\brief  Set a tag value of arbitrary type on an entity
     *
     * Set a tag value of arbitrary type on an entity.  Tag data
     * is passed as void*. tag_value_size specifies the size of the
     * memory pointed to by tag_value in terms of bytes. Applications may
     * use this function to set data of any type, not just iBase_BYTES.
     * However, because this function supports data of arbitrary type, in
     * all cases the size specified by tag_value_size is always in terms
     * of bytes.
     *
     * \param instance FBiGeom instance handle
     * \param entity_handle Entity on which tag is being set
     * \param tag_handle Tag being set on an entity
     * \param tag_value Pointer to tag data being set on entity
     * \param tag_value_size Size in bytes of tag data
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_setData( FBiGeom_Instance instance,
                      iBase_EntityHandle entity_handle,
                      iBase_TagHandle tag_handle,
                      const void* tag_value,
                      int tag_value_size, 
                      int *err );

    /**\brief  Set a tag value of integer type on an entity
     *
     * Set a tag value of integer type on an entity.
     * \param instance FBiGeom instance handle
     * \param entity_handle Entity on which tag is being set
     * \param tag_handle Tag being set on an entity
     * \param tag_value Tag value being set on entity
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_setIntData( FBiGeom_Instance instance,
                         iBase_EntityHandle entity_handle,
                         iBase_TagHandle tag_handle,
                         int tag_value, 
                         int *err );

    /**\brief  Set a tag value of double type on an entity
     *
     * Set a tag value of double type on an entity.
     * \param instance FBiGeom instance handle
     * \param entity_handle Entity on which tag is being set
     * \param tag_handle Tag being set on an entity
     * \param tag_value Tag value being set on entity
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_setDblData( FBiGeom_Instance instance,
                         iBase_EntityHandle entity_handle,
                         iBase_TagHandle tag_handle,
                         double tag_value, 
                         int *err );

    /**\brief  Set a tag value of entity handle type on an entity
     *
     * Set a tag value of entity handle type on an entity.
     * \param instance FBiGeom instance handle
     * \param entity_handle Entity on which tag is being set
     * \param tag_handle Tag being set on an entity
     * \param tag_value Tag value being set on entity
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_setEHData( FBiGeom_Instance instance,
                        iBase_EntityHandle entity_handle,
                        iBase_TagHandle tag_handle,
                        iBase_EntityHandle tag_value, 
                        int *err );

    /**\brief  Set a tag value of entity set handle type on an entity
     *
     * Set a tag value of entity set handle type on an entity.
     * \param instance FBiGeom instance handle
     * \param entity_handle Entity on which tag is being set
     * \param tag_handle Tag being set on an entity
     * \param tag_value Tag value being set on entity
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_setESHData( FBiGeom_Instance instance,
                         iBase_EntityHandle entity_handle,
                         iBase_TagHandle tag_handle,
                         iBase_EntitySetHandle tag_value, 
                         int *err );

    /**\brief  Get all the tags associated with a specified entity handle
     *
     * Get all the tags associated with a specified entity handle
     * \param instance FBiGeom instance handle
     * \param entity_handle Entity being queried
     * \param *tag_handles Pointer to array of tag_handles returned from 
     *        function
     * \param *tag_handles_allocated Pointer to allocated size of tag_handles 
     *        array
     * \param *tag_handles_size Pointer to occupied size of tag_handles array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getAllTags( FBiGeom_Instance instance,
                         iBase_EntityHandle entity_handle,
                         iBase_TagHandle** tag_handles,
                         int* tag_handles_allocated,
                         int* tag_handles_size, 
                         int *err );

    /**\brief  Remove a tag value from an entity
     *
     * Remove a tag value from an entity
     * \param instance FBiGeom instance handle
     * \param entity_handle Entity from which tag is being removed
     * \param tag_handle Tag handle of tag being removed
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_rmvTag( FBiGeom_Instance instance,
                     iBase_EntityHandle entity_handle,
                     iBase_TagHandle tag_handle, 
                     int *err );

    /**\brief  Subtract contents of one entity set from another
     *
     * Subtract contents of one entity set from another
     * \param instance FBiGeom instance handle
     * \param entity_set_1 Entity set from which other set is being subtracted
     * \param entity_set_2 Entity set being subtracted from other set
     * \param result_entity_set Pointer to entity set returned from function
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_subtract( FBiGeom_Instance instance,
                       iBase_EntitySetHandle entity_set_1,
                       iBase_EntitySetHandle entity_set_2,
                       iBase_EntitySetHandle* result_entity_set, 
                       int *err );

    /**\brief  Intersect contents of one entity set with another
     *
     * Intersect contents of one entity set with another
     * \param instance FBiGeom instance handle
     * \param entity_set_1 Entity set being intersected with another
     * \param entity_set_2 Entity set being intersected with another
     * \param result_entity_set Pointer to entity set returned from function
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_intersect( FBiGeom_Instance instance,
                        iBase_EntitySetHandle entity_set_1,
                        iBase_EntitySetHandle entity_set_2,
                        iBase_EntitySetHandle* result_entity_set, 
                        int *err );

    /**\brief  Unite contents of one entity set with another
     *
     * Unite contents of one entity set with another
     * \param instance FBiGeom instance handle
     * \param entity_set_1 Entity set being united with another
     * \param entity_set_2 Entity set being united with another
     * \param result_entity_set Pointer to entity set returned from function
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_unite( FBiGeom_Instance instance,
                    iBase_EntitySetHandle entity_set_1,
                    iBase_EntitySetHandle entity_set_2,
                    iBase_EntitySetHandle* result_entity_set, 
                    int *err );

    /**\brief  Return facet information from solid modeling engine
     *
     * Return facet information from solid modeling engine
     * \param instance FBiGeom instance handle
     * \param entity_handle Entity being queried
     * \param dist_tolerance Tolerance guidance for faceting engine
     * \param points List of vertices in faceting of curve or surface
     * \param points_allocated Allocated size of vertex list array
     * \param points_size Occupied size of vertex list array
     * \param facets List of facets in faceting of surface
     * \param facets_allocated Allocated size of facet list array
     * \param facets_size Occupied size of facet list array
     * \param *err Pointer to error type returned from function
     */
  void FBiGeom_getFacets(FBiGeom_Instance instance,
                       iBase_EntityHandle entity,
                       double dist_tolerance,
                       double **points, int *points_allocated, int *points_size,
                       int **facets, int *facets_allocated, int *facets_size,
                       int *err);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
