Removed Deprecated API
====
The following deprecated methods have been removed:

* Categories::Set::mode has been replaced with Categories::Set::inclusionMode
* Categories::Set::setMode has been replaced with Categories::Set::setInclusionMode
* Categories::Set::categoryNames has been replaced with Categories::Set::includedCategoryNames
* Categories::Set::set has been replaced with Categories::Set::setInclusions
* Categories::Set::insert has been replaced with Categories::Set::insertInclusion
* Categories::Set::erase has been replaced with Categories::Set::eraseInclusion
* Categories::Set::size has been replaced with Categories::Set::inclusionSize
* ReferenceItem::objectValue has been replaced with ReferenceItem::value
* ReferenceItem::setObjectValue has been replaced with ReferenceItem::setValue
* ReferenceItem::appendObjectValue has been replaced with ReferenceItem::appendValue
* ReferenceItem::setObjectValues has been replaced with ReferenceItem::setValues
* ReferenceItem::appendObjectValues has been replaced with ReferenceItem::appendValues
* PhraseModel::addSource now accepts const smtk::common::TypeContainer&
* PhraseModel::removeSource now accepts const smtk::common::TypeContainer&
