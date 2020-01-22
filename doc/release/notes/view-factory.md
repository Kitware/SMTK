## View factory redesign

- As part of a redesign of the view subsystem, view::View is being renamed to view::Configuration since its purpose is to hold a specification of a view rather than the view itself. In future releases, view::View will be reintroduced as a class responsible for presenting a view (in a way independent of any GUI library).
- The view::Manager class has taken over responsibility for creating qtBaseView-derived classes from qtUIManager, and uses the Registrar pattern.
- The cmake function `smtk_plugin_add_ui_view()` has been removed, and all qtBaseView-derived classes have been added to a Registrar. External plugins will need to make a similar change.
