//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_extension_qtUtility_h
#define smtk_extension_qtUtility_h

#include "smtk/extension/qt/Exports.h"

#include <QColor>
#include <QIcon>

#include <string>

namespace smtk
{
namespace extension
{

/// A function used to convert an SVG icon into a QIcon with its
/// color adjusted to the given \a background color.
///
/// This function will replace "black" with "white" when
/// \a background.lightnessF() >= 0.5. It also converts the
/// resulting SVG string into a QIcon.
///
/// This simple adjustment (exchanging black and white) is distinct
/// from qtUIManager::contrastWithText(), which attempts to adjust
/// an input color's lightness to contrast with Qt's text color in
/// the current palette.
QIcon SMTKQTEXT_EXPORT colorAdjustedIcon(const std::string& svg, const QColor& background);

} // namespace extension
} // namespace smtk

#endif
