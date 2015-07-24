/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright 2004 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000, there is a non-exclusive
  license for use of this work by or on behalf of the
  U.S. Government. Redistribution and use in source and binary forms, with
  or without modification, are permitted provided that this Notice and any
  statement of authorship are reproduced on all copies.

=========================================================================*/
/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include "assert.h"

#include "vtkObjectFactory.h"
#include "vtkUnstructuredGrid.h"
#include "vtkMOABUtils.h"
#include "moab/MeshTopoUtil.hpp"
#include "vtkPolyDataMapper.h"
#include "vtkDataSetMapper.h"
#include "vtkActor.h"
#include "vtkCamera.h"
#include "vtkProperty.h"
#include "vtkExtractEdges.h"
#include "vtkTubeFilter.h"
#include "vtkRenderer.h"
#include "vtkExtractGeometry.h"
#include "vtkPlane.h"
#include <algorithm>

void uiQVDual::fileNew()
{
  resetDisplay();
  
  delete vtkMOABUtils::mbImpl;
  vtkMOABUtils::mbImpl = NULL;
  
  redrawDisplay();
}


void uiQVDual::fileOpen()
{
  assert(NULL != vtkMOABUtils::mbImpl);
  QString filename = QFileDialog::getOpenFileName(
      QString::null,
      "Exodus files (*.g *.gen *.exoII);;Cub files (*.cub);;SLAC Netcdf files (*.ncdf);;All Files (*)", this
      );
  if(filename == QString::null)
    return;

  fileOpen(filename);
}

void uiQVDual::fileOpen( const QString &filename )
{
  // create a MOAB vtk reader and pass responsibility to that
  vtkMOABReader *reader = vtkMOABReader::New();
  
  // if we already have a grid, make sure it's that reader's output too
  if (NULL != vtkMOABUtils::myUG) reader->SetOutput(vtkMOABUtils::myUG);
  
  reader->SetFileName(filename.ascii());

  // need to update here, in case we're doing something else which requires moab data
  reader->Update();

  vtkMOABUtils::assign_global_ids();
  
  vtkMOABUtils::update_display(reader->GetOutput());
  
    // compute dual, if requested
  if (computeDual)
    constructDual();

  this->updateMesh();
  
  lastOpened = filename;

  reader->Delete();
}

void uiQVDual::fileSave()
{
  if (lastOpened == QString::null)
    fileSaveAs();
  else
    fileSaveAs(lastOpened);
}


void uiQVDual::fileSaveAs(const QString &filename)
{
  if(filename == QString::null)
    return;

  resetDisplay();
  
  vtkMOABUtils::dualTool->delete_whole_dual();

  computeDual = false;

  vtkMOABUtils::assign_global_ids();
  
    // make a set to specify what gets saved
  MBEntityHandle save_set;
  MBErrorCode result = vtkMOABUtils::mbImpl->create_meshset(MESHSET_SET, save_set);
  if (MB_SUCCESS != result) return;
  MBRange hexes;
  result = vtkMOABUtils::mbImpl->get_entities_by_type(0, MBHEX, hexes);
  if (MB_SUCCESS != result) return;
  result = vtkMOABUtils::mbImpl->add_entities(save_set, hexes);
  if (MB_SUCCESS != result) return;

  vtkMOABUtils::mbImpl->write_file(filename.ascii(), NULL, NULL, &save_set, 1);

  redrawDisplay();
}


void uiQVDual::filePrint()
{
  
}


void uiQVDual::fileExit()
{
  qApp->exit();
}


void uiQVDual::helpIndex()
{
  
}


void uiQVDual::helpContents()
{
  
}


void uiQVDual::helpAbout()
{
  
}


void uiQVDual::init()
{
  // initialize vtk stuff
  vtkRenderer *ren = vtkRenderer::New();

  if (NULL == vtkWidget) {
    vtkWidget = new QVTKWidget( centralWidget(), "vtkWidget" );
    vtkWidget->setGeometry( QRect( 400, 10, 470, 569 ) );
    vtkWidget->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 5, 1, vtkWidget->sizePolicy().hasHeightForWidth() ) );
    vtkWidget->setMinimumSize( QSize( 0, 0 ) );
  }
  
  vtkWidget->GetRenderWindow()->AddRenderer(ren);
  
  // save the renderer
  vtkMOABUtils::init(NULL, ren);
  
  currentWin = 0;

  cropToolPopup = NULL;

  computeDual = false;

  if (NULL == vtkMOABUtils::drawDual) 
    vtkMOABUtils::drawDual = new DrawDual(pickline1, pickline2);
}


void uiQVDual::destroy()
{
  vtkMOABUtils::destroy();
}



