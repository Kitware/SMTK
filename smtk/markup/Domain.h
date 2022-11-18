//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_Domain_h
#define smtk_markup_Domain_h

#include "smtk/markup/Exports.h"

#include "smtk/SharedFromThis.h"
#include "smtk/string/Token.h"

#include "nlohmann/json.hpp"

#include <memory>

namespace smtk
{
namespace markup
{

/// The domain of a discrete or parameterized dataset.
class SMTKMARKUP_EXPORT Domain : public std::enable_shared_from_this<Domain>
{
public:
  smtkTypeMacroBase(smtk::markup::Domain);

  Domain() = default;
  Domain(smtk::string::Token name);
  Domain(const nlohmann::json& data);
  virtual ~Domain() = default;

  smtk::string::Token name() const { return m_name; }

protected:
  smtk::string::Token m_name;
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_Domain_h
