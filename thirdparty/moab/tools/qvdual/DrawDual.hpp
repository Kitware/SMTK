#ifndef DRAWDUAL_HPP
#define DRAWDUAL_HPP

class Agnode_t;
class Agedge_t;
class Agraph_t;

class SheetDiagramPopup;

#include "moab/Interface.hpp"
#include "moab/Range.hpp"
#include <map>
#include <vector>

class vtkPolyData;
class vtkRenderer;
class vtkCellPicker;
class vtkUnstructuredGrid;
class vtkExtractCells;
class vtkObject;
class vtkFloatArray;
class vtkCellArray;
class vtkActor;
class Agsym_t;
class QLineEdit;

using namespace moab;

class DrawDual
{
public:
  DrawDual(QLineEdit *pickline1, QLineEdit *pickline2);
  ~DrawDual();

  bool draw_dual_surfs(Range &dual_surfs,
                       const bool use_offsets = false);
  bool print_dual_surfs(Range &dual_surfs,
                        const bool use_offsets = false);
  bool draw_dual_surfs(std::vector<EntityHandle> &dual_surfs,
                       const bool use_offsets = false);
  ErrorCode draw_dual_surf(EntityHandle dual_surf,
                             int offset_num = 0);
  
  EntityHandle lastPickedEnt; // last picked entity
  EntityHandle secondLastPickedEnt; // second last picked entity

    //! reset the drawing data for a sheet
  ErrorCode reset_drawing_data(EntityHandle dual_surf);

  ErrorCode reset_drawn_sheets(Range *drawn_sheets = NULL);
  
  void print_picked_ents(Range &picked_ents,
                         bool from_return = false);

private:

  static DrawDual *gDrawDual;
  QLineEdit *pickLine1, *pickLine2;

  static bool useGraphviz;

  class GVEntity
  {
  public:
    int numGvizEntities;
    EntityHandle dualSurfs[3];
    EntityHandle moabEntity;
    int pointPos[3][2];
    int vtkEntityIds[4]; // extra pt for edge mid-pts
    vtkActor *myActors[3];
    void *gvizPoints[5]; // extra 2 for edge mid-pts
    void *gvizEdges[4]; // extra 2 for extra edges
    
    GVEntity() 
      {
        numGvizEntities = 0;
        dualSurfs[0] = dualSurfs[1] = dualSurfs[2] = 0;
        moabEntity = 0;
        pointPos[0][0] = pointPos[0][1] = pointPos[0][2] = 
          pointPos[1][0] = pointPos[1][1] = pointPos[1][2] = 0;
        vtkEntityIds[0] = vtkEntityIds[1] = vtkEntityIds[2] = vtkEntityIds[3] = -1;
        myActors[0] = myActors[1] = myActors[2] = NULL;
        gvizPoints[0] = gvizPoints[1] = gvizPoints[2] = gvizPoints[3] = 
          gvizPoints[4] = NULL;
        gvizEdges[0] = gvizEdges[1] = gvizEdges[2] = gvizEdges[3] = NULL;
      }
    void reset(const int index);
    int get_index(const EntityHandle dual_surf) 
      {
        if (dual_surf == dualSurfs[0]) return 0;
        else if (dual_surf == dualSurfs[1]) return 1;
        else if (dual_surf == dualSurfs[2]) return 2;
        else if (dualSurfs[0] == 0) return -1;
        else if (dualSurfs[1] == 0) return -2;
        else if (dualSurfs[2] == 0) return -3;
        else return -10;
      }
  };

  class GraphWindows 
  {
  public:
    Agraph_t *gvizGraph;
    SheetDiagramPopup *sheetDiagram;
    vtkActor *pickActor;

    GraphWindows() : gvizGraph(NULL), sheetDiagram(NULL), pickActor(NULL) {}
    ErrorCode reset(EntityHandle dual_surf);
  };
  
    //! make sure all dual vertices and edges have graphviz nodes and edges
  ErrorCode construct_graphviz_data(EntityHandle dual_surf,
                                      Range &dcells, Range &dedges,
                                      Range &dverts, Range &face_verts,
                                      Range &loop_edges);
  
    //! given the loop vertices, compute and fix their points
  ErrorCode compute_fixed_points(EntityHandle dual_surf, Range &dverts,
                                   Range &face_verts, Range &loop_edges);

    //! compute fixed points for a pillow sheet
  ErrorCode compute_pillow_fixed_points(EntityHandle dual_surf, 
                                          Range &face_verts, 
                                          Range &face_edges);
  
    //! compute the position on the loop, accounting for multiple loops
  void get_loop_vertex_pos(unsigned int vert_num, 
                           unsigned int loop_num, 
                           unsigned int num_loops, 
                           double angle, int &xpos_pts, int &ypos_pts);
  
