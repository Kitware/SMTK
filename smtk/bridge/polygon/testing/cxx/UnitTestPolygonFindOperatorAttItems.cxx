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
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/VoidItem.h"
#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"
#include "smtk/model/Session.h"
#include "smtk/model/StringData.h"
#include <complex>

using namespace smtk::model;
using namespace smtk::attribute;

int UnitTestPolygonFindOperatorAttItems(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  smtk::model::ManagerPtr manager = smtk::model::Manager::create();
  smtk::model::SessionRef session = manager->createSession("polygon");

  // Explicitly instantiate item filters for gcc4.8
  std::function<bool(smtk::attribute::ItemPtr)> itemFilter = [](
    smtk::attribute::ItemPtr) { return true; };
  std::function<bool(smtk::attribute::DoubleItemPtr)> doubleFilter = [](
    DoubleItemPtr) { return true; };
  std::function<bool(smtk::attribute::IntItemPtr)> intFilter = [](IntItemPtr) { return true; };
  std::function<bool(smtk::attribute::ItemPtr)> intDoubleFilter = [](
    ItemPtr item) { return item->type() == Item::DoubleType || item->type() == Item::IntType; };
  std::function<bool(smtk::attribute::ModelEntityItemPtr)> modelEntityFilter = [](
    ModelEntityItemPtr) { return true; };
  std::function<bool(smtk::attribute::VoidItemPtr)> voidFilter = [](VoidItemPtr) { return true; };

  std::cout << "Use create model operator to test filterItems function in attribute." << std::endl;
  /// Use "create model" operator to valid valueItem
  smtk::model::OperatorPtr createModelOp = session.op("create model");
  std::vector<ItemPtr> cmItems, intDoubleItems;
  createModelOp->specification()->filterItems(cmItems, itemFilter, false);
  test(cmItems.size() == 10, "  Number of items including all children does not equal 10");
  // Find int and double items in all children
  createModelOp->specification()->filterItems(intDoubleItems, intDoubleFilter, false);
  test(intDoubleItems.size() == 9,
    "  Number of int and double items including all children does not equal 9");
  // Find items in active children
  std::vector<DoubleItemPtr> cmDoubleItems;
  createModelOp->specification()->filterItems(cmDoubleItems, doubleFilter);
  test(cmDoubleItems.size() == 4,
    "  Number of double items only including active child does not equal 4");
  test(cmDoubleItems[0]->name() == std::string("origin"), "  Double item origin is missing");
  test(cmDoubleItems[1]->name() == std::string("x axis"), "  Double item x axis is missing");
  test(cmDoubleItems[2]->name() == std::string("y axis"), "  Double item y axis is missing");
  test(cmDoubleItems[3]->name() == "feature size", "  Double item feature size is missing");

  // Find items in all children
  cmDoubleItems.clear();
  createModelOp->specification()->filterItems(cmDoubleItems, doubleFilter, false);
  test(
    cmDoubleItems.size() == 5, "  Number of double items include all children does not equal five");
  test(cmDoubleItems[0]->name() == "feature size", "  Double item feature size is missing");
  test(cmDoubleItems[1]->name() == std::string("origin"), "  Double item origin is missing");
  test(cmDoubleItems[2]->name() == std::string("x axis"), "  Double item x axis is missing");
  test(cmDoubleItems[3]->name() == std::string("y axis"), "  Double item y axis is missing");
  test(cmDoubleItems[4]->name() == std::string("z axis"), "  Double item z axis is missing");
  std::cout << "  All passed" << std::endl;

  std::cout << "Use entity group operator to test filterItems function in attribute." << std::endl;
  /// use "entity group" operator to valid valueItem
  smtk::model::OperatorPtr groupOp = session.op("entity group");
  std::vector<ItemPtr> groupItems, allGroupItems;
  std::vector<VoidItemPtr> voidGroupItems;
  std::vector<ModelEntityItemPtr> modelEntityGroupItems;
  groupOp->specification()->filterItems(groupItems, itemFilter);
  groupOp->specification()->filterItems(allGroupItems, itemFilter, false);
  groupOp->specification()->filterItems(voidGroupItems, voidFilter);

  test(groupItems.size() == 10, "Number of items only including active child does not equal 10");
  test(allGroupItems.size() == 13, "Number of items including all children does not equal 13");
  test(voidGroupItems.size() == 4,
    "Number of void items only including active child does not equal 4");
  test(voidGroupItems[0]->name() == std::string("Vertex"), "void item Vertex is missing");
  test(voidGroupItems[1]->name() == std::string("Edge"), "void item Edge is missing");
  test(voidGroupItems[2]->name() == std::string("Face"), "void item Face is missing");
  test(voidGroupItems[3]->name() == std::string("Volume"), "void item Volume is missing");

  groupOp->specification()->filterItems(modelEntityGroupItems, modelEntityFilter);
  test(modelEntityGroupItems.size() == 2,
    "Number of modelEntity items only including active child does not equal 2");
  modelEntityGroupItems.clear();
  groupOp->specification()->filterItems(modelEntityGroupItems, modelEntityFilter, false);
  test(modelEntityGroupItems.size() == 5,
    "Number of modelEntity items including childern does not equal 5");
  std::cout << "  All passed" << std::endl;

  std::cout << "Use entity group operator to test filterItems function in attribute." << std::endl;
  /// use "entity group" operator to valid valueItem
  smtk::model::OperatorPtr createEdge = session.op("create edge");
  std::vector<ItemPtr> CEItems, allCEItems;
  std::vector<IntItemPtr> CEIntItems;
  std::vector<DoubleItemPtr> CEDoubleItems;
  createEdge->specification()->filterItems(CEItems, itemFilter);
  createEdge->specification()->filterItems(CEIntItems, intFilter);
  createEdge->specification()->filterItems(CEDoubleItems, doubleFilter, false);
  createEdge->specification()->filterItems(allCEItems, itemFilter, false);

  test(CEItems.size() == 4, "Number of items only including active child does not equal 4");
  test(allCEItems.size() == 7, "Number of items including all children does not equal 7");
  test(CEIntItems.size() == 4, "Number of int items including all children does not equal 4");
  test(CEIntItems[0]->name() == std::string("assign names"), "Int item assign names is missing");
  test(CEIntItems[1]->name() == std::string("debug level"), "Int item debug level is missing");
  test(CEIntItems[2]->name() == std::string("construction method"),
    "Int item construction method is missing");
  test(
    CEIntItems[3]->name() == std::string("HelperGlobalID"), "Int item HelperGlobalID is missing");
  test(CEDoubleItems.size() == 1, "Number of double items including all children does not equal 1");
  test(CEDoubleItems[0]->name() == std::string("points"), "double item points is missing");

  std::cout << "  All passed" << std::endl;

  return 0;
}

// This macro ensures the polygon session library is loaded into the executable
smtkComponentInitMacro(smtk_polygon_session)
