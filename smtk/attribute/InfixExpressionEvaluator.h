//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_attribute_InfixExpressionEvaluator_h
#define __smtk_attribute_InfixExpressionEvaluator_h

#include "smtk/CoreExports.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Evaluator.h"

#include "smtk/common/InfixExpressionError.h"

namespace smtk
{
namespace attribute
{

// An Evaluator for infix math expressions.
class SMTKCORE_EXPORT InfixExpressionEvaluator : public smtk::attribute::Evaluator
{
public:
  smtkTypenameMacro(smtk::attribute::InfixExpressionEvaluator);

  InfixExpressionEvaluator(smtk::attribute::ConstAttributePtr att);

  bool evaluate(
    ValueType& result,
    smtk::io::Logger& log,
    const std::size_t& element,
    const DependentEvaluationMode& evalutionMode) override;

  bool canEvaluate(smtk::io::Logger& log) override;

  bool doesEvaluate(std::size_t element) override;
  bool doesEvaluate() override;

  std::size_t numberOfEvaluatableElements() override;

private:
  // Maps |err| to an error message and adds it as a record to |log|. Does
  // nothing if |err| == smtk::common::InfixExpressionError::ERROR_NONE.
  void logError(const smtk::common::InfixExpressionError& err, smtk::io::Logger& log) const;
};

} // namespace attribute
} // namespace smtk

#endif // __smtk_attribute_InfixExpressionEvaluator_h
