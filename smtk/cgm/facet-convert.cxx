/**
 * \file facet-convert.cpp
 *
 * \brief Generate an SMTK topology-only version of a faceted solid (a.k.a. discrete or non-parametric) model.
 */

#undef NDEBUG
#include <cassert>
#include <cmath>

#include "stdio.h"

#include "CubitCompat.hpp"
#include "FacetQueryEngine.hpp"
#include "GMem.hpp"
#include "GeometryQueryTool.hpp"
#include "GeometryModifyTool.hpp"
#include "RefVertex.hpp"
#include "RefEdge.hpp"
#include "RefFace.hpp"
#include "RefVolume.hpp"
#include "Body.hpp"
#include "CubitDefines.h"
#include "CubitBox.hpp"
#include "InitCGMA.hpp"
#include "DLIList.hpp"
#include "TDUniqueId.hpp"
#include "BodySM.hpp"
#include "CoVertex.hpp"
#include "CoEdge.hpp"
#include "CoFace.hpp"
#include "CoVolume.hpp"
#include "SenseEntity.hpp"
#include "Shell.hpp"
#include "Loop.hpp"
#include "Chain.hpp"

#include "smtk/model/Storage.h"
#include "smtk/model/ModelEntity.h"
#include "smtk/util/UUID.h"
#include "smtk/model/ExportJSON.h"
#include "cJSON.h"

#include <map>
#include <vector>

#ifndef SRCDIR
# define SRCDIR .
#endif

#define TEST_OCC
#ifdef TEST_ACIS
#  define ENGINE "ACIS"
#elif defined (TEST_OCC)
#  define ENGINE "OCC"
#else
#  error "Which engine to test?"
#endif

#define STRINGIFY_(X) #X
#define STRINGIFY(X) STRINGIFY_(X)
#define SRCPATH STRINGIFY(SRCDIR) "/"

GeometryQueryTool* gqt = NULL;

int AddEntityToBody(RefEntity* ent, smtk::model::StoragePtr storage)
{
  DLIList<RefEntity*> children;
  ent->get_child_ref_entities(children);
  int nc = children.size();
  //cout << "        has " << nc << " child entities:\n";
  for (int i = 0; i < nc; ++i)
    {
    RefEntity* child = children.get_and_step();
    //cout << "        ++" << child->class_name() << "\n";
    AddEntityToBody(child, storage);
    //cout << "        --" << child->class_name() << "\n";
    }

  return 0;
}

template<typename E>
void AddEntitiesToBody(
  DLIList<E*>& entities,
  smtk::model::StoragePtr storage,
  const smtk::util::UUID& owningBodyId,
  std::map<int,smtk::util::UUID>& translation)
{
  int ne = entities.size();
  //cout << "        Body has " << ne << " " << E::get_class_name() << " entities:\n";
  for (int i = 0; i < ne; ++i)
    {
    // First, create a cell for the given entity:
    E* entry = entities.get_and_step();
    smtk::model::UUIDWithEntity cell = storage->insertCellOfDimension(entry->dimension());
    int cgmId = TDUniqueId::get_unique_id(entry);
    // Now, if owningBodyId is non-NULL (because the entity is a "free" member
    // of the body (i.e., not attached to some higher-dimensional entity), then
    // we relate it to the body.
    if (owningBodyId)
      {
      cell->second.relations().push_back(owningBodyId);
      storage->arrangeEntity(
        cell->first, smtk::model::EMBEDDED_IN,
        smtk::model::Arrangement::CellEmbeddedInEntityWithIndex(0));
      smtk::model::Arrangements& bodyInclusions(
        storage->arrangementsOfKindForEntity(owningBodyId, smtk::model::INCLUDES));
      if (bodyInclusions.empty())
        {
        bodyInclusions.push_back(smtk::model::Arrangement());
        }
      bodyInclusions[0].details().push_back(
        storage->findEntity(owningBodyId)->findOrAppendRelation(
          cell->first));
      //cout << i << "  " << cgmId << "  " << cell->first << " in body " << owningBodyId << "\n";
      }
    //cout << "        " << i << "  " << cgmId << "  " << cell->first << "\n";
    translation[cgmId] = cell->first;
    // Now, since AddEntitiesToBody is always called with entities
    // of ascending dimension, add the children of this entity to
    // its relations. Then add this entity to each of its children's
    // relations.
    DLIList<RefEntity*> children;
    entry->get_child_ref_entities(children);
    int nc = children.size();
    for (int j = 0; j < nc; ++j)
      {
      RefEntity* child = children.get_and_step();
      smtk::util::UUID smtkChildId = translation[TDUniqueId::get_unique_id(child)];
      cell->second.relations().push_back(smtkChildId);
      storage->findEntity(smtkChildId)->relations().push_back(cell->first);
      }
    }
}

