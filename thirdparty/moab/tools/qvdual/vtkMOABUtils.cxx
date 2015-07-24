#include "vtkMOABUtils.h"
#include "vtkPoints.h"
#include "vtkUnstructuredGrid.h"
#include "vtkPropAssembly.h"
#include "vtkProperty.h"
#include "vtkProperty2D.h"
#include "vtkActor.h"
#include "vtkActor2D.h"
#include "vtkActorCollection.h"
#include "vtkIdList.h"
#include "vtkPropCollection.h"
#include "vtkObjectFactory.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkDataSetMapper.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyData.h"
#include "vtkExtractCells.h"
#include "vtkExtractGeometry.h"
#include "vtkExtractPolyDataGeometry.h"
#include "vtkImplicitFunction.h"
#include "vtkExtractEdges.h"
#include "vtkTubeFilter.h"
#include "vtkGeometryFilter.h"
#include "vtkLookupTable.h"
#include "vtkSource.h"

#include <assert.h>

#include "DrawDual.hpp"
#include "DualTool.hpp"
#include "MBTagConventions.hpp"

#define IS_BUILDING_MB
#include "MBCore.hpp"
// need internals to create whole-mesh handle
#include "MBInternals.hpp"
#undef IS_BUILDING_MB

//#define RED(x) ((x > 0.5) ? 0.0 : 1.0 - 2*x)
//#define BLUE(x) ((x < 0.5) ? 0.0 : 2*(x-0.5))
//#define GREEN(x) (x <= 0.5 ? 1.0 - RED(x) : 1.0 - BLUE(x))
#define RED(x) (1.0-x)
#define BLUE(x) (x)
#define GREEN(x) (x <= 0.5 ? 2.0*x : 2.0*(x-0.5))

int vtkMOABUtils::totalColors = 10;

vtkLookupTable *vtkMOABUtils::lookupTable = NULL;

const int vtkMOABUtils::vtk_cell_types[] = {
  1, 3, 5, 9, 7, 10, 14, 13, 0, 12, 0, 0, 0};

  //! static interface pointer, use this when accessing MOAB within vtk
MBInterface *vtkMOABUtils::mbImpl = NULL;

  //! static interface pointer, use this when accessing MOAB within vtk
DualTool *vtkMOABUtils::dualTool = NULL;

    //! static pointer to the renderer
vtkRenderer *vtkMOABUtils::myRen = NULL;
  
    //! static pointer to the renderer
vtkUnstructuredGrid *vtkMOABUtils::myUG = NULL;
  
  //! the default property
vtkProperty *vtkMOABUtils::topProperty = NULL;
  
  //! the highlight property
vtkProperty *vtkMOABUtils::highlightProperty = NULL;
  
  //! map between actors in the display and properties; if null, an actor
  //! inherits from topProperty
std::map<vtkActor*, vtkProperty*> vtkMOABUtils::actorProperties;

    //! map between props (actor2d's and actors) and sets they represent (0 if no set, 
    //! e.g. an extracted set)
std::map<vtkProp*, MBEntityHandle> vtkMOABUtils::propSetMap;

    //! topmost assembly for displaying contains relationships
vtkPropAssembly *vtkMOABUtils::topContainsAssy = NULL;

    //! topmost assembly for displaying parent/child relationships
vtkPropAssembly *vtkMOABUtils::topParentAssy = NULL;
  
  //! tag indicating whether a given set is in top contains assy
MBTag vtkMOABUtils::vtkTopContainsTag = 0;

    //! name for vtkTopContainsTag;
const char *vtkMOABUtils::vtkTopContainsTagName = "__vtkTopContainsTag";
  
  //! tag indicating whether a given set is in top parent assy
MBTag vtkMOABUtils::vtkTopParentTag = 0;

    //! name for vtkTopParentTag;
const char *vtkMOABUtils::vtkTopParentTagName = "__vtkTopParentTag";
  
  //! tag for pointing to vtk cell representing an entity
MBTag vtkMOABUtils::vtkCellTag = 0;
  
  //! name for vtkCellTag
const char *vtkMOABUtils::vtkCellTagName = "__vtkCellTag";

  //! tag for pointing to vtk actor for a set
MBTag vtkMOABUtils::vtkSetActorTag = 0;
  
    //! name for vtkSetActorTag
const char *vtkMOABUtils::vtkSetActorTagName = "__vtkSetActorTag";

    //! tag for pointing to vtk prop assembly for a set
MBTag vtkMOABUtils::vtkSetPropAssemblyTag = 0;
  
    //! name for vtkSetPropAssemblyTag
const char *vtkMOABUtils::vtkSetPropAssemblyTagName = "__vtkSetPropAssemblyTag";

  //! tag for determining whether a point has been allocated for a vertex
MBTag vtkMOABUtils::vtkPointAllocatedTag = 0;
  
  //! name for vtkPointAllocatedTag
const char *vtkMOABUtils::vtkPointAllocatedTagName = "__vtkPointAllocatedTag";

DrawDual *vtkMOABUtils::drawDual = NULL;

vtkCallbackCommand *vtkMOABUtils::eventCallbackCommand = NULL;

MBTag vtkMOABUtils::globalIdTag = 0;
MBTag vtkMOABUtils::categoryTag = 0;

bool vtkMOABUtils::debug = false;

//vtkStandardNewMacro(vtkMOABUtils);
  
MBErrorCode vtkMOABUtils::init(MBInterface *impl, vtkRenderer *this_ren)
{
  if (NULL == this_ren) return MB_FAILURE;
  myRen = this_ren;

  if (NULL == impl && NULL == mbImpl) {
    impl = new MBCore();
    vtkMOABUtils::mbImpl = impl;
  }

  if (NULL == dualTool)
    dualTool = new DualTool(mbImpl);

  MBErrorCode result = create_tags();
  
  make_properties();

  return result;
}

