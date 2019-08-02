## Changes to the View System and Qt Extensions

### Changes to qtBaseView

* The qtBaseView class has been split into qtBaseView and qtBaseAttributeView.
  All of the existing qtBaseView subclasses now instead inherit qtBaseAttributeView.
* The displayItem test now calls 2 new methods categoryTest and advanceLevelTest.  This makes it easier for derived classes to override the filtering behavior