template<typename E>
void AddArrangementsToBody(
  DLIList<E*>& entities,
  smtk::model::StoragePtr storage,
  std::map<int,smtk::util::UUID>& translation)
{
  int ne = entities.size();
  cout << "        Body has " << ne << " " << E::get_class_name() << " entities:\n";
  for (int i = 0; i < ne; ++i)
    {
    E* shell = entities.get_and_step();
    // First, create a new entity representing the shell.
    smtk::util::UUID shellSMTKId =
      storage->addEntityOfTypeAndDimension(
        smtk::model::SHELL_ENTITY |
        (1 << shell->dag_type().dimension()) |
        (1 << (shell->dag_type().dimension() + 1)),
        -1);
    // Now, find the SMTK cell bounded by the current shell:
    RefEntity* vol = shell->get_basic_topology_entity_ptr();
    int cgmId = TDUniqueId::get_unique_id(vol);
    smtk::util::UUID smtkId = translation[cgmId];
    smtk::model::Entity* vcell = storage->findEntity(smtkId);
    if (!vcell)
      {
      cerr << "Shell for unknown cell TDUniqueId " << cgmId << " UUID " << smtkId << ". Skipping.\n";
      continue;
      }
    //cout << "        " << i << "  " << shell << "  volume " << smtkId << "\n";

    // Now build a list of offsets of UUIDs in the cell's relations array:
    std::map<smtk::util::UUID,int> offsets;
    int offset = 0;
    for (smtk::util::UUIDArray::iterator it = vcell->relations().begin(); it != vcell->relations().end(); ++it, ++offset)
      {
      offsets[*it] = offset;
      }
    if (offsets.find(shellSMTKId) == offsets.end())
      {
      int shellOffset = static_cast<int>(vcell->relations().size());
      offsets[shellSMTKId] = shellOffset;
      vcell->relations().push_back(shellSMTKId);
      storage->arrangeEntity(smtkId, smtk::model::HAS_SHELL,
        smtk::model::Arrangement::CellHasShellWithIndex(shellOffset));
      }

    // Finally, we can create an arrangement, a, of offsets and signs that
    // describe the shell by enumerating each bounding-cell and its sense
    // relative to the shell.
    DLIList<SenseEntity*> cofaces;
    shell->get_sense_entity_list(cofaces);
    // FIXME: The above should be shell->ordered_co_edges() when E == Loop.
    int ns = cofaces.size();
    //std::cout << "shell " << i << " (" << shell << ") " << ns << " sense-entities\n";
    smtk::util::UUIDArray shellRelations;
    smtk::model::Arrangement shellArrOfUses; // the face-uses that compose the shell
    shellRelations.resize(ns + 1);
    shellRelations[ns] = smtkId;
    storage->arrangeEntity(shellSMTKId, smtk::model::HAS_CELL,
      smtk::model::Arrangement::ShellHasCellWithIndex(ns));
    //smtk::model::Arrangement a;
    //a.details().push_back(-1); // FIXME: Does not deal with multiple shells at the moment.
    for (int j = 0; j < ns; ++j)
      {
      // Obtain the SenseEntity. Since SenseEntity instances cannot have a
      // TDUniqueId (they do not inherit ToolDataUser), we look up the
      // ID of the corresponding basic_topological_entity (BTE) and translate
      // that into a UUID:
      SenseEntity* se = cofaces.get_and_step();
      //std::cout << "   " << j << "  " << (int)se->get_sense() << "\n";
      int faceId = TDUniqueId::get_unique_id(se->get_basic_topology_entity_ptr());
      smtk::util::UUID smtkFaceId = translation[faceId];
      // Now we know a face on the shell. Back up and see if
      // one of its uses has the same sense as our SenseEntity.
      // If not, then we need to create a new SMTK use entity.
      smtk::util::UUID smtkFaceUseId =
        storage->findOrCreateCellUseOfSenseAndOrientation(
          smtkFaceId, se->get_sense(),
          se->get_sense() == CUBIT_REVERSED ?
          smtk::model::NEGATIVE :
          smtk::model::POSITIVE);
      shellRelations[j] = smtkFaceUseId;
      /*
      cout
        << "          " << j << "  " << faceId
        << "  face " << smtkFaceId
        << " dir " << (se->get_sense() == CUBIT_FORWARD ? "+" : "-")
        << "\n";
       */
      std::map<smtk::util::UUID,int>::iterator oit = offsets.find(smtkFaceId);
      if (oit == offsets.end())
        {
        vcell->appendRelation(smtkFaceId);
        oit = offsets.insert(std::pair<smtk::util::UUID,int>(smtkFaceId,offset++)).first;
        }
      // Add [offset into relations, sense(neg/pos = 0/1)] to arrangement:
      //a.details().push_back(oit->second);
      //a.details().push_back(se->get_sense() == CUBIT_FORWARD ? +1 : 0);
      }
    // Now add the arrangement to the cell's list of arrangements:
    int aid = storage->arrangeEntity(smtkId, smtk::model::HAS_SHELL,
      smtk::model::Arrangement::CellHasShellWithIndex(0));
    storage->findEntity(shellSMTKId)->relations() = shellRelations;
    aid = storage->arrangeEntity(shellSMTKId, smtk::model::HAS_USE,
      smtk::model::Arrangement::ShellHasUseWithIndexRange(0, ns));
    (void)aid; // keep aid around for debugging
    //cout << "           +++ as shell " << aid << " of cell " << smtkId << "\n";
    }
}