MBErrorCode vtkMOABUtils::create_tags() 
{
  MBErrorCode result;

  {
    vtkIdType def_val = -1;
    result = vtkMOABUtils::mbImpl->tag_get_handle(vtkCellTagName, 
                                                  sizeof(vtkIdType), 
                                                  MB_TYPE_OPAQUE,
                                                  vtkCellTag, 
                                                  MB_TAG_DENSE|MB_TAG_CREAT, 
                                                  &def_val);
    if (MB_SUCCESS != result) return result;
  }
  
  {
    unsigned char def_val = 0x0;
    result = vtkMOABUtils::mbImpl->tag_get_handle(vtkTopContainsTagName, 
                                                  1, 
                                                  MB_TYPE_BIT, 
                                                  vtkTopContainsTag, 
                                                  MB_TAG_CREAT,
                                                  &def_val);
    if (MB_SUCCESS != result) return result;
  }
  
  {
    unsigned char def_val = 0x0;
    result = vtkMOABUtils::mbImpl->tag_get_handle(vtkTopParentTagName, 
                                                  1, 
                                                  MB_TAG_BIT, 
                                                  vtkTopParentTag, 
                                                  MB_TAG_CREAT,
                                                  &def_val);
    if (MB_SUCCESS != result) return result;
  }
  
  {
    vtkActor *def_val = NULL;
    result = vtkMOABUtils::mbImpl->tag_get_handle(vtkSetActorTagName, 
                                                  sizeof(vtkActor*), 
                                                  MB_TYPE_OPAQUE, 
                                                  vtkSetActorTag, 
                                                  MB_TAG_SPASRE|MB_TAG_CREAT,
                                                  &def_val);
    if (MB_SUCCESS != result) return result;
  }
  
  {
    vtkPropAssembly *def_val = NULL;
    result = vtkMOABUtils::mbImpl->tag_get_handle(vtkSetPropAssemblyTagName, 
                                                  sizeof(vtkPropAssembly*), 
                                                  MB_TYPE_OPAQUE,
                                                  vtkSetPropAssemblyTag, 
                                                  MB_TAG_SPARSE|MB_TAG_CREAT,
                                                  &def_val);
    if (MB_SUCCESS != result) return result;
  }
  
  {
    bool def_val = false;
    result = vtkMOABUtils::mbImpl->tag_get_handle(vtkPointAllocatedTagName, 
                                                  sizeof(bool), 
                                                  MB_TYPE_OPAQUE,
                                                  vtkPointAllocatedTag, 
                                                  MB_TAG_DENSE|MB_TAG_CREAT,
                                                  &def_val);
    if (MB_SUCCESS != result) return result;
  }

  return MB_SUCCESS;
}

void vtkMOABUtils::make_properties() 
{
    // make top property
  if (NULL == topProperty) {
    vtkMOABUtils::topProperty = vtkProperty::New();
      // need to increase use count of top property
    vtkMOABUtils::topProperty->Register(NULL);

      // want the top property to be wire frame to start with
    vtkMOABUtils::topProperty->SetRepresentationToWireframe();
      //vtkMOABUtils::topProperty->SetColor(0.0, 1.0, 0.0);
    vtkMOABUtils::topProperty->SetDiffuse(1.0);
    vtkMOABUtils::topProperty->SetAmbient(1.0);
    vtkMOABUtils::topProperty->SetSpecular(1.0);
      //vtkMOABUtils::topProperty->SetLineWidth(2.0);
      //vtkMOABUtils::topProperty->SetEdgeColor(0.0, 0.0, 0.0);
      //vtkMOABUtils::topProperty->EdgeVisibilityOn();
  }
  
  if (NULL == highlightProperty) {
    
      // make highlight property
    vtkMOABUtils::highlightProperty = vtkProperty::New();
      // need to increase use count of highlight property
    vtkMOABUtils::highlightProperty->Register(NULL);

      // want the highlight property to be shaded and highlight color
    vtkMOABUtils::highlightProperty->SetRepresentationToSurface();
    vtkMOABUtils::highlightProperty->SetColor(1.0, 0.67, 0.0);
    vtkMOABUtils::highlightProperty->SetEdgeColor(1.0, 0.67, 0.0);
    vtkMOABUtils::highlightProperty->SetDiffuse(1.0);
    vtkMOABUtils::highlightProperty->SetAmbient(1.0);
    vtkMOABUtils::highlightProperty->SetSpecular(1.0);
      //vtkMOABUtils::highlightProperty->SetLineWidth(2.0);
      //vtkMOABUtils::highlightProperty->SetEdgeColor(0.0, 0.0, 0.0);
      //vtkMOABUtils::highlightProperty->EdgeVisibilityOn();
  }
}

void vtkMOABUtils::destroy() 
{
  if (NULL != vtkMOABUtils::mbImpl) {
    delete vtkMOABUtils::mbImpl;
    vtkMOABUtils::mbImpl = NULL;
  }
}

MBErrorCode vtkMOABUtils::make_vertex_points(vtkUnstructuredGrid *&ug)
{

  MBRange verts;
  bool allocated;
  MBErrorCode result = vtkMOABUtils::mbImpl->get_entities_by_type(0, MBVERTEX, verts);
  if (MB_SUCCESS != result || verts.empty()) return result;
  
    // get the point array
  vtkPoints *points = ug->GetPoints();
  if (NULL == points) {
    points = vtkPoints::New();
    ug->SetPoints(points);
    points->Delete();
  }

    // iterate from reverse so we only re-allocate once
  MBRange::reverse_iterator rit = verts.rbegin();
  double coords[3];
  MBErrorCode tmp_result;
  for (rit = verts.rbegin(); rit != verts.rend(); rit++) {
      // need to check for allocation
    tmp_result = 
      vtkMOABUtils::mbImpl->tag_get_data(vtkPointAllocatedTag, &(*rit), 1, &allocated);
    if (MB_SUCCESS != tmp_result) result = tmp_result;
    if (allocated) continue;
    
    tmp_result = vtkMOABUtils::mbImpl->get_coords(&(*rit), 1, coords);
    if (MB_SUCCESS != tmp_result) result = tmp_result;
    else {
      points->InsertPoint(vtkMOABUtils::mbImpl->id_from_handle(*rit), coords);
      allocated = true;
      tmp_result = vtkMOABUtils::mbImpl->tag_set_data(vtkPointAllocatedTag, &(*rit), 1, &allocated);
      if (MB_SUCCESS != tmp_result) result = tmp_result;
    }
  }
  
  return result;
}

