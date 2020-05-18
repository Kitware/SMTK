//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_attribute_Evaluator_h
#define __smtk_attribute_Evaluator_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"

#include "smtk/attribute/Attribute.h"

#include "smtk/io/Logger.h"

#include <string>
#include <vector>

#include <boost/variant.hpp>

namespace smtk
{
namespace attribute
{

// Abstract base class for Attribute Evaluators.
class SMTKCORE_EXPORT Evaluator
{
public:
  smtkTypenameMacroBase(smtk::attribute::Evaluator);

  Evaluator(smtk::attribute::ConstAttributePtr att);

  virtual ~Evaluator() = default;

  // Returns true if |m_att| can be evaluated by the Evaluator. If not, messages
  // can be placed in |log|.
  virtual bool canEvaluate(smtk::io::Logger& log) = 0;

  using ValueType = boost::variant<
    std::string,
    int,
    double,
    std::vector<std::string>,
    std::vector<int>,
    std::vector<double>>;

  enum class DependentEvaluationMode
  {
    EVALUATE_DEPENDENTS = 0,
    DO_NOT_EVALUATE_DEPENDENTS = 1
  };

  // Evaluates m_att. Returns true if evaluation was succcessful.
  //
  // The result of the evaluation will be in |result|.
  // |log| will have errors if evaluation was unsuccessful.
  // It is the responsibility of the Evaluator subclass to determine what
  // evaluation at |element| means.
  virtual bool evaluate(
    ValueType& result,
    smtk::io::Logger& log,
    const std::size_t& element = 0,
    const DependentEvaluationMode& evaluationMode =
      DependentEvaluationMode::EVALUATE_DEPENDENTS) = 0;

  // Returns true if index |element| of |m_att| evaluates succesfully, specific
  // to the concrete implementation.
  virtual bool doesEvaluate(std::size_t element) = 0;

  // Returns true if all elements of |m_att| evaluates successfully, specific to
  // the concrete implementation.
  virtual bool doesEvaluate() = 0;

  // Returns the number of elements of |m_att| available for attempted
  // evaluation, specific to the concrete implementation.
  virtual std::size_t numberOfEvaluatableElements() = 0;

protected:
  smtk::attribute::ConstWeakAttributePtr attribute() const { return m_att; }

private:
  smtk::attribute::ConstWeakAttributePtr m_att;
};

} // namespace attribute
} // namespace smtk

#endif // __smtk_attribute_Evaluator_h
