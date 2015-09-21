#ifndef VTKMOABUTILS
#define VTKMOABUTILS

// Utility class to support use of MOAB in vtk
//
// mostly static variables
//

#include "MBInterface.hpp"
#include "MBRange.hpp"
#include <map>
#include "vtkUnstructuredGrid.h"
class vtkRenderer;
class vtkActor;
class vtkMapper;
class vtkExtractCells;
class vtkActor2D;
class vtkProp;
class vtkIdList;
class vtkCellPicker;
class vtkCallbackCommand;
class vtkProperty;
class vtkPropAssembly;
class vtkImplicitFunction;
class vtkLookupTable;
class DrawDual;
class DualTool;

class VTK_EXPORT vtkMOABUtils //: public vtkObject
{
public:

    //! standard constructor-type function
//  static vtkMOABUtils *New();

    //! static interface pointer, use this when accessing MOAB within vtk
  static MBInterface *mbImpl;

    //! static dualtool ptr
  static DualTool *dualTool;

    //! static pointer to the renderer
  static vtkRenderer *myRen;

    //! unstructured grid at the start of the pipeline
  static vtkUnstructuredGrid *myUG;
  
    //! the default property
  static vtkProperty *topProperty;
  
    //! the highlight property
  static vtkProperty *highlightProperty;
  
    //! map between actors in the display and properties; if null, an actor
    //! inherits from topProperty
  static std::map<vtkActor*, vtkProperty*> actorProperties;

    //! map between props (actor2d's and actors) and sets they represent (0 if no set, 
    //! e.g. an extracted set)
  static std::map<vtkProp*, MBEntityHandle> propSetMap;

    //! topmost assembly for displaying contains relationships
  static vtkPropAssembly *topContainsAssy;
  
    //! topmost assembly for displaying parent/child relationships
  static vtkPropAssembly *topParentAssy;
  
  //! tag indicating whether a given set is in top contains assy
  static MBTag vtkTopContainsTag;

    //! name for vtkTopContainsTag;
  static const char *vtkTopContainsTagName;
  
  //! tag indicating whether a given set is in top parent assy
  static MBTag vtkTopParentTag;

    //! name for vtkTopParentTag;
  static const char *vtkTopParentTagName;
  
    //! tag for pointing to vtk cell representing an entity
  static MBTag vtkCellTag;
  
    //! name for vtkCellTag
  static const char *vtkCellTagName;

    //! tag for pointing to vtk actor for a set
  static MBTag vtkSetActorTag;
  
    //! name for vtkSetActorTag
  static const char *vtkSetActorTagName;

    //! tag for pointing to vtk prop assembly for a set; a prop assembly
    //! for a set is only created if the set contains sets
  static MBTag vtkSetPropAssemblyTag;
  
    //! name for vtkSetPropAssemblyTag
  static const char *vtkSetPropAssemblyTagName;

    //! tag for determining whether a point has been allocated for a vertex
  static MBTag vtkPointAllocatedTag;
  
    //! name for vtkPointAllocatedTag
  static const char *vtkPointAllocatedTagName;

    //! picked entities
  static MBRange pickedEntities;

  static DrawDual *drawDual;
  
    //! vtk command structure for processing pick events; modeled after
    //! vtkInteractorStyle::EventCallbackCommand
  static vtkCallbackCommand *eventCallbackCommand;

    //! convenience tags and functions for accessing them
  static MBTag globalIdTag;
  static MBTag globalId_tag();
  static MBTag categoryTag;
  static MBTag category_tag();
  
    //! mapping from MOAB to vtk entity types
  static const int vtk_cell_types[];

    //! number of colors to use
  static int totalColors;

    //! a common lookup table for colors
  static vtkLookupTable *lookupTable;

    //! initialize the data in this class
  static MBErrorCode init(MBInterface *impl, 
                          vtkRenderer *this_ren);

    //! initialize some shared properties
  static void make_properties();

    //! destroy static data in this class
  static void destroy();
  
    //! for a given range of entities, make vtk cells and put in ug
    //! (only for entities with default vtkCellTag value)
  static MBErrorCode make_cells(MBRange &ents, 
                                vtkUnstructuredGrid *&ug);
  
    //! variations on make_cells above
  static MBErrorCode make_cells(MBEntityType in_type,
                                vtkUnstructuredGrid *&ug);
  