    //! construct the points & cells for the vtkPolyData from the MOAB/graphviz data
  ErrorCode make_vtk_data(EntityHandle dual_surf,
                            vtkPolyData *pd,
                            vtkRenderer *this_ren);

    //! construct dim-dimensional cells
  ErrorCode make_vtk_cells(const Range &cell_range, const int dim,
                             const float color_index,
                             const EntityHandle dual_surf,
                             std::map<EntityHandle, GVEntity *> &vert_gv_map,
                             vtkPolyData *pd,
                             vtkFloatArray *color_ids);
  
    //! given a qvtk widget, return the first polydata supplying data to it
  vtkPolyData *get_polydata(SheetDiagramPopup *this_sdpopup);
  
    //! get a clean polydata for this widget
  void get_clean_pd(EntityHandle dual_surf,
                    SheetDiagramPopup *&this_sdpopup, vtkPolyData *&pd);

    //! draw various labels with the sheet
  ErrorCode draw_labels(EntityHandle dual_surf,
                          vtkPolyData *pd,
                          vtkPolyData *new_pd);

  ErrorCode label_other_sheets(EntityHandle dual_surf,
                                 vtkPolyData *pd,
                                 vtkPolyData *&new_pd);
  
  void label_interior_verts(EntityHandle dual_surf,
                            vtkPolyData *pd,
                            vtkRenderer *ren);
  
  EntityHandle other_sheet(const EntityHandle this_chord,
                             const EntityHandle dual_surf);
  
  ErrorCode get_primal_ids(const Range &ents, std::vector<int> &ids);
  
  ErrorCode allocate_points(EntityHandle dual_surf,
                              vtkPolyData *pd,
                              Range &verts,
                              Range &loop_edges,
                              std::map<EntityHandle, GVEntity*> &vert_gv_map);
  
  static void add_picker(vtkRenderer *this_ren);
  
  static void process_events(vtkObject *caller, 
                             unsigned long event,
                             void* clientdata, 
                             void* /*vtkNotUsed(calldata)*/);
  
  static void process_pick(vtkRenderer *ren);

    //! given a dual surface and the pick point (in world coords), return a list 
    //! of picked entities on that sheet drawing
  ErrorCode process_pick(EntityHandle dual_surf, 
                           const double x, const double y,
                           Range &picked_ents);
  
    //! map of dual surfaces and windows they're drawn in
  std::map<EntityHandle, GraphWindows> surfDrawrings;
  
  
    //! cache some of the tags we use
  Tag gvEntityHandle, dualEntityTagHandle;

  static Tag dualSurfaceTagHandle, dualCurveTagHandle;

    //! gviz graphics context, seems to be needed for layout
  void *gvizGvc;

    //! information about the sheet drawing window
  int xSize, ySize, xOrigin, yOrigin;

    //! picker for dual data
  static vtkCellPicker *dualPicker;

    //! entities which are currently picked
  static Range pickRange;

  EntityHandle get_picked_cell(EntityHandle cell_set,
                                 const int dim,
                                 const int picked_cell);

  void update_high_polydatas();
  
  ErrorCode get_xform(EntityHandle dual_surf, Agsym_t *asym_pos, 
                        double &x_xform, double &y_xform);
  
  ErrorCode construct_graphviz_points(EntityHandle dual_surf, 
                                        Range &dverts, 
                                        Agsym_t *asym_pos);
  
  ErrorCode construct_graphviz_edges(EntityHandle dual_surf, 
                                       Range &dedges, 
                                       Range &loop_verts, 
                                       Agsym_t *asym_pos);
  
  Agsym_t *get_asym(EntityHandle dual_surf, const int dim,
                    const char *name, const char *def_val = NULL);
  
  ErrorCode fixup_degen_bchords(EntityHandle dual_surf);

    //! given some entities, get the corresponding gviz points on the sheet
  void get_points(const EntityHandle *ents, const int num_ents, 
                  const bool extra,
                  EntityHandle dual_surf, Agnode_t **points);

    //! smooth the points in the dual surface using length-weighted laplacian smoothing
  ErrorCode smooth_dual_surf(EntityHandle dual_surf,
                               Range &dcells, Range &dedges,
                               Range &dverts, Range &face_verts,
                               Range &loop_edges);
  
  ErrorCode set_graphpoint_pos(void *point, double *pos);
  
  ErrorCode get_graphpoint_pos(void *point, double *pos);
  
  void get_graph_points(const EntityHandle *ents, const int gnum_ents, 
                        EntityHandle dual_surf, void **points);

  void get_graph_points(Range ents,
                        EntityHandle dual_surf, void **points);

    //! given a renderer, return the sheet that this renderer renders
  EntityHandle get_dual_surf(vtkRenderer *this_ren);
};


#endif
