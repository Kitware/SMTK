//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_URL_h
#define smtk_markup_URL_h

#include "smtk/markup/Label.h"

#include "smtk/string/Token.h"

namespace smtk
{
namespace markup
{

namespace arcs
{
struct URLsToData;
}

class SMTKMARKUP_EXPORT URL : public smtk::markup::Label
{
public:
  smtkTypeMacro(smtk::markup::URL);
  smtkSuperclassMacro(smtk::markup::Label);

  template<typename... Args>
  URL(Args&&... args)
    : smtk::markup::Label(std::forward<Args>(args)...)
  {
  }

  template<typename... Args>
  URL(const smtk::string::Token& location, Args&&... args)
    : smtk::markup::Label(std::forward<Args>(args)...)
  {
    this->setLocation(location);
  }

  ~URL() override;

  /// Set/get the actual URL; either a filename or a full resource locator with protocol.
  bool setLocation(const smtk::string::Token& location);
  smtk::string::Token location() const;

  /// Set/get the type of data present at the URL (if known). May be an empty token.
  bool setType(const smtk::string::Token& mimeType);
  smtk::string::Token type() const;

  /// Return the default file extension to use for the mime type.
  /// If the location is set, this returns its extension.
  /// If not, a file type is generated from the mime type.
  ///
  /// The returned value will either be an empty string (if no mime type is
  /// set and the location is unset or has no extension) or an extension that
  /// begins with a period ('.')
  std::string extensionForType() const;

  /**\brief Return the components instantiated from data in this URL.
    */
  //@{
  ArcEndpointInterface<arcs::URLsToData, ConstArc, OutgoingArc> data() const;
  ArcEndpointInterface<arcs::URLsToData, NonConstArc, OutgoingArc> data();
  //@}
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_URL_h
