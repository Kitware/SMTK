//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/operators/CompositeAuxiliaryGeometry.h"

#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/AuxiliaryGeometryExtension.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Manager.txx"
#include "smtk/model/Model.h"
#include "smtk/model/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

using namespace smtk::model;
using smtk::attribute::FileItem;
using smtk::attribute::StringItem;

#include "smtk/model/CompositeAuxiliaryGeometry_xml.h"

smtkImplementsModelOperator(SMTKCORE_EXPORT, smtk::model::CompositeAuxiliaryGeometry,
  composite_auxiliary_geometry, "composite auxiliary geometry", CompositeAuxiliaryGeometry_xml,
  smtk::model::Session);
