#include "moab/DrawDual.hpp"
#include "SheetDiagramPopup.h"
#include "moab/MeshTopoUtil.hpp"
#include "MBTagConventions.hpp"
#include "moab/CartVect.hpp"
#include "vtkMOABUtils.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkPolyDataMapper.h"
#include "vtkDataSetMapper.h"
#include "vtkMapper2D.h"
#include "vtkMapper.h"
#include "vtkLabeledDataMapper.h"
#include "vtkActor2D.h"
#include "vtkTextActor.h"
#include "vtkIdFilter.h"
#include "vtkTextProperty.h"
#include "vtkPoints.h"
#include "vtkPointData.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRendererCollection.h"
#include "vtkActorCollection.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCellPicker.h"
#include "vtkCallbackCommand.h"
#include "vtkCamera.h"
#include "vtkCoordinate.h"
#include "vtkProperty2D.h"
#include "vtkProperty.h"
#include "vtkExtractEdges.h"
#include "vtkExtractCells.h"
#include "vtkTubeFilter.h"
#include "vtkGeometryFilter.h"
#include "vtkInteractorStyle.h"
#include "vtkLookupTable.h"
#include "QVTKWidget.h"
#include "qlineedit.h"
#include "vtkFloatArray.h"
#include "vtkMaskFields.h"
#include "vtkWindowToImageFilter.h"
#include "vtkPNGWriter.h"
#include "assert.h"
#include <sstream>

extern "C" 
{
#include "gvc.h"
//  extern GVC_t *gvContext();
}

const bool my_debug = false;

const int SHEET_WINDOW_SIZE = 500;

//const int RAD_PTS = 400;
const int RAD_PTS = 10;
const int CENT_X = 0;
const int CENT_Y = 0;

using namespace moab;

#define MBI vtkMOABUtils::mbImpl
#define RR if (MB_SUCCESS != result) return result
#define SWAP(a,b) {EntityHandle tmp_ent = a; a = b; b = tmp_ent;}

vtkCellPicker *DrawDual::dualPicker = NULL;

Tag DrawDual::dualCurveTagHandle = 0;
Tag DrawDual::dualSurfaceTagHandle = 0;

DrawDual *DrawDual::gDrawDual = NULL;
Range DrawDual::pickRange;

bool DrawDual::useGraphviz = false;

DrawDual::DrawDual(QLineEdit *pickline1, QLineEdit *pickline2) 
    : pickLine1(pickline1), pickLine2(pickline2)
{
    // make sure we have basic tags we need
  EntityHandle dum = 0;
  ErrorCode result = MBI->tag_get_handle(DualTool::DUAL_ENTITY_TAG_NAME, 
                                         1, MB_TYPE_HANDLE, dualEntityTagHandle,
                                         MB_TAG_DENSE|MB_TAG_CREAT, &dum);

  result = MBI->tag_get_handle(DualTool::DUAL_SURFACE_TAG_NAME, 1, MB_TYPE_HANDLE, dualSurfaceTagHandle);
  assert(MB_SUCCESS == result);

  result = MBI->tag_get_handle(DualTool::DUAL_CURVE_TAG_NAME, 1, MB_TYPE_HANDLE, dualCurveTagHandle);
  assert(MB_SUCCESS == result);

  if (useGraphviz) {
      // initialize dot
    aginit();
  }
  
  GVEntity *dum2 = NULL;
  result = MBI->tag_get_handle("__GVEntity", sizeof(GVEntity*), MB_TAG_OPAQUE,
                              gvEntityHandle, MB_TAG_DENSE|MB_TAG_CREAT, &dum2);
  assert(MB_SUCCESS == result && 0 != gvEntityHandle);

  assert(gDrawDual == NULL);
  gDrawDual = this;

  lastPickedEnt = 0;
  secondLastPickedEnt = 0;
  
  add_picker(vtkMOABUtils::myRen);

}

DrawDual::~DrawDual() 
{
  if (0 != gvEntityHandle) {
    Range ents;
    MBI->get_entities_by_type_and_tag(0, MBVERTEX, &gvEntityHandle,
                                      NULL, 1, ents, Interface::UNION);
    MBI->get_entities_by_type_and_tag(0, MBEDGE, &gvEntityHandle,
                                      NULL, 1, ents, Interface::UNION);
    MBI->get_entities_by_type_and_tag(0, MBPOLYGON, &gvEntityHandle,
                                      NULL, 1, ents, Interface::UNION);
    std::vector<GVEntity*> gvents(ents.size());
    std::fill(gvents.begin(), gvents.end(), (GVEntity*)NULL);
    ErrorCode result = MBI->tag_get_data(gvEntityHandle, ents, &gvents[0]);
    if (MB_SUCCESS == result) {
      for (unsigned int i = 0; i < gvents.size(); i++) 
        delete gvents[i];
    }
    MBI->tag_delete(gvEntityHandle);
  }
  
  if (NULL != dualPicker) {
    dualPicker->Delete();
    dualPicker = NULL;
  }
  
  if (NULL != vtkMOABUtils::eventCallbackCommand) {
    vtkMOABUtils::eventCallbackCommand->Delete();
    vtkMOABUtils::eventCallbackCommand = NULL;
  }
  
  if (NULL != gDrawDual) gDrawDual = NULL;

}

void DrawDual::add_picker(vtkRenderer *this_ren) 
{
  assert(NULL != this_ren);
  
  if (NULL == dualPicker) {
    
      // create a cell picker
    dualPicker = vtkCellPicker::New();
    dualPicker->SetTolerance(0.05);

      // set up the callback handler for the picker
    vtkMOABUtils::eventCallbackCommand = vtkCallbackCommand::New();
      // do we need to do this???
      // eventCallbackCommand->SetClientData(this); 
    vtkMOABUtils::eventCallbackCommand->SetCallback(process_events);

  }
  
    // make sure the renderer can handle observers
  vtkRenderWindow *this_ren_window = this_ren->GetRenderWindow();
  assert(NULL != this_ren_window);
  vtkRenderWindowInteractor *this_int = this_ren_window->GetInteractor();
  if (NULL == this_int)
    this_int = this_ren_window->MakeRenderWindowInteractor();
    
  assert(NULL != this_int);
  
  vtkInteractorStyle *this_style = vtkInteractorStyle::SafeDownCast(
    this_int->GetInteractorStyle());
  assert(NULL != this_style);
  this_style->HandleObserversOn();

    // register this command to process the LMB event
  this_ren->GetRenderWindow()->GetInteractor()->GetInteractorStyle()->
    AddObserver(vtkCommand::LeftButtonPressEvent, 
                vtkMOABUtils::eventCallbackCommand);
}

void DrawDual::process_events(vtkObject *caller, 
                              unsigned long event,
                              void* clientdata, 
                              void* /*vtkNotUsed(calldata)*/) 
{
  assert(event == vtkCommand::LeftButtonPressEvent);

  vtkInteractorStyle* style = reinterpret_cast<vtkInteractorStyle *>(caller);
  vtkRenderWindowInteractor *rwi = style->GetInteractor();

    // check to see if the <Ctrl> key is on
  if (rwi->GetControlKey()) {
      // control key is on - send pick event to picker
    if (style->GetState() == VTKIS_NONE) 
    {
      style->FindPokedRenderer(rwi->GetEventPosition()[0],
                               rwi->GetEventPosition()[1]);
      rwi->StartPickCallback();
      vtkRenderer *ren = style->GetCurrentRenderer();
      EntityHandle dual_surf = gDrawDual->get_dual_surf(ren);
      if (0 == dual_surf) return;

      int ix = rwi->GetEventPosition()[0], 
        iy = rwi->GetEventPosition()[1];
      
      double x = (ix - 250) * RAD_PTS/170.0;
      double y = iy * RAD_PTS/170.0;
      
      if (my_debug)
        std::cout << "Picked point: " << rwi->GetEventPosition()[0]
                  << ", " << rwi->GetEventPosition()[1] << "(screen), "
                  << dualPicker->GetMapperPosition()[0] << ", "
                  << dualPicker->GetMapperPosition()[1] << " (mapper)"
                  << x << ", "
                  << y << " (world)"
                  << std::endl;

      Range picked_ents;
      ErrorCode result = gDrawDual->process_pick(dual_surf, 
                                                   x, y,
                                                   picked_ents);
      if (MB_SUCCESS != result) return;

      gDrawDual->print_picked_ents(picked_ents);

      if (!picked_ents.empty()) {
          // now update the highlighted polydata
      }

      rwi->EndPickCallback();
    }
  }
  else {
      // otherwise send event to interactor style handler
    style->OnLeftButtonDown();
  }
}

void DrawDual::process_pick(vtkRenderer *ren) 
{
  Range picked_ents;

  bool old_picking = false;
  if (old_picking) {
    
    assert(0 != dualCurveTagHandle);
  
      // get the actor through the prop, to make sure we get the leaf of an
      // assembly
    vtkActorCollection *actors = dualPicker->GetActors();
  
    vtkActor *tmp_actor;
      //vtkActor *picked_actor = NULL;
      //EntityHandle picked_sheet = 0, picked_chord = 0;
  
    actors->InitTraversal();
    while ((tmp_actor = actors->GetNextItem())) {
      EntityHandle this_set = vtkMOABUtils::propSetMap[tmp_actor];
      if (0 == this_set || -1 == dualPicker->GetCellId()) continue;

        // get whether it's a dual surface or dual curve
      EntityHandle dum_handle = 0;
      ErrorCode result = MBI->tag_get_data(dualCurveTagHandle, &this_set, 1, 
                                             &dum_handle);

      std::cout << "Picked point " << dualPicker->GetCellId() << std::endl;
    
      EntityHandle picked_ent;
      if (MB_TAG_NOT_FOUND == result || 0 == dum_handle)
          // must be a sheet
        picked_ent = gDrawDual->get_picked_cell(this_set, 2, dualPicker->GetCellId());
      else {
        picked_ent = gDrawDual->get_picked_cell(this_set, 1, dualPicker->GetCellId());
      }
      picked_ents.insert(picked_ent);
    }
  }
  else {
    EntityHandle dual_surf = gDrawDual->get_dual_surf(ren);
    if (0 == dual_surf) return;
    
    ErrorCode result = gDrawDual->process_pick(dual_surf, 
                                                 dualPicker->GetPickPosition()[0], 
                                                 dualPicker->GetPickPosition()[1], 
                                                 picked_ents);
    if (MB_SUCCESS != result) return;
    
  }
  
  if (picked_ents.empty()) return;

  gDrawDual->print_picked_ents(picked_ents);
  
//    Range::iterator pit = pickRange.find(picked_ent);
//    if (pit == pickRange.end()) pickRange.insert(picked_ent);
//    else pickRange.erase(pit);
  
    // now update the highlighted polydata
  gDrawDual->update_high_polydatas();

}

void DrawDual::print_picked_ents(Range &picked_ents, bool from_return) 
{
  for (Range::iterator rit = picked_ents.begin(); rit != picked_ents.end(); rit++) {
    EntityHandle picked_ent = *rit;
    
      // get the vertices
    std::ostringstream oss;
    const EntityHandle *connect;
    int num_connect;
    ErrorCode result = MBI->get_connectivity(picked_ent, connect, num_connect);
    if (MB_SUCCESS != result) return;
    bool first = true;
    EntityHandle primals[20];
    std::vector<int> ids;
  
    assert(num_connect < 20);
    if (MBI->type_from_handle(picked_ent) == MBPOLYGON) oss << "2-cell: ";
    else if (MBI->type_from_handle(picked_ent) == MBEDGE) oss << "1-cell: ";
    else oss << "(unknown):";
    result = MBI->tag_get_data(dualEntityTagHandle, connect, num_connect, primals);
    ids.resize(num_connect);
    result = MBI->tag_get_data(vtkMOABUtils::globalId_tag(), primals, num_connect, &ids[0]);
    for (int i = 0; i < num_connect; i++) {
      if (!first) oss << "-";
      EntityType this_type = MBI->type_from_handle(primals[i]);
      if (this_type == MBHEX) oss << "h";
      else if (this_type == MBQUAD) oss << "f";
      else oss << "u";

      if (ids[i] != 0) oss << ids[i];
      else oss << MBI->id_from_handle(primals[i]);

      first = false;
    }

    if (from_return) {
      std::cout << oss.str() << " (" << picked_ent << ")" << std::endl;
      pickLine2->setText(QString(oss.str().c_str()));
      pickLine1->setText("");
      gDrawDual->secondLastPickedEnt = gDrawDual->lastPickedEnt;
      gDrawDual->lastPickedEnt = picked_ent;

      gDrawDual->update_high_polydatas();
    }
    else if (picked_ent == *picked_ents.begin()) {
        //std::cout << oss.str() << " (" << picked_ent << ")" << std::endl;
      pickLine2->setText(pickLine1->displayText());
      pickLine1->setText(QString(oss.str().c_str()));

      gDrawDual->secondLastPickedEnt = gDrawDual->lastPickedEnt;
      gDrawDual->lastPickedEnt = picked_ent;

      gDrawDual->update_high_polydatas();
    }
  }
}