template<typename E>
void AddTessellationsToBody(
  DLIList<E*>& entities,
  smtk::model::StoragePtr storage,
  std::map<int,smtk::util::UUID>& translation)
{
  int ne = entities.size();
  //cout << "        Tessellation " << ne << " " << E::get_class_name() << " entities:\n";
  for (int i = 0; i < ne; ++i)
    {
    // First, create a cell for the given entity:
    E* entry = entities.get_and_step();
    int cgmId = TDUniqueId::get_unique_id(entry);
    std::map<int,smtk::util::UUID>::iterator transIter =
      translation.find(cgmId);
    if (transIter == translation.end())
      {
      continue;
      }
    smtk::util::UUID uid(transIter->second);
    if (uid.isNull())
      {
      continue;
      }
    GMem primitives;
    double measure = entry->measure();
    double maxErr = pow(measure, 1./entry->dimension()) / 10.;
    double longestEdge = measure;
    entry->get_graphics(primitives, 15, maxErr, longestEdge);
    int connCount = primitives.fListCount;
    int npts = primitives.pointListCount;
    if (npts <= 0 || (connCount <= 0 && entry->dimension() > 1))
      {
      continue;
      }
    smtk::model::Tessellation blank;
    smtk::model::UUIDsToTessellations& tess(storage->tessellations());
    smtk::model::UUIDsToTessellations::iterator it =
      tess.insert(std::pair<smtk::util::UUID,smtk::model::Tessellation>(uid, blank)).first;
    // Now add data to the Tessellation "in situ" to avoid a copy.
    // First, copy point coordinates:
    it->second.coords().reserve(3 * npts);
    GPoint* inPts = primitives.point_list();
    for (int j = 0; j < npts; ++j, ++inPts)
      {
      it->second.addCoords(inPts->x, inPts->y, inPts->z);
      }
    // Now translate the connectivity:
    if (entry->dimension() > 1)
      {
      it->second.conn().reserve(connCount);
      int* inConn = primitives.facet_list();
      int ptsPerPrim = 0;
      for (int k = 0; k < connCount; k += (ptsPerPrim + 1), inConn += (ptsPerPrim + 1))
        {
        ptsPerPrim = *inConn;
        int* pConn = inConn + 1;
        switch (ptsPerPrim)
          {
        case 1:
          // This is a vertex
          it->second.addPoint(pConn[0]);
          break;
        case 2:
          it->second.addLine(pConn[0], pConn[1]);
          break;
        case 3:
          it->second.addTriangle(pConn[0], pConn[1], pConn[2]);
          break;
        default:
          std::cerr << "Unknown primitive has " << ptsPerPrim << " conn entries\n";
          break;
          }
        }
      }
    else
      {
      it->second.conn().reserve(npts + 1);
      it->second.conn().push_back(npts);
      for (int k = 0; k < npts; ++k)
        {
        it->second.conn().push_back(k);
        }
      }
    }
}