MBErrorCode vtkMOABUtils::make_cells(MBRange &ents, 
                                     vtkUnstructuredGrid *&ug) 
{
    // add a cell for each entity, setting its global id to the cell id
  std::vector<vtkIdType> pt_ids;
  const MBEntityHandle *connect;
  int num_connect;
  MBErrorCode tmp_result, result = MB_SUCCESS;

    // check for unallocated ug (shouldn't have to do this, really...)
  if (NULL == ug->GetCells())
    ug->Allocate();
    
  vtkIdType cell_id;
  for (MBRange::iterator rit = ents.begin(); rit != ents.end(); rit++) {
      // check to see if it's been allocated already
    tmp_result = vtkMOABUtils::mbImpl->tag_get_data(vtkCellTag, &(*rit), 1, &cell_id);
    if (MB_SUCCESS != tmp_result) result = tmp_result;
    if (cell_id != -1) continue;

      // get a list of point ids
    if (vtkMOABUtils::mbImpl->type_from_handle(*rit) == MBVERTEX) {
      connect = &(*rit);
      num_connect = 1;
    }
    else {
      tmp_result = vtkMOABUtils::mbImpl->get_connectivity(*rit, connect, num_connect);
      if (MB_SUCCESS != tmp_result) result = tmp_result;
    }
    pt_ids.reserve(num_connect);
    for (int i = 0; i < num_connect; i++)
      pt_ids[i] = vtkMOABUtils::mbImpl->id_from_handle(connect[i]);
      
      // make the actual polygon
    cell_id = ug->InsertNextCell(vtk_cell_types[vtkMOABUtils::mbImpl->type_from_handle(*rit)], 
                                 num_connect, &pt_ids[0]);

      // assign the resulting cell id to the global id
    tmp_result = vtkMOABUtils::mbImpl->tag_set_data(vtkCellTag, &(*rit), 1, &cell_id);
    if (MB_SUCCESS != tmp_result) result = tmp_result;
  }

  return MB_SUCCESS;
}

MBErrorCode vtkMOABUtils::make_cells(vtkUnstructuredGrid *&ug) 
{
    // add a cell for all entities
  MBRange ents;
  MBErrorCode result = MB_SUCCESS;
  for (MBEntityType in_type = MBVERTEX; in_type < MBENTITYSET; in_type++) {

      // skip cell types vtk doesn't understand
    if (vtk_cell_types[in_type] == 0) continue;

    ents.clear();
    MBErrorCode tmp_result = mbImpl->get_entities_by_type(0, in_type, ents);
    if (MB_SUCCESS != tmp_result || ents.empty()) {
      result = tmp_result;
      continue;
    }
    
    tmp_result = make_cells(ents, ug);
    if (MB_SUCCESS != tmp_result) result = tmp_result;
  }
  
  return result;
}

MBErrorCode vtkMOABUtils::make_cells(MBEntityType in_type,
                                     vtkUnstructuredGrid *&ug) 
{
    // skip cell types vtk doesn't understand
  if (vtk_cell_types[in_type] == 0) return MB_FAILURE;
    
    // add a cell for entities of types passed in
  MBRange ents;
  MBErrorCode result = mbImpl->get_entities_by_type(0, in_type, ents);
  if (MB_SUCCESS != result || ents.empty())
    return result;
  
  return make_cells(ents, ug);
}

MBErrorCode vtkMOABUtils::update_all_actors(MBEntityHandle this_set,
                                            vtkUnstructuredGrid *ug,
                                            const bool shaded,
                                            const bool tubed,
                                            const bool colored)
{
  assert(NULL != ug);

  MBRange update_sets;
  MBErrorCode result = vtkMOABUtils::mbImpl->get_entities_by_type(this_set,
                                                                  MBENTITYSET,
                                                                  update_sets);
  if (MB_SUCCESS != result) return result;

    // finally, the sets
  MBRange chord_sets, sheet_sets;
  
  result = dualTool->get_dual_hyperplanes(vtkMOABUtils::mbImpl, 1, chord_sets);
  if (MB_SUCCESS != result) return result;

  result = dualTool->get_dual_hyperplanes(vtkMOABUtils::mbImpl, 2, sheet_sets);
  if (MB_SUCCESS != result) return result;

  result = vtkMOABUtils::update_set_actors(chord_sets, vtkMOABUtils::myUG, true, false, true);
  if (MB_SUCCESS != result) return result;

  result = vtkMOABUtils::update_set_actors(sheet_sets, vtkMOABUtils::myUG, true, false, true);
  if (MB_SUCCESS != result) return result;

  int table_size = ((int) (chord_sets.size()+sheet_sets.size()) > vtkMOABUtils::totalColors ? 
                    chord_sets.size()+sheet_sets.size() : vtkMOABUtils::totalColors);
  vtkMOABUtils::construct_lookup_table(table_size);
  
  update_sets = update_sets.subtract(chord_sets);
  update_sets = update_sets.subtract(sheet_sets);
  
  return vtkMOABUtils::update_set_actors(update_sets, ug, shaded, tubed, colored);
}