void DrawDual::update_high_polydatas() 
{
    // go through each graph window and rebuild picked entities
  std::map<EntityHandle, GraphWindows>::iterator mit;
  for (mit = surfDrawrings.begin(); mit != surfDrawrings.end(); mit++) {
      // reset or initialize
    if (NULL == mit->second.pickActor) {
      vtkPolyData *pd = vtkPolyData::New();
      vtkPolyDataMapper *mapper = vtkPolyDataMapper::New();
      mapper->SetInput(pd);
      mit->second.pickActor = vtkActor::New();
      mit->second.pickActor->SetMapper(mapper);
      pd->FastDelete();
      mapper->FastDelete();
    }
    else
      vtkPolyData::SafeDownCast(mit->second.pickActor->GetMapper()->GetInput())->Reset();
  }

    // now go through highlight entities, adding to highlight poly in each window
  ErrorCode result;
  std::vector<EntityHandle> dual_sets;
  std::vector<EntityHandle>::iterator vit;
  GVEntity **gvents;
  
  result = MBI->tag_get_data(gvEntityHandle, pickRange, &gvents); 
  if (MB_SUCCESS != result) return;
  unsigned int i, j;
  Range::iterator rit;
  vtkIdList *id_list = vtkIdList::New();
  id_list->Allocate(1);
  
  for (i = 0, rit = pickRange.begin(); rit != pickRange.end(); i++, rit++) {
      // can be up to 3 instances of this entity
    for (j = 0; j < 3; j++) {
      if (gvents[i]->myActors[j] == NULL) continue;
      
        // get the vtk cell and duplicate it
      vtkPolyData *this_pd = 
        vtkPolyData::SafeDownCast(gvents[i]->myActors[j]->GetMapper()->GetInput());
      assert(NULL != this_pd);
      assert(gvents[i]->dualSurfs[j] != 0);
      GraphWindows &gw = surfDrawrings[gvents[i]->dualSurfs[j]];
      vtkPolyData *pd = vtkPolyData::SafeDownCast(gw.pickActor->GetMapper()->GetInput());
      pd->Allocate();
      id_list->Reset();
      id_list->InsertNextId(gvents[i]->vtkEntityIds[j]);
      pd->CopyCells(this_pd, id_list);


      vtkInteractorStyle *this_style = vtkInteractorStyle::SafeDownCast(
        gw.sheetDiagram->sheet_diagram()->GetRenderWindow()->GetInteractor()->GetInteractorStyle());
      this_style->HighlightProp(gw.pickActor);
    }
  }
  id_list->FastDelete();
}

EntityHandle DrawDual::get_picked_cell(EntityHandle cell_set,
                                         const int dim,
                                         const int picked_cell) 
{
    // get the cells in the set
  Range cells;
  ErrorCode result;
  if (1 == dim)
    result = vtkMOABUtils::dualTool->get_dual_entities(cell_set, NULL, &cells, NULL, NULL, NULL);
  else if (2 == dim)
    result = vtkMOABUtils::dualTool->get_dual_entities(cell_set, &cells, NULL, NULL, NULL, NULL);
  else
    assert(false);
  
  if (MB_SUCCESS != result) return 0;
  
  Range::iterator rit = cells.begin();
  
  for (int i = 0; i < picked_cell; i++, rit++);
  
  return *rit;
}

bool DrawDual::print_dual_surfs(Range &dual_surfs,
                                const bool /*use_offsets*/) 
{
  ErrorCode success = MB_SUCCESS;
  int offset = 0;

      // create vtkWindowToImageFilter
  vtkWindowToImageFilter *wtif = vtkWindowToImageFilter::New();

      // create vtkPNGWriter
  vtkPNGWriter *pngw = vtkPNGWriter::New();
  
      // set vtkPNGWriter input to output port of vtkWindowToImageFilter
  pngw->SetInput(wtif->GetOutput());

  char filename[15];
  std::vector<int> surf_ids(dual_surfs.size());
  success = vtkMOABUtils::MBI->tag_get_data(vtkMOABUtils::globalId_tag(), 
                                            dual_surfs, &surf_ids[0]);
  if (MB_SUCCESS != success) return success;
  Range::iterator rit;
  int i;
  
  for (rit = dual_surfs.begin(), i = 0; 
       rit != dual_surfs.end(); rit++, i++) {

    GraphWindows &this_gw = surfDrawrings[*rit];

    EntityHandle dum_handle = 0;
    ErrorCode tmp_success = MBI->tag_get_data(dualSurfaceTagHandle, &(*rit), 1, 
                                           &dum_handle);
    if (MB_TAG_NOT_FOUND == tmp_success || dum_handle == 0) continue;
  
    tmp_success = draw_dual_surf(*rit, offset);
    if (MB_SUCCESS != tmp_success) success = tmp_success;
      // if (use_offsets) offset++;

      // set vtkWindowToImageFilter input to render window
    wtif->SetInput(this_gw.sheetDiagram->sheet_diagram()->GetRenderWindow());
    wtif->Update();

      // set file name
    sprintf(filename, "s%d.png", surf_ids[i]);
    pngw->SetFileName(filename);
    
      // call Write function on png writer
    this_gw.sheetDiagram->sheet_diagram()->GetRenderWindow()->Render();
    pngw->Write();
  }

  wtif->Delete();
  pngw->Delete();
  
  return (MB_SUCCESS == success ? true : false);
}

bool DrawDual::draw_dual_surfs(Range &dual_surfs,
                               const bool /*use_offsets*/) 
{
  ErrorCode success = MB_SUCCESS;
  int offset = 0;
  for (Range::reverse_iterator rit = dual_surfs.rbegin(); rit != dual_surfs.rend(); rit++) {
    EntityHandle dum_handle = 0;
    ErrorCode tmp_success = MBI->tag_get_data(dualSurfaceTagHandle, &(*rit), 1, 
                                           &dum_handle);
    if (MB_TAG_NOT_FOUND == tmp_success || dum_handle == 0) continue;
  
    tmp_success = draw_dual_surf(*rit, offset);
    if (MB_SUCCESS != tmp_success) success = tmp_success;
      // if (use_offsets) offset++;
  }
  
  return (MB_SUCCESS == success ? true : false);
}

bool DrawDual::draw_dual_surfs(std::vector<EntityHandle> &dual_surfs,
                               const bool /*use_offsets*/) 
{
  ErrorCode success = MB_SUCCESS;
  int offset = 0;
  for (std::vector<EntityHandle>::reverse_iterator vit = dual_surfs.rbegin();
       vit != dual_surfs.rend(); vit++) {
    EntityHandle dum_handle = 0;
    ErrorCode tmp_success = MBI->tag_get_data(dualSurfaceTagHandle, &(*vit), 1, 
                                           &dum_handle);
    if (MB_TAG_NOT_FOUND == tmp_success || dum_handle == 0) continue;
  
    tmp_success = draw_dual_surf(*vit, offset);
    if (MB_SUCCESS != tmp_success) success = tmp_success;
      //if (use_offsets) offset++;
  }
  
  return (MB_SUCCESS == success ? true : false);
}

ErrorCode DrawDual::draw_dual_surf(EntityHandle dual_surf,
                                     int offset_num) 
{
    // draw this dual surface

    // 3. make a new pd for this drawring
  GraphWindows &this_gw = surfDrawrings[dual_surf];
  vtkPolyData *pd;
  get_clean_pd(dual_surf, this_gw.sheetDiagram, pd);
  
  this_gw.sheetDiagram->show();

    // get the cells and vertices on this dual surface
  Range dcells, dedges, dverts, face_verts, loop_edges;
  ErrorCode result = vtkMOABUtils::dualTool->get_dual_entities(dual_surf, &dcells, &dedges, 
                                                   &dverts, &face_verts, &loop_edges);
  if (MB_SUCCESS != result) return result;

  if (dcells.empty() || dedges.empty() || dverts.empty()) return MB_FAILURE;
  
    // 1. gather/construct data for graphviz
  ErrorCode success = construct_graphviz_data(dual_surf, dcells, dedges, dverts, 
                                                face_verts, loop_edges);
  if (MB_SUCCESS != success) return success;


    // 2. tell graphviz to compute graph
//  gvBindContext((GVC_t*)gvizGvc, this_gw.gvizGraph);
//  neato_layout(this_gw.gvizGraph);
  if (my_debug) {
    std::cout << "Before layout:" << std::endl;
    if (useGraphviz) agwrite(this_gw.gvizGraph, stdout);
  }
//  neato_init_graph(this_gw.gvizGraph);
  if (useGraphviz) gvLayout(gvContext(), this_gw.gvizGraph, (char*)"neato");
  else smooth_dual_surf(dual_surf, dcells, dedges, dverts, 
                        face_verts, loop_edges);
  
  if (my_debug) {
    std::cout << "After layout, before vtk:" << std::endl;
    if (useGraphviz) agwrite(this_gw.gvizGraph, stdout);
  }

  success = fixup_degen_bchords(dual_surf);
  if (MB_SUCCESS != success) return success;
  
  vtkRenderer *this_ren = 
    this_gw.sheetDiagram->sheet_diagram()->GetRenderWindow()->GetRenderers()->GetFirstRenderer();

    // 4. create vtk points, cells for this 2d dual surface drawing
  success = make_vtk_data(dual_surf, pd, this_ren);
  if (MB_SUCCESS != success) return success;
  if (my_debug) {
    std::cout << "After layout, after vtk:" << std::endl;
    if (useGraphviz) agwrite(this_gw.gvizGraph, stdout);
  }

    // 5. generate "other sheet" labels
  vtkPolyData *new_pd;
  result = label_other_sheets(dual_surf, pd, new_pd);
  if (MB_SUCCESS != result) return result;

  pd->Update();
  
    //this_gw.qvtkWidget->GetRenderWindow()->DebugOn();
  
    // 5. render the window
  this_gw.sheetDiagram->sheet_diagram()->GetRenderWindow()->Render();

    // now that we've rendered, can get the size of various things
  int *tmp_siz = this_ren->GetSize();
  xSize = tmp_siz[0];
  ySize = tmp_siz[1];
  tmp_siz = this_ren->GetOrigin();
  xOrigin = tmp_siz[0];
  yOrigin = tmp_siz[1];
  
    // 6. draw labels for dual surface, points, dual curves
  success = draw_labels(dual_surf, pd, new_pd);
  if (MB_SUCCESS != success) return success;
  pd->FastDelete();
  new_pd->FastDelete();

    // 7. add a picker
  add_picker(this_ren);

  if (my_debug && useGraphviz) agwrite(this_gw.gvizGraph, stdout);

  int old_pos[2] = {0, 0};
  
  int *win_siz = this_gw.sheetDiagram->sheet_diagram()->GetRenderWindow()->GetSize();
  int new_pos[2] = {(int) ((double)old_pos[0] + win_siz[0]*(.1*offset_num)), old_pos[1]};
  this_gw.sheetDiagram->sheet_diagram()->GetRenderWindow()->SetPosition(new_pos);

  return success;
}