void uiQVDual::constructDual()
{
  // tell MOAB to construct the dual first
  MBErrorCode result = vtkMOABUtils::dualTool->construct_hex_dual(NULL, 0);

  redrawDisplay();
  
  QListViewItemIterator it = QListViewItemIterator(TagListView1);
  while ( it.current() ) {
    QString this_name((*it)->text(0));
    
    if (this_name == "DUAL_SURFACE") {
      (*it)->setOpen(true);
      break;
    }

    ++it;
  }

  MBRange sheet_sets;
  result = vtkMOABUtils::dualTool->get_dual_hyperplanes(vtkMOABUtils::mbImpl, 2, sheet_sets);
  if (MB_SUCCESS != result) return;

  if (!sheet_sets.empty()) {
      // draw the first sheet
    if (NULL == vtkMOABUtils::drawDual) vtkMOABUtils::drawDual = 
                                            new DrawDual(pickline1, pickline2);
    vtkMOABUtils::drawDual->draw_dual_surf(*sheet_sets.begin());
  }
}

void uiQVDual::updateMesh()
{
  // mesh was updated; update the various UI elements
  this->updateTagList();
  this->updateActorList();
}



void uiQVDual::DebugButton_pressed()
{
  vtkMOABUtils::print_debug();

}


void uiQVDual::updateTagList()
{
  // clean the tag list view
  TagListView1->clear();    
  QListViewItemIterator it = QListViewItemIterator(TagListView1);
  while ( it.current() ) {
    itemSetMap.erase(*it);
    ++it;
  }
  
  // get a list of tags
  std::vector<MBTag> tag_handles;
  MBErrorCode result = vtkMOABUtils::mbImpl->tag_get_tags(tag_handles);
  if (MB_SUCCESS != result) return;

  // display each tag as a parent item
  QListViewItem *tags_item;
  
  for (std::vector<MBTag>::iterator tag_it = tag_handles.begin(); tag_it != tag_handles.end();
       tag_it++) {
    
    std::string tag_name;
    result = vtkMOABUtils::mbImpl->tag_get_name(*tag_it, tag_name);
    if (MB_SUCCESS != result) continue;

    // don't display tags with "__" prefix
    if (0 == strncmp(tag_name.c_str(), "__", 2))
      continue;
    
    // get all the sets which contain this tag
    MBRange tag_sets;
    result = vtkMOABUtils::mbImpl->get_entities_by_type_and_tag(
      0, MBENTITYSET, &(*tag_it), NULL, 1, tag_sets, MBInterface::UNION);
    if (MB_SUCCESS != result) continue;

    // create a parent tag item
    tags_item = new QListViewItem(TagListView1, tag_name.c_str());
    if (tag_name != "DUAL_SURFACE")
      tags_item->setOpen(false);
    else
      tags_item->setOpen(true);

    itemSetMap[tags_item] = 0;

    int i;
    MBRange::iterator set_it;
    QString set_id;
    QListViewItem *set_item;
    for (i = 0, set_it = tag_sets.begin(); set_it != tag_sets.end(); set_it++, i++) {
    // make an item for this set
      char set_name[CATEGORY_TAG_SIZE];
      result = vtkMOABUtils::get_set_category_name(*set_it, set_name);
      if (MB_SUCCESS != result) sprintf(set_name, "(none)\0");
      set_item = new QListViewItem(tags_item, set_name);
      itemSetMap[set_item] = *set_it;
    }
  }
}


void uiQVDual::updateActorList()
{
    // clean the ActorView
  ActorListView1->clear();    
  QListViewItemIterator it = QListViewItemIterator(ActorListView1);
  while ( it.current() ) {
    itemSetMap.erase(*it);
    ++it;
  }
  
    // update "contains" view
  MBRange top_sets;
  QListViewItem *last_item;
  MBErrorCode result = vtkMOABUtils::get_top_contains_sets(top_sets);
  if (MB_SUCCESS == result && !top_sets.empty()) {
  
      // keep track of last actor line shown
    last_item = new QListViewItem(ActorListView1, "Contains Sets");
    itemSetMap[last_item] = 0;
    last_item->setOpen(false);

    for (MBRange::iterator rit = top_sets.begin(); rit != top_sets.end(); rit++)
      updateActorContainsList(last_item, *rit);
  }
  
    // update "parent/child" view
  top_sets.clear();
  result = vtkMOABUtils::get_top_parent_sets(top_sets);
  if (MB_SUCCESS == result && !top_sets.empty()) {
  
      // keep track of last actor line shown
    last_item = new QListViewItem(ActorListView1, "Parent/Child Sets");
    itemSetMap[last_item] = 0;
    last_item->setOpen(false);

    for (MBRange::iterator rit = top_sets.begin(); rit != top_sets.end(); rit++)
      updateActorParentList(last_item, *rit);
  }
}