MBErrorCode vtkMOABUtils::update_set_actors(const MBRange &update_sets,
                                            vtkUnstructuredGrid *ug,
                                            const bool shaded,
                                            const bool tubed,
                                            const bool colored)
{
  assert(NULL != ug);

    // update (empty & re-populate) actors corresponding to these sets
  MBErrorCode tmp_result;
  vtkActor *this_actor;
  MBRange ents;
  MBErrorCode result = MB_SUCCESS;
  
  for (MBRange::const_iterator rit = update_sets.begin(); rit != update_sets.end(); rit++) {
    ents.clear();

      // get a list of vtk cell ids for this set
    vtkIdList *ids;
    tmp_result = vtkMOABUtils::get_id_list(*rit, ids);
    if (MB_SUCCESS != tmp_result || NULL == ids) {
      result = tmp_result;
      continue;
    }

    if (ids->GetNumberOfIds() == 0) {
      ids->Delete();
      continue;
    }

      // non-zero list of ids; get an actor to put them in
    vtkExtractCells *ec;
    vtkMapper *this_mapper;
    this_actor = vtkMOABUtils::get_actor(*rit);

      // if the actor already exists, replace its id list with this one and go on
      // to next set
    if (NULL != this_actor) {
      
        // get ec from this actor
      vtkSource *this_source = this_actor->GetMapper()->GetInput()->GetSource();
      ec = vtkExtractCells::SafeDownCast(this_source);

      if (debug) {
        std::cout << "Set " << mbImpl->id_from_handle(*rit) << ", actor " << this_actor
                  << ", extractcells = ";
        if (NULL == ec) std::cout << "(null)";
        else std::cout << ec;
        std::cout << std::endl;
      }

        // don't act on root entity set (that'll be the only one not connected
        // to an extract cells filter)
      
        // empty out the extract cells filter
      if (NULL != ec) ec->SetCellList(ids);
      ids->Delete();
      continue;
    }

      // otherwise, we need to make a new one; also make an extractor to extract
      // the right ids
    this_actor = vtkMOABUtils::get_actor(*rit, true);
    ec = vtkExtractCells::New();
    ec->SetInput(ug);
    ec->AddCellList(ids);

    if (debug) {
      std::cout << "Set " << mbImpl->id_from_handle(*rit) << ", actor " << this_actor
                << ", extractcells = ";
      if (NULL == ec) std::cout << "(null)";
      else std::cout << ec;
      std::cout << std::endl;
      std::cout << "Number of ids is " << ids->GetNumberOfIds() << std::endl;
    }
    
      // if tube filter is requested, do special stuff to extract edges and
      // wrap them in tubes, returning a mapper
    if (tubed) vtkMOABUtils::setup_tube_filter(*rit, ec, this_mapper);

      // otherwise just put a mapper around the ec
    else {
      vtkDataSetMapper *ds_mapper = vtkDataSetMapper::New();
      ds_mapper->SetInput(ec->GetOutput());
      this_mapper = ds_mapper;
    }
        
    this_actor->SetMapper(this_mapper);
    vtkMOABUtils::myRen->AddActor(this_actor);

    vtkProperty *this_prop;

      // only if we're shaded or colored do we get our own property, otherwise
      // we inherit from the top-level property
    if (shaded || colored) 
      this_prop = vtkMOABUtils::get_property(this_actor, true);

    else
      this_actor->SetProperty(topProperty);
    
      // now set the actual properties
    if (shaded) this_prop->SetRepresentationToSurface();

      // if we're drawing shaded and not tubed, it's probably a dual surface, so
      // turn off backface culling so we can see both sides
    if (shaded && !tubed) this_prop->BackfaceCullingOff();

      // only set color on the property if it's tubed
    if (colored) vtkMOABUtils::set_color(*rit, this_prop);

      // otherwise, use scalars to set the color
//    else if (colored) {
//      this_mapper->SetLookupTable(lookupTable);
//      this_mapper->UseLookupTableScalarRangeOn();
//    }

    ids->Delete();
    this_mapper->Delete();
    ec->Delete();
  }
  
  return result;
}

MBErrorCode vtkMOABUtils::setup_tube_filter(MBEntityHandle this_set, 
                                            vtkExtractCells *ec,
                                            vtkMapper *&this_mapper) 
{
    // decide whether an edge extractor is needed (if > 1d entities exist, it is)
  int hd_ents, dum;
  MBErrorCode result = 
    vtkMOABUtils::mbImpl->get_number_entities_by_dimension(this_set, 2, hd_ents, true),
    tmp_result = 
    vtkMOABUtils::mbImpl->get_number_entities_by_dimension(this_set, 3, dum, true);
  hd_ents += dum;
  if (MB_SUCCESS != result) return result;
  if (MB_SUCCESS != tmp_result) return tmp_result;
          
    // put an edge extractor and tube filter around these
  vtkTubeFilter *tf = vtkTubeFilter::New();
  if (hd_ents > 0) {
    vtkExtractEdges *ee = vtkExtractEdges::New();
    ee->SetInput(ec->GetOutput());
    tf->SetInput(ee->GetOutput());
//    ee->Delete();
  }
  else {
      // need a geometry filter to change unstructuredgrid to polydata
    vtkGeometryFilter *gf = vtkGeometryFilter::New();
    gf->SetInput(ec->GetOutput());
    tf->SetInput(gf->GetOutput());
//    gf->Delete();
  }
          
  tf->SetNumberOfSides(6);
  tf->SetRadius(0.005);
  vtkPolyDataMapper *pd_mapper = vtkPolyDataMapper::New();
  pd_mapper->SetInput(tf->GetOutput());
  this_mapper = pd_mapper;

  return MB_SUCCESS;
}
 
MBErrorCode vtkMOABUtils::get_id_list(MBEntityHandle this_set, vtkIdList *&ids) 
{
  MBRange ents;
  MBErrorCode result = vtkMOABUtils::mbImpl->get_entities_by_handle(this_set, ents);
  if (MB_SUCCESS != result || ents.empty()) {
    ids = NULL;
    return result;
  }

  return get_id_list(ents, ids);
}

MBErrorCode vtkMOABUtils::get_id_list(MBRange &ents, vtkIdList *&ids) 
{
      
  MBErrorCode tmp_result, result = MB_SUCCESS;
  ids = vtkIdList::New();
  vtkIdType this_id;
  for (MBRange::iterator rit = ents.begin(); rit != ents.end(); rit++) {
    if (vtkMOABUtils::mbImpl->type_from_handle(*rit) == MBENTITYSET) continue;
    tmp_result = vtkMOABUtils::mbImpl->tag_get_data(vtkCellTag, &(*rit), 1, &this_id);
    if (MB_SUCCESS != tmp_result) result = tmp_result;
    else if (this_id != -1) {
      ids->InsertNextId(this_id);
    }
  }

  return result;
}

MBErrorCode vtkMOABUtils::empty_assy(vtkPropAssembly *this_assy) 
{
    // remove all the parts from this assy
  vtkPropCollection *parts = this_assy->GetParts();
  vtkProp *this_part = NULL, *next_part = parts->GetNextProp();
  while (NULL != next_part) {
    this_part = next_part;
    next_part = parts->GetNextProp();
    this_assy->RemovePart(this_part);
  }
  
  return MB_SUCCESS;
}