ErrorCode DrawDual::fixup_degen_bchords(EntityHandle dual_surf) 
{
    // make sure the mid-pt of degenerate blind chord segments is on the correct
    // side of the other chord it splits
  Range dcells, dedges, dverts, face_verts, loop_edges;
  ErrorCode result = vtkMOABUtils::dualTool->get_dual_entities(dual_surf, &dcells, &dedges, 
                                                   &dverts, &face_verts, &loop_edges); RR;
  
  Range tmp_edges, degen_2cells;

  double avg_pos0[3], avg_pos1[3], dum_pos0[3], dum_pos1[3], dum_pos2[3];
    
  for (Range::iterator rit = dcells.begin(); rit != dcells.end(); rit++) {
      // first, find if it's degenerate
    tmp_edges.clear();
    result = MBI->get_adjacencies(&(*rit), 1, 1, false, tmp_edges); RR;
    if (tmp_edges.size() != 2) continue;
      // also skip if it's on the boundary
    else if (loop_edges.find(*tmp_edges.begin()) != loop_edges.end() ||
             loop_edges.find(*tmp_edges.rbegin()) != loop_edges.end())
      continue;

    degen_2cells.insert(*rit);
  }
  
  MeshTopoUtil mtu(vtkMOABUtils::mbImpl);
  
  while (!degen_2cells.empty()) {
      // grab the first one off the list
    EntityHandle tcell1 = *degen_2cells.begin();

      // look for any adjacent degenerate 2cells also on the list
    const EntityHandle *connect;
    int num_connect;
    Range adj_2cells;
    result = MBI->get_connectivity(tcell1, connect, num_connect); RR;
    result = MBI->get_adjacencies(connect, num_connect, 2, false, adj_2cells); RR;
    adj_2cells = intersect( degen_2cells, adj_2cells);
    if (!adj_2cells.empty()) degen_2cells = subtract( degen_2cells, adj_2cells);
    
      // ok, have all the adjacent degen 2cells; get the 1cells
    Range adj_1cells;
    result = vtkMOABUtils::mbImpl->get_adjacencies(adj_2cells, 1, false, adj_1cells, 
                                                   Interface::UNION);
    if (MB_SUCCESS != result) return result;

      // decide whether this sheet is blind or not
    bool sheet_is_blind = vtkMOABUtils::dualTool->is_blind(dual_surf);
    
      // branch depending on what kind of arrangement we have
    if (adj_2cells.size() == 2 && !sheet_is_blind) {
      assert(3 == adj_1cells.size());
        // blind chord intersecting another chord
        // get the middle edge and chord, and the next 2 verts along the chord
      Range dum;
      result = MBI->get_adjacencies(adj_2cells, 1, false, dum); RR;
      assert(1 == dum.size());
      EntityHandle middle_edge = *dum.begin();
      EntityHandle chord = vtkMOABUtils::dualTool->get_dual_hyperplane(middle_edge);
      assert(0 != chord);
      EntityHandle verts[2];
      result = vtkMOABUtils::dualTool->get_opposite_verts(middle_edge, chord, verts); RR;

        // get the gv points for the four vertices
      void *next_points[2], *points[2];
      get_graph_points(verts, 2, dual_surf, next_points);
      assert(next_points[0] != NULL && next_points[1] != NULL);
      result = MBI->get_connectivity(middle_edge, connect, num_connect); RR;
      get_graph_points(connect, 2, dual_surf, points);
      assert(points[0] != NULL && points[1] != NULL);
      
        // now space points along the line segment joining the next_points
      get_graphpoint_pos(next_points[0], dum_pos0);
      get_graphpoint_pos(next_points[1], dum_pos1);
      
      avg_pos0[0] = .5*(dum_pos0[0] + dum_pos1[0]);
      avg_pos0[1] = .5*(dum_pos0[1] + dum_pos1[1]);
      dum_pos0[0] = 0.5 * (avg_pos0[0] + dum_pos0[0]);
      dum_pos0[1] = 0.5 * (avg_pos0[1] + dum_pos0[1]);
      dum_pos1[0] = 0.5 * (avg_pos0[0] + dum_pos1[0]);
      dum_pos1[1] = 0.5 * (avg_pos0[1] + dum_pos1[1]);
      set_graphpoint_pos(points[0], dum_pos0);
      set_graphpoint_pos(points[1], dum_pos1);

        // also fix the middle point on this edge
      points[0] = NULL;
      get_graph_points(&middle_edge, 1, dual_surf, points);
      set_graphpoint_pos(points[0], avg_pos0);

        // now fix the other 2 dedges
      adj_1cells.erase(middle_edge);
      for (Range::iterator rit = adj_1cells.begin(); rit != adj_1cells.end(); rit++) {
          // get the other 2cell
        Range dum = dcells;
        result = MBI->get_adjacencies(&(*rit), 1, 2, false, dum);
        dum = subtract( dum, adj_2cells);
        assert(1 == dum.size());
          // get the vertices and points of them, and average their positions
        const EntityHandle *connect2;
        result = MBI->get_connectivity(*dum.begin(), connect2, num_connect); RR;
        std::vector<void*> tc_points(num_connect);
        get_graph_points(connect2, num_connect, dual_surf, &tc_points[0]);
        avg_pos1[0] = avg_pos1[1] = avg_pos1[2] = 0.0;
        for (int i = 0; i < num_connect; i++) {
          if (connect2[i] != connect[0] && connect2[i] != connect[1]) {
            get_graphpoint_pos(tc_points[i], dum_pos0);
            avg_pos1[0] += dum_pos0[0];
            avg_pos1[1] += dum_pos0[1];
          }
        }
        avg_pos1[0] = (.4*avg_pos1[0]/(num_connect-2) + .6*avg_pos0[0]);
        avg_pos1[1] = (.4*avg_pos1[1]/(num_connect-2) + .6*avg_pos0[1]);
        get_graph_points(&(*rit), 1, dual_surf, &tc_points[0]);
        set_graphpoint_pos(tc_points[0], avg_pos1);
      }
    }
    else if (adj_2cells.size() == 1) {
      assert(adj_1cells.size() == 2);

        // get vertices making up degen 2cell and their avg position
      const EntityHandle *connect;
      result = MBI->get_connectivity(*adj_2cells.begin(), connect, num_connect); RR;
      std::vector<void*> tc_points(num_connect);
      get_graph_points(connect, num_connect, dual_surf, &tc_points[0]);
      get_graphpoint_pos(tc_points[0], dum_pos0);
      get_graphpoint_pos(tc_points[1], dum_pos1);
      
      avg_pos0[0] = .5*(dum_pos0[0] + dum_pos1[0]);
      avg_pos0[1] = .5*(dum_pos0[1] + dum_pos1[1]);

        // for each 1cell, get the vertices on the adjacent non-degen 2cell 
        // and points of them, and average their positions
      for (Range::iterator rit = adj_1cells.begin(); rit != adj_1cells.end(); rit++) {
          // get the other 2cell
        Range dum = dcells;
        result = MBI->get_adjacencies(&(*rit), 1, 2, false, dum);
        dum = subtract( dum, adj_2cells);
        assert(1 == dum.size());
          // get the vertices and points of them, and average their positions
        const EntityHandle *connect;
        result = MBI->get_connectivity(*dum.begin(), connect, num_connect); RR;
        std::vector<void*> tc_points(num_connect);
        get_graph_points(connect, num_connect, dual_surf, &tc_points[0]);
        avg_pos1[0] = avg_pos1[1] = avg_pos1[2] = 0.0;
        for (int i = 0; i < num_connect; i++) {
          get_graphpoint_pos(tc_points[i], dum_pos0);
          avg_pos1[0] += dum_pos0[0];
          avg_pos1[1] += dum_pos0[1];
        }
        avg_pos1[0] = (.2*avg_pos1[0]/num_connect + .8*avg_pos0[0]);
        avg_pos1[1] = (.2*avg_pos1[1]/num_connect + .8*avg_pos0[1]);
        get_graph_points(&(*rit), 1, dual_surf, &tc_points[0]);
        set_graphpoint_pos(tc_points[0], avg_pos1);
      }
    }
    else if ((adj_2cells.size() == 4 && adj_1cells.size() == 4) ||
             sheet_is_blind) {
        // pillow sheet, right after atomic pillow; just place 1cell mid-pts so
        // we can see them
        // get # chords, since drawing method depends on that
      Range chords;
      result = MBI->get_child_meshsets(dual_surf, chords);
      if (MB_SUCCESS != result) return result;
      
      if (2 == chords.size()) {
        
        const EntityHandle *connect;
        int num_connect;
        result = MBI->get_connectivity(*adj_1cells.begin(), connect, num_connect); RR;
        void *vert_pts[2], *edge_pts[4];
        get_graph_points(connect, 2, dual_surf, vert_pts);
        std::vector<EntityHandle> edges;
        std::copy(adj_1cells.begin(), adj_1cells.end(), std::back_inserter(edges));

          // check that 1cells are in proper order, i.e. they share 2cell with adj 1cell
        Range dum;
        result = MBI->get_adjacencies(&edges[0], 2, 2, false, dum); RR;
        dum = intersect( dum, adj_2cells);
        if (dum.empty()) {
            // not adjacent - switch list order
          EntityHandle dum_h = edges[1];
          edges[1] = edges[2];
          edges[2] = dum_h;
        }
      
        get_graph_points(&edges[0], 4, dual_surf, edge_pts);
        dum_pos0[0] = CENT_X; dum_pos0[1] = CENT_Y+RAD_PTS;
        dum_pos1[0] = CENT_X; dum_pos1[1] = CENT_Y-RAD_PTS;
        set_graphpoint_pos(vert_pts[0], dum_pos0);
        set_graphpoint_pos(vert_pts[1], dum_pos1);
        for (int i = 0; i < 4; i++) {
          dum_pos0[1] = CENT_Y; dum_pos0[0] = CENT_X + (2*i-3)*2*RAD_PTS/5.0;
          set_graphpoint_pos(edge_pts[i], dum_pos0);
        }
      }
      else if (3 == chords.size()) {
          // get the middle chord
        EntityHandle middle_chord = 0;
        for (Range::iterator rit = chords.begin(); rit != chords.end(); rit++) {
          int num_ents;
          result = MBI->get_number_entities_by_type(*rit, MBEDGE, num_ents);
          if (MB_SUCCESS != result) return result;
          if (2 < num_ents) {
            middle_chord = *rit;
            break;
          }
        }
        if (0 == middle_chord) return MB_FAILURE;
        
        chords.erase(middle_chord);

          // get the edges on each of the non-middle chords
        std::vector<EntityHandle> chord_edges[2];
        result = MBI->get_entities_by_handle(*chords.begin(), chord_edges[0]); RR;
        result = MBI->get_entities_by_handle(*chords.rbegin(), chord_edges[1]); RR;

          // align them such that chord_edges[0][0] and chord_edges[1][0] do not share a 2cell 
          // on this sheet; arbitrarily choose chord_edges[0][0] to be left-most
        EntityHandle shared_2cell = mtu.common_entity(chord_edges[0][0], chord_edges[1][0], 2);
        if (0 != shared_2cell && vtkMOABUtils::dualTool->get_dual_hyperplane(shared_2cell) == dual_surf) {
          shared_2cell = chord_edges[0][0];
          chord_edges[0][0] = chord_edges[0][1];
          chord_edges[0][1] = shared_2cell;
        }
        if (0 != mtu.common_entity(chord_edges[0][0], chord_edges[1][0], 2)) 
          return MB_FAILURE;

        double num_x = 2;
        double xdelta = (RAD_PTS/num_x)/3.0, xcent = CENT_X;
        double xpos = CENT_X - xdelta;
        
        for (int i = 0; i < num_x; i++) {
          
            // get the edge on the middle chord between chord_edges[i][0] and chord_edges[i][1]; that will
            // be the intersection of edges on middle chord and edges adjacent to vertices
            // bounding chord_edges[i][0]
          Range middle_edges;
          result = MBI->get_entities_by_handle(middle_chord, middle_edges); RR;
          const EntityHandle *connect;
          int num_connect;
          result = MBI->get_connectivity(*chord_edges[i].begin(), connect, num_connect); RR;
          result = MBI->get_adjacencies(connect, num_connect, 1, false, middle_edges); RR;
          if (1 != middle_edges.size()) {
              // take off any edges that aren't on this sheet? no, just punt...
            std::cout << "Warning: not a clear middle edge..." << std::endl;
          }
        
            // get the points for the two vertices and the 3 edges
            // non-middle chord; get the edges too
          void *vert_pts[2], *edge_pts[3];
          get_graph_points(connect, 2, dual_surf, vert_pts);
          get_graph_points(&chord_edges[i][0], 2, dual_surf, edge_pts);
          get_graph_points(&(*middle_edges.begin()), 1, dual_surf, &edge_pts[2]);

          dum_pos0[0] = xpos; xpos += xdelta;
          dum_pos2[0] = (xpos < xcent ? xpos-xdelta/2 : xpos+xdelta/2);
          avg_pos0[0] = xpos;
          avg_pos1[0] = xpos; xpos += xdelta;
          dum_pos1[0] = xpos; xpos += xdelta;

          avg_pos0[1] = CENT_Y - .5*RAD_PTS;
          avg_pos1[1] = CENT_Y + .5*RAD_PTS;
          dum_pos0[1] = dum_pos1[1] = dum_pos2[1] = CENT_Y;
          
          set_graphpoint_pos(vert_pts[0], avg_pos0);
          set_graphpoint_pos(vert_pts[1], avg_pos1);
          set_graphpoint_pos(edge_pts[0], dum_pos0);
          set_graphpoint_pos(edge_pts[1], dum_pos1);
          set_graphpoint_pos(edge_pts[2], dum_pos2);
          
          xpos += xdelta;
        }
      }
    }
  }

  return MB_SUCCESS;
}

ErrorCode DrawDual::get_graphpoint_pos(void *point, double *pos) 
{
  if (useGraphviz) {
    Agnode_t *this_node = (Agnode_t*) point;
    pos[0] = ND_coord_i(this_node).x;
    pos[1] = ND_coord_i(this_node).y;
    pos[2] = 0.0;
  }
  else {
    EntityHandle this_node = (EntityHandle) point;
    ErrorCode result = MBI->get_coords(&this_node, 1, pos);
    if (MB_SUCCESS != result) return result;
  }

  return MB_SUCCESS;
}

ErrorCode DrawDual::set_graphpoint_pos(void *point, double *pos) 
{
  if (useGraphviz) {
    Agnode_t *this_node = (Agnode_t*) point;
    ND_coord_i(this_node).x = pos[0];
    ND_coord_i(this_node).y = pos[1];
  }
  else {
    pos[2] = 0.0;
    EntityHandle this_node = (EntityHandle) point;
    ErrorCode result = MBI->set_coords(&this_node, 1, pos);
    if (MB_SUCCESS != result) return result;
  }

  return MB_SUCCESS;
}
 