    //! variations on make_cells above
  static MBErrorCode make_cells(vtkUnstructuredGrid *&ug);
  
    //! for vertices, construct points and put in ug
    //! assumes point ids will match vertex ids
  static MBErrorCode make_vertex_points(vtkUnstructuredGrid *&ug);
  
    //! for all sets contained in this set (exclusive), check for an actor, and make 
    //! one if one's not found
  static MBErrorCode update_all_actors(MBEntityHandle this_set,
                                       vtkUnstructuredGrid *ug,
                                       const bool shaded = true,
                                       const bool tubed = false,
                                       const bool colored = false);
  
    //! for all sets in this range, check for an actor, and make 
    //! one if one's not found
  static MBErrorCode update_set_actors(const MBRange &update_sets,
                                       vtkUnstructuredGrid *ug,
                                       const bool shaded = true,
                                       const bool tubed = false,
                                       const bool colored = false);
  
    //! set up a tube filter on this set & ec, and assign it to a (new) mapper
  static MBErrorCode setup_tube_filter(MBEntityHandle this_set, 
                                       vtkExtractCells *ec,
                                       vtkMapper *&this_mapper);
  
    //! given a set, get the list of vtk cell ids for its entities
  static MBErrorCode get_id_list(MBEntityHandle this_set, vtkIdList *&ids);
  
    //! given a range, get the list of vtk cell ids for its entities
  static MBErrorCode get_id_list(MBRange &ents, vtkIdList *&ids);
  
    //! given a prop assembly, remove all its parts
  static MBErrorCode empty_assy(vtkPropAssembly *this_assy);

    //! return all top-level "contains" sets, i.e. those which aren't contained in others
  static MBErrorCode get_top_contains_sets(MBRange &top_sets);
  
    //! return all top-level parent sets, i.e. those with children who aren't 
    //! children of others
  static MBErrorCode get_top_parent_sets(MBRange &top_sets);

    //! given a set, return the colors based on input total colors & global id
  static MBErrorCode get_colors(MBEntityHandle dual_set,
                                const int total_colors, int &global_id,
                                double &red, double &green, double &blue);
  
    //! construct the color lookup table
  static void construct_lookup_table(const int max_scalars);
  
    //! given a set and property, set the color based on total_colors & global id
  static MBErrorCode set_color(MBEntityHandle dual_set,
                               vtkProperty *this_property,
                               const int total_colors = totalColors);
  
    //! given an entity set, return the property of the associated actor; if none,
    //! and make_if_missing is true, make one
  static vtkProperty *get_property(MBEntityHandle this_set, const bool make_if_missing = false);

    //! return the property of the associated actor; if none,
    //! and make_if_missing is true, make one
  static vtkProperty *get_property(vtkActor *this_actor, const bool make_if_missing = false);

    //! given a set, get the actor, making a new one if requested and necessary
  static vtkActor *get_actor(MBEntityHandle this_set,
                             const bool make_if_missing = false);
  
    //! given a prop, get the corresponding set
  static MBEntityHandle get_set(vtkProp *this_prop);
  
    //! return a category name, if any, for this set in this_name
  static MBErrorCode get_set_category_name( MBEntityHandle this_set, char *this_name );

    //! print debug information about all the actors
  static void print_debug();
  
    //! make sets visible/invisible (property set on actors)
  static void change_set_visibility( MBRange &visible_sets, MBRange &invisible_sets );
  
    //! highlight or unhighlight sets
  static void change_set_properties(MBRange &high_mbsets, MBRange &unhigh_mbsets);

    //! toggle the wireframe/shaded property
  static void toggle_wireframe_shaded(MBRange &high_mbsets);

    //! put the specified extractor at the head of the pipeline, just after myUG
  static void add_geom_extractors(vtkImplicitFunction *this_func);

    //! remove the specified extractor from the head of the pipeline, just after myUG
  static void remove_geom_extractors();

    //! update the display with all the mesh
  static void update_display(vtkUnstructuredGrid *ug = NULL);

    //! get rid of all the vtk drawing stuff
  static void reset_drawing_data();

    //! assign global ids, in preparation for writing mesh
  static void assign_global_ids();

    //! debug parameter
  static bool debug;
  
  static MBErrorCode create_tags();
  
private:
    //! private constructor so nobody can construct one
  vtkMOABUtils();
  
};

#endif
