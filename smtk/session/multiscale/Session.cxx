//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#include "smtk/session/multiscale/Session.h"

#include "smtk/common/PythonInterpreter.h"

#include "smtk/model/ArrangementHelper.h"
#include "smtk/model/Model.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include <pybind11/embed.h>
SMTK_THIRDPARTY_POST_INCLUDE

using namespace smtk::model;
using namespace smtk::common;

namespace smtk
{
namespace session
{
namespace multiscale
{

Session::Session() {}

Session::~Session() {}

} // namespace multiscale
} // namespace session
} // namespace smtk
