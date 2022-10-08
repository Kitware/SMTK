Qt UI for extensible file items
-------------------------------

Previously, the file-browser dialog for all filesystem items
– even extensible ones – only allowed you to select a single
file at a time. Now extensible filesystem items allow you to
choose multiple files and the qtItem subclass will add entries
as needed to hold your list of selected files (if possible).