ErrorCode DrawDual::make_vtk_data(EntityHandle dual_surf,
                                    vtkPolyData *pd,
                                    vtkRenderer *this_ren)
{
    // get the cells and vertices on this dual surface
  Range dcells, dverts, dverts_loop, dedges, dedges_loop;
  ErrorCode result = vtkMOABUtils::dualTool->get_dual_entities(dual_surf, &dcells, &dedges, 
                                                   &dverts, &dverts_loop, &dedges_loop);
  if (MB_SUCCESS != result) return result;

    // get the GVEntity for each entity
  std::map<EntityHandle, GVEntity *> vert_gv_map;
  
    // allocate cells and points; start by getting the point and cell arrays
  result = allocate_points(dual_surf, pd, dverts, dedges, vert_gv_map);
  if (MB_SUCCESS != result) return result;

    // get an int field to put the color id into
  vtkFloatArray *color_ids = vtkFloatArray::New();
  color_ids->SetName("ColorId");
  color_ids->Initialize();
  
    // make the 2d cells
  int global_id;
  pd->Allocate();
  result = vtkMOABUtils::MBI->tag_get_data(vtkMOABUtils::globalId_tag(), &dual_surf, 
                                           1, &global_id);
  if (MB_SUCCESS != result) return result;

  result = make_vtk_cells(dcells, 2, (float) global_id,
                          dual_surf, vert_gv_map, pd,
                          color_ids);
  if (MB_SUCCESS != result) return result;

    // make the color ids the active scalar
  pd->GetCellData()->AddArray(color_ids);
  pd->GetCellData()->SetScalars(color_ids);
  pd->GetCellData()->SetActiveAttribute("ColorId", 0);
  color_ids->FastDelete();

    // make the 1d cells chord by chord
  std::vector<EntityHandle> chords;
  result = MBI->get_child_meshsets(dual_surf, chords);
  if (MB_SUCCESS != result) return result;

  for (std::vector<EntityHandle>::iterator vit = chords.begin();
       vit != chords.end(); vit++) {
      // set color of chord to other sheet's color
    EntityHandle color_set = other_sheet(*vit, dual_surf);
    result = vtkMOABUtils::MBI->tag_get_data(vtkMOABUtils::globalId_tag(), &color_set,
                                             1, &global_id);
    if (MB_SUCCESS != result) return result;

      // get edges in this chord
    Range dedges;
    result = vtkMOABUtils::dualTool->get_dual_entities(*vit, NULL, &dedges, NULL, NULL, NULL);
    if (MB_SUCCESS != result) return result;
    
      // construct a new polydata, and borrow the points from the sheet's pd
    vtkPolyData *chord_pd = vtkPolyData::New();
    chord_pd->Allocate();
    chord_pd->SetPoints(pd->GetPoints());
    vtkPolyDataMapper *chord_mapper = vtkPolyDataMapper::New();
    chord_mapper->SetInput(chord_pd);
    vtkActor *chord_actor = vtkActor::New();
    vtkMOABUtils::propSetMap[chord_actor] = *vit;
    chord_actor->SetMapper(chord_mapper);
    this_ren->AddActor(chord_actor);
    double red, green, blue;
    vtkMOABUtils::get_colors(color_set, 0, global_id, red, green, blue);
    chord_actor->GetProperty()->SetColor(red, green, blue);
    chord_actor->GetProperty()->SetLineWidth(2.0);
    
      // now make the 1-cells
    result = make_vtk_cells(dedges, 1, (float) global_id,
                            dual_surf, vert_gv_map, chord_pd, NULL);
    chord_pd->FastDelete();
    chord_actor->FastDelete();
    chord_mapper->FastDelete();
    if (MB_SUCCESS != result) return result;
  }

  return MB_SUCCESS;
}

ErrorCode DrawDual::make_vtk_cells(const Range &cell_range, const int dim,
                                     const float color_index,
                                     const EntityHandle dual_surf,
                                     std::map<EntityHandle, GVEntity *> &vert_gv_map,
                                     vtkPolyData *pd,
                                     vtkFloatArray *color_ids) 
{
  std::vector<EntityHandle> cell_verts;
  std::vector<GVEntity *> gv_cells(cell_range.size());
  int cell_num;
  Range::iterator rit;
  vtkIdType cell_points[20];
  static int vtk_cell_type[] = {VTK_VERTEX, VTK_LINE, VTK_POLYGON};

  ErrorCode result = MBI->tag_get_data(gvEntityHandle, cell_range, &gv_cells[0]);
  if (MB_SUCCESS != result) return result;

  Range cell_edges, shared_edges, tmp_verts;
  for (rit = cell_range.begin(), cell_num = 0; 
       rit != cell_range.end(); rit++, cell_num++) {
      // get the vertices in this cell; must be done through vector for polygons
    cell_verts.clear();
    result = MBI->get_adjacencies(&(*rit), 1, 0, false, cell_verts); RR;
    assert(cell_verts.size() <= 20);
    cell_edges.clear();
    result = MBI->get_adjacencies(&(*rit), 1, 1, false, cell_edges); RR;
    
      // for each, check for vtk point & build one if it's missing, 
      // then create the vtk entity

      // in this loop, num_pts keeps the place in the edge/2cell's vtk point 
      // vector (cell_points), while i keeps the place in the (linear) vertex list
      // for the edge/2cell
    int num_pts = 0;
    for (unsigned int i = 0; i < cell_verts.size(); i++) {
      GVEntity *this_gv = vert_gv_map[cell_verts[i]];
      assert(this_gv != NULL);
      int index = this_gv->get_index(dual_surf);
      assert(index >= 0 && NULL != this_gv->gvizPoints[index]);
      cell_points[num_pts++] = this_gv->vtkEntityIds[index];

        // check for higher-order edge
      shared_edges.clear();
      tmp_verts.clear();
      tmp_verts.insert(cell_verts[i]);
      tmp_verts.insert(cell_verts[(i+1)%cell_verts.size()]);
      result = MBI->get_adjacencies(tmp_verts, 1, false, shared_edges); RR;
      if (shared_edges.size() > 1) {
          // filter for ones in this cell
        shared_edges = intersect( shared_edges, cell_edges);
        assert(!shared_edges.empty());
          // get the mid-pt of this edge and include in list; if we're inside a 2-edge
          // cell and we're on the 2nd vertex, take the 2nd edge
        EntityHandle this_edge = *shared_edges.begin();
        if (cell_verts.size() == 2 && i != 0) this_edge = *shared_edges.rbegin();
        this_gv = vert_gv_map[this_edge];
        assert(this_gv != NULL);
        index = this_gv->get_index(dual_surf);
        assert(index >= 0 && this_gv->gvizPoints[index] != NULL);
        cell_points[num_pts++] = this_gv->vtkEntityIds[index+2];
      }
    }

    if (dim == 2)
      cell_points[num_pts++] = cell_points[0];
    
      // create the vtk cell
    if (NULL == gv_cells[cell_num]) {
      gv_cells[cell_num] = new GVEntity();
      gv_cells[cell_num]->moabEntity = *rit;
    }
    
    int index = gv_cells[cell_num]->get_index(dual_surf);
    assert(index > -10);
    if (index < 0) {
      index = -index - 1;
      gv_cells[cell_num]->dualSurfs[index] = dual_surf;
    }
    
    int this_type = vtk_cell_type[dim];
    if (dim == 1 && num_pts > 2) this_type = VTK_POLY_LINE;
    int this_cell = pd->InsertNextCell(this_type, num_pts, 
                                       cell_points);
    gv_cells[cell_num]->vtkEntityIds[index] = this_cell;
    if (NULL != color_ids)
      color_ids->InsertValue(this_cell, color_index);
  }

  result = MBI->tag_set_data(gvEntityHandle, cell_range, &gv_cells[0]);
  if (MB_SUCCESS != result) return result;
  
  return MB_SUCCESS;
}

ErrorCode DrawDual::allocate_points(EntityHandle dual_surf,
                                      vtkPolyData *pd,
                                      Range &verts,
                                      Range &edges,
                                      std::map<EntityHandle, GVEntity*> &vert_gv_map) 
{
  vtkPoints *points = pd->GetPoints();
  if (NULL == points) {
    points = vtkPoints::New();
    pd->SetPoints(points);
    points->Delete();
  }
  pd->Allocate();
  
  std::vector<GVEntity*> gv_verts;
  gv_verts.resize(verts.size());
  
    // get the gventities
  ErrorCode result = MBI->tag_get_data(gvEntityHandle, verts, &gv_verts[0]);
  if (MB_SUCCESS != result) {
    std::cout << "Couldn't get gv vertex data." << std::endl;
    return result;
  }

  unsigned int i;
  Range::iterator rit;
  char dum_str[80];
  Agsym_t *asym_pos = (useGraphviz ? get_asym(dual_surf, 0, "pos") : NULL);
  double dum_pos[3];

    // get the transform based on old/new positions for loop vert(s)
  double x_xform = 1.0, y_xform = 1.0;
  if (useGraphviz)
    result = get_xform(dual_surf, asym_pos, x_xform, y_xform);

  for (rit = verts.begin(), i = 0; rit != verts.end(); rit++, i++) {
    assert(NULL != gv_verts[i]);
    int index = gv_verts[i]->get_index(dual_surf);
    assert(index >= 0 && NULL != gv_verts[i]->gvizPoints[index]);
    if (gv_verts[i]->vtkEntityIds[index] == -1) {
      get_graphpoint_pos(gv_verts[i]->gvizPoints[index], dum_pos);
      if (useGraphviz) {
        sprintf(dum_str, "      %d,%d", (int)dum_pos[0], (int)dum_pos[1]);
        agxset(gv_verts[i]->gvizPoints[index], asym_pos->index, dum_str);
      }
      
      gv_verts[i]->vtkEntityIds[index] = 
        points->InsertNextPoint(dum_pos[0], dum_pos[1], dum_pos[2]);
    }
    vert_gv_map[*rit] = gv_verts[i];
  }

  if (edges.empty()) return MB_SUCCESS;
  
    // check for mid-edge points; reuse gv_verts
  gv_verts.resize(edges.size());
  
    // get the gventities
  result = MBI->tag_get_data(gvEntityHandle, edges, &gv_verts[0]);
  if (MB_SUCCESS != result) {
    std::cout << "Couldn't get gv edge data." << std::endl;
    return result;
  }
  for (rit = edges.begin(), i = 0; rit != edges.end(); rit++, i++) {
    assert(NULL != gv_verts[i]);
    int index = gv_verts[i]->get_index(dual_surf);
    assert(index >= 0);
    if (NULL != gv_verts[i]->gvizPoints[index] &&
        gv_verts[i]->vtkEntityIds[index+2] == -1) {
      get_graphpoint_pos(gv_verts[i]->gvizPoints[index], dum_pos);
      if (useGraphviz) {
        sprintf(dum_str, "%d,%d", (int)(dum_pos[0]*x_xform), (int)(dum_pos[1]*y_xform));
        agxset(gv_verts[i]->gvizPoints[index], asym_pos->index, dum_str);
      }
      
      gv_verts[i]->vtkEntityIds[index+2] = 
        points->InsertNextPoint(dum_pos[0], dum_pos[1], dum_pos[2]);
      vert_gv_map[*rit] = gv_verts[i];
    }
  }
  
  return MB_SUCCESS;
}

ErrorCode DrawDual::get_xform(EntityHandle dual_surf, Agsym_t *asym_pos, 
                                double &x_xform, double &y_xform)
{
  Range face_verts, face_verts_dum;

  x_xform = y_xform = 1.0;
  return MB_SUCCESS;

  ErrorCode result = vtkMOABUtils::dualTool->get_dual_entities(dual_surf, NULL, NULL, 
                                                   &face_verts_dum, &face_verts, NULL);
  if (MB_SUCCESS != result) return result;
  
    // get the gventities
  std::vector<GVEntity*> gv_verts;
  gv_verts.resize(face_verts.size());
  result = MBI->tag_get_data(gvEntityHandle, face_verts, &gv_verts[0]);
  if (MB_SUCCESS != result) return result;

    // find a vertex with non-zero x, y coordinates
  for (std::vector<GVEntity*>::iterator vit = gv_verts.begin(); vit != gv_verts.end(); vit++) {
    assert(NULL != *vit);
    int index = (*vit)->get_index(dual_surf);
    assert(index >= 0 && NULL != (*vit)->gvizPoints[index]);
      // get new point position
    double dum_pos[3];
    get_graphpoint_pos((*vit)->gvizPoints[index], dum_pos);
    if (dum_pos[0] != 0 && dum_pos[1] != 0) {
        // get old point position, which is set on attribute
      char *agx = agxget((*vit)->gvizPoints[index], asym_pos->index);
      int x = -1, y = -1;
      sscanf(agx, "%d,%d", &x, &y);
      if (0 == x || 0 == y) continue;

        // ok, non-zeros in all quantities, can compute xform now as old over new
      x_xform = ((double)x) / dum_pos[0];
      y_xform = ((double)y) / dum_pos[1];
      
      return MB_SUCCESS;
    }
  }

    // didn't get any usable data; set to unity
  x_xform = 1.0;
  y_xform = 1.0;

  if (my_debug) std::cout << "Didn't find transform." << std::endl;
  
  return MB_FAILURE;
}

