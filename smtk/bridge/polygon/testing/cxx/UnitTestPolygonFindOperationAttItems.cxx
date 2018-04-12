//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/bridge/polygon/operators/CreateEdge.h"
#include "smtk/bridge/polygon/operators/CreateModel.h"

#include "smtk/common/testing/cxx/helpers.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/StringData.h"
#include "smtk/model/operators/EntityGroupOperation.h"

#include <complex>

using namespace smtk::model;
using namespace smtk::attribute;

int UnitTestPolygonFindOperationAttItems(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  // Explicitly instantiate item filters for gcc4.8
  std::function<bool(smtk::attribute::ItemPtr)> itemFilter = [](
    smtk::attribute::ItemPtr) { return true; };
  std::function<bool(smtk::attribute::DoubleItemPtr)> doubleFilter = [](
    DoubleItemPtr) { return true; };
  std::function<bool(smtk::attribute::IntItemPtr)> intFilter = [](IntItemPtr) { return true; };
  std::function<bool(smtk::attribute::ItemPtr)> intDoubleFilter = [](
    ItemPtr item) { return item->type() == Item::DoubleType || item->type() == Item::IntType; };
  std::function<bool(smtk::attribute::ComponentItemPtr)> componentFilter = [](
    ComponentItemPtr) { return true; };
  std::function<bool(smtk::attribute::VoidItemPtr)> voidFilter = [](VoidItemPtr) { return true; };

  std::cout << "Use create model operator to test filterItems function in attribute." << std::endl;
  /// Use "create model" operator to valid valueItem
  smtk::bridge::polygon::Operation::Ptr createModelOp =
    smtk::bridge::polygon::CreateModel::create();
  std::vector<ItemPtr> cmItems, intDoubleItems;
  createModelOp->parameters()->filterItems(cmItems, itemFilter, false);
  smtkTest(cmItems.size() == 10, "  Number of items including all children does not equal 10");
  // Find int and double items in all children
  createModelOp->parameters()->filterItems(intDoubleItems, intDoubleFilter, false);
  smtkTest(intDoubleItems.size() == 8,
    "  Number of int and double items including all children does not equal 8");
  // Find items in active children
  std::vector<DoubleItemPtr> cmDoubleItems;
  createModelOp->parameters()->filterItems(cmDoubleItems, doubleFilter);
  smtkTest(cmDoubleItems.size() == 4,
    "  Number of double items only including active child does not equal 4");
  smtkTest(cmDoubleItems[0]->name() == std::string("origin"), "  Double item origin is missing");
  smtkTest(cmDoubleItems[1]->name() == std::string("x axis"), "  Double item x axis is missing");
  smtkTest(cmDoubleItems[2]->name() == std::string("y axis"), "  Double item y axis is missing");
  smtkTest(cmDoubleItems[3]->name() == "feature size", "  Double item feature size is missing");

  // Find items in all children
  cmDoubleItems.clear();
  createModelOp->parameters()->filterItems(cmDoubleItems, doubleFilter, false);
  smtkTest(
    cmDoubleItems.size() == 5, "  Number of double items include all children does not equal five");
  smtkTest(cmDoubleItems[0]->name() == "feature size", "  Double item feature size is missing");
  smtkTest(cmDoubleItems[1]->name() == std::string("origin"), "  Double item origin is missing");
  smtkTest(cmDoubleItems[2]->name() == std::string("x axis"), "  Double item x axis is missing");
  smtkTest(cmDoubleItems[3]->name() == std::string("y axis"), "  Double item y axis is missing");
  smtkTest(cmDoubleItems[4]->name() == std::string("z axis"), "  Double item z axis is missing");
  std::cout << "  All passed" << std::endl;

  std::cout << "Use entity group operator to test filterItems function in attribute." << std::endl;
  /// use "entity group" operator to valid valueItem
  smtk::operation::Operation::Ptr groupOp = smtk::model::EntityGroupOperation::create();
  std::vector<ItemPtr> groupItems, allGroupItems;
  std::vector<VoidItemPtr> voidGroupItems;
  std::vector<ComponentItemPtr> componentGroupItems;
  groupOp->parameters()->filterItems(groupItems, itemFilter);
  groupOp->parameters()->filterItems(allGroupItems, itemFilter, false);
  groupOp->parameters()->filterItems(voidGroupItems, voidFilter);

  smtkTest(groupItems.size() == 8, "Number of items only including active child ("
      << groupItems.size() << ") does not equal 8");
  smtkTest(allGroupItems.size() == 11, "Number of items including all children ("
      << allGroupItems.size() << ") does not equal 11");
  smtkTest(voidGroupItems.size() == 4, "Number of void items only including active child ("
      << voidGroupItems.size() << ") does not equal 4");
  smtkTest(voidGroupItems[0]->name() == std::string("Vertex"), "void item Vertex is missing");
  smtkTest(voidGroupItems[1]->name() == std::string("Edge"), "void item Edge is missing");
  smtkTest(voidGroupItems[2]->name() == std::string("Face"), "void item Face is missing");
  smtkTest(voidGroupItems[3]->name() == std::string("Volume"), "void item Volume is missing");

  groupOp->parameters()->filterItems(componentGroupItems, componentFilter);
  smtkTest(
    componentGroupItems.size() == 1, "Number of component items only including active child ("
      << componentGroupItems.size() << ") does not equal 1");
  componentGroupItems.clear();
  groupOp->parameters()->filterItems(componentGroupItems, componentFilter, false);
  smtkTest(componentGroupItems.size() == 4, "Number of component items including children ("
      << componentGroupItems.size() << ") does not equal 4");
  std::cout << "  All passed" << std::endl;

  std::cout << "Use entity group operator to test filterItems function in attribute." << std::endl;
  /// use "entity group" operator to valid valueItem
  smtk::bridge::polygon::Operation::Ptr createEdge = smtk::bridge::polygon::CreateEdge::create();
  std::vector<ItemPtr> CEItems, allCEItems;
  std::vector<IntItemPtr> CEIntItems;
  std::vector<DoubleItemPtr> CEDoubleItems;
  createEdge->parameters()->filterItems(CEItems, itemFilter);
  createEdge->parameters()->filterItems(CEIntItems, intFilter);
  createEdge->parameters()->filterItems(CEDoubleItems, doubleFilter, false);
  createEdge->parameters()->filterItems(allCEItems, itemFilter, false);

  smtkTest(CEItems.size() == 3, "Number of items only including active child does not equal 3");
  smtkTest(allCEItems.size() == 6, "Number of items including all children does not equal 6");
  smtkTest(CEIntItems.size() == 3, "Number of int items including all children does not equal 3");
  smtkTest(CEIntItems[0]->name() == std::string("debug level"), "Int item debug level is missing");
  smtkTest(CEIntItems[1]->name() == std::string("construction method"),
    "Int item construction method is missing");
  smtkTest(
    CEIntItems[2]->name() == std::string("HelperGlobalID"), "Int item HelperGlobalID is missing");
  smtkTest(
    CEDoubleItems.size() == 1, "Number of double items including all children does not equal 1");
  smtkTest(CEDoubleItems[0]->name() == std::string("points"), "double item points is missing");

  std::cout << "  All passed" << std::endl;

  return 0;
}