void uiQVDual::ActorTreeView_selectionChanged()
{
  currentWin = 1;

    // go through selected items, acting accordingly
  static std::set<QListViewItem*> high_items, unhigh_items, tmp_sel, tmp_unsel;
  high_items.clear(); unhigh_items.clear(); tmp_sel.clear(); tmp_unsel.clear();
  getSelected(ActorListView1, tmp_sel, tmp_unsel);
  // newly selected is difference between selected and itemSelList
  std::set_difference(tmp_sel.begin(), tmp_sel.end(),
                      itemSelList.begin(), itemSelList.end(),
                      std::inserter(high_items, high_items.begin()));
  
  // newly unselected is intersection between unselected and itemSelList
  std::set_intersection(tmp_unsel.begin(), tmp_unsel.end(),
                        itemSelList.begin(), itemSelList.end(),
                        std::inserter(unhigh_items, unhigh_items.begin()));

  if (!high_items.empty() || !unhigh_items.empty()) {
      // reset the properties of the items
    changeSetProperty(high_items, unhigh_items);
    
      // Re-render
    vtkWidget->GetRenderWindow()->Render();
  }
}

void uiQVDual::TagTreeView_selectionChanged()
{
  currentWin = 2;
  
    // go through selected items, acting accordingly
  static std::set<QListViewItem*> high_items, unhigh_items, tmp_sel, tmp_unsel;
  high_items.clear(); unhigh_items.clear(); tmp_sel.clear(); tmp_unsel.clear();
  getSelected(TagListView1, tmp_sel, tmp_unsel);
  // newly selected is difference between selected and itemSelList
  std::set_difference(tmp_sel.begin(), tmp_sel.end(),
                      itemSelList.begin(), itemSelList.end(),
                      std::inserter(high_items, high_items.begin()));
  
  // newly unselected is intersection between unselected and itemSelList
  std::set_intersection(tmp_unsel.begin(), tmp_unsel.end(),
                        itemSelList.begin(), itemSelList.end(),
                        std::inserter(unhigh_items, unhigh_items.begin()));
  
  if (!high_items.empty() || !unhigh_items.empty()) {
      // reset the properties of the items
    changeSetProperty(high_items, unhigh_items);
    
      // Re-render
    vtkWidget->GetRenderWindow()->Render();
  }
}

void uiQVDual::updateActorContainsList( QListViewItem *item, MBEntityHandle set_handle )
{
  vtkActor *set_actor;
  QListViewItem *set_item;

    // get the actor for this set
  MBErrorCode result = vtkMOABUtils::mbImpl->tag_get_data(vtkMOABUtils::vtkSetActorTag,
                                                          &set_handle, 1, &set_actor);
  if (MB_SUCCESS != result && MB_TAG_NOT_FOUND != result) return;
  
  int num_sets;
  if (NULL == set_actor) {
      // no entities - check for contained sets, and if none, return
    result = vtkMOABUtils::mbImpl->get_number_entities_by_type(set_handle, MBENTITYSET,
                                                               num_sets);
    if (MB_SUCCESS != result || 0 == num_sets) return;
  }
  
    // has an actor, or contains sets; allocate a list item
  char set_name[CATEGORY_TAG_SIZE];
  vtkMOABUtils::get_set_category_name(set_handle, set_name);
  set_item = new QListViewItem(item, set_name);
  itemSetMap[set_item] = set_handle;
  set_item->setOpen(false);

  if (NULL == set_actor && 0 == num_sets) return;
  
    // get the list of sets in this set
  MBRange contained_sets;
  result = vtkMOABUtils::mbImpl->get_entities_by_type(set_handle, MBENTITYSET, 
                                                      contained_sets);
  if (MB_SUCCESS != result) return;

  for (MBRange::iterator rit = contained_sets.begin(); rit != contained_sets.end(); rit++)
    updateActorContainsList(set_item, *rit);
}


void uiQVDual::updateActorParentList( QListViewItem *item, MBEntityHandle set_handle )
{
  vtkActor *set_actor;
  QListViewItem *set_item;

    // get the actor for this set
  MBErrorCode result = vtkMOABUtils::mbImpl->tag_get_data(vtkMOABUtils::vtkSetActorTag,
                                                          &set_handle, 1, &set_actor);
  if (MB_SUCCESS != result && MB_TAG_NOT_FOUND != result) return;
  
  int num_sets;
  if (NULL == set_actor) {
      // no entities - check for child sets, and if none, return
    result = vtkMOABUtils::mbImpl->num_child_meshsets(set_handle, &num_sets);
    if (MB_SUCCESS != result || 0 == num_sets) return;
  }
  
    // has an actor, or parent sets; allocate a list item
  char set_name[CATEGORY_TAG_SIZE];
  vtkMOABUtils::get_set_category_name(set_handle, set_name);
  set_item = new QListViewItem(item, set_name);
  itemSetMap[set_item] = set_handle;
  set_item->setOpen(false);

    // get the list of child sets
  if (NULL == set_actor && 0 == num_sets) return;

  std::vector<MBEntityHandle> child_sets;
  result = vtkMOABUtils::mbImpl->get_child_meshsets(set_handle, child_sets);
  if (MB_SUCCESS != result) return;

  for (std::vector<MBEntityHandle>::iterator vit = child_sets.begin(); 
       vit != child_sets.end(); vit++)
    updateActorParentList(set_item, *vit);
}

