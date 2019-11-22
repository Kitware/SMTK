## Changes to Qt View System

### Improving Observer Stability
It was discovered that passing the **this** pointer into an Observer's lambda expression in a Qt-based class can cause a crash if the object is deleted while there is an event still in the Qt Event loop.  The solution is to use a QPointer instead so that it can be tested for nullptr.

### Adding Observers to Views
* Observation for attribute creation, modification, and expungement have been added to qtAttributeView.
* Observation for attribute modification has been added to qtInstanceView

### Changes to qtGroupItem
* The first Column is no longer marked with 1 for extensible groups.
* Fixed issue with updating extensible qtGroupItems due to the number of columns being set to 0 instead of 1