// Vertices don't support get_graphics (yet), so specialize the function.
template<>
void AddTessellationsToBody(
  DLIList<RefVertex*>& entities,
  smtk::model::StoragePtr storage,
  std::map<int,smtk::util::UUID>& translation)
{
  int ne = entities.size();
  //cout << "        Tessellation " << ne << " " << E::get_class_name() << " entities:\n";
  for (int i = 0; i < ne; ++i)
    {
    // First, create a cell for the given entity:
    RefVertex* entry = entities.get_and_step();
    int cgmId = TDUniqueId::get_unique_id(entry);
    std::map<int,smtk::util::UUID>::iterator transIter =
      translation.find(cgmId);
    if (transIter == translation.end())
      {
      continue;
      }
    smtk::util::UUID uid(transIter->second);
    if (uid.isNull())
      {
      continue;
      }
    CubitVector coords = entry->coordinates();
    smtk::model::Tessellation blank;
    smtk::model::UUIDsToTessellations& tess(storage->tessellations());
    smtk::model::UUIDsToTessellations::iterator it =
      tess.insert(std::pair<smtk::util::UUID,smtk::model::Tessellation>(uid, blank)).first;
    // Now add data to the Tessellation "in situ" to avoid a copy.
    // First, copy point coordinates:
    it->second.coords().resize(3);
    coords.get_xyz(&it->second.coords()[0]);
    }
}

template<typename E>
void AddUseToBody(
  DLIList<E*>& entities,
  smtk::model::StoragePtr storage,
  std::map<int,smtk::util::UUID>& translation)
{
  (void)entities;
  (void)storage;
  (void)translation;
}

