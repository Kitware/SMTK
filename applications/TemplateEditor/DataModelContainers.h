#ifndef DataModelContainers_h
#define DataModelContainers_h
#include <string>

/**
 * \brief Container for parameters to create an attribute definition.
 * SMTK does not currently have an interface to create an AttrDef without
 * inserting it into the attribute::collection.  This container is used to
 * pass input parameters (from the dialog) to the DataModel given that
 * DataModels are the only components in the application inserting/removing
 * data into/from the attribute::collection.
 *
 * \note If a Definition could be created before insertion into the collection,
 * that instance itself could be used instead of this container.
 */
struct AttDefContainer
{
  AttDefContainer() = default;

  std::string Type;
  std::string BaseType;
  std::string Label;
  bool IsUnique = false;
  bool IsAbstract = false;
};
#endif // DataModelContainers_h
