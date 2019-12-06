## Changes to Qt View System

### Improving Observer Stability
It was discovered that passing the **this** pointer into an Observer's lambda expression in a Qt-based class can cause a crash if the object is deleted while there is an event still in the Qt Event loop.  The solution is to use a QPointer instead so that it can be tested for nullptr.

### Adding Observers to Views
* Observation for attribute creation, modification, and expungement have been added to qtAttributeView.
* Observation for attribute modification has been added to qtInstanceView

### Changes to qtGroupItem
* The first Column is no longer marked with 1 for extensible groups.
* Fixed issue with updating extensible qtGroupItems due to the number of columns being set to 0 instead of 1

### API Changes
These changes were made to help simplify/cleanup the qtView infrastructure.  There were several places where onShowCategory() was being called in order to update the UI.  This resulted in confusion as to the role of the method.  In many cases these calls have been replaced with updateUI.

* **qtBaseView::updateViewUI - has been removed.** It was not being used.
* **qtBaseAttributeView::updateAttributeData - has been removed.** This method's role was to update the attribute content of a View.  You should now call updateUI() instead.
* qtBaseAttributeView no longer overrides updateUI()