MBErrorCode vtkMOABUtils::get_top_contains_sets(MBRange &top_sets) 
{
    // get top contains sets, which are those containing sets and not contained in any other sets
  top_sets.clear();
  MBErrorCode result = vtkMOABUtils::mbImpl->get_entities_by_type(0, MBENTITYSET, top_sets);
  if (MB_SUCCESS != result || top_sets.empty()) return result;
  
  MBRange dum_sets = top_sets, tmp_set;
  
    // now go through all children, removing them from top sets
  MBErrorCode tmp_result;
  for (MBRange::iterator rit = dum_sets.begin(); rit != dum_sets.end(); rit++) {
    tmp_set.clear();
    tmp_result = vtkMOABUtils::mbImpl->get_entities_by_type(*rit, MBENTITYSET, tmp_set);
    if (MB_SUCCESS != tmp_result) result = tmp_result;
    else if (!tmp_set.empty())
        // remove all the children from the top sets
      top_sets = top_sets.subtract(tmp_set);
    else
        // else no children - shouldn't be a top set
      top_sets.erase(*rit);
    
    if (top_sets.empty()) break;
  }
  
  return result;
}

MBErrorCode vtkMOABUtils::get_top_parent_sets(MBRange &top_sets) 
{
    // get top parent sets, which are those who aren't children of others but have
    // some children
  top_sets.clear();
  MBRange dum_sets;
  MBErrorCode result = vtkMOABUtils::mbImpl->get_entities_by_type(0, MBENTITYSET, dum_sets);
  if (MB_SUCCESS != result || dum_sets.empty()) return result;

  MBErrorCode tmp_result;
  int num_children, num_parents;
  for (MBRange::iterator rit = dum_sets.begin(); rit != dum_sets.end(); rit++) {
    tmp_result = vtkMOABUtils::mbImpl->num_child_meshsets(*rit, &num_children);
    if (MB_SUCCESS != tmp_result) result = tmp_result;
    tmp_result = vtkMOABUtils::mbImpl->num_parent_meshsets(*rit, &num_parents);
    if (MB_SUCCESS != tmp_result) result = tmp_result;
    if (num_parents == 0 && num_children > 0) top_sets.insert(*rit);
  }
  
  return result;
}

MBErrorCode vtkMOABUtils::set_color(MBEntityHandle this_set,
                                     vtkProperty *this_property,
                                     const int total_colors) 
{
  double red, green, blue;
  int dum;
  MBErrorCode result = vtkMOABUtils::get_colors(this_set, total_colors, dum,
                                                red, green, blue);
  this_property->SetColor(red, green, blue);
  return result;
}

vtkProperty *vtkMOABUtils::get_property(vtkActor *this_actor, const bool make_if_missing) 
{
  
  vtkProperty *this_prop = vtkMOABUtils::actorProperties[this_actor];
  if (NULL == this_prop && make_if_missing) {
    this_prop = this_actor->MakeProperty();
    actorProperties[this_actor] = this_prop;
    this_actor->SetProperty(this_prop);
  }
  return this_prop;
}

vtkProperty *vtkMOABUtils::get_property(MBEntityHandle this_set, const bool make_if_missing) 
{
  vtkActor *this_actor;
  MBErrorCode result = vtkMOABUtils::mbImpl->tag_get_data(vtkSetActorTag, &this_set,
                                                          1, &this_actor);
  if (MB_SUCCESS != result || NULL == this_actor) return NULL;

  return get_property(this_actor, make_if_missing);
}

vtkActor *vtkMOABUtils::get_actor(MBEntityHandle this_set, 
                                  const bool make_if_missing) 
{
  vtkActor *this_actor = NULL;
  assert(0 != vtkSetActorTag);
  
  if (0 == this_set)
    vtkMOABUtils::mbImpl->tag_get_data(vtkSetActorTag, NULL, 0, &this_actor);
  else
    vtkMOABUtils::mbImpl->tag_get_data(vtkSetActorTag, &this_set,
                                                1, &this_actor);

  if (make_if_missing && NULL == this_actor) {
    this_actor = vtkActor::New();
    propSetMap[this_actor] = this_set;
    if (0 == this_set)
      vtkMOABUtils::mbImpl->tag_set_data(vtkSetActorTag, NULL, 0, 
                                                      &this_actor);
    else
      vtkMOABUtils::mbImpl->tag_set_data(vtkSetActorTag, &this_set, 
                                                  1, &this_actor);

  }
  
  return this_actor;
}

    //! given a prop, get the corresponding set
MBEntityHandle vtkMOABUtils::get_set(vtkProp *this_prop) 
{
  return propSetMap[this_prop];
}
  
MBTag vtkMOABUtils::globalId_tag() 
{
  if (0 == globalIdTag)
    vtkMOABUtils::mbImpl->tag_get_handle(GLOBAL_ID_TAG_NAME, 1, MB_TYPE_INTEGER, globalIdTag);
  return globalIdTag;
}

MBTag vtkMOABUtils::category_tag() 
{
  if (0 == categoryTag)
    vtkMOABUtils::mbImpl->tag_get_handle(CATEGORY_TAG_NAME, NAME_TAG_SIZE, MB_TYPE_OPAQUE, categoryTag);
  return categoryTag;
}

MBErrorCode vtkMOABUtils::get_set_category_name( MBEntityHandle this_set, 
                                                 char *this_name )
{
  if (0 == vtkMOABUtils::globalId_tag()) return MB_SUCCESS;
  
  int this_id;
  MBErrorCode result = vtkMOABUtils::mbImpl->tag_get_data(vtkMOABUtils::globalId_tag(),
                                                          &this_set, 1, &this_id);
  if (MB_SUCCESS != result) return result;
  
    // get the id of the set
  char cat_name[CATEGORY_TAG_SIZE];
  sprintf(cat_name, "\0");
  if (0 != vtkMOABUtils::category_tag())
    result = vtkMOABUtils::mbImpl->tag_get_data(vtkMOABUtils::category_tag(),
                                                &this_set, 1, &cat_name);
  if (MB_SUCCESS != result || !strcmp(cat_name, "\0")) {
    if (0 == this_id) {
      this_id = vtkMOABUtils::mbImpl->id_from_handle(this_set);
      sprintf(cat_name, "Set handle");
    }
    else {
      sprintf(cat_name, "Set id");
    }
  }

  sprintf(this_name, "%s %d", cat_name, this_id);
  return MB_SUCCESS;
}

void vtkMOABUtils::print_debug() 
{
  for (std::map<vtkActor*,vtkProperty*>::const_iterator mit = actorProperties.begin();
       mit != actorProperties.end(); mit++)
    mit->first->GetMapper()->GetInput()->PrintSelf(std::cout, vtkIndent(0));
}