int ImportBody(
  Body* cgmBody,
  smtk::model::StoragePtr storage,
  std::map<int,smtk::util::UUID>& translation)
{
  DLIList<RefEntity*> children;
  BodySM* cgmBodySM = cgmBody->get_body_sm_ptr();
  TopologyEntity* cgmBodyTopo = cgmBodySM ? cgmBodySM->topology_entity() : NULL;
  int cgmBodyId = TDUniqueId::get_unique_id(cgmBody);
  smtk::util::UUID smtkNullId;
  smtk::util::UUID smtkBodyId;
  std::map<int,smtk::util::UUID>::iterator inTable = translation.find(cgmBodyId);
  if (inTable != translation.end())
    {
    smtkBodyId = inTable->second;
    }
  else
    { // TODO: No matching entry... create one? Barf for now.
    return -1;
    }
  CubitStatus status;
  if (cgmBodyTopo)
    {
      {
      DLIList<RefVertex*> ref_vertex_list;
      status = cgmBodyTopo->ref_vertices(ref_vertex_list);
      AddEntitiesToBody(ref_vertex_list, storage, smtkNullId, translation);
      AddTessellationsToBody(ref_vertex_list, storage, translation);
      }
      {
      DLIList<RefEdge*> ref_edge_list;
      status = cgmBodyTopo->ref_edges(ref_edge_list);
      AddEntitiesToBody(ref_edge_list, storage, smtkNullId, translation);
      AddTessellationsToBody(ref_edge_list, storage, translation);
      }
      {
      DLIList<RefFace*> ref_face_list;
      status = cgmBodyTopo->ref_faces(ref_face_list);
      AddEntitiesToBody(ref_face_list, storage, smtkNullId, translation);
      AddTessellationsToBody(ref_face_list, storage, translation);
      }
      {
      DLIList<RefVolume*> ref_volume_list;
      status = cgmBodyTopo->ref_volumes(ref_volume_list);
      AddEntitiesToBody(ref_volume_list, storage, smtkBodyId, translation);
      // AddTessellationsToBody() does not make sense here.
      }
      {
      DLIList<CoVertex*> co_vertex_list;
      status = cgmBodyTopo->co_vertices(co_vertex_list);
      std::cout << co_vertex_list.size() << " co-vertices\n";
      AddUseToBody(co_vertex_list, storage, translation);
      //AddArrangementsToBody(co_vertex_list, storage, translation);
      }
      {
      DLIList<CoEdge*> co_edge_list;
      status = cgmBodyTopo->co_edges(co_edge_list);
      std::cout << co_edge_list.size() << " co-edges\n";
      AddUseToBody(co_edge_list, storage, translation);
      //AddArrangementsToBody(co_edge_list, storage, translation);
      }
      {
      DLIList<CoFace*> co_face_list;
      status = cgmBodyTopo->co_faces(co_face_list);
      std::cout << co_face_list.size() << " co-faces\n";
      AddUseToBody(co_face_list, storage, translation);
      //AddArrangementsToBody(co_face_list, storage, translation);
      }
      /* {
      DLIList<CoVolume*> co_volume_list;
      status = cgmBodyTopo->co_volumes(co_volume_list);
      AddArrangementsToBody(co_volume_list, storage, translation);
      } */
      {
      DLIList<Shell*> shell_list;
      status = cgmBodyTopo->shells(shell_list);
      AddArrangementsToBody(shell_list, storage, translation);
      }
      {
      DLIList<Loop*> loop_list;
      status = cgmBodyTopo->loops(loop_list);
      AddArrangementsToBody(loop_list, storage, translation);
      }
      {
      DLIList<Chain*> chain_list;
      status = cgmBodyTopo->chains(chain_list);
      AddArrangementsToBody(chain_list, storage, translation);
      }
    }

  return 0;
}

void ExportBodyToJSONFile(
  smtk::model::StoragePtr storage,
  const std::string& filename)
{
  cJSON* json = cJSON_CreateObject();
  smtk::model::ExportJSON::fromModel(json, storage);
  char* exported = cJSON_Print(json);
  cJSON_Delete(json);
  FILE* fid = fopen(filename.c_str(), "w");
  fprintf(fid, "%s\n", exported);
  fclose(fid);
  free(exported);
}

CubitStatus ConvertModel(
  DLIList<Body*>& imported,
  std::vector<smtk::model::StoragePtr>& bodies,
  std::map<int,smtk::util::UUID>& translation,
  bool singleModel)
{
  int ni = imported.size();
  //cout << "Imported " << ni << " entities:\n";
  for (int i = 0; i < ni; ++i)
    {
    Body* ent = imported.get_and_step();
    if (
      (singleModel && i == 0) ||
      (!singleModel))
      {
      smtk::model::StoragePtr blank = smtk::model::Storage::create();
      bodies.push_back(blank);
      }
    // Add an entity corresponding to the Body:
    int cgmBodyId = TDUniqueId::get_unique_id(ent);
    smtk::model::ModelEntity smtkBody = (*bodies.rbegin())->addModel();
    translation[cgmBodyId] = smtkBody.entity();
    //cout << "  Body " << ent << "\n";
    ImportBody(dynamic_cast<Body*>(ent), *bodies.rbegin(), translation);
    }
  //cout << "\n";
  return CUBIT_SUCCESS;
}

