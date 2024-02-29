//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_Comment_h
#define smtk_markup_Comment_h

#include "smtk/markup/Label.h"

#include "smtk/string/Token.h"

namespace smtk
{
namespace markup
{

class SMTKMARKUP_EXPORT Comment : public smtk::markup::Label
{
public:
  smtkTypeMacro(smtk::markup::Comment);
  smtkSuperclassMacro(smtk::markup::Label);

  template<typename... Args>
  Comment(Args&&... args)
    : Superclass(std::forward<Args>(args)...)
  {
  }

  ~Comment() override;

  /// Provide an initializer for resources to call after construction.
  void initialize(const nlohmann::json& data, smtk::resource::json::Helper& helper) override;

  /// Text of the comment.
  bool setData(const smtk::string::Token& data);
  const smtk::string::Token& data() const;
  smtk::string::Token& data();

  /// Mime type of the comment. The default is "text/plain".
  bool setMimetype(const smtk::string::Token& mimetype);
  const smtk::string::Token& mimetype() const;
  smtk::string::Token& mimetype();

  /// Assign this node's state from \a source.
  bool assign(const smtk::graph::Component::ConstPtr& source, smtk::resource::CopyOptions& options)
    override;

protected:
  smtk::string::Token m_data;
  smtk::string::Token m_mimetype;
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_Comment_h