void vtkMOABUtils::change_set_visibility( MBRange &visible_sets, MBRange &invisible_sets )
{
  static std::vector<vtkActor*> tmp_actors;
  
    // always do invisible first
  tmp_actors.reserve(invisible_sets.size());
  MBErrorCode result = mbImpl->tag_get_data(vtkSetActorTag, invisible_sets, &tmp_actors[0]);
  if (MB_SUCCESS != result) {
    ; // do nothing, just place to stop the debugger
  }
  for (unsigned int i = 0; i < invisible_sets.size(); i++)
    if (NULL != tmp_actors[i]) tmp_actors[i]->VisibilityOff();

  tmp_actors.reserve(visible_sets.size());
  result = mbImpl->tag_get_data(vtkSetActorTag, visible_sets, &tmp_actors[0]);
  if (MB_SUCCESS != result) {
    ; // do nothing, just place to stop the debugger
  }
  for (unsigned int i = 0; i < visible_sets.size(); i++)
    if (NULL != tmp_actors[i]) tmp_actors[i]->VisibilityOn();
    
}

void vtkMOABUtils::change_set_properties(MBRange &high_mbsets, MBRange &unhigh_mbsets) 
{
  vtkProperty *curr_prop;
  MBRange::iterator rit;
  
  for (rit = high_mbsets.begin(); rit != high_mbsets.end(); rit++) {
    vtkActor *this_actor = get_actor(*rit);
    if (NULL == this_actor) continue;
    curr_prop = this_actor->GetProperty();
    if (curr_prop != highlightProperty)
      this_actor->SetProperty(highlightProperty);
  }

  for (rit = unhigh_mbsets.begin(); rit != unhigh_mbsets.end(); rit++) {
    vtkActor *this_actor = get_actor(*rit);
    if (NULL == this_actor) continue;
    curr_prop = this_actor->GetProperty();
    if (curr_prop == highlightProperty) {
      if ((curr_prop = actorProperties[this_actor]))
        this_actor->SetProperty(curr_prop);
      else
        this_actor->SetProperty(topProperty);
    }
  }
}

    //! toggle the wireframe/shaded property
void vtkMOABUtils::toggle_wireframe_shaded(MBRange &high_mbsets) 
{
  for (MBRange::iterator rit = high_mbsets.begin(); rit != high_mbsets.end(); rit++) {
    vtkProperty *this_prop = get_property(*rit, true);

    if (this_prop->GetRepresentation() == VTK_WIREFRAME)
      this_prop->SetRepresentation(VTK_SURFACE);
    else if (this_prop->GetRepresentation() == VTK_SURFACE)
      this_prop->SetRepresentation(VTK_WIREFRAME);
  }
}

    //! put the specified extractor at the head of the pipeline, just after myUG
void vtkMOABUtils::add_geom_extractors(vtkImplicitFunction *this_func) 
{
  assert(NULL != myRen);

  vtkActorCollection *my_actors = myRen->GetActors();
  my_actors->InitTraversal();
  vtkActor *this_actor = my_actors->GetNextActor();
  while (NULL != this_actor) {
      // trace back to mapper input
    vtkDataSetMapper *this_dsmapper = vtkDataSetMapper::SafeDownCast(this_actor->GetMapper());
    vtkPolyDataMapper *this_pdmapper = vtkPolyDataMapper::SafeDownCast(this_actor->GetMapper());

    if (NULL != this_dsmapper) {
      vtkDataSet *this_ds = this_dsmapper->GetInput();
    
      // re-route this data set through a new vtkExtractGeometry and that to the mapper
      vtkExtractGeometry *eg = vtkExtractGeometry::New();
      eg->SetImplicitFunction(this_func);
      eg->SetInput(this_ds);
      this_dsmapper->SetInput(eg->GetOutput());
    }
    else if (NULL != this_pdmapper) {
      vtkPolyData *this_pd = this_pdmapper->GetInput();
    
      // re-route this data set through a new vtkExtractGeometry and that to the mapper
      vtkExtractPolyDataGeometry *epg = vtkExtractPolyDataGeometry::New();
      epg->SetImplicitFunction(this_func);
      epg->SetInput(this_pd);
      this_pdmapper->SetInput(epg->GetOutput());
    }
    else {
      assert(false);
    }

    this_actor = my_actors->GetNextActor();
  }
  

/*
  this_extr->SetInput(myUG);

    // put this extractor between myUG and all its consumers
  int num_cons = myUG->GetNumberOfConsumers();

    // build a list of consumers
  std::vector<vtkObject*> consumers;
  for (int i = 0; i < num_cons; i++)
    consumers.push_back(myUG->GetConsumer(i));

    // now reset the inputs of these consumers to be this_extr; should remove the consumers
    // from ug also, I'd think
  for (int i = 0; i < num_cons; i++) {
    vtkObject *this_obj = consumers[i];
    
      // try the various types they can be
    vtkDataSetToPolyDataFilter *ds = vtkDataSetToPolyDataFilter::SafeDownCast(this_obj);
    if (NULL != ds) {
      ds->SetInput(this_extr->GetOutput());
      ds->Update();
      continue;
    }
    
    vtkDataSetToUnstructuredGridFilter *dsu = vtkDataSetToUnstructuredGridFilter::SafeDownCast(this_obj);
    if (NULL != dsu) {
      dsu->SetInput(this_extr->GetOutput());
      dsu->Update();
      continue;
    }
    
    vtkDataSetMapper *dsm = vtkDataSetMapper::SafeDownCast(this_obj);
    if (NULL != dsm) {
      dsm->SetInput(this_extr->GetOutput());
      dsm->Update();
      continue;
    }
    
  }
*/
  
}

    //! remove the specified extractor from the head of the pipeline, just after myUG