FacetFileFormat fileFormatFromString(const std::string& txt)
{
  if (txt == "CUBIT_FACET")
    return CUBIT_FACET_FILE;
  if (txt == "AVS")
    return AVS_FILE;
  if (txt == "CHOLLA")
    return CHOLLA_FILE;
  if (txt == "FROM_FACET_LIST")
    return FROM_FACET_LIST;

  return STL_FILE;
}

// main program - initialize, then send to proper function
int main (int argc, char **argv)
{
  CubitObserver::init_static_observers();
  CGMApp::instance()->startup( argc, argv );
  CubitStatus s = InitCGMA::initialize_cgma( ENGINE );
  if (CUBIT_SUCCESS != s) return 1;

  if (argc < 2)
    {
    std::cerr
      << "Usage:\n"
      << "  " << argv[0] << " filename [filetype ['1' [result]]]\n"
      << "where\n"
      << "  filename   - is the path to a solid model that CGM can import\n"
      << "  filetype   - is a solid model filetype to help CGM.\n"
      << "  1          - is an optional argument specifying that a single\n"
      << "               model file should be written instead of one per body.\n"
      << "  result     - is an output filename. The default is \"smtkModel.json\"\n"
      << "\n"
      << "The default filetype is \"STL\"\n"
      << "Valid values for filetype:"
      << "\n  "
      <<     " CUBIT_FACET"
      <<     " AVS"
      <<     " CHOLLA"
      <<     " FROM_FACET_LIST"
      <<     " STL"
      << "\n"
      << "\n"
      ;
    return 1;
    }

  // I. Import a solid model
  ModelImportOptions opts;
  DLIList<Body*> imported;
  gqt = GeometryQueryTool::instance();
  FacetQueryEngine* fqe = FacetQueryEngine::instance();

  DLIList<CubitQuadFacet *> quad_facet_list;
  DLIList<CubitFacet *> tri_facet_list;
  DLIList<Surface *> surface_list;
  CubitStatus stat = fqe->import_facets(
    argv[1], /*file to read*/
    TRUE, /*use_feature_angle*/
    30., /*feature_angle*/
    1e-6, /*tolerance*/
    0, /*interp_order*/
    FALSE, /*smooth_non_manifold*/
    TRUE, /*split_surfaces*/
    FALSE, /*stitch*/
    FALSE, /*improve*/
    quad_facet_list,
    tri_facet_list,
    surface_list,
    fileFormatFromString(argc > 2 ? argv[2] : "STL_FILE"));
  if (stat != CUBIT_SUCCESS)
    {
    std::cerr << "Failed to import CGM model, status " << stat << "\n";
    return -1;
    }
  gqt->bodies(imported);
  std::cout << "Imported " << imported.size() << " bodies " << tri_facet_list.size() << " tris " << quad_facet_list.size() << " quads " << surface_list.size() << " surfs\n";

  // Convert to an SMTK model.
  std::vector<smtk::model::StoragePtr> bodies;
  std::map<int,smtk::util::UUID> translation;
  bool singleModel = argc > 3 ? true : false;
  stat = ConvertModel(imported, bodies, translation, singleModel);
  if (stat != CUBIT_SUCCESS)
    {
    std::cerr << "Failed to convert CGM model, status " << stat << "\n";
    return -1;
    }

  // Save as an SMTK JSON dataset.
  if (bodies.size() == 1)
    {
    ExportBodyToJSONFile(bodies[0], argc > 4 ? argv[4] : "smtkModel.json");
    }
  else
    {
    int bnum = 0;
    std::vector<smtk::model::StoragePtr>::iterator it;
    for (it = bodies.begin(); it != bodies.end(); ++it, ++bnum)
      {
      std::ostringstream filename;
      filename << "smtkModel-" << bnum << ".json";
      ExportBodyToJSONFile(*it, filename.str());
      }
    }

  int ret_val = ( CubitMessage::instance()->error_count() );
  if ( ret_val != 0 )
    cerr << "Errors found during session.\n";
  else
    ret_val = 0;

  GeometryQueryTool::delete_instance();

  return ret_val;
}
