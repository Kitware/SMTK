String token API
----------------

String tokens now have additional API:
+ :smtk:`Token::hasValue() <smtk::string::Token::hasValue>`
  returns true the string manager contains a string for the token's integer hash.
  Note that tokens constructed at compiled-time via the string-literal operator
  will not insert the source string into the manager.
+ :smtk:`Token::valid() <smtk::string::Token::valid>`
  returns true if the token has a valid value.
  The string manager reserves a special value (``smtk::string::Manager::Invalid``)
  to indicate an uninitialized or unavailable hash.
