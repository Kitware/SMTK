//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_graph_ArcTraits_h
#define smtk_graph_ArcTraits_h

#include "smtk/common/CompilerInformation.h" // for SMTK_ALWAYS_EXPORT
#include "smtk/graph/Directionality.h"

#include <type_traits> // for std::integral_constant<>

namespace smtk
{
namespace graph
{

/// For endpoint interfaces, provide tag classes used to select const or non-const APIs.
template<bool Constness>
struct SMTK_ALWAYS_EXPORT ArcConstness : std::integral_constant<bool, Constness>
{
};
using ConstArc = ArcConstness<true>;
using NonConstArc = ArcConstness<false>;

/// For endpoint interfaces, provide tag classes used to select incoming or outgoing APIs.
template<bool OutgoingDirection>
struct SMTK_ALWAYS_EXPORT ArcDirection : std::integral_constant<bool, OutgoingDirection>
{
};
using OutgoingArc = ArcDirection<true>;
using IncomingArc = ArcDirection<false>;

} // namespace graph
} // namespace smtk

#endif // smtk_graph_ArcTraits_h