void uiQVDual::changeSetProperty( std::set<QListViewItem *> &high_sets, std::set<QListViewItem *> &unhigh_sets )
{
  MBRange high_mbsets, unhigh_mbsets;
  MBEntityHandle item_set;
  vtkActor *this_actor;
  
  for (std::set<QListViewItem *>::iterator sit = high_sets.begin(); 
       sit != high_sets.end(); sit++) {

    item_set = itemSetMap[*sit];
    if (0 == item_set)
        // must have child items - evaluate them too
      evalItem(*sit, true, high_mbsets, unhigh_mbsets);

    else {
      this_actor = vtkMOABUtils::get_actor(item_set);
      if (NULL == this_actor) evalItem(*sit, true, high_mbsets, unhigh_mbsets);
      else
        high_mbsets.insert(item_set);
    }
    
      // either way, add item to selection list
    itemSelList.insert(*sit);
  }
  for (std::set<QListViewItem *>::iterator sit = unhigh_sets.begin(); 
       sit != unhigh_sets.end(); sit++) {

    item_set = itemSetMap[*sit];
    if (0 == item_set)
        // must have child items - evaluate them too
      evalItem(*sit, false, unhigh_mbsets, unhigh_mbsets);

    else {
      this_actor = vtkMOABUtils::get_actor(item_set);
      if (NULL == this_actor) evalItem(*sit, true, high_mbsets, unhigh_mbsets);
      else
        unhigh_mbsets.insert(item_set);
    }

      // either way, remove item from selection list
    itemSelList.erase(*sit);
  }

  vtkMOABUtils::change_set_properties(high_mbsets, unhigh_mbsets);
}

void uiQVDual::evalItem( QListViewItem * item, const bool high, 
                         MBRange & high_mbsets, MBRange & unhigh_mbsets )
{
    // item is a parent, evaluate all the child items
  QListViewItem *child_item = item->firstChild();
  while (NULL != child_item) {
    MBEntityHandle child_set = itemSetMap[child_item];
    if (0 == child_set) 
        // if child item also has no set, must be a parent too
      this->evalItem(child_item, high, high_mbsets, unhigh_mbsets);
    else if (child_item->isSelected() != high) {
        // selection state is changing; put item set in proper (high or unhigh) set
      if (high) high_mbsets.insert(child_set);
      else unhigh_mbsets.insert(child_set);
        // since selection state changed on parent item and not child, need to set
        // selection state of child directly
      child_item->setSelected(high);
    }
    
    child_item = child_item->nextSibling();
  }
}


void uiQVDual::displayVisible()
{
  MBRange selected, unselected;
  getSelected(NULL, selected, unselected);

    // get child sets of selected ones, they should be drawn too
  std::vector<MBEntityHandle> children;
  for (MBRange::iterator rit = selected.begin(); rit != selected.end(); rit++)
    vtkMOABUtils::mbImpl->get_child_meshsets(*rit, children, 0);

  std::copy(children.begin(), children.end(), range_inserter(selected));
  
  unselected.clear();
  vtkMOABUtils::change_set_visibility(selected, unselected);
  vtkWidget->GetRenderWindow()->Render();
}


void uiQVDual::displayDraw()
{
  MBRange selected, unselected;
  getSelected(NULL, selected, unselected);
  
    // get child sets of selected ones, they should be drawn too
  std::vector<MBEntityHandle> children;
  for (MBRange::iterator rit = selected.begin(); rit != selected.end(); rit++)
    vtkMOABUtils::mbImpl->get_child_meshsets(*rit, children, 0);

  std::copy(children.begin(), children.end(), range_inserter(selected));
  
  vtkMOABUtils::change_set_visibility(selected, unselected);

    // unhighlight the visible ones; don't change the others
  unselected.clear();
  vtkMOABUtils::change_set_properties(unselected, selected);

  vtkWidget->GetRenderWindow()->Render();
}


void uiQVDual::displayWireframeShaded()
{
  MBRange selected, unselected;
  getSelected(NULL, selected, unselected);
  
  vtkMOABUtils::toggle_wireframe_shaded(selected);
}

void uiQVDual::displayInvertSelection()
{

}

void uiQVDual::ActorListView1_rightButtonPressed( QListViewItem *item, const QPoint &, int )
{
  currentWin = 1;
  Display->exec(QCursor::pos());
}