vtkPolyData *DrawDual::get_polydata(SheetDiagramPopup *this_sdpopup) 
{
  vtkRendererCollection *rcoll = this_sdpopup->sheet_diagram()->GetRenderWindow()->GetRenderers();
  assert(rcoll != NULL);
  rcoll->InitTraversal();
  vtkActorCollection *acoll = rcoll->GetNextItem()->GetActors();
  assert(NULL != acoll);
  acoll->InitTraversal();
  return vtkPolyData::SafeDownCast(acoll->GetNextActor()->GetMapper()->GetInput());
}

void DrawDual::get_clean_pd(EntityHandle dual_surf,
                            SheetDiagramPopup *&this_sdpopup,
                            vtkPolyData *&pd)
{
  if (NULL != this_sdpopup) {
    ErrorCode result = reset_drawing_data(dual_surf);
    if (MB_SUCCESS != result) {
      std::cerr << "Trouble resetting drawing data for sheet." << std::endl;
    }
  }
  
  if (NULL == this_sdpopup) {
    vtkRenderer *this_ren = vtkRenderer::New();
    pd = vtkPolyData::New();
    
    const bool twod = false;
    
    if (twod) {
      
        // the easy route - just make new stuff
      vtkPolyDataMapper2D *this_mapper = vtkPolyDataMapper2D::New();
//    vtkPolyDataMapper *this_mapper = vtkPolyDataMapper::New();

        // need to set a coordinate system for this window, so that display coordinates
        // are re-normalized to window size
      vtkCoordinate *this_coord = vtkCoordinate::New();
      this_coord->SetCoordinateSystemToWorld();
      this_mapper->SetTransformCoordinate(this_coord);
      this_coord->FastDelete();

      this_mapper->ScalarVisibilityOn();
      this_mapper->SetLookupTable(vtkMOABUtils::lookupTable);
      this_mapper->UseLookupTableScalarRangeOn();
      this_mapper->SetScalarModeToUseCellData();
    
        // put an edge extractor before the mapper
      vtkExtractEdges *ee = vtkExtractEdges::New();
      ee->SetInput(pd);
      this_mapper->SetInput(ee->GetOutput());
      this_mapper->SetInput(pd);
      ee->FastDelete();

      vtkActor2D *this_actor = vtkActor2D::New();
      this_actor->PickableOn();
      vtkMOABUtils::propSetMap[this_actor] = dual_surf;
//    vtkActor *this_actor = vtkActor::New();
      this_actor->SetMapper(this_mapper);
      this_actor->GetProperty()->SetLineWidth(2.0);
      this_ren->AddActor(this_actor);
      this_actor->FastDelete();
      this_mapper->FastDelete();
    }
    else {
        // the easy route - just make new stuff
      vtkPolyDataMapper *this_mapper = vtkPolyDataMapper::New();

        // need to set a coordinate system for this window, so that display coordinates
        // are re-normalized to window size
//      vtkCoordinate *this_coord = vtkCoordinate::New();
//      this_coord->SetCoordinateSystemToWorld();
//      this_mapper->SetTransformCoordinate(this_coord);
      this_mapper->ScalarVisibilityOn();
        //this_mapper->SetLookupTable(vtkMOABUtils::lookupTable);
      this_mapper->UseLookupTableScalarRangeOn();
      this_mapper->SetScalarModeToUseCellData();
    
        // put an edge extractor before the mapper
//    vtkExtractEdges *ee = vtkExtractEdges::New();
//    ee->SetInput(pd);
//    this_mapper->SetInput(ee->GetOutput());
      this_mapper->SetInput(pd);

      vtkActor *this_actor = vtkActor::New();
      this_actor->PickableOn();
      vtkMOABUtils::propSetMap[this_actor] = dual_surf;
      this_actor->SetMapper(this_mapper);
      this_mapper->FastDelete();
      this_actor->GetProperty()->SetLineWidth(2.0);

      double red, green, blue;
      int dum;
      vtkMOABUtils::get_colors(dual_surf, vtkMOABUtils::totalColors, dum,
                               red, green, blue);
      vtkProperty *this_property = this_actor->GetProperty();
      this_property->SetColor(red, green, blue);
    
      this_ren->AddActor(this_actor);
      this_actor->FastDelete();
    }

    vtkCamera *camera = vtkCamera::New();
    camera->SetPosition(CENT_X, CENT_Y, 3);
    camera->SetFocalPoint(CENT_X, CENT_Y, 0);
    camera->SetViewUp(0,1,0);

    this_sdpopup = new SheetDiagramPopup();
    if (my_debug) {
        //this_sdpopup->sheet_diagram()->GetRenderWindow()->DebugOn();
    }

    this_sdpopup->sheet_diagram()->GetRenderWindow()->AddRenderer(this_ren);
    this_sdpopup->sheet_diagram()->GetRenderWindow()->SetSize(SHEET_WINDOW_SIZE, SHEET_WINDOW_SIZE);

    this_sdpopup->show();
  
    this_ren->ResetCamera();
    this_ren->FastDelete();

    camera->Delete();
    
    return;
  }
  
    // trace back until we find the dataset
  pd = get_polydata(this_sdpopup);
  assert(NULL != pd);
    // re-initialize the data, then we're done
  pd->Initialize();
}

ErrorCode DrawDual::construct_graphviz_data(EntityHandle dual_surf,
                                              Range &dcells, Range &dedges,
                                              Range &dverts, Range &face_verts,
                                              Range &loop_edges) 
{
    // gather/construct the data for graphviz

    // get which drawring we're referring to
  char dum_str[80];
  GraphWindows &this_gw = surfDrawrings[dual_surf];
  if (useGraphviz && NULL == this_gw.gvizGraph) {
      // allocate a new graph and create a new window
    sprintf(dum_str, "%d", (int)MBI->id_from_handle(dual_surf));
    this_gw.gvizGraph = agopen(dum_str, AGRAPH);
    if (this_gw.gvizGraph == NULL) return MB_FAILURE;
  }
    
    // for each vertex, allocate a graphviz point if it doesn't already have one
  Agsym_t *asym_pos = (useGraphviz ? get_asym(dual_surf, 0, "pos") : NULL);

  ErrorCode result = construct_graphviz_points(dual_surf, dverts, asym_pos); RR;
  
    // for each edge, allocate a graphviz edge if it doesn't already have one
  result = construct_graphviz_edges(dual_surf, dedges, face_verts, asym_pos); RR;
  
  return result;
}

ErrorCode DrawDual::construct_graphviz_edges(EntityHandle dual_surf, 
                                               Range &dedges, 
                                               Range &loop_verts, 
                                               Agsym_t *asym_pos) 
{
  const EntityHandle *connect;
  int num_connect;
  ErrorCode result = MB_SUCCESS;
  char dum_str[80];
  GraphWindows &this_gw = surfDrawrings[dual_surf];
    //Agsym_t *asym_weight = (useGraphviz ? get_asym(dual_surf, 1, "weight") : NULL), 
    //*asym_len = (useGraphviz ? get_asym(dual_surf, 1, "len") : NULL);
  std::vector<GVEntity*> dedge_gv(dedges.size());
  result = MBI->tag_get_data(gvEntityHandle, dedges, &dedge_gv[0]); RR;
  GVEntity *dvert_gv[2];
 
  Range::iterator rit;
  int i;
  for (rit = dedges.begin(), i = 0; rit != dedges.end(); rit++, i++) {

      // get the DEdgeGVEdge for this dual edge
    GVEntity *this_gv = dedge_gv[i];
    if (NULL == this_gv) {
      this_gv = new GVEntity();
      this_gv->moabEntity = *rit;
      result = MBI->tag_set_data(gvEntityHandle, &(*rit), 1, &this_gv);
      if (MB_SUCCESS != result) return result;
      dedge_gv[i] = this_gv;
    }

      // get the index for this dual surface
    int dsindex = this_gv->get_index(dual_surf);
    assert(dsindex > -10);
    if (dsindex < 0) {
        // need to make a graphviz entity; get the graphviz nodes, then make the edge
        // next sheet index is negative of value returned from get_index
      dsindex = -dsindex - 1;
      this_gv->dualSurfs[dsindex] = dual_surf;

      result = MBI->get_connectivity(*rit, connect, num_connect);
      if (MB_SUCCESS != result) return result;
      result = MBI->tag_get_data(gvEntityHandle, connect, 2, dvert_gv);
      if (MB_SUCCESS != result) return result;
      assert(NULL != dvert_gv[0] && NULL != dvert_gv[1]);
      int index0 = dvert_gv[0]->get_index(dual_surf);
      int index1 = dvert_gv[1]->get_index(dual_surf);
      assert(index0 >= 0 && index1 >= 0);
      sprintf(dum_str, "%d", (int)*rit);

        // first, check to see if it's degenerate; if so, add a mid-pt
      Range tmp_edges;
      result = MBI->get_adjacencies(connect, 2, 1, false, tmp_edges);
      if (MB_SUCCESS == result && tmp_edges.size() > 1) {
        if (useGraphviz) {
            // add a graphviz pt for the edge
          Agnode_t *mid_gvpt = agnode(this_gw.gvizGraph, dum_str);
          sprintf(dum_str, "%up", (int)MBI->id_from_handle(*rit));
          this_gv->gvizPoints[dsindex] = mid_gvpt;
      
          Agedge_t *this_gved = agedge(this_gw.gvizGraph,
                                       (Agnode_t*)dvert_gv[0]->gvizPoints[index0],
                                       (Agnode_t*)mid_gvpt);
          this_gv->gvizEdges[dsindex] = this_gved;
        
          this_gved = agedge(this_gw.gvizGraph,
                             mid_gvpt,
                             (Agnode_t*)dvert_gv[1]->gvizPoints[index1]);
          this_gv->gvizEdges[dsindex+2] = this_gved;
        }
        else {
            // add a vertex for the edge
          sprintf(dum_str, "%up", (unsigned int) MBI->id_from_handle(*rit));
          EntityHandle mid_vert, edge1, edge2, edge_verts[2];
          double dum_pos[] = {CENT_X, CENT_Y, 0.0};
          result = MBI->create_vertex(dum_pos, mid_vert);
          if (MB_SUCCESS != result) return result;
          this_gv->gvizPoints[dsindex] = (void*)mid_vert;
      
          edge_verts[0] = (EntityHandle) dvert_gv[0]->gvizPoints[index0];
          edge_verts[1] = (EntityHandle) mid_vert;
          result = MBI->create_element(MBEDGE, edge_verts, 2, edge1);
          if (MB_SUCCESS != result) return result;
          this_gv->gvizEdges[dsindex] = (void*)edge1;
          result = MBI->tag_set_data(dualCurveTagHandle, &edge1, 1, &(*rit));
          if (MB_SUCCESS != result) return result;
        
          edge_verts[0] = (EntityHandle) mid_vert;
          edge_verts[1] = (EntityHandle) dvert_gv[1]->gvizPoints[index1];
          result = MBI->create_element(MBEDGE, edge_verts, 2, edge2);
          if (MB_SUCCESS != result) return result;
          this_gv->gvizEdges[dsindex+2] = (void*)edge2;
          result = MBI->tag_set_data(dualCurveTagHandle, &edge2, 1, &(*rit));
          if (MB_SUCCESS != result) return result;
        }
      }
      else {
        if (useGraphviz) {
          Agedge_t *this_gved = agedge(this_gw.gvizGraph,
                                       (Agnode_t*)dvert_gv[0]->gvizPoints[index0],
                                       (Agnode_t*)dvert_gv[1]->gvizPoints[index1]);
          this_gv->gvizEdges[dsindex] = this_gved;
        }
        else {
          EntityHandle edge_verts[2], edge1;
          edge_verts[0] = (EntityHandle) dvert_gv[0]->gvizPoints[index0];
          edge_verts[1] = (EntityHandle) dvert_gv[1]->gvizPoints[index1];
          result = MBI->create_element(MBEDGE, edge_verts, 2, edge1);
          if (MB_SUCCESS != result) return result;
          this_gv->gvizEdges[dsindex] = (void*) edge1;
          result = MBI->tag_set_data(dualCurveTagHandle, &edge1, 1, &(*rit));
          if (MB_SUCCESS != result) return result;
        }
      }

        // check to see if it's an interior edge connected to the loop, and
        // adjust its weight and length if so
      Range::iterator firstv = loop_verts.find(connect[0]),
        lastv = loop_verts.find(connect[1]),
        endloop = loop_verts.end();
      if ((firstv == endloop && lastv != endloop) ||
          (firstv != endloop && lastv == endloop)) {
          //  agxset(this_gved, asym_weight->index, "10");
          //  agxset(this_gved, asym_len->index, "0.1");
      }
    }

      // note to self: check for self-intersections here, to handle 2 instances
      // of edge in drawring
  }

  return result;
}

