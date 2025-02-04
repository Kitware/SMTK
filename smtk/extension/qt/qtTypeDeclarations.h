//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtTypeDeclarations_h
#define smtk_extension_qtTypeDeclarations_h

// #include "smtk/extension/qt/Exports.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Definition.h"
#include "smtk/resource/PersistentObject.h"
#include "smtk/view/BadgeSet.h"
#include "smtk/view/DescriptivePhrase.h"

#include <QMetaType>
#include <QVector>

// Allow QVariant objects to hold shared pointers to
// persistent objects and descriptive phrases.
Q_DECLARE_METATYPE(smtk::resource::PersistentObject::Ptr)
Q_DECLARE_METATYPE(smtk::view::BadgeSet::BadgeList)
Q_DECLARE_METATYPE(smtk::view::DescriptivePhrase::Ptr)
Q_DECLARE_METATYPE(smtk::attribute::DefinitionPtr)
Q_DECLARE_METATYPE(QVector<int>)

#endif // smtk_extension_qtTypeDeclarations_h