void uiQVDual::displayInvisible()
{
  MBRange selected, unselected;
  getSelected(NULL, selected, unselected);

    // get child sets of selected ones, they should be set too
  std::vector<MBEntityHandle> children;
  for (MBRange::iterator rit = selected.begin(); rit != selected.end(); rit++)
    vtkMOABUtils::mbImpl->get_child_meshsets(*rit, children, 0);

  std::copy(children.begin(), children.end(), range_inserter(selected));
  
  unselected.clear();
  vtkMOABUtils::change_set_visibility(unselected, selected);
  vtkWidget->GetRenderWindow()->Render();
}

void uiQVDual::getSelected( QListView *listv, 
                            std::set<QListViewItem *> &selected, 
                            std::set<QListViewItem *> &unselected )
{
  if (NULL == listv && currentWin == 0) {
      // check all items
    for (std::map<QListViewItem*, MBEntityHandle>::iterator mit = itemSetMap.begin();
         mit != itemSetMap.end(); mit++) {
      if ((*mit).first->isSelected())
        selected.insert((*mit).first);
      else
        unselected.insert((*mit).first);
    }
    
    return;
  }
  
  QListViewItemIterator it;
  if (NULL == listv) {
    if (1 == currentWin) 
      it = QListViewItemIterator(ActorListView1);
    else if (2 == currentWin)
      it = QListViewItemIterator(TagListView1);
  }
  else it = QListViewItemIterator(listv);
  
  while ( it.current() ) {
    if ( it.current()->isSelected())
     selected.insert(*it);
    else
      unselected.insert(*it);
    ++it;
  }
}

void uiQVDual::getSelected( QListView *listv, MBRange &selected, MBRange &unselected )
{
  static std::set<QListViewItem*> selected_s, unselected_s;
  selected_s.clear(); unselected_s.clear();
  getSelected(listv, selected_s, unselected_s);
  getItemSets(selected_s, selected);
  getItemSets(unselected_s, unselected);
}

void uiQVDual::TagListView1_rightButtonPressed( QListViewItem *, const QPoint &, int )
{
  currentWin = 2;
  Display->exec(QCursor::pos());
}


void uiQVDual::displayDisplayAll()
{
  currentWin = 0;
  MBRange selected, unselected;
  getSelected(NULL, selected, unselected);
  selected.merge(unselected);
  selected.erase(1);
  unselected.clear();
  vtkMOABUtils::change_set_visibility(selected, unselected);
  vtkWidget->GetRenderWindow()->Render();

}

void uiQVDual::getItemSets( std::set<QListViewItem *> &items, MBRange &sets )
{
  std::set<QListViewItem*>::iterator sit;
  std::map<QListViewItem*, MBEntityHandle>::iterator mit;
  for (sit = items.begin(); sit != items.end(); sit++) {
    mit = itemSetMap.find(*sit);
    if (mit != itemSetMap.end() && (*mit).second != 0) 
      sets.insert((*mit).second);
  }
}


void uiQVDual::CropToolButton_clicked()
{
    /*
  if (NULL == cropToolPopup) {
    cropToolPopup = new CropToolPopup();
    cropToolPopup->vtk_widget(vtkWidget);
  }

  cropToolPopup->show();
    */
  vtkMOABUtils::dualTool->check_dual_adjs();
}


void uiQVDual::displayDrawSheetAction_activated()
{
  MBRange selected, unselected;
  getSelected(NULL, selected, unselected);

    // get selected sets which are dual surfaces
  MBRange dual_surfs;
  MBEntityHandle dum_tag;
  MBErrorCode result;
  for (MBRange::iterator rit = selected.begin(); rit != selected.end(); rit++) {
    result = vtkMOABUtils::mbImpl->tag_get_data(vtkMOABUtils::dualTool->dualSurface_tag(), 
                                                &(*rit), 1, &dum_tag);
    if (MB_SUCCESS == result && 0 != dum_tag)
      dual_surfs.insert(*rit);
  }

  if (NULL == vtkMOABUtils::drawDual) vtkMOABUtils::drawDual = new DrawDual(pickline1, pickline2);

  // now draw them
  bool success = vtkMOABUtils::drawDual->draw_dual_surfs(dual_surfs);
  if (!success) 
    std::cout << "Problem drawing dual surface(s)." << std::endl;
}


