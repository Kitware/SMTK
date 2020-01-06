## View factory redesign

- As part of a redesign of the view subsystem, view::View is being renamed to view::Configuration since its purpose is to hold a specification of a view rather than the view itself. In future releases, view::View will be reintroduced as a class responsible for presenting a view (in a way independent of any GUI library).
- The view::Manager class has taken over responsibility for creating qtBaseView-derived classes from qtUIManager, and uses the Registrar pattern.