void vtkMOABUtils::remove_geom_extractors() 
{
  assert(NULL != myRen);

  vtkActorCollection *my_actors = myRen->GetActors();
  my_actors->InitTraversal();
  vtkActor *this_actor = my_actors->GetNextActor();
  while (NULL != this_actor) {
      // trace back to mapper input
    vtkDataSetMapper *this_dsmapper = vtkDataSetMapper::SafeDownCast(this_actor->GetMapper());
    vtkPolyDataMapper *this_pdmapper = vtkPolyDataMapper::SafeDownCast(this_actor->GetMapper());

    if (NULL != this_dsmapper) {
      vtkExtractGeometry *eg = vtkExtractGeometry::SafeDownCast(this_dsmapper->GetInput()->GetSource());
      if (NULL != eg) {
        vtkDataSet* set = vtkDataSet::SafeDownCast(eg->GetInput());
        this_dsmapper->SetInput(set);
        eg->Delete();
      }
      else 
        std::cerr << "Didn't find vtkExtractGeometry!" << std::endl;
    }
    else if (NULL != this_pdmapper) {
      vtkExtractPolyDataGeometry *epg = 
        vtkExtractPolyDataGeometry::SafeDownCast(this_pdmapper->GetInput()->GetSource());
      if (NULL != epg) {
        vtkPolyData* pd = vtkPolyData::SafeDownCast(epg->GetInput());
        this_pdmapper->SetInput(pd);
        epg->Delete();
      }
      else 
        std::cerr << "Didn't find vtkExtractPolyDataGeometry!" << std::endl;
    }
    else {
      assert(false);
    }

    this_actor = my_actors->GetNextActor();
  }

/*


  assert(NULL != myUG);
  
    // remove this extractor from the pipeline
  int num_cons = this_extr->GetOutput()->GetNumberOfConsumers();

    // build a list of consumers
  std::vector<vtkObject*> consumers;
  for (int i = 0; i < num_cons; i++)
    consumers.push_back(this_extr->GetOutput()->GetConsumer(i));

    // now reset the inputs of these consumers to be this_extr; should remove the consumers
    // from ug also, I'd think
  for (int i = 0; i < num_cons; i++) {
    vtkObject *this_obj = consumers[i];
    
      // try the various types they can be
    vtkDataSetToUnstructuredGridFilter *dsu = vtkDataSetToUnstructuredGridFilter::SafeDownCast(this_obj);
    if (NULL != dsu) {
      dsu->SetInput(myUG);
      continue;
    }
    
    vtkDataSetToPolyDataFilter *ds = vtkDataSetToPolyDataFilter::SafeDownCast(this_obj);
    if (NULL != ds) {
      ds->SetInput(myUG);
      continue;
    }
    
    vtkDataSetMapper *dsm = vtkDataSetMapper::SafeDownCast(this_obj);
    if (NULL != dsm) {
      dsm->SetInput(myUG);
      continue;
    }
    
  }
  
  this_extr->SetInput(NULL);

*/
}

void vtkMOABUtils::construct_lookup_table(const int max_scalars) 
{
    // constructs color lookup table and populates 
  if (NULL != lookupTable) lookupTable->Delete();
  
  lookupTable = vtkLookupTable::New();

    // set the range to 0..max_scalars
  lookupTable->SetTableRange(0.0, (double) max_scalars);
  lookupTable->Build();
  lookupTable->SetRampToLinear();
}

MBErrorCode vtkMOABUtils::get_colors(MBEntityHandle dual_set,
                                     const int total_colors, int &global_id,
                                     double &red, double &green, double &blue) 
{
#define MIN(a,b) (a > b ? b : a)
#define MAX(a,b) (a < b ? b : a)

  if (NULL == lookupTable) construct_lookup_table(total_colors);
  
  MBErrorCode result = 
    vtkMOABUtils::mbImpl->tag_get_data(vtkMOABUtils::globalId_tag(), &dual_set, 
                                       1, &global_id);
  if (MB_SUCCESS != result) return result;

  int max_val = (int) lookupTable->GetTableRange()[1];
  int color_id = (global_id > max_val ? global_id % max_val : global_id);
  
  double colors[3];
  lookupTable->GetColor((double)color_id, colors);
  
  red = colors[0];
  green = colors[1];
  blue = colors[2];

/*  
  if (global_id >= 0) {
      // set the color by this id
    double factor = ((double)(global_id%total_colors)) / ((double)total_colors);
    red = RED(factor);
    green = GREEN(factor);
    blue = BLUE(factor);
  }

*/
  
  return MB_SUCCESS;
}

void vtkMOABUtils::update_display(vtkUnstructuredGrid *ug) 
{
  if (NULL == myUG) {
    if (NULL == ug) ug = vtkUnstructuredGrid::New();
    myUG = ug;

      // make sure there's a mapper, actor for the whole mesh in ug, put in renderer
    vtkPolyDataMapper *poly_mapper;
    vtkDataSetMapper *set_mapper;
    vtkTubeFilter *tube_filter;
    vtkExtractEdges *edge_filter;

    vtkActor *mesh_actor = vtkActor::New();
  
    bool tubes = true;

    if (tubes) {
        // extract edges and build a tube filter for them
      poly_mapper =  vtkPolyDataMapper::New();
      mesh_actor->SetMapper(poly_mapper);
      tube_filter = vtkTubeFilter::New();
      poly_mapper->SetInput(tube_filter->GetOutput());
      edge_filter = vtkExtractEdges::New();
      tube_filter->SetInput(edge_filter->GetOutput());

        /*
      cell_filter = vtkExtractCells::New();
      vtkIdList *ids;
      MBRange ents, dum_ents;
      MBErrorCode result = mbImpl->get_entities_by_dimension(0, 3, dum_ents);
      if (MB_SUCCESS != result) return;
      for (MBRange::iterator rit = dum_ents.begin(); rit != dum_ents.end(); rit++)
        if (mbImpl->type_from_handle(*rit) != MBPOLYHEDRON) ents.insert(*rit);
      
      result = vtkMOABUtils::get_id_list(ents, ids);
      cell_filter->SetInput(myUG);
      cell_filter->AddCellList(ids);
      ids->Delete();
      edge_filter->SetInput(cell_filter->GetOutput());
        */
      edge_filter->SetInput(myUG);
      
      tube_filter->SetNumberOfSides(6);
      tube_filter->SetRadius(0.005);
      poly_mapper->ImmediateModeRenderingOn();
    }
    else {
      set_mapper = vtkDataSetMapper::New();
      set_mapper->SetInput(vtkMOABUtils::myUG);
      mesh_actor->SetMapper(set_mapper);
      set_mapper->ImmediateModeRenderingOn();
    }
  
    vtkMOABUtils::myRen->AddActor(mesh_actor);
    MBErrorCode result = vtkMOABUtils::mbImpl->tag_set_data(vtkMOABUtils::vtkSetActorTag, 
                                                            NULL, 0, &mesh_actor);

    if (MB_SUCCESS != result) {
      std::cerr << "Failed to set actor for mesh in vtkMOABUtils::update_display()." << std::endl;
      return;
    }
    
      // now turn around and set a different property for the mesh, because we want the tubes
      // to be shaded in red
    vtkMOABUtils::actorProperties[mesh_actor] = NULL;
    vtkProperty *this_prop = vtkMOABUtils::get_property(mesh_actor, true);
    vtkMOABUtils::actorProperties[mesh_actor] = this_prop;
    this_prop->SetRepresentationToSurface();
    this_prop->SetColor(0.0, 1.0, 0.0);
    this_prop->SetEdgeColor(0.0, 1.0, 0.0);
//  mesh_actor->VisibilityOff();
  

      /*
        // center the camera on the center of the ug
        vtkMOABUtils::myRen->GetActiveCamera()->SetFocalPoint(ug->GetPoint(1));
        vtkMOABUtils::myRen->GetActiveCamera()->SetPosition(0, 0, 50.0);
        vtkMOABUtils::myRen->GetActiveCamera()->SetViewUp(0, 1.0, 0.0);
  
        std::cout << "Set focal point to " 
        << ug->GetPoint(1)[0] 
        << ", "
        << ug->GetPoint(1)[1] 
        << ", "
        << ug->GetPoint(1)[2] 
        << std::endl;
      */

    mesh_actor->Delete();
    if (tubes) {
//    tube_filter->Delete();
//    edge_filter->Delete();
//    poly_mapper->Delete();
    }
    else {
      set_mapper->Delete();
    }
      
  }
  
  MBErrorCode result = vtkMOABUtils::make_vertex_points(myUG);
  if (MB_SUCCESS != result)
    {
      std::cerr << "Failed to make vertex points." << std::endl;
      return;
    }

    // now make the cells
  result = vtkMOABUtils::make_cells(myUG);
  if (MB_SUCCESS != result)
    {
      std::cerr << "Failed to make cells." << std::endl;
    return;
    }

  result = update_all_actors(0, myUG, false);
  if (MB_SUCCESS != result)
  {
    std::cerr << "Failed to update. " << std::endl;
  }
  
  // Render
  myRen->GetRenderWindow()->Render();

    // Reset camera
  vtkMOABUtils::myRen->ResetCamera();
}

  //! get rid of all the vtk drawing stuff