ErrorCode DrawDual::construct_graphviz_points(EntityHandle dual_surf, 
                                                Range &dverts, 
                                                Agsym_t *asym_pos) 
{
  Range::iterator rit;
  int i;
  ErrorCode result = MB_SUCCESS;
  GraphWindows &this_gw = surfDrawrings[dual_surf];
  char dum_str[80];
  std::vector<GVEntity*> dvert_gv(dverts.size());
  result = MBI->tag_get_data(gvEntityHandle, dverts, &dvert_gv[0]);
  
  for (rit = dverts.begin(), i = 0; rit != dverts.end(); rit++, i++) {

      // get the DVertexPoint for this dual vertex
    GVEntity *this_gv = dvert_gv[i];
    if (NULL == this_gv) {
      this_gv = new GVEntity();
      this_gv->moabEntity = *rit;
      result = MBI->tag_set_data(gvEntityHandle, &(*rit), 1, &this_gv);
      if (MB_SUCCESS != result) return result;
      dvert_gv[i] = this_gv;
    }

      // get the index for this dual surface
    int dsindex = this_gv->get_index(dual_surf);
    assert(dsindex > -10);
    if (dsindex < 0) {
      dsindex = -dsindex - 1;
      if (useGraphviz) {
          // need to make a graphviz point
        sprintf(dum_str, "%u", (unsigned int)*rit);
        Agnode_t *this_gvpt = agnode(this_gw.gvizGraph, dum_str);
        if (NULL == this_gvpt) return MB_FAILURE;
        this_gv->gvizPoints[dsindex] = this_gvpt;
      }
      else {
          // use an MBVertex instead
        EntityHandle new_vertex;
        double dum_pos[] = {CENT_X, CENT_Y, 0.0};
        result = MBI->create_vertex(dum_pos, new_vertex);
        if (MB_SUCCESS != result) return result;
        this_gv->gvizPoints[dsindex] = (void*) new_vertex;
      }
      
      this_gv->dualSurfs[dsindex] = dual_surf;
    }
  }

  return result;
}

Agsym_t *DrawDual::get_asym(EntityHandle dual_surf, const int dim,
                            const char *name, const char *def_val) 
{
  Agsym_t *asym = NULL;

  if (0 == dim) {
    asym = agfindattr(surfDrawrings[dual_surf].gvizGraph->proto->n, 
                      const_cast<char*>(name));
    if (NULL == asym) {
      if (NULL == def_val) asym = agnodeattr(surfDrawrings[dual_surf].gvizGraph, const_cast<char*>(name), (char*)"");
      
      else asym = agnodeattr(surfDrawrings[dual_surf].gvizGraph, const_cast<char*>(name), 
                             const_cast<char*>(def_val));
    }
  }
  else {
    asym = agfindattr(surfDrawrings[dual_surf].gvizGraph->proto->e, 
                      const_cast<char*>(name));
    if (NULL == asym) {
      if (NULL == def_val) asym = agedgeattr(surfDrawrings[dual_surf].gvizGraph, const_cast<char*>(name), (char*)"");
      
      else asym = agedgeattr(surfDrawrings[dual_surf].gvizGraph, const_cast<char*>(name), 
                             const_cast<char*>(def_val));
    }
  }
    
  assert(NULL != asym);
  return asym;
}

ErrorCode DrawDual::compute_fixed_points(EntityHandle dual_surf, Range &dverts,
                                           Range &face_verts, Range &loop_edges) 
{
  std::vector<std::vector<EntityHandle> > loops;
  Range new_face_verts(face_verts);

  while (!new_face_verts.empty()) {
      // get the next first vertex on the loop
    EntityHandle this_v = *new_face_verts.begin();
    EntityHandle first_v = 0, last_v = 0;
    std::vector<EntityHandle> loop_vs;
    Range temp_face_verts;

    if (new_face_verts.size() == 2) {
        // quick way to do this, assuming both vertices are on the loop
      loop_vs.push_back(*new_face_verts.begin());
      loop_vs.push_back(*new_face_verts.rbegin());
      temp_face_verts.insert(*new_face_verts.begin());
      temp_face_verts.insert(*new_face_verts.rbegin());
      this_v = first_v;
    }
    
    while (this_v != first_v) {
      if (0 == first_v) first_v = this_v;
      
        // put this vertex on the loop, then get the next one
      loop_vs.push_back(this_v);
      temp_face_verts.insert(this_v);
      EntityHandle temp_v = this_v;
      this_v = vtkMOABUtils::dualTool->next_loop_vertex(last_v, this_v, dual_surf);
      assert(0 != this_v);
      last_v = temp_v;
    }

      // save this vector in the map
    loops.push_back(loop_vs);
    
      // ok, we've got them all; first, remove them from face_verts
    Range temp_range = subtract( new_face_verts, temp_face_verts);
    new_face_verts.swap(temp_range);
  }
  
    // now compute vertex coordinates for each loop
  Agsym_t *asym_pos, *asym_pin;
  if (useGraphviz) {
    asym_pos = get_asym(dual_surf, 0, "pos");
    asym_pin = get_asym(dual_surf, 0, "pin", "false");
  }

  if (loops.empty()) 
    return compute_pillow_fixed_points(dual_surf, face_verts, loop_edges);
  
  char tmp_pos[80];
  int loop_num, num_loops = loops.size();
  std::vector<std::vector<EntityHandle> >::iterator mit;
  if (my_debug) std::cout << "Loop points: " << std::endl;
  for (mit = loops.begin(), loop_num = 0; mit != loops.end(); mit++, loop_num++) {

      // if we have more than two loops, let graphviz compute the best position
    if (num_loops > 2 && loop_num > 0) break;
    
      // now, go around the loop, assigning vertex positions; assume a 6-inch diameter circle
      // (at 72 pts/inch)
    unsigned int loop_size = (*mit).size();
    double angle = (2.0*acos(-1.0))/((double)loop_size);
    int xpos_pts, ypos_pts;
    
    for (unsigned int i = 0; i < loop_size; i++) {
        // get the position
      get_loop_vertex_pos(i, loop_num, num_loops, angle, xpos_pts, ypos_pts);

        // now set that position on the node
      GVEntity *this_gv;
      ErrorCode result = MBI->tag_get_data(gvEntityHandle, &((*mit)[i]), 1, &this_gv); RR;
      
      int index = this_gv->get_index(dual_surf);
      assert(index >= 0);

      if (useGraphviz) {
        Agnode_t *this_gpt = (Agnode_t*)this_gv->gvizPoints[index];
      
          // set position and pin attributes for the node
        sprintf(tmp_pos, "%d,%d!", xpos_pts, ypos_pts);
        agxset(this_gpt, asym_pos->index, tmp_pos);
        agxset(this_gpt, asym_pin->index, (char*)"true");

          // also try setting them in the data structure directly
        ND_coord_i(this_gpt).x = xpos_pts;
        ND_coord_i(this_gpt).y = ypos_pts;
        ND_pinned(this_gpt) = true;
      }
      else {
        EntityHandle this_vert = (EntityHandle) this_gv->gvizPoints[index];
        double dum_pos[] = {xpos_pts, ypos_pts, 0.0};
        result = MBI->set_coords(&this_vert, 1, dum_pos);
        if (MB_SUCCESS != result) return result;
      }
        
      if (my_debug) std::cout << "Point " << MBI->id_from_handle((*mit)[i])
                              << ": x = " << xpos_pts << ", y = " << ypos_pts << std::endl;
    }

    if (loop_size == 2) {
        // 2-vertex loop: place mid-edge nodes too
      assert(loop_edges.size() == 2);

      int offset = 0;
        // put one at pi/2, the other at 3pi/2
      for (Range::iterator rit = loop_edges.begin(); rit != loop_edges.end(); rit++) {
        GVEntity *this_gv;
        ErrorCode result = MBI->tag_get_data(gvEntityHandle, &(*rit), 1, &this_gv);
        if (MB_SUCCESS != result) continue;
        int index = this_gv->get_index(dual_surf);
        assert(index >= 0 && this_gv->gvizPoints[index] != NULL);
        get_loop_vertex_pos(1+2*offset, loop_num, num_loops, .5*angle, xpos_pts, ypos_pts);

        if (useGraphviz) {
          Agnode_t *this_gpt = (Agnode_t*)this_gv->gvizPoints[index];
          sprintf(tmp_pos, "%d,%d!", xpos_pts, ypos_pts);
          agxset(this_gpt, asym_pos->index, tmp_pos);
          agxset(this_gpt, asym_pin->index, (char*)"true");

            // also try setting them in the data structure directly
          ND_coord_i(this_gpt).x = xpos_pts;
          ND_coord_i(this_gpt).y = ypos_pts;
          ND_pinned(this_gpt) = true;
        }
        else {
          EntityHandle this_vert = (EntityHandle) this_gv->gvizPoints[index];
          double dum_pos[] = {xpos_pts, ypos_pts, 0.0};
          result = MBI->set_coords(&this_vert, 1, dum_pos);
          if (MB_SUCCESS != result) return result;
        }
        
        if (my_debug) std::cout << "Edge point for edge " << MBI->id_from_handle(*rit)
                                << ": x = " << xpos_pts << ", y = " << ypos_pts << std::endl;
        offset += 1;
      }
    }
  }

  return MB_SUCCESS;
}

ErrorCode DrawDual::compute_pillow_fixed_points(EntityHandle dual_surf, 
                                                  Range &face_verts, 
                                                  Range &face_edges) 
{
    // find the points we'll call fixed for this sheet
    // first, get the chords, in an ordered list
  std::vector<EntityHandle> chords;
  ErrorCode result = MBI->get_child_meshsets(dual_surf, chords);
  if (MB_SUCCESS != result) return result;

    // if there are only two, don't bother, they'll get fixed up later
  if (chords.size() <= 2) return MB_SUCCESS;
  
    // if more, get the first two vertices on the first chord, then a 3rd
    // from another that's not in the 1st set
  Range tmp_range;
  result = MBI->get_entities_by_dimension(chords[0], 1, tmp_range); RR;
  const EntityHandle *connect;
  int num_connect;
  result = MBI->get_connectivity(*tmp_range.begin(), connect, num_connect); RR;
  face_verts.insert(connect[0]);
  face_verts.insert(connect[1]);
  tmp_range.clear();
  Range tmp_verts;
  result = MBI->get_entities_by_dimension(chords[1], 1, tmp_range); RR;
  result = MBI->get_adjacencies(tmp_range, 0, false, tmp_verts, Interface::UNION); RR;
  tmp_verts = subtract( tmp_verts, face_verts);
  assert(!tmp_verts.empty());
  face_verts.insert(*tmp_verts.begin());
  
    // should have 3; put at equal angles round the circle
//  double PI = acos(-1.0);
  double dum_pos[9];
  dum_pos[0] = CENT_X;
  dum_pos[1] = RAD_PTS;
  dum_pos[3] = CENT_X - RAD_PTS*cos(PI/6.0);
  dum_pos[4] = CENT_Y - RAD_PTS*sin(PI/6.0);
  dum_pos[6] = CENT_X + RAD_PTS*cos(PI/6.0);
  dum_pos[7] = dum_pos[4];
  dum_pos[2] = dum_pos[5] = dum_pos[8] = 0.0;

  std::vector<EntityHandle> graph_points(face_verts.size());
  get_graph_points(face_verts, dual_surf, (void**) &graph_points[0]);
  result = MBI->set_coords(&graph_points[0], face_verts.size(), dum_pos); RR;
  
  return result;
}

void DrawDual::get_loop_vertex_pos(unsigned int vert_num, 
                                   unsigned int loop_num, 
                                   unsigned int num_loops, 
                                   double angle, int &xpos_pts, int &ypos_pts) 
{
  double this_angle = vert_num * angle;
  if (num_loops > 2 && loop_num > 0) {
    xpos_pts = 0;
    ypos_pts = 0;
    return;
  }

  xpos_pts = (int) (((double)RAD_PTS) * cos(this_angle));
  ypos_pts = (int) (((double)RAD_PTS) * sin(this_angle));

  if (loop_num > 0) {
    xpos_pts /= 6;
    ypos_pts /= 6;
  }

  xpos_pts += CENT_X;
  ypos_pts += CENT_Y;
}

ErrorCode DrawDual::draw_labels(EntityHandle dual_surf, vtkPolyData *pd,
                                  vtkPolyData *new_pd) 
{
    // get the renderer you'll be writing the text to
  GraphWindows &this_gw = surfDrawrings[dual_surf];
  vtkRenderer *ren = 
    this_gw.sheetDiagram->sheet_diagram()->GetRenderWindow()->GetRenderers()->GetFirstRenderer();

    // sheet id first
  char set_name[CATEGORY_TAG_SIZE];
  int dum;
  ErrorCode result = vtkMOABUtils::MBI->tag_get_data(vtkMOABUtils::globalId_tag(),
                                                       &dual_surf, 1, &dum);
  if (MB_SUCCESS != result) return result;
  sprintf(set_name, "%d\n", dum);

    // create a text actor
  vtkTextActor *text_actor = vtkTextActor::New();
  vtkMOABUtils::propSetMap[text_actor] = dual_surf;
  text_actor->SetInput(set_name);

    // compute proper position for string
  double LABEL_FRACTION = .90;
  vtkCoordinate *this_pos = text_actor->GetPositionCoordinate();
  this_pos->SetCoordinateSystemToWorld();
  this_pos->SetValue(LABEL_FRACTION*RAD_PTS+CENT_X, LABEL_FRACTION*RAD_PTS+CENT_Y, 0);
  text_actor->GetTextProperty()->SetColor(1.0, 1.0, 1.0);
  text_actor->GetTextProperty()->BoldOn();
  ren->AddActor(text_actor);
  text_actor->FastDelete();

    // now vertex ids

    // first, get new polydata's with interior and loop points
  label_interior_verts(dual_surf, pd, ren);

    // create other sheet labels and add to renderer
  vtkLabeledDataMapper *os_labels = vtkLabeledDataMapper::New();
  os_labels->SetInput(new_pd);
  os_labels->SetLabelModeToLabelFieldData();
  os_labels->SetLabelFormat("___/%g");
  vtkActor2D *lda2 = vtkActor2D::New();
  lda2->SetMapper(os_labels);
  ren->AddActor(lda2);

    // set label actor to be non-pickable
  lda2->PickableOff();

  os_labels->FastDelete();
  lda2->FastDelete();

  return MB_SUCCESS;
}