void uiQVDual::APbutton_clicked()
{

    // make sure the last picked entity is an edge
  MBEntityHandle edge = vtkMOABUtils::drawDual->lastPickedEnt;
  if (0 == edge) {
    std::cerr << "Didn't find a picked entity." << std::endl;
    return;
  }
  
  if (MBEDGE != vtkMOABUtils::mbImpl->type_from_handle(edge)) {
    std::cerr << "AP must apply to a dual edge." << std::endl;
    return;
  }

  MBErrorCode result = vtkMOABUtils::drawDual->reset_drawn_sheets();

    // otherwise, do the AP
  MBEntityHandle quad1, quad2;
  result = vtkMOABUtils::dualTool->atomic_pillow(edge, quad1, quad2);
  if (MB_SUCCESS != result) {
    std::cerr << "AP failed." << std::endl;
    return;
  }

    // get the dual surfaces for those quads
  MBEntityHandle chord = vtkMOABUtils::dualTool->
      get_dual_hyperplane(vtkMOABUtils::dualTool->get_dual_entity(quad1));
  if (0 == chord) return;
  
  std::vector<MBEntityHandle> sheets;
  result = vtkMOABUtils::mbImpl->get_parent_meshsets(chord, sheets);
  if (MB_SUCCESS != result) {
    std::cerr << "Couldn't get parent dual surfaces of dual edge." << std::endl;
    return;
  }
  
  std::cerr << "AP succeeded" << std::endl;

  redrawDisplay();

    // now draw the sheets affected
  bool success = vtkMOABUtils::drawDual->draw_dual_surfs(sheets, true);
  if (!success)
    std::cerr << "Problem drawing dual surfaces from atomic pillow." << std::endl;
}


void uiQVDual::negAPbutton_clicked()
{
    // make sure the last picked entity is a 2cell
  MBEntityHandle tcell = vtkMOABUtils::drawDual->lastPickedEnt;
  if (0 == tcell) {
    std::cerr << "Didn't find a picked entity." << std::endl;
    return;
  }
  
  if (MBPOLYGON != vtkMOABUtils::mbImpl->type_from_handle(tcell)) {
    std::cerr << "-AP must apply to a dual face." << std::endl;
    return;
  }

  MBErrorCode result = vtkMOABUtils::drawDual->reset_drawn_sheets();

    // get the dual surface containing that 2cell
  MBEntityHandle sheet = vtkMOABUtils::dualTool->get_dual_hyperplane(tcell);
  MBRange chords;
  result = vtkMOABUtils::mbImpl->get_child_meshsets(sheet, chords);
  if (MB_SUCCESS != result) {
    std::cerr << "Couldn't get child dual chords of dual surface." << std::endl;
    return;
  }
  else if (2 != chords.size()) {
    std::cerr << "Wrong number of (child) chords for a dual surface; not a pillow?." 
              << std::endl;
    return;
  }

  MBRange other_sheets;
  for (MBRange::iterator rit = chords.begin(); rit != chords.end(); rit++) {
    result = vtkMOABUtils::mbImpl->get_parent_meshsets(*rit, other_sheets);
    if (MB_SUCCESS != result) {
      std::cerr << "Trouble getting chord parents." << std::endl;
      return;
    }
  }
  other_sheets.erase(sheet);
  
    // otherwise, do the -AP
  
  result = vtkMOABUtils::dualTool->rev_atomic_pillow(sheet, chords);
  if (MB_SUCCESS != result) {
    std::cerr << "-AP failed." << std::endl;
    return;
  }

  redrawDisplay();

    // now draw the other sheets
  bool success = vtkMOABUtils::drawDual->draw_dual_surfs(other_sheets, true);
  if (!success)
    std::cerr << "Problem drawing other dual surfaces from reverse atomic pillow." 
              << std::endl;
}


void uiQVDual::FOCbutton_clicked()
{
    // make sure the last picked entities are edges
  MBEntityHandle edge1 = vtkMOABUtils::drawDual->lastPickedEnt,
    edge2 = vtkMOABUtils::drawDual->secondLastPickedEnt;
  if (0 == edge1 || 0 == edge2) {
    std::cerr << "Didn't find a picked entity." << std::endl;
    return;
  }

  MeshTopoUtil mtu(vtkMOABUtils::mbImpl);
  if (0 == mtu.common_entity(edge1, edge2, 2)) {
    std::cerr << "Dual edges don't share a common 2-cell." << std::endl;
    return;
  }
  
  MBErrorCode result;
  
  if (MBEDGE != vtkMOABUtils::mbImpl->type_from_handle(edge1) ||
      MBEDGE != vtkMOABUtils::mbImpl->type_from_handle(edge2)) {
    std::cerr << "FOC must apply to dual edges." << std::endl;
    return;
  }

    // save the quad from edge1, 'cuz the dual sheets/chord might change;
  MBEntityHandle quad = vtkMOABUtils::dualTool->get_dual_entity(edge1);
  assert(0 != quad);

    // reset any drawn sheets (will get redrawn later)
  MBRange drawn_sheets;
  result = vtkMOABUtils::drawDual->reset_drawn_sheets(&drawn_sheets);
  
    // otherwise, do the FOC
  result = vtkMOABUtils::dualTool->face_open_collapse(edge1, edge2);
  if (MB_SUCCESS != result) {
    std::cerr << "FOC failed." << std::endl;
    return;
  }

  std::cerr << "FOC succeeded." << std::endl;

  redrawDisplay();

    // get the dual surfaces for the edges
  edge1 = vtkMOABUtils::dualTool->get_dual_entity(quad);
  MBEntityHandle chord = vtkMOABUtils::dualTool->get_dual_hyperplane(edge1);
  MBRange sheets;
  result = vtkMOABUtils::mbImpl->get_parent_meshsets(chord, sheets);
  if (MB_SUCCESS == result) drawn_sheets.merge(sheets);
  std::vector<MBEntityHandle> dum_sheets;
  for (MBRange::iterator rit = drawn_sheets.begin(); rit != drawn_sheets.end(); rit++) {
    int dum;
    if (vtkMOABUtils::mbImpl->get_number_entities_by_handle(*rit, dum) == MB_SUCCESS &&
        dum > 0) {
      dum_sheets.push_back(*rit);
    }
  }
  bool success = vtkMOABUtils::drawDual->draw_dual_surfs(dum_sheets, true);
  if (!success)
    std::cerr << "Problem drawing previously-drawn dual surfaces." << std::endl;
}


