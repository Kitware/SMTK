//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_common_Processing_h
#define smtk_common_Processing_h

#include "smtk/CoreExports.h"

namespace smtk
{
namespace common
{

/**\brief An enumeration visitors use to control processing after they yield.
  *
  * Values indicate whether processing should stop or continue.
  * If visitation is threaded, note that returning STOP may not completely
  * eliminate future calls to the visitor (on other threads).
  *
  * In the future, other options may be added (e.g., STOP_ALL_THREADS vs STOP_CURRENT_THREAD).
  */
enum class Processing
{
  CONTINUE, //!< Do not terminate early, continue invoking the functor.
  STOP      //!< Terminate early; avoid invoking the functor again.
};

/**\brief An enumeration returned by visitors indicating whether they terminated early.
  *
  */
enum class Termination
{
  NORMAL, //!< Processing was not interrupted
  EARLY   //!< Processing was halted before iteration was complete.
};

} // namespace common
} // namespace smtk

#endif // smtk_common_Processing_h