ErrorCode DrawDual::label_other_sheets(EntityHandle dual_surf,
                                         vtkPolyData *pd,
                                         vtkPolyData *&new_pd) 
{
    // gather the chord sets
  std::vector<EntityHandle> chords, chord_surfs;
  Range dedges, dverts, dverts_loop;
  ErrorCode result = MBI->get_child_meshsets(dual_surf, chords);
  if (MB_SUCCESS != result || 0 == chords.size()) return result;

    // start a new polydata with points for labeling
  new_pd = vtkPolyData::New();
  vtkPoints *new_points = new_pd->GetPoints();
  if (NULL == new_points) {
    new_points = vtkPoints::New();
    new_pd->SetPoints(new_points);
    new_points->Delete();
  }
  new_pd->Allocate();
  vtkIntArray *id_array = vtkIntArray::New();
  id_array->SetName("LoopVertexIds");

    // for each chord:
  ErrorCode tmp_result;
  std::vector<EntityHandle>::iterator vit1;
  Range::iterator rit;
  std::vector<GVEntity*> gv_edges;
  for (vit1 = chords.begin(); vit1 != chords.end(); vit1++) {

      // get a color for this chord; make it the color of the "other" sheet, unless
      // it's this sheet, in which case it's black
    EntityHandle color_set = other_sheet(*vit1, dual_surf);
    double red, green, blue;
    int global_id;
    if (0 == color_set) red = green = blue = 0.0;
    else
      vtkMOABUtils::get_colors(color_set, vtkMOABUtils::totalColors, global_id,
                               red, green, blue);

      // create a series of edges in the original pd
    dedges.clear(); dverts.clear(); dverts_loop.clear();
    tmp_result = vtkMOABUtils::dualTool->get_dual_entities(*vit1, NULL, &dedges, &dverts, &dverts_loop, 
                                             NULL);
    if (MB_SUCCESS != tmp_result) {
      result = tmp_result;
      continue;
    }
    
    gv_edges.reserve(dedges.size());
    tmp_result = MBI->tag_get_data(gvEntityHandle, dedges, &gv_edges[0]);
    if (MB_SUCCESS != tmp_result) {
      result = tmp_result;
      continue;
    }
    
    int edge_vtk_vs[2];
    GVEntity *gv_verts[2];
    const EntityHandle *edge_vs;
    int num_edge_vs;
    int edge_num;
    int index;
    for (rit = dedges.begin(), edge_num = 0; rit != dedges.end(); rit++, edge_num++) {
      tmp_result = MBI->get_connectivity(*rit, edge_vs, num_edge_vs);
      if (MB_SUCCESS != tmp_result) {
        result = tmp_result;
        continue;
      }
        // get the gventities
      tmp_result = MBI->tag_get_data(gvEntityHandle, edge_vs, 2, gv_verts);
      if (MB_SUCCESS != tmp_result) {
        result = tmp_result;
        continue;
      }
      for (int i = 0; i < 2; i++) {
        index = gv_verts[i]->get_index(dual_surf);
        assert(index >= 0 && NULL != gv_verts[i]->gvizPoints[index]);
        edge_vtk_vs[i] = gv_verts[i]->vtkEntityIds[index];

          // look for loop points, and add label if there are any
        if (dverts_loop.find(edge_vs[i]) != dverts_loop.end()) {
          double dum_pos[3];
          get_graphpoint_pos(gv_verts[i]->gvizPoints[index], dum_pos);
          new_points->InsertNextPoint(dum_pos[0], dum_pos[1], dum_pos[2]);
          id_array->InsertNextValue(global_id);
        }
      }
    }
  }

    // assign id data to the pd for drawing other sheet labels
  new_pd->GetPointData()->AddArray(id_array);
  new_pd->GetPointData()->SetScalars(id_array);
  new_pd->GetPointData()->SetActiveAttribute("LoopVertexIds", 0);
  id_array->FastDelete();

  return result;
}

void DrawDual::label_interior_verts(EntityHandle dual_surf,
                                    vtkPolyData *pd,
                                    vtkRenderer *ren) 
{
    // this function originally designed to filter out interior vertices,
    // extract them and add data just to them; however, there isn't a filter
    // in vtk to extract just vertices, so we label all the vertices here
    // 
    // get the cells and vertices on this dual surface
  Range dcells, dverts, face_verts;

  ErrorCode result = vtkMOABUtils::dualTool->get_dual_entities(dual_surf, &dcells, NULL, 
                                                   &dverts, &face_verts, NULL);
  if (MB_SUCCESS != result) return;
  
    // get the GVentity for vertices, so we can find the vtk point on the sheet drawing
  std::vector<GVEntity*> gv_ents;
  gv_ents.reserve(dverts.size());
  result = MBI->tag_get_data(gvEntityHandle, dverts, &gv_ents[0]);
  if (MB_SUCCESS != result) {
    std::cout << "Failed to get GV entities." << std::endl;
    return;
  }

    // get the ids of the primal entities of these vertices
  std::vector<int> vert_ids;
  result = get_primal_ids(dverts, vert_ids);
  if (MB_SUCCESS != result) {
    std::cout << "Failed to get primal ids." << std::endl;
    return;
  }

    // now put those ids in the id list, and the vtk point ids
    // in an extract cells list
  vtkPolyData *label_pd = vtkPolyData::New();
  vtkPoints *new_points = label_pd->GetPoints();
  if (NULL == new_points) {
    new_points = vtkPoints::New();
    label_pd->SetPoints(new_points);
    new_points->Delete();
  }
  label_pd->Allocate();
  vtkIntArray *int_ids = vtkIntArray::New();
  int_ids->SetName("VertexIds");
  int_ids->SetNumberOfValues(dverts.size());

  for (unsigned int i = 0; i != dverts.size(); i++) {
    int index = gv_ents[i]->get_index(dual_surf);
    assert(index >= 0);
    
      // insert the primal entity id into a list for labeling
    double dum_pos[3];
    get_graphpoint_pos(gv_ents[i]->gvizPoints[index], dum_pos);
    new_points->InsertNextPoint(dum_pos[0], dum_pos[1], dum_pos[2]);
    int_ids->InsertValue(i, vert_ids[i]);
  }

    // make a new pd and copy the points from the last one
  label_pd->GetPointData()->AddArray(int_ids);
  label_pd->GetPointData()->SetActiveAttribute("VertexIds", 0);

  vtkLabeledDataMapper *ldm = vtkLabeledDataMapper::New();

  ldm->SetInput(label_pd);
  ldm->SetLabelModeToLabelScalars();
  ldm->SetLabelFormat("%g");
  vtkActor2D *lda = vtkActor2D::New();
  lda->SetMapper(ldm);
  ren->AddActor(lda);

    // set label actor to be non-pickable
  lda->PickableOff();

  label_pd->FastDelete();
  int_ids->FastDelete();
  ldm->FastDelete();
  lda->FastDelete();
}

EntityHandle DrawDual::other_sheet(const EntityHandle this_chord,
                                     const EntityHandle dual_surf) 
{
  static std::vector<EntityHandle> chord_surfs;
  EntityHandle val = dual_surf;
  ErrorCode result = MBI->get_parent_meshsets(this_chord, chord_surfs);
  if (MB_SUCCESS == result && !chord_surfs.empty()) {
    if (chord_surfs[0] != dual_surf) val = chord_surfs[0];
    else if (chord_surfs.size() > 1 && chord_surfs[1] != dual_surf)
      val = chord_surfs[1];
    else if (chord_surfs[0] == dual_surf &&
             (chord_surfs.size() == 1 || chord_surfs[1] == dual_surf))
      val = 0;
  }
  chord_surfs.clear();
  return val;
}

ErrorCode DrawDual::get_primal_ids(const Range &ents, std::vector<int> &ids)
{
    // get the ids of these verts, equal to entity ids of their primal entities
  static std::vector<EntityHandle> primals;
  primals.resize(ents.size());
  ids.resize(ents.size());
  ErrorCode result = MBI->tag_get_data(dualEntityTagHandle, ents, &primals[0]);
  if (MB_SUCCESS != result) {
    for (unsigned int i = 0; i < ents.size(); i++) ids[i] = 0;
  }

  result = MBI->tag_get_data(vtkMOABUtils::globalId_tag(), &primals[0], ents.size(),
                             &ids[0]);
  if (MB_SUCCESS != result && MB_TAG_NOT_FOUND != result)
    std::cerr << "tag_get_data returned non-zero result in get_primal_ids." << std::endl;
    
  for (unsigned int i = 0; i < ents.size(); i++) {
    if (0 == ids[i]) ids[i] = MBI->id_from_handle(primals[i]);
  }

  return MB_SUCCESS;
}

ErrorCode DrawDual::reset_drawn_sheets(Range *drawn_sheets) 
{
  ErrorCode result = MB_SUCCESS, tmp_result;
  for (std::map<EntityHandle,GraphWindows>::iterator mit = surfDrawrings.begin();
       mit != surfDrawrings.end(); mit++) {
    if (NULL != drawn_sheets &&
        NULL != (*mit).second.sheetDiagram) 
      drawn_sheets->insert((*mit).first);
    tmp_result = (*mit).second.reset((*mit).first);
    if (MB_SUCCESS != tmp_result) result = tmp_result;
  }
  
  return tmp_result;
}

ErrorCode DrawDual::GraphWindows::reset(EntityHandle dual_surf) 
{
  ErrorCode result = MB_SUCCESS;
  
  if (sheetDiagram) {
    result = gDrawDual->reset_drawing_data(dual_surf);
    delete sheetDiagram;
    sheetDiagram = NULL;
  }
  
  if (pickActor) {
    pickActor->Delete();
    pickActor = NULL;
  }

  return result;
}

ErrorCode DrawDual::reset_drawing_data(EntityHandle dual_surf) 
{
    // deleting a sheet drawing; reset the data on moab tags so it'll draw right
    // next time

    // get the widget
  GraphWindows &this_gw = surfDrawrings[dual_surf];
  
  vtkRenderer *ren = 
    this_gw.sheetDiagram->sheet_diagram()->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
  
  vtkActorCollection *acoll = ren->GetActors();
  assert(NULL != acoll);
  acoll->InitTraversal();
  vtkActor *tmp_actor;
  Range saved_sets;

    // get all actors, check sets in propSetMap; save set, remove actor from map
  while ((tmp_actor = acoll->GetNextItem())) {
    std::map<vtkProp*,EntityHandle>::iterator mit = 
      vtkMOABUtils::propSetMap.find(tmp_actor);
    if (mit == vtkMOABUtils::propSetMap.end()) continue;
    saved_sets.insert((*mit).second);
    vtkMOABUtils::propSetMap.erase(mit);
  }
  
    // for dual surface set:
    // get 0-, 1-, 2-cells, GVEntity for each
  Range tcells, all_cells;
  ErrorCode result = MBI->get_entities_by_type(dual_surf, MBPOLYGON,
                                                 tcells);
  if (MB_SUCCESS != result) return result;
  result = MBI->get_adjacencies(tcells, 0, false, all_cells, Interface::UNION);
  if (MB_SUCCESS != result) return result;
  result = MBI->get_adjacencies(tcells, 1, false, all_cells, Interface::UNION);
  if (MB_SUCCESS != result) return result;
  
  for (Range::const_reverse_iterator rit = all_cells.rbegin(); rit != all_cells.rend(); rit++) {
      // get the GVEntity
    GVEntity *gv_ent;
    result = MBI->tag_get_data(gvEntityHandle, &(*rit), 1, &gv_ent);
    if (MB_TAG_NOT_FOUND == result || 0 == gv_ent) continue;
    
      // reset the data on this gv_ent for this dual surf
    int index = gv_ent->get_index(dual_surf);
    if (index >= 0) gv_ent->reset(index);
  }

  if (this_gw.gvizGraph) {
    free(this_gw.gvizGraph);
    this_gw.gvizGraph = NULL;
  }
  
  if (this_gw.pickActor) {
    this_gw.pickActor->Delete();
    this_gw.pickActor = NULL;
  }

  return MB_SUCCESS;
}

