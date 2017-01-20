#include <iostream>
#include <map>
#include "FBiGeom_MOAB.hpp"
#include "moab/GeomTopoTool.hpp"
#include "moab/OrientedBoxTreeTool.hpp"
#include "moab/CartVect.hpp"
#include "moab/FileOptions.hpp"
#include "MBTagConventions.hpp"
#include <stdlib.h>
#include <cstring>
#include <map>
#include "assert.h"

using namespace moab;

static int compare_no_case1(const char *str1, const char *str2, size_t n) {
  for (size_t i = 1; i != n && *str1 && toupper(*str1) == toupper(*str2);
       ++i, ++str1, ++str2);
  return toupper(*str2) - toupper(*str1);
}
// Filter out non-MOAB options and remove the "moab:" prefix
static std::string filter_options1(const char *begin, const char *end)
{
  const char *opt_begin = begin;
  const char *opt_end   = begin;

  std::string filtered;
  bool first = true;

  while (opt_end != end) {
    opt_end = std::find(opt_begin, end, ' ');

    if (opt_end-opt_begin >= 5 && compare_no_case1(opt_begin, "moab:", 5) == 0) {
      if (!first)
        filtered.push_back(';');
      first = false;
      filtered.append(opt_begin+5, opt_end);
    }

    opt_begin = opt_end+1;
  }
  return filtered;
}

bool debug_igeom = false;
bool Debug_surf_eval = false;

#define COPY_RANGE(r, vec) {                                          \
      EntityHandle *tmp_ptr = reinterpret_cast<EntityHandle*>(vec);	\
      std::copy(r.begin(), r.end(), tmp_ptr);}

#define TAG_HANDLE(tagh) reinterpret_cast<Tag>(tagh)

#define COPY_DOUBLEVEC(r, vec) {                            \
      double *tmp_ptr = reinterpret_cast<double*>(vec);     \
      std::copy(r.begin(), r.end(), tmp_ptr);}

void FBiGeom_getDescription(FBiGeom_Instance instance, char* descr, int descr_len) {
  iMesh_getDescription( IMESH_INSTANCE(instance), descr, descr_len);
}

void FBiGeom_getErrorType(FBiGeom_Instance instance, /*out*/int *error_type) {
  iMesh_getErrorType( IMESH_INSTANCE(instance), /*out*/error_type) ;
}

void FBiGeom_newGeom(char const* options, FBiGeom_Instance* instance_out, int* err,
                     int options_len) {

  std::string tmp_options = filter_options1(options, options+options_len);
  FileOptions opts(tmp_options.c_str());
    // process some options?

  MBiGeom **mbigeom = reinterpret_cast<MBiGeom**> (instance_out);
  *mbigeom = NULL;
  *mbigeom = new MBiGeom();
  *err = iBase_SUCCESS;
}

void FBiGeom_dtor(FBiGeom_Instance instance, int* err) {
  MBiGeom **mbigeom = reinterpret_cast<MBiGeom**> (&instance);
  if (*mbigeom)
    delete *mbigeom;
  *err = iBase_SUCCESS;
}

void FBiGeom_newGeomFromMesh(iMesh_Instance mesh, iBase_EntitySetHandle set,
                             const char *options, FBiGeom_Instance *geom,
                             int *err, int )
{
  MBiMesh * mbimesh = reinterpret_cast<MBiMesh *>(mesh);
  moab::Interface * mbi = mbimesh->mbImpl;
  moab::EntityHandle rootSet = reinterpret_cast<moab::EntityHandle> (set);
  moab::GeomTopoTool * gtt = new moab::GeomTopoTool(mbi, true, rootSet);
  bool smooth = false; // decide from options
  char smth[] = "SMOOTH;";
  const char * res = strstr(options, smth);
  if (res!=NULL)
    smooth = true;
  moab::FBEngine * fbe = new moab::FBEngine(mbi, gtt, smooth);
  MBiGeom **mbigeom = reinterpret_cast<MBiGeom**> (geom);
  *mbigeom = NULL;
  *mbigeom = new MBiGeom(mbimesh, fbe);
    // will do now the initialization of the engine;
    // heavy duty computation
  fbe->Init();
  *err = iBase_SUCCESS;
}
// corresponding to constructor 2, from iMesh instance
void FBiGeom_dtor2(FBiGeom_Instance instance, int* err) {
  moab::FBEngine * fbe = FBE_cast(instance);
  if (fbe)
  {
    moab::GeomTopoTool * gtt = fbe->get_gtt();
    if (gtt)
      delete gtt;
    delete fbe;
  }
  MBiGeom **mbigeom = reinterpret_cast<MBiGeom**> (&instance);
  if (*mbigeom)
    delete *mbigeom;
  *err = iBase_SUCCESS;
}

void FBiGeom_load(FBiGeom_Instance instance, char const* name, char const* options,
                  int* err, int , int options_len) {
    // first remove option for smooth facetting

  const char smth[] = "SMOOTH;";
  bool smooth = false;
  const char * res = NULL;

  char * reducedOptions = NULL;
  bool localReduce = false;
  if (options)
    res = strstr(options, smth);
  if (res) {
      // extract that option, will not be recognized by our moab/imesh
    reducedOptions = new char[options_len - 6];
    localReduce = true;
    int preLen = (int) (res - options);
    strncpy(reducedOptions, options, preLen);
    int postLen = options_len - 7 - preLen;

    char * tmp = reducedOptions + preLen;

    strncpy(tmp, res + 7, postLen);
    reducedOptions[options_len - 7] = 0;
    std::cout << reducedOptions << std::endl;
    smooth = true;

  } else {
    reducedOptions = const_cast<char *> (options);
  }
    // load mesh-based geometry
  const EntityHandle* file_set = 0;
  ErrorCode rval = MBI->load_file(name, file_set, reducedOptions);
  if (localReduce)
    delete [] reducedOptions;
  CHKERR(rval, "can't load mesh file");

  FBEngine * fbe = FBE_cast(instance);
  if (fbe == NULL) {
    *err = iBase_FAILURE;
    return;
  }
  GeomTopoTool * gtt = GETGTT(instance);
  if (gtt == NULL) {
    *err = iBase_FAILURE;
    return;
  }
    // keep mesh-based geometries in Range
  rval = gtt->find_geomsets();
  CHKERR(rval, "Failure to find geometry lists.");

  if (smooth) fbe->set_smooth();// assumes that initialization did not happen yet

  fbe->Init();// major computation

  RETURN(iBase_SUCCESS);
}

void FBiGeom_save(FBiGeom_Instance instance, char const* name, char const* options,
                  int* err, int name_len, int options_len) {
  iMesh_save(IMESH_INSTANCE(instance), NULL, name, options, err, name_len,
             options_len);
}

void FBiGeom_getRootSet(FBiGeom_Instance instance, iBase_EntitySetHandle* root_set,
                        int* err) {
  EntityHandle modelSet;
  ErrorCode rval = FBE_cast(instance)->getRootSet(&modelSet);
  CHKERR(rval,"can't get root set ");
  *root_set =  (iBase_EntitySetHandle)modelSet;
  RETURN(iBase_SUCCESS);
}

