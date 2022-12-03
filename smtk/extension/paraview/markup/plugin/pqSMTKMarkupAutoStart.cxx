//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/markup/plugin/pqSMTKMarkupAutoStart.h"

#include "smtk/view/Selection.h"

#include "smtk/extension/paraview/markup/qtOntologyItem.h"

#include "smtk/extension/qt/qtSMTKUtilities.h"

pqSMTKMarkupAutoStart::pqSMTKMarkupAutoStart(QObject* parent)
  : Superclass(parent)
{
}

pqSMTKMarkupAutoStart::~pqSMTKMarkupAutoStart() = default;

void pqSMTKMarkupAutoStart::startup()
{
  // Register qtItem widget subclasses that add markup dependencies.
  qtSMTKUtilities::registerItemConstructor(
    "qtOntologyItem", smtk::extension::qtOntologyItem::createItemWidget);
}

void pqSMTKMarkupAutoStart::shutdown() {}
