### Changes to Categories
#### Original Design

Attribute Category Information is currently stored as a set of string (representing the names of the categories) and a combination mode.  When inheriting the information (currently only the set of strings are passed to parent items, attributes and possibly children items (based on their inherit category mode).

#### The Problem
Without the combination mode, the inherited categories allow attributes and items to be displayed with no internal children.  Consider the following example:
* Attribute Definition A has no local categories but contains 2 Items
* Item i1 has local categories (c1, c2) with combination mode ALL
* Item i2 has local categories (c3, c4) with combination mode ALL

In the current system A inherits categories (c1, c2, c3, c4) with mode ANY. If the GUI is displaying information associated with c1, any attributes of type A will be displayed but will have no content.

#### New Design
Replace the std::set\<std:string> , mode representation of category information with a class called smtk::attribute::Categories that provides the following functionality:
* Represents sets of categories along with the combination mode
* Provides a mechanism of combining 2 Categories instances
* Provides a passes method that takes in a set of category strings and returns true if the Categories instance would be "include" in that set.

```c++
namespace smtk {
namespace attribute {
class Categories
{
   class Set
   {
      public:
         enum class CombinationMode
            { Any, All };
         CombinationMode mode() const { return m_mode;}
         void setMode(const Set::CombinationMode& newMode);
         const std::set<std::string>& values { return m_set;}
         void set(const std::set<std::string>& values, CombinationMode mode);
         bool empty() const { return m_categoryNames.empty(); }
         std::size_t size() const { return m_categoryNames.size(); }
         bool operator<(const Set& rhs) const;
      private:
         Combination m_mode;
         std::set<std::string> m_set;
   };
public:
   bool passes(const std::set<std::string>& cats) const;
   void append(const Set& set) { m_sets.insert(set); }
   void reset() { m_sets.clear(); }
   const std::set<Set>& sets() const { return m_sets;}
   std::size_t size() const { return m_sets.size(); }
private:
   std::set<Set> m_sets;
};
};
};
```
#### API Changes
* Attribute
  * New Methods
     * categories() - returns a reference to the Categories object associated with the Attribute
  * Removed Methods
     * isMemberOf(...) -> replace with categories().passes(...)
* Definition
  * Changed Methods
     * categories() - now returns a reference to the Categories object associated with the Definition
     * localCategories() - now returns a reference to the Categories::Set object associated with the Definition's local categories
     * applyCategories - method now takes in Categories and a set of strings
  * Removed Methods
     * isMemberOf(...) -> replace with categories().passes(...)
     * numberOfCategories() - replaced with categories().size()
     * addLocalCategories(...) - replaced with localCategories().insert(...)
     * removeLocalCategories(...) - replaced with localCategories().erase(...)
* ItemDefinition
  * Changed Methods
     * categories() - now returns a reference to the Categories object associated with the ItemDefinition
     * localCategories() - now returns a reference to the Categories::Set object associated with the ItemDefinition's local categories
     * applyCategories - method now takes in Categories and a set of strings
   * CategoryCheckMode Enums removed - replaced by Categories::Set::CombinationMode
  * Removed Methods
     * isMemberOf(...) -> replace with categories().passes(...)
     * numberOfCategories() - replaced with categories().size()
     * addLocalCategories(...) - replaced with localCategories().insert(...)
     * removeLocalCategories(...) - replaced with localCategories().erase(...)
     * setCategoryCheckMode(...) - replaced with localCategories().setMode(...)
     * categoryCheckMode() - replaced with localCategories().mode()
* Item
  * New Methods
     * categories() - returns a reference to the Categories object associated with the Item
  * Removed Methods
     * isMemberOf(...) -> replace with categories().passes(...)
     * passCategoryCheck(...) - replaced with categories().passes(...)
* ValueItemDefinition
   * Changed Methods
       * setEnumCategories - now takes in Categories::Set instead of a set of strings
       * enumCategories - now returns a const reference to a Categories::Set instead of a set of strings

#### Notes
##### Enum Category Constraints
Category constraints placed on enums are inherited by the ValueItem but not by the ValueItem's children.
