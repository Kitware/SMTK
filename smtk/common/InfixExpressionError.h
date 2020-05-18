//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_common_InfixExpressionError_h
#define __smtk_common_InfixExpressionError_h
/*!\file InfixExpressionError.h - Error codes for infix expression parsing and evaluation. */

namespace smtk
{
namespace common
{

enum class InfixExpressionError
{
  ERROR_NONE = 0,
  ERROR_INVALID_TOKEN = 1,
  ERROR_INVALID_SYNTAX = 2,
  ERROR_UNKNOWN_FUNCTION = 3,
  ERROR_UNKNOWN_OPERATOR = 4,
  ERROR_MATH_ERROR = 5,
  ERROR_SUBEVALUATION_FAILED = 6
};

} // namespace common
} // namespace smtk

#endif // __smtk_common_InfixExpressionError_h
