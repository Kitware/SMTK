## Item Rotate Feature

Two new methods were added for rotating the elements contained in attribute items:

    bool smtk::GroupItem::rotate(
      std::size_t fromPosition, std::size_t toPosition)

    bool smtk::ValueItemTemplate::rotate(
      std::size_t fromPosition, std::size_t toPosition)

(Note: ValueItemTemplate is superclass to DoubleItem, IntItem, and StringItem.)

Each method takes in two std::size_t arguments and moves the element at
the first position argument to the position indicated by the second argument,
and shifts the remaining elements accordingly. The method returns true if
the rotation was successfully applied. The rotation is not applied if either
position argument is outside the range of the item's data, or if the
position arguments are equal to each other (which would be a no-op).

Potential use for these methods are to support drag and drop operations
in user interface code.