void vtkMOABUtils::reset_drawing_data() 
{
  MBRange these_sets;
  MBErrorCode result;
  dualTool->get_dual_hyperplanes(vtkMOABUtils::mbImpl, 1, these_sets);
  dualTool->get_dual_hyperplanes(vtkMOABUtils::mbImpl, 2, these_sets);

  for (MBRange::iterator rit = these_sets.begin(); rit != these_sets.end(); rit++)
    vtkMOABUtils::get_actor(*rit)->Delete();

  actorProperties.clear();

    //! map between props (actor2d's and actors) and sets they represent (0 if no set, 
    //! e.g. an extracted set)
  propSetMap.clear();

  if (NULL != topContainsAssy) {
    topContainsAssy->Delete();
    topContainsAssy = NULL;
  }
  
    //! topmost assembly for displaying parent/child relationships
  if (NULL != topParentAssy) {
    topParentAssy->Delete();
    topParentAssy = NULL;
  }

  if (NULL != drawDual) {
    delete drawDual;
    drawDual = NULL;
  }
  
  if (NULL != myUG) {
    myUG->Delete();
    myUG = NULL;
  }

  if (NULL == mbImpl) return;
  
  //! tag indicating whether a given set is in top contains assy
  result = mbImpl->tag_delete(vtkTopContainsTag);
  if (MB_SUCCESS != result && MB_TAG_NOT_FOUND != result) 
    std::cout << "Trouble deleting tag." << std::endl;
  vtkTopContainsTag = NULL;

  //! tag indicating whether a given set is in top parent assy
  result = mbImpl->tag_delete(vtkTopParentTag);
  if (MB_SUCCESS != result && MB_TAG_NOT_FOUND != result) 
    std::cout << "Trouble deleting tag." << std::endl;
  vtkTopParentTag = NULL;
  result = mbImpl->tag_delete(vtkCellTag);
  if (MB_SUCCESS != result && MB_TAG_NOT_FOUND != result) 
    std::cout << "Trouble deleting tag." << std::endl;
  vtkCellTag = NULL;
  result = mbImpl->tag_delete(vtkSetActorTag);
  if (MB_SUCCESS != result && MB_TAG_NOT_FOUND != result) 
    std::cout << "Trouble deleting tag." << std::endl;
  vtkSetActorTag = NULL;
  result = mbImpl->tag_delete(vtkSetPropAssemblyTag);
  if (MB_SUCCESS != result && MB_TAG_NOT_FOUND != result) 
    std::cout << "Trouble deleting tag." << std::endl;
  vtkSetPropAssemblyTag = NULL;
  result = mbImpl->tag_delete(vtkPointAllocatedTag);
  if (MB_SUCCESS != result && MB_TAG_NOT_FOUND != result) 
    std::cout << "Trouble deleting tag." << std::endl;
  vtkPointAllocatedTag = NULL;

  create_tags();
}

void vtkMOABUtils::assign_global_ids()
{
  std::vector<int> ids;
  MBTag gid = globalId_tag();
  MBErrorCode result;

  MBEntityType types[] = {MBVERTEX, MBEDGE, MBQUAD, MBHEX};
  
  for (unsigned int j = 0; j <= 3; j++) {
    MBRange ents;
    result = mbImpl->get_entities_by_type(0, types[j], ents);
    if (MB_SUCCESS != result) return;
  
    ids.resize(ents.size());
    std::fill(ids.begin(), ids.end(), -1);
    mbImpl->tag_get_data(gid, ents, &ids[0]);

    bool need_set = false;
  
      // get max id
    int max_id = -1;
    for (unsigned int i = 0; i < ids.size(); i++)
      if (ids[i] > max_id) max_id = ids[i];

    for (unsigned int i = 0; i < ids.size(); i++) {
      if (ids[i] == 0) {
        ids[i] = ++max_id;
        need_set = true;
      }
    }

    if (need_set) mbImpl->tag_set_data(gid, ents, &ids[0]);
  }

}
