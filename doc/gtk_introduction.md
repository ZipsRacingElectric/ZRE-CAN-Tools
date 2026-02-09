# GTK Introduction - Zips Racing

GTK is the library Zips Racing uses for creating custom GUI applications. GTK is ideal as it is a cross-platform, C library that greatly simplifies the process of creating graphical applications.

For a general introduction to GTK, see the tutorial written by the GTK developers:

https://docs.gtk.org/gtk4/getting_started.html

Note that most sections after the *Building user interfaces* Section are not really relevant to our applications, as they deal with a method of creating GUIs that ZRE-CAN-Tools does not utilize.

For more in-depth resources, the GTK API documentation is useful. As it is an object-oriented library, the most useful information is in the class documentation of GTK's classes. The *GTK Classes Hierarchy* is a good reference for finding specific class documentation.

https://docs.gtk.org/gtk4/classes_hierarchy.html

Note that GTK classes utilize inheritance quite often. Sometimes the documentation you are looking for may be found in a parent class's documentation.

## Secondary Material

An unofficial, but more technical tutorial is linked below. This goes into more detail about the exact functions and objects needed for certain circumstances. The sections that are most relevant to our applications are also listed below.

https://toshiocp.github.io/Gtk4-tutorial/

- 3. GtkApplication and GtkApplicationWindow
- 4. Widgets(1)
- 12. Signals
- 23. Pango, CSS and Application
- 24. GtkDraingArea and Cairo
- 25. Periodic Events