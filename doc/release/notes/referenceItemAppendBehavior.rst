Changing ReferenceItem::AppendValue to Support Append Non-Unique
============================================
* Added a nonUnique parameter that defaults to false
* This avoids unnecessarily having to scan the entire item when duplicates are allowed
* Item now also tracks the location of the first unset value in order to speed up the append process