void uiQVDual::FSbutton_clicked()
{
    // make sure the last picked entity is an edge
  MBEntityHandle edge = vtkMOABUtils::drawDual->lastPickedEnt;
  if (0 == edge) {
    std::cerr << "Didn't find a picked entity." << std::endl;
    return;
  }
  
  if (MBEDGE != vtkMOABUtils::mbImpl->type_from_handle(edge)) {
    std::cerr << "FS must apply to a dual edge." << std::endl;
    return;
  }

    // save the quad, 'cuz the dual sheets/chord might change
  MBEntityHandle quad = vtkMOABUtils::dualTool->get_dual_entity(edge);
  assert(0 != quad);

    // reset any drawn sheets (will get redrawn later)
  MBRange drawn_sheets;
  MBErrorCode result = vtkMOABUtils::drawDual->reset_drawn_sheets(&drawn_sheets);
  
    // otherwise, do the FS
  result = vtkMOABUtils::dualTool->face_shrink(edge);
  if (MB_SUCCESS != result) {
    std::cerr << "FS failed." << std::endl;
    return;
  }

  std::cerr << "FS succeeded." << std::endl;

  redrawDisplay();

    // get the dual surfaces for that edge
  edge = vtkMOABUtils::dualTool->get_dual_entity(quad);
  MBEntityHandle chord = vtkMOABUtils::dualTool->get_dual_hyperplane(edge);
  MBRange sheets;
  std::vector<MBEntityHandle> dum_sheets;
  result = vtkMOABUtils::mbImpl->get_parent_meshsets(chord, sheets);
  if (MB_SUCCESS == result) {
    drawn_sheets = drawn_sheets.subtract(sheets);
    for (MBRange::iterator rit = drawn_sheets.begin(); rit != drawn_sheets.end(); rit++) {
      int dum;
      if (vtkMOABUtils::mbImpl->get_number_entities_by_handle(*rit, dum) == MB_SUCCESS &&
          dum > 0) {
        dum_sheets.push_back(*rit);
        
      }
    }
    
    std::copy(sheets.begin(), sheets.end(), std::back_inserter(dum_sheets));
    
    bool success = vtkMOABUtils::drawDual->draw_dual_surfs(dum_sheets, true);
    if (!success)
      std::cerr << "Problem drawing dual surfaces for face shrink." << std::endl;
    
  }
  else {
    std::cerr << "Couldn't get parent dual surfaces of dual edge." << std::endl;
  }
}


void uiQVDual::negFCbutton_clicked()
{
    // make sure the last picked entity is an edge
  MBEntityHandle edge = vtkMOABUtils::drawDual->lastPickedEnt;
  if (0 == edge) {
    std::cerr << "Didn't find a picked entity." << std::endl;
    return;
  }
  
  if (MBEDGE != vtkMOABUtils::mbImpl->type_from_handle(edge)) {
    std::cerr << "Reverse FS must apply to a dual edge." << std::endl;
    return;
  }


    // save the quad, 'cuz the dual sheets/chord might change
  MBEntityHandle quad = vtkMOABUtils::dualTool->get_dual_entity(edge);
  assert(0 != quad);

    // reset any drawn sheets (will get redrawn later)
  MBRange drawn_sheets;
  MBErrorCode result = vtkMOABUtils::drawDual->reset_drawn_sheets(&drawn_sheets);
  
    // otherwise, do the rev FS
  result = vtkMOABUtils::dualTool->rev_face_shrink(edge);
  if (MB_SUCCESS != result) {
    std::cerr << "Reverse FS failed." << std::endl;
    vtkMOABUtils::drawDual->draw_dual_surfs(drawn_sheets);
    return;
  }

  redrawDisplay();

  std::cerr << "Reverse FS succeeded." << std::endl;

    // get the dual surfaces for that edge
  edge = vtkMOABUtils::dualTool->get_dual_entity(quad);
  MBEntityHandle chord = vtkMOABUtils::dualTool->get_dual_hyperplane(edge);
  MBRange sheets;
  std::vector<MBEntityHandle> dum_sheets;
  result = vtkMOABUtils::mbImpl->get_parent_meshsets(chord, sheets);
  if (MB_SUCCESS == result) {
    drawn_sheets = drawn_sheets.subtract(sheets);
    for (MBRange::iterator rit = drawn_sheets.begin(); rit != drawn_sheets.end(); rit++) {
      int dum;
      if (vtkMOABUtils::mbImpl->get_number_entities_by_handle(*rit, dum) == MB_SUCCESS &&
          dum > 0) {
        dum_sheets.push_back(*rit);
        
      }
    }
    
    std::copy(sheets.begin(), sheets.end(), std::back_inserter(dum_sheets));
    
    if (NULL == vtkMOABUtils::drawDual) vtkMOABUtils::drawDual = new DrawDual(pickline1, pickline2);
    bool success = vtkMOABUtils::drawDual->draw_dual_surfs(dum_sheets, true);
    if (!success)
      std::cerr << "Problem drawing dual surfaces for reverse face shrink." << std::endl;
    
  }
  else {
    std::cerr << "Couldn't get parent dual surfaces of dual edge." << std::endl;
  }
}


