#ifndef __smtk_model_PropertyType_h
#define __smtk_model_PropertyType_h

namespace smtk {
  namespace model {

/// Primitive storage types for model properties
enum PropertyType
{
  FLOAT_PROPERTY,    //!< Property is an array of floating-point numbers
  STRING_PROPERTY,   //!< Property is an array of strings
  INTEGER_PROPERTY,  //!< Property is an array of integers
  INVALID_PROPERTY   //!< Property has no storage.
};

  } // model namespace
} // smtk namespace

#endif // __smtk_model_PropertyType_h