void DrawDual::GVEntity::reset(const int index)    
{
  assert(index >= 0);
  dualSurfs[index] = 0;
  pointPos[index][0] = pointPos[index][1] = 0;
  vtkEntityIds[index] = -1;

  int dim = MBI->dimension_from_handle(moabEntity);
  
    // use gvizEdges to tell whether we're an edge or not
  if (0 == dim) {
    if (gvizPoints[index]) {
      if (useGraphviz)
        free(gvizPoints[index]);
      else 
        MBI->delete_entities((EntityHandle*)&gvizPoints[index], 1);
      gvizPoints[index] = NULL;
    }
  }
  else if (1 == dim) {
    vtkEntityIds[index+2] = -1;
    vtkEntityIds[index+3] = -1;
    if (gvizEdges[index]) {
      if (useGraphviz)
        free(gvizEdges[index]);
      else {
        assert(gvizEdges[index]);
        MBI->delete_entities((EntityHandle*)&gvizEdges[index], 1);
      }
      gvizEdges[index] = NULL;
    }
    if (gvizEdges[index+2]) {
      if (useGraphviz) {
        free(gvizEdges[index+2]);
        if (gvizPoints[index]) free(gvizPoints[index]);
      }
      else {
        assert(gvizEdges[index+2]);
        MBI->delete_entities((EntityHandle*)&gvizEdges[index+2], 1);
        if (gvizPoints[index])
          MBI->delete_entities((EntityHandle*)&gvizPoints[index], 1);
      }
      gvizEdges[index+2] = NULL;
      gvizPoints[index] = NULL;
    }
  }
  
  myActors[index] = NULL;
}
  
void DrawDual::get_graph_points(const EntityHandle *ents, const int num_ents, 
                                EntityHandle dual_surf, void **points) 
{
  std::vector<GVEntity *> gvs(num_ents);
  ErrorCode result = MBI->tag_get_data(gvEntityHandle, ents, num_ents, &gvs[0]);
  if (MB_SUCCESS != result) return;
  
  for (int i = 0; i < num_ents; i++) {
    assert(NULL != gvs[i]);
    int index = gvs[i]->get_index(dual_surf);
    assert(index >= 0 && index < 3);
    points[i] = gvs[i]->gvizPoints[index];
  }
}

void DrawDual::get_graph_points(Range ents,
                                EntityHandle dual_surf, void **points) 
{
  std::vector<GVEntity *> gvs(ents.size());
  ErrorCode result = MBI->tag_get_data(gvEntityHandle, ents, &gvs[0]);
  if (MB_SUCCESS != result) return;
  
  for (unsigned int i = 0; i < ents.size(); i++) {
    int index = gvs[i]->get_index(dual_surf);
    assert(index >= 0 && index < 3);
    points[i] = gvs[i]->gvizPoints[index];
  }
}

ErrorCode DrawDual::smooth_dual_surf(EntityHandle dual_surf, 
                                       Range &dcells,
                                       Range &dedges, Range &dverts,
                                       Range &face_verts, Range &loop_edges) 
{
    // compute the starting positions of the boundary points, and fix them;
    // has to come after construct_graphviz_edges 'cuz we need edge gventities for 2-pt loops
    // copy face_verts 'cuz compute_fixed_points may add face verts for pillow sheets
  ErrorCode result = compute_fixed_points(dual_surf, dverts, 
                                            face_verts, loop_edges); RR;

  const int num_its = 10;
  
  dverts = subtract( dverts, face_verts);
  double tmp_coords[12];
  MeshTopoUtil mtu(vtkMOABUtils::mbImpl);

  std::vector<EntityHandle> graph_points(dverts.size());
  get_graph_points(dverts, dual_surf, (void**) &graph_points[0]);

    // get any edge-adjacent points which are valence-2 and add to list of
    // smooth points; these are edge mid-pts, which should be smoothed so they
    // don't remain at the center
  for (unsigned int j = 0; j < dverts.size(); j++) {
      EntityHandle this_point = graph_points[j];
      
        // get all neighbor verts
      Range nverts, tmp_edges;
      result = mtu.get_bridge_adjacencies(this_point, 1, 0, nverts); RR;
      if (!(4 == nverts.size() || 3 == nverts.size())) {
        std::cerr << "Smoothing sheet failed; dumping file." << std::endl;
        vtkMOABUtils::dualTool->delete_whole_dual();
        EntityHandle save_set;
        ErrorCode result = vtkMOABUtils::mbImpl->create_meshset(MESHSET_SET, save_set);
        if (MB_SUCCESS != result) return MB_FAILURE;
        Range hexes;
        result = vtkMOABUtils::mbImpl->get_entities_by_type(0, MBHEX, hexes);
        if (MB_SUCCESS != result) return MB_FAILURE;
        result = vtkMOABUtils::mbImpl->add_entities(save_set, hexes);
        if (MB_SUCCESS != result) return MB_FAILURE;
        vtkMOABUtils::mbImpl->write_file("tmp.h5m", NULL, NULL, &save_set, 1);
        return MB_FAILURE;
      }
      
      for (Range::iterator rit = nverts.begin(); rit != nverts.end(); rit++) {
        tmp_edges.clear();
        result = MBI->get_adjacencies(&(*rit), 1, 1, false, tmp_edges); RR;
        if (2 == tmp_edges.size() && 
            std::find(graph_points.begin(), graph_points.end(), *rit) == graph_points.end()) 
          graph_points.push_back(*rit);
      }
  }
  
  std::vector<double> new_coords(3*graph_points.size()), 
      old_coords(3*graph_points.size());

  for (int i = 0; i < num_its; i++) {
      // get starting coords for all verts
    if (0 == i) {
      result = MBI->get_coords(&graph_points[0], graph_points.size(), 
                               &old_coords[0]); RR;
    }
    else old_coords.swap(new_coords);

    
    for (unsigned int j = 0; j < graph_points.size(); j++) {
      EntityHandle this_point = graph_points[j];
      
        // get all neighbor verts
      Range nverts;
      result = mtu.get_bridge_adjacencies(this_point, 1, 0, nverts); RR;
      assert(4 == nverts.size() || 3 == nverts.size() || 2 == nverts.size());
      
        // get coords for those verts
      result = MBI->get_coords(nverts, &tmp_coords[0]); RR;

        // compute new coords using inverse length-weighted laplacian
      double denom = 0.0, delta[3] = {0.0, 0.0, 0.0};
      for (unsigned int k = 0; k < nverts.size(); k++) {
        double tdelta[3];
        tdelta[0] = (tmp_coords[3*k] - old_coords[3*j]);
        tdelta[1] = tmp_coords[3*k+1] - old_coords[3*j+1];
        tdelta[2] = tmp_coords[3*k+2] - old_coords[3*j+2];
        double lsq = sqrt(tdelta[0]*tdelta[0] + tdelta[1]*tdelta[1] + 
                          tdelta[2]*tdelta[2]);
        if (true) lsq = 1.0;
        denom += lsq;
        for (int l = 0; l < 3; l++) delta[l] += lsq*tdelta[l];
      }
      if (0 != denom) {
        for (int l = 0; l < 3; l++) 
          new_coords[3*j+l] = old_coords[3*j+l] + delta[l]/denom;
      }
      else {
        for (int l = 0; l < 3; l++) 
          new_coords[3*j+l] = old_coords[3*j+l];
      }
    }
    
      // set the new coordinate positions after each iteration
    result = MBI->set_coords(&graph_points[0], graph_points.size(), 
                             &new_coords[0]); RR;
  }
  
  return MB_SUCCESS;
}

ErrorCode DrawDual::process_pick(EntityHandle dual_surf, 
                                   const double x, const double y,
                                   Range &picked_ents) 
{
    // get vertices on interior of 3d sheet
  Range int_verts, face_verts;
  ErrorCode result = vtkMOABUtils::dualTool->get_dual_entities(dual_surf, NULL, NULL, 
                                                   &int_verts, &face_verts, NULL);
  int_verts = subtract( int_verts, face_verts);
  
    // get vertices on sheet drawing for that sheet, and their positions
  std::vector<EntityHandle> int_points(int_verts.size());
  get_graph_points(int_verts, dual_surf, (void**) &int_points[0]);
  std::vector<double> point_coords(3*int_verts.size());
  result = MBI->get_coords(&int_points[0], int_points.size(), 
                           &point_coords[0]); RR;
  
    // get the closest point to those
  double dist, closest_coords[3];
  EntityHandle closest_point;
  for (unsigned int i = 0; i < int_verts.size(); i++) {
    double this_dist = (point_coords[3*i] - x)*(point_coords[3*i] - x) +
      (point_coords[3*i+1] - y)*(point_coords[3*i+1] - y);
    if (0 == i || this_dist < dist) {
      dist = this_dist;
      closest_point = int_points[i];
      for (int j = 0; j < 3; j++) closest_coords[j] = point_coords[3*i+j];
    }
  }
  
    // get connected edges, points, in star order
  std::vector<EntityHandle> conn_edges(4);
  EntityHandle dual_edges_3d[4], dual_curves[4], conn_verts[4];
  result = MBI->get_adjacencies(&closest_point, 1, 1, false, conn_edges); RR;
  assert(4 == conn_edges.size());
    // conn_edges are on sheet diag; each points to 3d dual edge through curve tag,
    // and each 3d dual edge points to dual curve set through that same tag
  result = MBI->tag_get_data(dualCurveTagHandle, &conn_edges[0], 4, 
                             dual_edges_3d); RR;
  result = MBI->tag_get_data(dualCurveTagHandle, dual_edges_3d, 4, 
                             dual_curves); RR;
    // should only be 2 distinct curves; don't handle all dual curves the same
    // for now, to avoid geometric check
  assert((dual_curves[0] == dual_curves[1] && dual_curves[0] != dual_curves[2]) ||
         (dual_curves[0] == dual_curves[2] && dual_curves[0] != dual_curves[1]) ||
         (dual_curves[0] == dual_curves[3] && dual_curves[0] != dual_curves[1]));
  
    // if same curves are next to each other, switch edges
  if (dual_curves[0] == dual_curves[1]) {
    SWAP(conn_edges[1], conn_edges[2]);
    SWAP(dual_curves[1], dual_curves[2]);
    SWAP(dual_edges_3d[1], dual_edges_3d[2]);
  }

  else if (dual_curves[1] == dual_curves[2]) {
    SWAP(conn_edges[0], conn_edges[1]);
    SWAP(dual_curves[0], dual_curves[1]);
    SWAP(dual_edges_3d[0], dual_edges_3d[1]);
  }
  assert(dual_curves[0] == dual_curves[2] &&
         dual_curves[1] == dual_curves[3]);

    // get connected points & their coords
  for (int i = 0; i < 4; i++) {
    const EntityHandle *connect;
    int num_connect;
    result = MBI->get_connectivity(conn_edges[i], connect, num_connect); RR;
    if (connect[0] == closest_point) conn_verts[i] = connect[1];
    else if (connect[1] == closest_point) conn_verts[i] = connect[0];
    else assert(false);
  }
  
  double conn_coords[12];
  result = MBI->get_coords(conn_verts, 4, conn_coords); RR;
  CartVect pick_closest(x - closest_coords[0], y - closest_coords[1], 0.0);
  CartVect conn_vects[4];

    // pre-compute normalized vectors from closest to each connected vertex
  for (int i = 0; i < 4; i++) {
    conn_vects[i] = CartVect(conn_coords[3*i]-closest_coords[0],
                               conn_coords[3*i+1]-closest_coords[1], 0.0);
    conn_vects[i].normalize();
  }

    // now evaluate to see if we picked an edge or 2cell
  for (int i = 0; i < 4; i++) {
    double this_dot = conn_vects[i] % pick_closest;

      // if positive dot prod & dist < some tol, picked the edge
    if (0.0 < this_dot &&
        abs(dist - this_dot*this_dot) < 1) 
      picked_ents.insert(dual_edges_3d[i]);
    
      // else if cross products w/ neighboring vectors opposite, we're in the 2cell
    else if (0.0 < this_dot &&
             (conn_vects[i] * pick_closest) % (conn_vects[(i+1)%4] * pick_closest)
             < 0.0) {
      Range common_2cell;
      EntityHandle edges[2] = {dual_edges_3d[i], dual_edges_3d[(i+1)%4]};
      result = MBI->get_adjacencies(edges, 2, 2, false, common_2cell);
      assert(common_2cell.size() > 0);
      picked_ents.insert(*common_2cell.begin());
    }
  }

  return MB_SUCCESS;
}

EntityHandle DrawDual::get_dual_surf(vtkRenderer *this_ren) 
{
  std::map<EntityHandle, GraphWindows>::iterator mit;
  
  for (mit = surfDrawrings.begin(); mit != surfDrawrings.end(); mit++) {
    vtkRenderer *gw_ren = NULL;
    if ((*mit).second.sheetDiagram) 
      gw_ren = (*mit).second.sheetDiagram->sheet_diagram()->GetRenderWindow()->
        GetRenderers()->GetFirstRenderer();    
    if (gw_ren == this_ren) return (*mit).first;
  }
  
  return 0;
}