void uiQVDual::fileSaveAs()
{
  QString filename = QFileDialog::getSaveFileName(
      QString::null,
      "Exodus files (*.g *.gen *.exoII);;Cub files (*.cub);;SLAC Netcdf files (*.ncdf);;All Files (*)", this
      );
  if(filename == QString::null)
    return;

  fileSaveAs(filename);
}


void uiQVDual::resetDisplay()
{
  bool save_compute = computeDual;

  if (NULL != vtkMOABUtils::drawDual) {
    vtkMOABUtils::drawDual->reset_drawn_sheets();
  }

  vtkMOABUtils::reset_drawing_data();

  computeDual = save_compute;
}


void uiQVDual::redrawDisplay()
{
  vtkMOABUtils::update_display();

  this->updateMesh();
}


void uiQVDual::pickline1_returnPressed()
{
    // get the currently picked entity
  QString line = pickline1->text();

  MBRange cell_ents, picked_ents;
  int last_match = 0, next_match = 0;
  MBHandleUtils hu(vtkMOABUtils::mbImpl->proc_rank(), 
                   vtkMOABUtils::mbImpl->proc_size());
  while (true) {
    last_match = next_match;
    if (-1 == last_match) break;
    if (0 != last_match) last_match++;
    next_match = line.find("-", last_match);
    int ent_no = -1;
    MBEntityType etype = MBMAXTYPE;
    if (line.ascii()[last_match] == 'h') etype = MBHEX;
    else if (line.ascii()[last_match] == 'f') etype = MBQUAD;
    sscanf(line.ascii()+last_match+1, "%d", &ent_no);
    if (-1 != ent_no && MBMAXTYPE != etype)
      cell_ents.insert(hu.create_handle(etype, ent_no, 0));
  }
#define PR(a) {pickline1->setText(QString(a));return;}

  if (cell_ents.size() < 2) PR("(not a dual entity!)");

  MBErrorCode result;
  if (cell_ents.size() > 2) {
      // assume it's a dual 2-cell
    result = vtkMOABUtils::mbImpl->get_adjacencies(cell_ents, 1, false, 
                                                   picked_ents);
    if (picked_ents.empty()) PR("(not a dual 2-cell!)");
  }
  else {
    result = vtkMOABUtils::mbImpl->get_adjacencies(cell_ents, 2, false, 
                                                   picked_ents);
    if (picked_ents.empty()) PR("(not a dual 1-cell!)");
  }
  
  MBEntityHandle dual_ent = vtkMOABUtils::dualTool->get_dual_entity(*picked_ents.begin());
  if (0 ==  dual_ent) PR("(no dual entity!)");
  picked_ents.clear();
  picked_ents.insert(dual_ent);
    
  vtkMOABUtils::drawDual->print_picked_ents(picked_ents, true);
}


void uiQVDual::displayPrintSheet()
{
  MBRange selected, unselected;
  getSelected(NULL, selected, unselected);

    // get selected sets which are dual surfaces
  MBRange dual_surfs;
  MBEntityHandle dum_tag;
  MBErrorCode result;
  for (MBRange::iterator rit = selected.begin(); rit != selected.end(); rit++) {
    result = vtkMOABUtils::mbImpl->tag_get_data(vtkMOABUtils::dualTool->dualSurface_tag(), 
                                                &(*rit), 1, &dum_tag);
    if (MB_SUCCESS == result && 0 != dum_tag)
      dual_surfs.insert(*rit);
  }

  if (NULL == vtkMOABUtils::drawDual) 
    vtkMOABUtils::drawDual = new DrawDual(pickline1, pickline2);

  // now draw them
  bool success = vtkMOABUtils::drawDual->print_dual_surfs(dual_surfs);
  if (!success) 
    std::cout << "Problem drawing dual surface(s)." << std::endl;

}
