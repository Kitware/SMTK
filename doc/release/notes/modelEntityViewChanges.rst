Changes to qtModelEntityView
----------------------------

* Selection of the Type of Attribute via the ComboBox no longer requires any button press after the selection has been made

* Also addressed a bug related to the View being updated when selecting a different attribute to be assigned to the model entity.  The symptom was that instead of accepting the requested type, the View would instead display "Please Select" indicating that the model entity had no attribute associated with it. The reason was due to the fact that the View was being updated via the call to removeAttribute prior to the new attribute being created.  The fix was to postpone calling that method until after the new attribute was in place.