void FBiGeom_getBoundBox(FBiGeom_Instance instance, double* , double* ,
                         double* , double* , double* , double* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_getEntities(FBiGeom_Instance instance,
                         iBase_EntitySetHandle set_handle, int entity_type,
                         iBase_EntityHandle** entity_handles, int* entity_handles_allocated,
                         int* entity_handles_size, int* err) {

  if (0 > entity_type || 4 < entity_type) {
    ERROR(iBase_INVALID_ENTITY_TYPE, "Bad entity type.");
  } else/* 0<= entity_type <= 4) */ {
    Range gentities;
    ErrorCode rval= FBE_cast(instance)->getEntities((EntityHandle)set_handle, entity_type, gentities);
    CHKERR(rval,"can't get entities ");
    *entity_handles_size = gentities.size();

    CHECK_SIZE(*entity_handles, *entity_handles_allocated,
               *entity_handles_size, iBase_EntityHandle, NULL);
    COPY_RANGE(gentities, *entity_handles);
  }

  RETURN(iBase_SUCCESS);
}

void FBiGeom_getNumOfType(FBiGeom_Instance instance,
                          iBase_EntitySetHandle set_handle, int entity_type, int* num_out, int* err) {
  if (0 > entity_type || 4 < entity_type) {
    ERROR(iBase_INVALID_ENTITY_TYPE, "Bad entity type.");
  }
  ErrorCode rval = FBE_cast(instance)->getNumOfType((EntityHandle)set_handle, entity_type, num_out);
  CHKERR(rval,"can't get number of type ");

  RETURN(iBase_SUCCESS);
}


void FBiGeom_getEntType(FBiGeom_Instance instance,
                        iBase_EntityHandle entity_handle, int* type, int* err) {

  ErrorCode rval = FBE_cast(instance)->getEntType((EntityHandle)entity_handle, type);
  CHKERR(rval,"can't get entity type ");

  RETURN(iBase_SUCCESS);
}

void FBiGeom_getArrType(FBiGeom_Instance instance,
                        iBase_EntityHandle const* entity_handles, int entity_handles_size,
                        int** type, int* type_allocated, int* type_size, int* err) {
  CHECK_SIZE(*type, *type_allocated, entity_handles_size, int, NULL);
  *type_size=entity_handles_size;

  int tmp_err;

  for (int i = 0; i < entity_handles_size; i++) {
    FBiGeom_getEntType(instance, entity_handles[i], *type + i, &tmp_err);
    if (iBase_SUCCESS != tmp_err) {
      ERROR(tmp_err, "Failed to get entity type in FBiGeom_getArrType.");
    }
  }

  RETURN(iBase_SUCCESS);
}

void FBiGeom_getEntAdj(FBiGeom_Instance instance, iBase_EntityHandle entity_handle,
                       int to_dimension, iBase_EntityHandle** adj_entities,
                       int* adj_entities_allocated, int* adj_entities_size, int* err) {
  Range adjs;
  EntityHandle this_ent = MBH_cast(entity_handle);

  ErrorCode rval = FBE_cast(instance)->getEntAdj(this_ent, to_dimension,
                                                 adjs);

  CHKERR(rval, "Failed to get adjacent entities in FBiGeom_getEntAdj.");

    // copy adjacent entities
  *adj_entities_size = adjs.size();
  CHECK_SIZE(*adj_entities, *adj_entities_allocated,
             *adj_entities_size, iBase_EntityHandle, NULL);
  COPY_RANGE(adjs, *adj_entities);

  RETURN(iBase_SUCCESS);
}

// I suspect this is wrong
void FBiGeom_getArrAdj(FBiGeom_Instance instance,
                       iBase_EntityHandle const* entity_handles, int entity_handles_size,
                       int requested_entity_type, iBase_EntityHandle** adj_entity_handles,
                       int* adj_entity_handles_allocated, int* adj_entity_handles_size,
                       int** offset, int* offset_allocated, int* offset_size, int* err) {
    // check offset array size
  Range temp_range, total_range;
  CHECK_SIZE(*offset, *offset_allocated, entity_handles_size + 1, int, NULL);
  *offset_size = entity_handles_size + 1;

    // get adjacent entities
  for (int i = 0; i < entity_handles_size; ++i) {
    (*offset)[i] = total_range.size();
    temp_range.clear();
    ErrorCode rval = FBE_cast(instance)->getEntAdj( MBH_cast(entity_handles[i]),
                                                    requested_entity_type,
                                                    temp_range);
    CHKERR(rval, "Failed to get adjacent entities in FBiGeom_getArrAdj.");
    total_range.merge(temp_range);
  }
  int nTot = total_range.size();
  (*offset)[entity_handles_size] = nTot;

    // copy adjacent entities
  CHECK_SIZE(*adj_entity_handles, *adj_entity_handles_allocated,
             nTot, iBase_EntityHandle, NULL);
  COPY_RANGE(total_range, *adj_entity_handles);
  *adj_entity_handles_size = nTot;

  RETURN(iBase_SUCCESS);
}

void FBiGeom_getEnt2ndAdj(FBiGeom_Instance instance,
                          iBase_EntityHandle entity_handle, int bridge_dimension, int to_dimension,
                          iBase_EntityHandle** adjacent_entities, int* adjacent_entities_allocated,
                          int* adjacent_entities_size, int* err) {
  Range to_ents, bridge_ents, tmp_ents;
  ErrorCode rval = FBE_cast(instance)->getEntAdj(MBH_cast(entity_handle), bridge_dimension,
                                                 bridge_ents);

  CHKERR(rval, "Failed to get adjacent entities in FBiGeom_getEnt2ndAdj.");

  Range::iterator iter, jter, kter, end_jter;
  Range::iterator end_iter = bridge_ents.end();
  for (iter = bridge_ents.begin(); iter != end_iter; ++iter) {
    rval = FBE_cast(instance)->getEntAdj(*iter, to_dimension,
                                         tmp_ents);

    CHKERR(rval, "Failed to get adjacent entities in FBiGeom_getEnt2ndAdj.");

    for (jter = tmp_ents.begin(); jter != end_jter; ++jter) {
      if (to_ents.find(*jter) == to_ents.end()) {
        to_ents.insert(*jter);
      }
    }
    tmp_ents.clear();
  }

  *adjacent_entities_size = to_ents.size();
  CHECK_SIZE(*adjacent_entities, *adjacent_entities_allocated,
             *adjacent_entities_size, iBase_EntityHandle, NULL);
  COPY_RANGE(to_ents, *adjacent_entities);

  RETURN(iBase_SUCCESS);
}


void FBiGeom_getArr2ndAdj(FBiGeom_Instance instance,
                          iBase_EntityHandle const* , int ,
                          int , int ,
                          iBase_EntityHandle** ,
                          int* , int* ,
                          int** , int* , int* , int* err) {
    // not implemented
// who would need this monster, anyway?
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_isEntAdj(FBiGeom_Instance instance, iBase_EntityHandle entity_handle1,
                      iBase_EntityHandle entity_handle2, int* are_adjacent, int* err) {

  bool adjacent_out;
  ErrorCode rval = FBE_cast(instance)->isEntAdj(MBH_cast(entity_handle1), MBH_cast(entity_handle2),
                                                adjacent_out);
  CHKERR(rval, "Failed to get adjacent info");
  *are_adjacent = (int)adjacent_out; // 0 or 1, really

  RETURN(iBase_SUCCESS);
}

void FBiGeom_isArrAdj(FBiGeom_Instance instance,
                      iBase_EntityHandle const* entity_handles_1, int entity_handles_1_size,
                      iBase_EntityHandle const* entity_handles_2, int entity_handles_2_size,
                      int** is_adjacent_info, int* is_adjacent_info_allocated,
                      int* is_adjacent_info_size, int* err) {
  int index1 = 0;
  int index2 = 0;
  size_t index1_step, index2_step;
  int count;

    // If either list contains only 1 entry, compare that entry with
    // every entry in the other list.
  if (entity_handles_1_size == entity_handles_2_size) {
    index1_step = index2_step = 1;
    count = entity_handles_1_size;
  } else if (entity_handles_1_size == 1) {
    index1_step = 0;
    index2_step = 1;
    count = entity_handles_2_size;
  } else if (entity_handles_2_size == 1) {
    index1_step = 1;
    index2_step = 0;
    count = entity_handles_1_size;
  } else {
    RETURN(iBase_INVALID_ENTITY_COUNT);
  }

  CHECK_SIZE(*is_adjacent_info, *is_adjacent_info_allocated,
             count, int, NULL);

  for (int i = 0; i < count; ++i) {
    FBiGeom_isEntAdj(instance, entity_handles_1[index1],
                     entity_handles_2[index2], &((*is_adjacent_info)[i]), err);
    FWDERR();

    index1 += index1_step;
    index2 += index2_step;
  }
  // it is now safe to set the size
  *is_adjacent_info_size = count;

  RETURN(iBase_SUCCESS);
}

void FBiGeom_getEntClosestPt(FBiGeom_Instance instance,
                             iBase_EntityHandle entity_handle, double near_x, double near_y,
                             double near_z, double* on_x, double* on_y, double* on_z, int* err) {

  ErrorCode rval = FBE_cast(instance)->getEntClosestPt(MBH_cast(entity_handle), near_x,
                                                       near_y, near_z, on_x, on_y, on_z);
  CHKERR(rval, "Failed to get closest point");

  RETURN(iBase_SUCCESS);
}


void FBiGeom_getArrClosestPt(FBiGeom_Instance instance,
                             iBase_EntityHandle const* entity_handles, int entity_handles_size,
                             int storage_order, double const* near_coordinates,
                             int near_coordinates_size, double** on_coordinates,
                             int* on_coordinates_allocated, int* on_coordinates_size, int* err) {
  CHECK_SIZE(*on_coordinates, *on_coordinates_allocated,
             near_coordinates_size, double, NULL);

  for (int i = 0; i < entity_handles_size; i++) {
    if (storage_order == iBase_INTERLEAVED) {
      FBiGeom_getEntClosestPt(instance, entity_handles[i],
          near_coordinates[3*i], near_coordinates[3*i+1], near_coordinates[3*i+2],
          on_coordinates[3*i], on_coordinates[3*i+1], on_coordinates[3*i+2],
          err);
    } else if (storage_order == iBase_BLOCKED) {
      FBiGeom_getEntClosestPt(instance, entity_handles[i],
          near_coordinates[i], near_coordinates[i+entity_handles_size], near_coordinates[i+2*entity_handles_size],
          on_coordinates[i], on_coordinates[i+entity_handles_size], on_coordinates[i+2*entity_handles_size],
          err);
    }
    FWDERR();
  }
  *on_coordinates_size = near_coordinates_size;

  RETURN(iBase_SUCCESS);
}

void FBiGeom_getEntNrmlXYZ(FBiGeom_Instance instance,
                           iBase_EntityHandle entity_handle, double x, double y, double z,
                           double* nrml_i, double* nrml_j, double* nrml_k, int* err) {

  ErrorCode rval = FBE_cast(instance)->getEntNrmlXYZ(MBH_cast(entity_handle),  x,
                                                     y,  z,   nrml_i,  nrml_j,  nrml_k);
  CHKERR(rval, "Failed to get normal");
  RETURN(iBase_SUCCESS);
}

void FBiGeom_getArrNrmlXYZ(FBiGeom_Instance instance,
                           iBase_EntityHandle const* entity_handles, int entity_handles_size,
                           int storage_order, double const* coordinates, int coordinates_size,
                           double** normals, int* normals_allocated, int* normals_size, int* err) {
    // set up iteration according to storage order.
    // allow either gentity_handles or near_coordinates to contain
    // only one value, where that single value is applied for every
    // entry in the other list.
  size_t index = 0;
  size_t coord_step, norm_step = 1, ent_step;
  int count;
  if (3 * entity_handles_size == coordinates_size) {
    coord_step = ent_step = 1;
    count = entity_handles_size;
  } else if (coordinates_size == 3) {
    coord_step = 0;
    ent_step = 1;
    count = entity_handles_size;
  } else if (entity_handles_size == 1) {
    coord_step = 1;
    ent_step = 0;
    count = coordinates_size / 3;
  } else {
    ERROR(iBase_INVALID_ENTITY_COUNT, "Mismatched array sizes");
  }

    // check or pre-allocate the coordinate arrays
  CHECK_SIZE(*normals, *normals_allocated, 3*count, double, NULL);

  const double *coord_x, *coord_y, *coord_z;
  double *norm_x, *norm_y, *norm_z;
  if (storage_order == iBase_BLOCKED) {
    coord_x = coordinates;
    coord_y = coord_x + coordinates_size / 3;
    coord_z = coord_y + coordinates_size / 3;
    norm_x = *normals;
    norm_y = norm_x + count;
    norm_z = norm_y + count;
    norm_step = 1;
  } else {
    storage_order = iBase_INTERLEAVED; /* set if unspecified */
    coord_x = coordinates;
    coord_y = coord_x + 1;
    coord_z = coord_x + 2;
    norm_x = *normals;
    norm_y = norm_x + 1;
    norm_z = norm_x + 2;
    coord_step *= 3;
    norm_step = 3;
  }

  for (int i = 0; i < count; ++i) {
    FBiGeom_getEntNrmlXYZ(instance, entity_handles[index], *coord_x, *coord_y,
                          *coord_z, norm_x, norm_y, norm_z, err);
    FWDERR();

    index += ent_step;
    coord_x += coord_step;
    coord_y += coord_step;
    coord_z += coord_step;
    norm_x += norm_step;
    norm_y += norm_step;
    norm_z += norm_step;
  }
  *normals_size = count;
  RETURN(iBase_SUCCESS);
}

void FBiGeom_getEntNrmlPlXYZ(FBiGeom_Instance instance,
                             iBase_EntityHandle entity_handle, double x, double y, double z,
                             double* pt_x, double* pt_y, double* pt_z, double* nrml_i, double* nrml_j,
                             double* nrml_k, int* err) {
    // just do for surface and volume
  int type;
  FBiGeom_getEntType(instance, entity_handle, &type, err);
  FWDERR();

  if (type != 2 && type != 3) {
    ERROR(iBase_INVALID_ENTITY_TYPE,
          "Entities passed into gentityNormal must be face or volume.");
  }

    // do 2 searches, so it is not fast enough
  FBiGeom_getEntClosestPt(instance,
                          entity_handle, x, y, z,  pt_x, pt_y, pt_z,  err);

  FWDERR();
  FBiGeom_getEntNrmlXYZ(instance,
                        entity_handle, *pt_x, *pt_y, *pt_z,
                        nrml_i,   nrml_j,  nrml_k,   err);
  FWDERR();

  RETURN(iBase_SUCCESS);
}

void FBiGeom_getArrNrmlPlXYZ(FBiGeom_Instance instance,
                             iBase_EntityHandle const* entity_handles, int entity_handles_size,
                             int storage_order, double const* near_coordinates,
                             int near_coordinates_size, double** on_coordinates,
                             int* on_coordinates_allocated, int* on_coordinates_size,
                             double** normals, int* normals_allocated, int* normals_size, int* err) {
    // set up iteration according to storage order.
    // allow either gentity_handles or near_coordinates to contain
    // only one value, where that single value is applied for every
    // entry in the other list.
  size_t index = 0;
  size_t near_step, on_step = 1, ent_step;
  int count;
  if (3 * entity_handles_size == near_coordinates_size) {
    near_step = ent_step = 1;
    count = entity_handles_size;
  } else if (near_coordinates_size == 3) {
    near_step = 0;
    ent_step = 1;
    count = entity_handles_size;
  } else if (entity_handles_size == 1) {
    near_step = 1;
    ent_step = 0;
    count = near_coordinates_size / 3;
  } else {
    ERROR(iBase_INVALID_ENTITY_COUNT, "Mismatched array sizes");
  }

    // check or pre-allocate the coordinate arrays
  CHECK_SIZE(*on_coordinates, *on_coordinates_allocated, 3*count, double, NULL);
  CHECK_SIZE(*normals, *normals_allocated, 3*count, double, NULL);

  const double *near_x, *near_y, *near_z;
  double *on_x, *on_y, *on_z;
  double *norm_x, *norm_y, *norm_z;
  if (storage_order == iBase_BLOCKED) {
    near_x = near_coordinates;
    near_y = near_x + near_coordinates_size / 3;
    near_z = near_y + near_coordinates_size / 3;
    on_x = *on_coordinates;
    on_y = on_x + count;
    on_z = on_y + count;
    norm_x = *normals;
    norm_y = norm_x + count;
    norm_z = norm_y + count;
    on_step = 1;
  } else {
    storage_order = iBase_INTERLEAVED; /* set if unspecified */
    near_x = near_coordinates;
    near_y = near_x + 1;
    near_z = near_x + 2;
    on_x = *on_coordinates;
    on_y = on_x + 1;
    on_z = on_x + 2;
    norm_x = *normals;
    norm_y = norm_x + 1;
    norm_z = norm_x + 2;
    near_step *= 3;
    on_step = 3;
  }

  for (int i = 0; i < count; ++i) {
    FBiGeom_getEntNrmlPlXYZ(instance, entity_handles[index], *near_x, *near_y,
                            *near_z, on_x, on_y, on_z, norm_x, norm_y, norm_z, err);
    FWDERR();

      //entities += ent_step;
    index += ent_step;
    near_x += near_step;
    near_y += near_step;
    near_z += near_step;
    on_x += on_step;
    on_y += on_step;
    on_z += on_step;
    norm_x += on_step;
    norm_y += on_step;
    norm_z += on_step;
  }
  *on_coordinates_size=count*3;
  *normals_size = count;
  RETURN(iBase_SUCCESS);
}

void FBiGeom_getEntTgntXYZ(FBiGeom_Instance instance,
                           iBase_EntityHandle ,
                           double , double , double ,
                           double* , double* , double* ,
                           int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_getArrTgntXYZ(FBiGeom_Instance instance,
                           iBase_EntityHandle const* ,
                           int ,
                           int , double const* ,
                           int ,
                           double** , int* ,
                           int* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_getEntBoundBox(FBiGeom_Instance instance,
                            iBase_EntityHandle entity_handle, double* min_x, double* min_y,
                            double* min_z, double* max_x, double* max_y, double* max_z, int* err) {
  ErrorCode rval;
  int type;
  FBiGeom_getEntType(instance, entity_handle, &type, err);
  FWDERR();

  if (type == 0) {
    FBiGeom_getVtxCoord(instance, entity_handle, min_x, min_y, min_z, err);
    FWDERR();
    max_x = min_x;
    max_y = min_y;
    max_z = min_z;
  } else if (type == 1) {
      // it could be relatively easy to support
    *err = iBase_NOT_SUPPORTED;
    FWDERR();
  } else if (type == 2 || type == 3) {

    EntityHandle root;
    CartVect center, axis[3];
    GeomTopoTool * gtt = GETGTT(instance);
    if (!gtt)
      ERROR(iBase_FAILURE, "Can't get geom topo tool.");
    rval = gtt->get_root(MBH_cast(entity_handle), root);
    CHKERR(rval, "Failed to get tree root in FBiGeom_getEntBoundBox.");
    rval = gtt->obb_tree()->box(root, center.array(),
                                axis[0].array(), axis[1].array(), axis[2].array());
    CHKERR(rval, "Failed to get box from obb tree.");

    CartVect absv[3];
    for (int i=0; i<3; i++)
    {
      absv[i]= CartVect( fabs(axis[i][0]), fabs(axis[i][1]), fabs(axis[i][2]) );
    }
    CartVect min, max;
    min = center - absv[0] - absv[1] - absv[2];
    max = center + absv[0] + absv[1] + absv[2];
    *min_x = min[0];
    *min_y = min[1];
    *min_z = min[2];
    *max_x = max[0];
    *max_y = max[1];
    *max_z = max[2];
  } else
    RETURN(iBase_INVALID_ENTITY_TYPE);

  RETURN(iBase_SUCCESS);
}

void FBiGeom_getArrBoundBox(FBiGeom_Instance instance,
                            iBase_EntityHandle const* entity_handles, int entity_handles_size,
                            int storage_order, double** min_corner, int* min_corner_allocated,
                            int* min_corner_size, double** max_corner, int* max_corner_allocated,
                            int* max_corner_size, int* err) {
    // check or pre-allocate the coordinate arrays
  CHECK_SIZE(*min_corner, *min_corner_allocated, 3*entity_handles_size, double, NULL);
  CHECK_SIZE(*max_corner, *max_corner_allocated, 3*entity_handles_size, double, NULL);

  size_t step, init;
  if (storage_order == iBase_BLOCKED) {
    step = 1;
    init = entity_handles_size;
  } else {
    step = 3;
    init = 1;
  }
  double *min_x, *min_y, *min_z, *max_x, *max_y, *max_z;
  min_x = *min_corner;
  max_x = *max_corner;
  min_y = min_x + init;
  max_y = max_x + init;
  min_z = min_y + init;
  max_z = max_y + init;

  for (int i = 0; i < entity_handles_size; ++i) {
    FBiGeom_getEntBoundBox(instance, entity_handles[i], min_x, min_y, min_z,
                           max_x, max_y, max_z, err);
    FWDERR();

    min_x += step;
    max_x += step;
    min_y += step;
    max_y += step;
    min_z += step;
    max_z += step;
  }
  *min_corner_size = 3*entity_handles_size;
  *max_corner_size = 3*entity_handles_size;
  RETURN(iBase_SUCCESS);
}

void FBiGeom_getVtxCoord(FBiGeom_Instance instance,
                         iBase_EntityHandle vertex_handle, double* x, double* y, double* z,
                         int* err) {
  ErrorCode rval = FBE_cast(instance)->getVtxCoord(MBH_cast(vertex_handle), x, y, z);
  CHKERR(rval, "Failed to vertex position");
  RETURN(iBase_SUCCESS);
}

void FBiGeom_getVtxArrCoords(FBiGeom_Instance instance,
                             iBase_EntityHandle const* entity_handles, int entity_handles_size,
                             int storage_order, double** coordinates, int* coordinates_allocated,
                             int* coordinates_size, int* err) {
    // check or pre-allocate the coordinate arrays
  CHECK_SIZE(*coordinates, *coordinates_allocated, 3*entity_handles_size, double, NULL);

  double *x, *y, *z;
  size_t step;
  if (storage_order == iBase_BLOCKED) {
    x = *coordinates;
    y = x + entity_handles_size;
    z = y + entity_handles_size;
    step = 1;
  } else {
    x = *coordinates;
    y = x + 1;
    z = x + 2;
    step = 3;
  }

  for (int i = 0; i < entity_handles_size; i++) {
    FBiGeom_getVtxCoord(instance, entity_handles[i], x, y, z, err);
    x += step;
    y += step;
    z += step;
  }
  *coordinates_size= 3*entity_handles_size;
  RETURN(iBase_SUCCESS);
}

void FBiGeom_getPntRayIntsct(FBiGeom_Instance instance, double x, double y, double z,
                             double dir_x, double dir_y, double dir_z,
                             iBase_EntityHandle** intersect_entity_handles,
                             int* intersect_entity_handles_allocated,
                             int* intersect_entity_handles_size, int storage_order,
                             double** intersect_coords, int* intersect_coords_allocated,
                             int* intersect_coords_size, double** param_coords,
                             int* param_coords_allocated, int* param_coords_size, int* err) {
    // this is pretty cool
    // we will return only surfaces
    //
    // storage order is ignored
  std::vector<EntityHandle> intersect_handles;
  std::vector<double> coords;
  std::vector<double> params;
  ErrorCode rval = FBE_cast(instance)->getPntRayIntsct(x, y, z, dir_x,
                                                       dir_y, dir_z,intersect_handles, coords, params);
  CHKERR(rval,"can't get ray intersections ");
  *intersect_entity_handles_size = (int)intersect_handles.size();

  CHECK_SIZE(*intersect_entity_handles, *intersect_entity_handles_allocated,
             *intersect_entity_handles_size, iBase_EntityHandle, NULL);
  *intersect_coords_size = 3*(int)intersect_handles.size();
  CHECK_SIZE(*intersect_coords, *intersect_coords_allocated,
             *intersect_coords_size, double, NULL);
  *param_coords_size=(int)intersect_handles.size();
  CHECK_SIZE(*param_coords, *param_coords_allocated,
             *param_coords_size, double, NULL);

  COPY_RANGE(intersect_handles, *intersect_entity_handles);

  COPY_DOUBLEVEC(params, *param_coords);
  if (storage_order == iBase_BLOCKED) {
    int sz=(int)intersect_handles.size();
    for (int i=0; i<sz; i++)
    {
      *intersect_coords[i]=coords[3*i];
      *intersect_coords[sz+i]=coords[3*i+1];
      *intersect_coords[2*sz+i]=coords[3*i+2];
    }
  } else {
    COPY_DOUBLEVEC(coords, *intersect_coords);
  }

  RETURN(iBase_SUCCESS);
}

void FBiGeom_getPntArrRayIntsct(FBiGeom_Instance instance, int ,
                                const double* , int , const double* ,
                                int , iBase_EntityHandle** ,
                                int* ,
                                int* , int** , int* ,
                                int* , double** ,
                                int* , int* ,
                                double** , int* ,
                                int* , int* err) {
    // not implemented
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_getEntNrmlSense(FBiGeom_Instance instance, iBase_EntityHandle face,
                             iBase_EntityHandle region, int* sense_out, int* err) {
  moab::EntityHandle mbregion = (moab::EntityHandle) region;
  moab::EntityHandle mbface = (moab::EntityHandle) face;
  moab::ErrorCode rval = FBE_cast(instance)->getEgFcSense(  mbface, mbregion, *sense_out );
  CHKERR(rval,"can't get normal sense ");
  RETURN(iBase_SUCCESS);
}
void FBiGeom_getArrNrmlSense(FBiGeom_Instance instance,
                             iBase_EntityHandle const* , int ,
                             iBase_EntityHandle const* , int ,
                             int** , int* , int* , int* err) {
    // not implemented
  RETURN(iBase_NOT_SUPPORTED);
}

/**\brief Get the sense of an edge with respect to a face
 * Get the sense of an edge with respect to a face.  Sense returned is -1, 0, or 1,
 * representing "reversed", "both", or "forward".  "both" sense indicates that edge bounds
 * the face once with each sense.
 * \param edge Edge being queried
 * \param face Face being queried
 * \param sense_out Sense of edge with respect to face
 */

void FBiGeom_getEgFcSense(FBiGeom_Instance instance, iBase_EntityHandle edge,
                          iBase_EntityHandle face, int* sense_out, int* err) {
    // this one is important, for establishing the orientation of the edges in faces
    // bummer, I "thought" it is already implemented
    // use senses
  ErrorCode rval = FBE_cast(instance)->getEgFcSense(MBH_cast(edge), MBH_cast (face),
                                                    *sense_out);

  CHKERR(rval, "Failed to get edge senses in FBiGeom_getEgFcSense.");
  RETURN(iBase_SUCCESS);

}
void FBiGeom_getEgFcArrSense(FBiGeom_Instance instance,
                             iBase_EntityHandle const* , int ,
                             iBase_EntityHandle const* , int ,
                             int** , int* , int* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_getEgVtxSense(FBiGeom_Instance instance, iBase_EntityHandle edge,
                           iBase_EntityHandle vertex1, iBase_EntityHandle vertex2, int* sense_out,
                           int* err) {

  ErrorCode rval = FBE_cast(instance)->getEgVtxSense(MBH_cast(edge), MBH_cast(vertex1),
                                                     MBH_cast(vertex2), *sense_out);
  CHKERR(rval, "Failed to get vertex sense wrt edge in FBiGeom_getEgVtxSense");
  RETURN(iBase_SUCCESS);
}
void FBiGeom_getEgVtxArrSense(FBiGeom_Instance instance,
                              iBase_EntityHandle const* , int ,
                              iBase_EntityHandle const* , int ,
                              iBase_EntityHandle const* , int ,
                              int** , int* , int* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_measure(FBiGeom_Instance instance,
                     iBase_EntityHandle const* entity_handles, int entity_handles_size,
                     double** measures, int* measures_allocated, int* measures_size, int* err) {

  CHECK_SIZE(*measures, *measures_allocated, entity_handles_size, double, NULL);
  ErrorCode rval = FBE_cast(instance)->measure((EntityHandle *) (entity_handles) ,
                                               entity_handles_size,  *measures);
  CHKERR(rval, "Failed to get measures");
  *measures_size=entity_handles_size;
  RETURN(iBase_SUCCESS);
}

void FBiGeom_getFaceType(FBiGeom_Instance instance, iBase_EntityHandle ,
                         char* face_type, int* err, int* face_type_length) {
  std::string type = "nonplanar"; // for swept faces created with rays between surfaces,
    // we could actually create planar surfaces; maybe we should recognize them as such
  face_type = new char[type.length()+1];
  strcpy(face_type, type.c_str());
  *face_type_length = type.length()+1;
  RETURN(iBase_SUCCESS);
}
void FBiGeom_getParametric(FBiGeom_Instance instance, int* is_parametric, int* err) {
  *is_parametric = 0; //(false)
  RETURN(iBase_SUCCESS);
}
void FBiGeom_isEntParametric(FBiGeom_Instance instance, iBase_EntityHandle entity_handle,
                             int* parametric, int* err) {
  int type = -1;
  FBiGeom_getEntType(instance, entity_handle, &type, err);
  if (type==1)
    *parametric = 1;// true
  else
    *parametric = 0; // false
  RETURN(iBase_SUCCESS);
}
void FBiGeom_isArrParametric(FBiGeom_Instance instance,
                             iBase_EntityHandle const* , int ,
                             int** , int* ,
                             int* , int* err) {
    // not implemented
  RETURN(iBase_NOT_SUPPORTED);
}
void FBiGeom_getEntUVtoXYZ(FBiGeom_Instance instance, iBase_EntityHandle ,
                           double , double , double* , double* , double* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}
void FBiGeom_getArrUVtoXYZ(FBiGeom_Instance instance,
                           iBase_EntityHandle const* , int ,
                           int , double const* , int , double** ,
                           int* , int* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}


void FBiGeom_getEntUtoXYZ(FBiGeom_Instance instance,
                          iBase_EntityHandle entity_handle, double u, double* x, double* y,
                          double* z, int* err) {
  int type ;
  FBiGeom_getEntType(instance, entity_handle, &type, err);
  FWDERR();

  if (type != 1)  // not edge
    RETURN(iBase_NOT_SUPPORTED);

  ErrorCode rval = FBE_cast(instance)->getEntUtoXYZ(
      (EntityHandle) entity_handle, u, *x, *y, *z );
  CHKERR(rval, "Failed to get position from parameter");
  RETURN(iBase_SUCCESS);
}

void FBiGeom_getArrUtoXYZ(FBiGeom_Instance instance,
                          iBase_EntityHandle const* , int ,
                          double const* , int , int , double** ,
                          int* , int* , int* err) {
    // not implemented
  RETURN(iBase_NOT_SUPPORTED);
}
void FBiGeom_getEntXYZtoUV(FBiGeom_Instance instance, iBase_EntityHandle ,
                           double , double , double , double* , double* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}
void FBiGeom_getEntXYZtoU(FBiGeom_Instance instance, iBase_EntityHandle ,
                          double , double , double , double* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}
void FBiGeom_getArrXYZtoUV(FBiGeom_Instance instance,
                           iBase_EntityHandle const* , int ,
                           int , double const* , int ,
                           double** , int* , int* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}
void FBiGeom_getArrXYZtoU(FBiGeom_Instance instance,
                          iBase_EntityHandle const* , int ,
                          int , double const* , int ,
                          double** , int* , int* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}
void FBiGeom_getEntXYZtoUVHint(FBiGeom_Instance instance, iBase_EntityHandle ,
                               double , double , double , double* , double* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}
void FBiGeom_getArrXYZtoUVHint(FBiGeom_Instance instance,
                               iBase_EntityHandle const* , int ,
                               int , double const* , int , double** ,
                               int* , int* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}
void FBiGeom_getEntUVRange(FBiGeom_Instance instance, iBase_EntityHandle ,
                           double* , double* , double* , double* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_getEntURange(FBiGeom_Instance instance,
                          iBase_EntityHandle entity_handle, double* u_min, double* u_max, int* err) {
  ErrorCode rval = FBE_cast(instance)->getEntURange((EntityHandle) entity_handle,
                                                    *u_min,  *u_max );
  CHKERR(rval, "Failed to get range");
  RETURN(iBase_SUCCESS);
}
void FBiGeom_getArrUVRange(FBiGeom_Instance instance,
                           iBase_EntityHandle const* , int ,
                           int , double** , int* ,
                           int* , double** , int* ,
                           int* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}
void FBiGeom_getArrURange(FBiGeom_Instance instance,
                          iBase_EntityHandle const* , int ,
                          double** , int* , int* , double** ,
                          int* , int* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}
void FBiGeom_getEntUtoUV(FBiGeom_Instance instance, iBase_EntityHandle ,
                         iBase_EntityHandle , double , double* , double* ,
                         int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}
void FBiGeom_getVtxToUV(FBiGeom_Instance instance, iBase_EntityHandle ,
                        iBase_EntityHandle , double* , double* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}
void FBiGeom_getVtxToU(FBiGeom_Instance instance, iBase_EntityHandle ,
                       iBase_EntityHandle , double* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}
void FBiGeom_getArrUtoUV(FBiGeom_Instance instance, iBase_EntityHandle const* ,
                         int , iBase_EntityHandle const* ,
                         int , double const* , int ,
                         int , double** , int* , int* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}
void FBiGeom_getVtxArrToUV(FBiGeom_Instance instance,
                           iBase_EntityHandle const* , int ,
                           iBase_EntityHandle const* , int ,
                           int , double** , int* , int* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}
void FBiGeom_getVtxArrToU(FBiGeom_Instance instance,
                          iBase_EntityHandle const* , int ,
                          iBase_EntityHandle const* , int ,
                          double** , int* , int* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}
void FBiGeom_getEntNrmlUV(FBiGeom_Instance instance, iBase_EntityHandle ,
                          double , double , double* , double* , double* ,
                          int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}
void FBiGeom_getArrNrmlUV(FBiGeom_Instance instance, iBase_EntityHandle const* ,
                          int , int , double const* ,
                          int , double** , int* ,
                          int* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}
void FBiGeom_getEntTgntU(FBiGeom_Instance instance, iBase_EntityHandle entity_handle,
                         double u, double* tgnt_i, double* tgnt_j, double* tgnt_k, int* err) {
  ErrorCode rval = FBE_cast(instance)->getEntTgntU( (moab::EntityHandle)entity_handle ,   u,
                                                    *tgnt_i, *tgnt_j,  *tgnt_k);
  CHKERR(rval, "Failed to get tangent from u");
  RETURN(iBase_SUCCESS);
}
void FBiGeom_getArrTgntU(FBiGeom_Instance instance, iBase_EntityHandle const* ,
                         int , int , double const* ,
                         int , double** , int* ,
                         int* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}
void FBiGeom_getEnt1stDrvt(FBiGeom_Instance instance, iBase_EntityHandle ,
                           double , double , double** , int* ,
                           int* , double** , int* ,
                           int* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}
void FBiGeom_getArr1stDrvt(FBiGeom_Instance instance,
                           iBase_EntityHandle const* , int ,
                           int , double const* , int , double** ,
                           int* , int* , int** ,
                           int* , int* , double** ,
                           int* , int* , int** ,
                           int* , int* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}
void FBiGeom_getEnt2ndDrvt(FBiGeom_Instance instance, iBase_EntityHandle ,
                           double , double , double** , int* ,
                           int* , double** , int* ,
                           int* , double** , int* ,
                           int* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}
void FBiGeom_getArr2ndDrvt(FBiGeom_Instance instance,
                           iBase_EntityHandle const* , int ,
                           int , double const* , int , double** ,
                           int* , int* , int** ,
                           int* , int* , double** ,
                           int* , int* , int** ,
                           int* , int* , double** ,
                           int* , int* , int** ,
                           int* , int* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}
void FBiGeom_getFcCvtrUV(FBiGeom_Instance instance, iBase_EntityHandle ,
                         double , double , double* , double* , double* ,
                         double* , double* , double* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}
void FBiGeom_getFcArrCvtrUV(FBiGeom_Instance instance,
                            iBase_EntityHandle const* , int ,
                            int , double const* , int , double** ,
                            int* , int* , double** ,
                            int* , int* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}
void FBiGeom_isEntPeriodic(FBiGeom_Instance /*instance*/, iBase_EntityHandle /*entity_handle*/,
                           int* in_u, int* in_v , int* err) {
  *in_u = 0;
  *in_v = 0;
  *err = 0;
  return;
}
void FBiGeom_isArrPeriodic(FBiGeom_Instance instance,
                           iBase_EntityHandle const* , int ,
                           int** , int* , int* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}
void FBiGeom_isFcDegenerate(FBiGeom_Instance instance, iBase_EntityHandle ,
                            int* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}
void FBiGeom_isFcArrDegenerate(FBiGeom_Instance instance,
                               iBase_EntityHandle const* , int ,
                               int** , int* , int* ,
                               int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_initEntIter(FBiGeom_Instance instance, iBase_EntitySetHandle ,
                         int , iBase_EntityIterator* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_initEntArrIter(FBiGeom_Instance instance,
                            iBase_EntitySetHandle , int ,
                            int , iBase_EntityArrIterator* ,
                            int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_getNextEntIter(FBiGeom_Instance instance, iBase_EntityIterator,
                            iBase_EntityHandle* , int* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_getNextEntArrIter(FBiGeom_Instance instance, iBase_EntityArrIterator,
                               iBase_EntityHandle** , int* ,
                               int* , int* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_resetEntIter(FBiGeom_Instance instance, iBase_EntityIterator, int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_resetEntArrIter(FBiGeom_Instance instance, iBase_EntityArrIterator, int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_endEntIter(FBiGeom_Instance instance, iBase_EntityIterator, int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_endEntArrIter(FBiGeom_Instance instance, iBase_EntityArrIterator, int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_copyEnt(FBiGeom_Instance instance, iBase_EntityHandle ,
                     iBase_EntityHandle* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_sweepEntAboutAxis(FBiGeom_Instance instance, iBase_EntityHandle ,
                               double , double , double ,
                               double , iBase_EntityHandle* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_deleteAll(FBiGeom_Instance instance, int* err) {
    // it means deleting some sets from moab db ; is this what we want?
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_deleteEnt(FBiGeom_Instance instance, iBase_EntityHandle /*entity_handle*/,
                       int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_createSphere(FBiGeom_Instance instance, double ,
                          iBase_EntityHandle* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_createPrism(FBiGeom_Instance instance, double , int ,
                         double , double , iBase_EntityHandle* ,
                         int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_createBrick(FBiGeom_Instance instance, double , double , double ,
                         iBase_EntityHandle* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_createCylinder(FBiGeom_Instance instance, double , double ,
                            double , iBase_EntityHandle* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_createCone(FBiGeom_Instance instance, double , double ,
                        double , double , iBase_EntityHandle* ,
                        int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_createTorus(FBiGeom_Instance instance, double , double ,
                         iBase_EntityHandle* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_moveEnt(FBiGeom_Instance instance, iBase_EntityHandle , double ,
                     double , double , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_rotateEnt(FBiGeom_Instance instance, iBase_EntityHandle ,
                       double , double , double ,
                       double , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_reflectEnt(FBiGeom_Instance instance, iBase_EntityHandle ,
                        double, double, double, double,
                        double, double, int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_scaleEnt(FBiGeom_Instance instance, iBase_EntityHandle ,
                      double,  double, double, double,
                      double, double, int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_uniteEnts(FBiGeom_Instance instance, iBase_EntityHandle const* ,
                       int , iBase_EntityHandle* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_subtractEnts(FBiGeom_Instance instance, iBase_EntityHandle ,
                          iBase_EntityHandle , iBase_EntityHandle* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_intersectEnts(FBiGeom_Instance instance, iBase_EntityHandle ,
                           iBase_EntityHandle , iBase_EntityHandle* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_sectionEnt(FBiGeom_Instance instance, iBase_EntityHandle ,
                        double , double , double ,
                        double , int , iBase_EntityHandle* , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_imprintEnts(FBiGeom_Instance instance, iBase_EntityHandle const* ,
                         int , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_mergeEnts(FBiGeom_Instance instance, iBase_EntityHandle const* ,
                       int , double , int* err) {
  RETURN(iBase_NOT_SUPPORTED);
}
// start copy old

void FBiGeom_createEntSet(FBiGeom_Instance instance, int isList,
                          iBase_EntitySetHandle* entity_set_created, int *err) {
  iMesh_createEntSet(IMESH_INSTANCE(instance), isList, entity_set_created, err);
  FWDERR();
}

void FBiGeom_destroyEntSet(FBiGeom_Instance instance,
                           iBase_EntitySetHandle , int *err) {
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_isList(FBiGeom_Instance instance, iBase_EntitySetHandle ,
                    int* , int *err) {
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_getNumEntSets(FBiGeom_Instance instance,
                           iBase_EntitySetHandle entity_set_handle, int  /*num_hops*/, int *num_sets,
                           int *err) {
    //  here, we get only the entity sets that have gents as members
    // we have the convention that entity sets of geom dimension 4 are
    // sets of geo entities; they should contain only gentities as elements (or other sets of gents)
    // we should also consider the number of hops
    // first, get all sets of geo dim 4 from the entity_set_handle; then intersect with
    // the range from geom topo tool
  Tag geomTag;
  ErrorCode rval = MBI->tag_get_handle(GEOM_DIMENSION_TAG_NAME, 1, MB_TYPE_INTEGER, geomTag);
  if (MB_SUCCESS != rval)
    RETURN(iBase_FAILURE);
  GeomTopoTool * gtt = GETGTT(instance);
  const Range * gRange =gtt-> geoRanges();
    // get all sets of geom dimension 4 from the entity set
  EntityHandle moabSet = (EntityHandle)entity_set_handle;
  const int four = 4;
  const void* const four_val[] = { &four };
  Range tmp;
  rval = MBI->get_entities_by_type_and_tag(moabSet, MBENTITYSET, &geomTag,
                                           four_val, 1, tmp);
  CHKERR(rval,"can't get sets of geo dim 4 ");
  tmp=intersect(tmp, gRange[4]);
  *num_sets = tmp.size();// ignore, for the time being, number of hops

  RETURN(iBase_SUCCESS);
}

void FBiGeom_getEntSets(FBiGeom_Instance instance,
                        iBase_EntitySetHandle entity_set_handle, int ,
                        iBase_EntitySetHandle** contained_set_handles,
                        int* contained_set_handles_allocated, int* contained_set_handles_size,
                        int *err) {
    //  we get only the entity sets that have gents as members
    // we keep the convention that entity sets of geom dimension 4 are
    // sets of geo entities; they should contain only gentities as elements (or other sets of gents)
  Tag geomTag;
  ErrorCode rval = MBI->tag_get_handle(GEOM_DIMENSION_TAG_NAME, 1, MB_TYPE_INTEGER, geomTag);
  if (MB_SUCCESS != rval)
    RETURN(iBase_FAILURE);
  GeomTopoTool * gtt = GETGTT(instance);
  const Range * gRange =gtt-> geoRanges();
    // get all sets of geom dimension 4 from the entity set
  EntityHandle moabSet = (EntityHandle)entity_set_handle;
  const int four = 4;
  const void* const four_val[] = { &four };
  Range tmp;
  rval = MBI->get_entities_by_type_and_tag(moabSet, MBENTITYSET, &geomTag,
                                           four_val, 1, tmp);
  CHKERR(rval,"can't get sets of geo dim 4 ");
  tmp=intersect(tmp, gRange[4]);
  *contained_set_handles_size = tmp.size();
  CHECK_SIZE(*contained_set_handles, *contained_set_handles_allocated,
             *contained_set_handles_size, iBase_EntitySetHandle, NULL);
  COPY_RANGE(tmp, *contained_set_handles);

  RETURN(iBase_SUCCESS);
}

void FBiGeom_addEntToSet(FBiGeom_Instance instance,
                         iBase_EntityHandle entity_handle, iBase_EntitySetHandle entity_set,
                         int *err) {
  iMesh_addEntToSet(IMESH_INSTANCE(instance), entity_handle, entity_set, err);
}

void FBiGeom_rmvEntFromSet(FBiGeom_Instance instance,
                           iBase_EntityHandle entity_handle, iBase_EntitySetHandle entity_set,
                           int *err) {
  iMesh_rmvEntFromSet(IMESH_INSTANCE(instance), entity_handle, entity_set, err);
}

void FBiGeom_addEntArrToSet(FBiGeom_Instance instance,
                            const iBase_EntityHandle* entity_handles, int entity_handles_size,
                            iBase_EntitySetHandle entity_set, int *err) {
  iMesh_addEntArrToSet(IMESH_INSTANCE(instance), entity_handles,
                       entity_handles_size, entity_set, err);
}

void FBiGeom_rmvEntArrFromSet(FBiGeom_Instance instance,
                              const iBase_EntityHandle* entity_handles, int entity_handles_size,
                              iBase_EntitySetHandle entity_set, int *err) {
  iMesh_rmvEntArrFromSet(IMESH_INSTANCE(instance), entity_handles,
                         entity_handles_size, entity_set, err);
}

void FBiGeom_addEntSet(FBiGeom_Instance instance,
                       iBase_EntitySetHandle entity_set_to_add,
                       iBase_EntitySetHandle entity_set_handle, int *err) {
  iMesh_addEntSet(IMESH_INSTANCE(instance), entity_set_to_add,
                  entity_set_handle, err);
}

void FBiGeom_rmvEntSet(FBiGeom_Instance instance,
                       iBase_EntitySetHandle entity_set_to_remove,
                       iBase_EntitySetHandle entity_set_handle, int *err) {
  iMesh_rmvEntSet(IMESH_INSTANCE(instance), entity_set_to_remove,
                  entity_set_handle, err);
}

void FBiGeom_isEntContained(FBiGeom_Instance instance,
                            iBase_EntitySetHandle containing_entity_set,
                            iBase_EntityHandle contained_entity, int *is_contained, int *err) {
  iMesh_isEntContained(IMESH_INSTANCE(instance), containing_entity_set,
                       contained_entity, is_contained, err);
}

void FBiGeom_isEntArrContained(FBiGeom_Instance instance,
                               iBase_EntitySetHandle containing_set,
                               const iBase_EntityHandle* entity_handles, int num_entity_handles,
                               int** is_contained, int* is_contained_allocated, int* is_contained_size,
                               int* err) {
  iMesh_isEntArrContained(IMESH_INSTANCE(instance), containing_set,
                          entity_handles, num_entity_handles, is_contained,
                          is_contained_allocated, is_contained_size, err);
}

void FBiGeom_isEntSetContained(FBiGeom_Instance instance,
                               iBase_EntitySetHandle containing_entity_set,
                               iBase_EntitySetHandle contained_entity_set, int *is_contained, int *err) {
  iMesh_isEntSetContained(IMESH_INSTANCE(instance), containing_entity_set,
                          contained_entity_set, is_contained, err);
}

void FBiGeom_addPrntChld(FBiGeom_Instance instance,
                         iBase_EntitySetHandle parent_entity_set,
                         iBase_EntitySetHandle child_entity_set, int *err) {
  iMesh_addPrntChld(IMESH_INSTANCE(instance), parent_entity_set,
                    child_entity_set, err);
}

void FBiGeom_rmvPrntChld(FBiGeom_Instance instance,
                         iBase_EntitySetHandle parent_entity_set,
                         iBase_EntitySetHandle child_entity_set, int *err) {
  iMesh_rmvPrntChld(IMESH_INSTANCE(instance), parent_entity_set,
                    child_entity_set, err);
}

void FBiGeom_isChildOf(FBiGeom_Instance instance,
                       iBase_EntitySetHandle parent_entity_set,
                       iBase_EntitySetHandle child_entity_set, int *is_child, int *err) {
  iMesh_isChildOf(IMESH_INSTANCE(instance), parent_entity_set,
                  child_entity_set, is_child, err);
}

void FBiGeom_getNumChld(FBiGeom_Instance instance,
                        iBase_EntitySetHandle entity_set, int num_hops, int *num_child, int *err) {
  iMesh_getNumChld(IMESH_INSTANCE(instance), entity_set, num_hops, num_child,
                   err);
}

void FBiGeom_getNumPrnt(FBiGeom_Instance instance,
                        iBase_EntitySetHandle entity_set, int num_hops, int *num_parent, int *err) {
  iMesh_getNumPrnt(IMESH_INSTANCE(instance), entity_set, num_hops, num_parent,
                   err);
}

void FBiGeom_getChldn(FBiGeom_Instance instance,
                      iBase_EntitySetHandle from_entity_set, int num_hops,
                      iBase_EntitySetHandle** entity_set_handles,
                      int* entity_set_handles_allocated, int* entity_set_handles_size, int *err) {
  iMesh_getChldn(IMESH_INSTANCE(instance), from_entity_set, num_hops,
                 entity_set_handles, entity_set_handles_allocated,
                 entity_set_handles_size, err);
}

void FBiGeom_getPrnts(FBiGeom_Instance instance,
                      iBase_EntitySetHandle from_entity_set, int num_hops,
                      iBase_EntitySetHandle** entity_set_handles,
                      int* entity_set_handles_allocated, int* entity_set_handles_size, int *err) {
  iMesh_getPrnts(IMESH_INSTANCE(instance), from_entity_set, num_hops,
                 entity_set_handles, entity_set_handles_allocated,
                 entity_set_handles_size, err);
}

void FBiGeom_createTag(FBiGeom_Instance instance, const char* tag_name,
                       int tag_size, int tag_type, iBase_TagHandle* tag_handle, int *err,
                       int tag_name_len) {

  iMesh_createTag(IMESH_INSTANCE(instance), tag_name, tag_size, tag_type,
                  tag_handle, err, tag_name_len);
}


void FBiGeom_destroyTag(FBiGeom_Instance instance, iBase_TagHandle tag_handle,
                        int , int *err) {
  ErrorCode rval = MBI->tag_delete(TAG_HANDLE(tag_handle));
  CHKERR(rval, "Failed to delete tag");
  RETURN(iBase_SUCCESS);
}

void FBiGeom_getTagName(FBiGeom_Instance instance, iBase_TagHandle tag_handle,
                        char *name, int* err, int name_len) {
  iMesh_getTagName(IMESH_INSTANCE(instance), tag_handle, name, err, name_len);
}

void FBiGeom_getTagSizeValues(FBiGeom_Instance instance,
                              iBase_TagHandle tag_handle, int *tag_size, int *err) {
  iMesh_getTagSizeValues(IMESH_INSTANCE(instance), tag_handle, tag_size, err);
}

void FBiGeom_getTagSizeBytes(FBiGeom_Instance instance, iBase_TagHandle tag_handle,
                             int *tag_size, int *err) {
  iMesh_getTagSizeBytes(IMESH_INSTANCE(instance), tag_handle, tag_size, err);
}

void FBiGeom_getTagHandle(FBiGeom_Instance instance, const char* tag_name,
                          iBase_TagHandle *tag_handle, int *err, int tag_name_len) {
  iMesh_getTagHandle(IMESH_INSTANCE(instance), tag_name, tag_handle, err,
                     tag_name_len);
}

void FBiGeom_getTagType(FBiGeom_Instance instance, iBase_TagHandle tag_handle,
                        int *tag_type, int *err) {
  iMesh_getTagType(IMESH_INSTANCE(instance), tag_handle, tag_type, err);
}

void FBiGeom_setEntSetData(FBiGeom_Instance instance,
                           iBase_EntitySetHandle entity_set_handle, iBase_TagHandle tag_handle,
                           const void* tag_value, int tag_value_size, int *err) {
  iMesh_setEntSetData(IMESH_INSTANCE(instance), entity_set_handle, tag_handle,
                      tag_value, tag_value_size, err);
}

void FBiGeom_setEntSetIntData(FBiGeom_Instance instance,
                              iBase_EntitySetHandle entity_set, iBase_TagHandle tag_handle,
                              int tag_value, int *err) {
  iMesh_setEntSetIntData(IMESH_INSTANCE(instance), entity_set, tag_handle,
                         tag_value, err);
}

void FBiGeom_setEntSetDblData(FBiGeom_Instance instance,
                              iBase_EntitySetHandle entity_set, iBase_TagHandle tag_handle,
                              double tag_value, int *err) {
  iMesh_setEntSetDblData(IMESH_INSTANCE(instance), entity_set, tag_handle,
                         tag_value, err);
}

void FBiGeom_setEntSetEHData(FBiGeom_Instance instance,
                             iBase_EntitySetHandle entity_set, iBase_TagHandle tag_handle,
                             iBase_EntityHandle tag_value, int *err) {
  iMesh_setEntSetEHData(IMESH_INSTANCE(instance), entity_set, tag_handle,
                        tag_value, err);
}

void FBiGeom_setEntSetESHData(FBiGeom_Instance instance,
                              iBase_EntitySetHandle entity_set, iBase_TagHandle tag_handle,
                              iBase_EntitySetHandle tag_value, int *err) {
  iMesh_setEntSetESHData(IMESH_INSTANCE(instance), entity_set, tag_handle,
                         tag_value, err);
}

void FBiGeom_getEntSetData(FBiGeom_Instance instance,
                           iBase_EntitySetHandle entity_set_handle, iBase_TagHandle tag_handle,
                           void** tag_value, int* tag_value_allocated, int* tag_value_size, int *err) {
  iMesh_getEntSetData(IMESH_INSTANCE(instance), entity_set_handle, tag_handle,
                      tag_value, tag_value_allocated, tag_value_size, err);
}

void FBiGeom_getEntSetIntData(FBiGeom_Instance instance,
                              iBase_EntitySetHandle entity_set, iBase_TagHandle tag_handle,
                              int *out_data, int *err) {
  iMesh_getEntSetIntData(IMESH_INSTANCE(instance), entity_set, tag_handle,
                         out_data, err);
}

void FBiGeom_getEntSetDblData(FBiGeom_Instance instance,
                              iBase_EntitySetHandle entity_set, iBase_TagHandle tag_handle,
                              double *out_data, int *err) {
  iMesh_getEntSetDblData(IMESH_INSTANCE(instance), entity_set, tag_handle,
                         out_data, err);
}

void FBiGeom_getEntSetEHData(FBiGeom_Instance instance,
                             iBase_EntitySetHandle entity_set, iBase_TagHandle tag_handle,
                             iBase_EntityHandle *out_data, int *err) {
  iMesh_getEntSetEHData(IMESH_INSTANCE(instance), entity_set, tag_handle,
                        out_data, err);
}

void FBiGeom_getEntSetESHData(FBiGeom_Instance instance,
                              iBase_EntitySetHandle entity_set, iBase_TagHandle tag_handle,
                              iBase_EntitySetHandle *out_data, int *err) {
  iMesh_getEntSetESHData(IMESH_INSTANCE(instance), entity_set, tag_handle,
                         out_data, err);
}

void FBiGeom_getAllEntSetTags(FBiGeom_Instance instance,
                              iBase_EntitySetHandle entity_set_handle, iBase_TagHandle** tag_handles,
                              int* tag_handles_allocated, int* tag_handles_size, int *err) {
  iMesh_getAllEntSetTags(IMESH_INSTANCE(instance), entity_set_handle,
                         tag_handles, tag_handles_allocated, tag_handles_size, err);
}

void FBiGeom_rmvEntSetTag(FBiGeom_Instance instance,
                          iBase_EntitySetHandle entity_set_handle, iBase_TagHandle tag_handle,
                          int *err) {
  iMesh_rmvEntSetTag(IMESH_INSTANCE(instance), entity_set_handle, tag_handle,
                     err);
}

void FBiGeom_getArrData(FBiGeom_Instance instance,
                        const iBase_EntityHandle* entity_handles, int entity_handles_size,
                        iBase_TagHandle tag_handle, void** tag_values, int* tag_values_allocated,
                        int* tag_values_size, int *err) {
  iMesh_getArrData(IMESH_INSTANCE(instance), entity_handles,
                   entity_handles_size, tag_handle, tag_values, tag_values_allocated,
                   tag_values_size, err);
}

void FBiGeom_getIntArrData(FBiGeom_Instance instance,
                           const iBase_EntityHandle* entity_handles, int entity_handles_size,
                           iBase_TagHandle tag_handle, int** tag_values, int* tag_values_allocated,
                           int* tag_values_size, int *err) {
  iMesh_getIntArrData(IMESH_INSTANCE(instance), entity_handles,
                      entity_handles_size, tag_handle, tag_values, tag_values_allocated,
                      tag_values_size, err);
}

void FBiGeom_getDblArrData(FBiGeom_Instance instance,
                           const iBase_EntityHandle* entity_handles, int entity_handles_size,
                           iBase_TagHandle tag_handle, double** tag_values,
                           int* tag_values_allocated, int* tag_values_size, int *err) {
  iMesh_getDblArrData(IMESH_INSTANCE(instance), entity_handles,
                      entity_handles_size, tag_handle, tag_values, tag_values_allocated,
                      tag_values_size, err);
}

void FBiGeom_getEHArrData(FBiGeom_Instance instance,
                          const iBase_EntityHandle* entity_handles, int entity_handles_size,
                          iBase_TagHandle tag_handle, iBase_EntityHandle** tag_value,
                          int* tag_value_allocated, int* tag_value_size, int *err) {
  iMesh_getEHArrData(IMESH_INSTANCE(instance), entity_handles,
                     entity_handles_size, tag_handle, tag_value, tag_value_allocated,
                     tag_value_size, err);
}

void FBiGeom_getESHArrData(FBiGeom_Instance instance,
                           const iBase_EntityHandle* entity_handles, int entity_handles_size,
                           iBase_TagHandle tag_handle, iBase_EntitySetHandle** tag_value,
                           int* tag_value_allocated, int* tag_value_size, int *err) {
  iMesh_getESHArrData(IMESH_INSTANCE(instance), entity_handles,
                      entity_handles_size, tag_handle, tag_value, tag_value_allocated,
                      tag_value_size, err);
}

void FBiGeom_setArrData(FBiGeom_Instance instance,
                        const iBase_EntityHandle* entity_handles, int entity_handles_size,
                        iBase_TagHandle tag_handle, const void* tag_values, int tag_values_size,
                        int *err) {
  iMesh_setArrData(IMESH_INSTANCE(instance), entity_handles,
                   entity_handles_size, tag_handle, tag_values, tag_values_size, err);
}

void FBiGeom_setIntArrData(FBiGeom_Instance instance,
                           const iBase_EntityHandle* entity_handles, int entity_handles_size,
                           iBase_TagHandle tag_handle, const int* tag_values, int tag_values_size,
                           int *err) {
  iMesh_setIntArrData(IMESH_INSTANCE(instance), entity_handles,
                      entity_handles_size, tag_handle, tag_values, tag_values_size, err);
}

void FBiGeom_setDblArrData(FBiGeom_Instance instance,
                           const iBase_EntityHandle* entity_handles, int entity_handles_size,
                           iBase_TagHandle tag_handle, const double* tag_values,
                           const int tag_values_size, int *err) {
  iMesh_setDblArrData(IMESH_INSTANCE(instance), entity_handles,
                      entity_handles_size, tag_handle, tag_values, tag_values_size, err);
}

void FBiGeom_setEHArrData(FBiGeom_Instance instance,
                          const iBase_EntityHandle* entity_handles, int entity_handles_size,
                          iBase_TagHandle tag_handle, const iBase_EntityHandle* tag_values,
                          int tag_values_size, int *err) {
  iMesh_setEHArrData(IMESH_INSTANCE(instance), entity_handles,
                     entity_handles_size, tag_handle, tag_values, tag_values_size, err);
}

void FBiGeom_setESHArrData(FBiGeom_Instance instance,
                           const iBase_EntityHandle* entity_handles, int entity_handles_size,
                           iBase_TagHandle tag_handle, const iBase_EntitySetHandle* tag_values,
                           int tag_values_size, int *err) {
  iMesh_setESHArrData(IMESH_INSTANCE(instance), entity_handles,
                      entity_handles_size, tag_handle, tag_values, tag_values_size, err);
}

void FBiGeom_rmvArrTag(FBiGeom_Instance instance,
                       const iBase_EntityHandle* entity_handles, int entity_handles_size,
                       iBase_TagHandle tag_handle, int *err) {
  iMesh_rmvArrTag(IMESH_INSTANCE(instance), entity_handles,
                  entity_handles_size, tag_handle, err);
}

void FBiGeom_getData(FBiGeom_Instance instance, iBase_EntityHandle entity_handle,
                     iBase_TagHandle tag_handle, void** tag_value, int *tag_value_allocated,
                     int *tag_value_size, int *err) {
  iMesh_getData(IMESH_INSTANCE(instance), entity_handle, tag_handle,
                tag_value, tag_value_allocated, tag_value_size, err);
}

void FBiGeom_getIntData(FBiGeom_Instance instance,
                        iBase_EntityHandle entity_handle, iBase_TagHandle tag_handle,
                        int *out_data, int *err) {
  iMesh_getIntData(IMESH_INSTANCE(instance), entity_handle, tag_handle,
                   out_data, err);
}

void FBiGeom_getDblData(FBiGeom_Instance instance,
                        const iBase_EntityHandle entity_handle, const iBase_TagHandle tag_handle,
                        double *out_data, int *err) {
  iMesh_getDblData(IMESH_INSTANCE(instance), entity_handle, tag_handle,
                   out_data, err);
}

void FBiGeom_getEHData(FBiGeom_Instance instance, iBase_EntityHandle entity_handle,
                       iBase_TagHandle tag_handle, iBase_EntityHandle *out_data, int *err) {
  iMesh_getEHData(IMESH_INSTANCE(instance), entity_handle, tag_handle,
                  out_data, err);
}

void FBiGeom_getESHData(FBiGeom_Instance instance, iBase_EntityHandle entity_handle,
                        iBase_TagHandle tag_handle, iBase_EntitySetHandle *out_data, int *err) {
  iMesh_getESHData(IMESH_INSTANCE(instance), entity_handle, tag_handle,
                   out_data, err);
}

void FBiGeom_setData(FBiGeom_Instance instance, iBase_EntityHandle entity_handle,
                     iBase_TagHandle tag_handle, const void* tag_value, int tag_value_size,
                     int *err) {
  iMesh_setData(IMESH_INSTANCE(instance), entity_handle, tag_handle,
                tag_value, tag_value_size, err);
}

void FBiGeom_setIntData(FBiGeom_Instance instance,
                        iBase_EntityHandle entity_handle, iBase_TagHandle tag_handle,
                        int tag_value, int *err) {
  iMesh_setIntData(IMESH_INSTANCE(instance), entity_handle, tag_handle,
                   tag_value, err);
}

void FBiGeom_setDblData(FBiGeom_Instance instance,
                        iBase_EntityHandle entity_handle, iBase_TagHandle tag_handle,
                        double tag_value, int *err) {
  iMesh_setDblData(IMESH_INSTANCE(instance), entity_handle, tag_handle,
                   tag_value, err);
}

void FBiGeom_setEHData(FBiGeom_Instance instance, iBase_EntityHandle entity_handle,
                       iBase_TagHandle tag_handle, iBase_EntityHandle tag_value, int *err) {
  iMesh_setEHData(IMESH_INSTANCE(instance), entity_handle, tag_handle,
                  tag_value, err);
}

void FBiGeom_setESHData(FBiGeom_Instance instance, iBase_EntityHandle entity_handle,
                        iBase_TagHandle tag_handle, iBase_EntitySetHandle tag_value, int *err) {
  iMesh_setESHData(IMESH_INSTANCE(instance), entity_handle, tag_handle,
                   tag_value, err);
}

void FBiGeom_getAllTags(FBiGeom_Instance instance,
                        iBase_EntityHandle entity_handle, iBase_TagHandle** tag_handles,
                        int* tag_handles_allocated, int* tag_handles_size, int *err) {
  iMesh_getAllTags(IMESH_INSTANCE(instance), entity_handle, tag_handles,
                   tag_handles_allocated, tag_handles_size, err);
}

void FBiGeom_rmvTag(FBiGeom_Instance instance, iBase_EntityHandle entity_handle,
                    iBase_TagHandle tag_handle, int *err) {
  iMesh_rmvTag(IMESH_INSTANCE(instance), entity_handle, tag_handle, err);
}

void FBiGeom_subtract(FBiGeom_Instance instance,
                      iBase_EntitySetHandle , iBase_EntitySetHandle ,
                      iBase_EntitySetHandle* , int *err) {
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_intersect(FBiGeom_Instance instance,
                       iBase_EntitySetHandle , iBase_EntitySetHandle ,
                       iBase_EntitySetHandle* , int *err) {
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_unite(FBiGeom_Instance instance, iBase_EntitySetHandle ,
                   iBase_EntitySetHandle ,
                   iBase_EntitySetHandle* , int *err) {
  RETURN(iBase_NOT_SUPPORTED);
}

// TODO methods not yet implemented
void FBiGeom_getEntClosestPtTrimmed( FBiGeom_Instance instance,
                                     iBase_EntityHandle ,
                                     double ,
                                     double ,
                                     double ,
                                     double* ,
                                     double* ,
                                     double* ,
                                     int* err )
{
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_getFcCvtrXYZ( FBiGeom_Instance instance,
                           iBase_EntityHandle ,
                           double ,
                           double ,
                           double ,
                           double* ,
                           double* ,
                           double* ,
                           double* ,
                           double* ,
                           double* ,
                           int* err )
{
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_getEgCvtrXYZ( FBiGeom_Instance instance,
                           iBase_EntityHandle ,
                           double ,
                           double ,
                           double ,
                           double* ,
                           double* ,
                           double* ,
                           int* err )
{
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_getEntArrCvtrXYZ( FBiGeom_Instance instance,
                               iBase_EntityHandle const* ,
                               int ,
                               int ,
                               double const* ,
                               int ,
                               double** ,
                               int* ,
                               int* ,
                               double** ,
                               int* ,
                               int* ,
                               int* err )
{
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_getEgEvalXYZ( FBiGeom_Instance instance,
                           iBase_EntityHandle edge,
                           double x,
                           double y,
                           double z,
                           double* on_x,
                           double* on_y,
                           double* on_z,
                           double* tngt_i,
                           double* tngt_j,
                           double* tngt_k,
                           double* cvtr_i,
                           double* cvtr_j,
                           double* cvtr_k,
                           int* err )
{
  ErrorCode rval = FBE_cast(instance)->getEgEvalXYZ( (moab::EntityHandle) edge,
                                                     x, y, z,
                                                     *on_x, *on_y, *on_z,
                                                     *tngt_i, *tngt_j, *tngt_k,
                                                     *cvtr_i, *cvtr_j, *cvtr_k );
  CHKERR(rval,"can't get point on edge ");
  RETURN(iBase_SUCCESS);
}

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
                           int* err )
{
    /*
      moab::ErrorCode rval = _fbEngine->getFcEvalXYZ( (moab::EntityHandle) face,
      x,  y, z,
      on_x, on_y, on_z,
      nrml_i, nrml_j, nrml_k,
      cvtr1_i, cvtr1_j, cvtr1_k,
      cvtr2_i, cvtr2_j,  cvtr2_k );*/
    // camal really does not use curvatures
    // the most it is calling for normals and for closest point
    // return all curvatures = 0 for the time being, because we
    // know camal is not requesting them

  *cvtr1_i=*cvtr1_j=*cvtr1_k= *cvtr2_i= *cvtr2_j=  *cvtr2_k=0.;
    // first get closest point, then normal, separately
  ErrorCode rval = FBE_cast(instance)->getEntClosestPt((moab::EntityHandle) face_handle,
                                                       x, y, z,
                                                       on_x,  on_y, on_z );
  CHKERR(rval,"can't get closest point on surface ");
  rval = FBE_cast(instance)->getEntNrmlXYZ( (moab::EntityHandle) face_handle,
                                            x,   y,  z,
                                            nrml_i, nrml_j, nrml_k ); // some inconsistency here, we use pointers, not refs
  CHKERR(rval,"can't get normal on closest point on surface ");
  RETURN(iBase_SUCCESS);
}

void FBiGeom_getArrEgEvalXYZ( FBiGeom_Instance instance,
                              iBase_EntityHandle const* ,
                              int ,
                              int ,
                              double const* ,
                              int ,
                              double** ,
                              int* ,
                              int* ,
                              double** ,
                              int* ,
                              int* ,
                              double** ,
                              int* ,
                              int* ,
                              int* err )
{
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_getArrFcEvalXYZ( FBiGeom_Instance instance,
                              iBase_EntityHandle const* ,
                              int ,
                              int ,
                              double const* ,
                              int ,
                              double** ,
                              int* ,
                              int* ,
                              double** ,
                              int* ,
                              int* ,
                              double** ,
                              int* ,
                              int* ,
                              double** ,
                              int* ,
                              int* ,
                              int* err )
{
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_getPntClsf( FBiGeom_Instance instance,
                         double ,
                         double ,
                         double ,
                         iBase_EntityHandle* ,
                         int* err )
{
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_getPntArrClsf( FBiGeom_Instance instance,
                            int ,
                            double const* ,
                            int ,
                            iBase_EntityHandle** ,
                            int* ,
                            int* ,
                            int* err )
{
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_getFacets(FBiGeom_Instance instance,
                       iBase_EntityHandle ,
                       double ,
                       double , int , int ,
                       int , int , int ,
                       int *err)
{
  RETURN(iBase_NOT_SUPPORTED);
}

void FBiGeom_getEntTolerance( FBiGeom_Instance instance,
                              iBase_EntityHandle ,
                              double* ,
                              int* err )
{
  RETURN(iBase_NOT_SUPPORTED);
}
void FBiGeom_getArrTolerance( FBiGeom_Instance instance,
                              iBase_EntityHandle const* ,
                              int ,
                              double** ,
                              int* ,
                              int* ,
                              int* err )
{
  RETURN(iBase_NOT_SUPPORTED);
}
void FBiGeom_getTolerance( FBiGeom_Instance instance,
                           int* ,
                           double* ,
                           int* err )
{
  RETURN(iBase_NOT_SUPPORTED);
}

