//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qt_GroupOps_h
#define smtk_extension_qt_GroupOps_h

/*!\file GroupOps.h - User interfaces that determine operation dispositions.
 *
 * This file is intended to provide Qt-based user interfaces (UIs) that
 * present users with choices about applying an operation-group to a set of
 * persistent objects.
 * These UI functions match the smtk::operation::DispositionFunction signature.
 * This file is a partner to the `smtk/operation/GroupOps.h` header that declares
 * functions which expect DispositionFunctions as parameters.
 */

#include "smtk/extension/qt/Exports.h"
#include "smtk/operation/GroupOps.h"

namespace smtk
{
namespace extension
{

/// A function to resolve issues when deleting persistent objects.
///
/// Pass this function to smtk::operation::deleteObjects(); it will
/// prompt users if some subset of the input objects cannot be
/// deleted.
bool SMTKQTEXT_EXPORT qtDeleterDisposition(smtk::operation::DispositionBatch& batch);

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qt_GroupOps_h
