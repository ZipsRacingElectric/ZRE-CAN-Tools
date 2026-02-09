# Dashboard GUI Application - Zips Racing

The dashboard GUI, also referred to as `dashboard-gui`, is the graphical user interface for the dashboard of Zips Racing's vehicles.

## Application Structure

At its core, the dashboard GUI is simply a set of "pages" to render to the user. Each page is designed to display a specific set of information. This information is displayed using "widgets". A widget is a simple, re-usable, component of a GUI, for instance, a label or a button.

### The Configuration File

In order to be more flexible, the layout of the GUI is loaded in from a configuration file. This configuration system is what allows a single application to act as the dashboard for *any* of Zips Racing's vehicles. The application's configuration, like most in this project, is based on JSON files. A template configuration file is provided below. Note the comments (denoted by "//") are not part of the file and must be removed to make the file valid.

```
{
    // Name to give the application. Appears in title bar.
    "name": <Application Name>,

    // Base style to apply to all pages in the application. Unless overridden,
    // all pages inherit this style.
    "baseStyle":
    {
        "backgroundColor":          "#000000",
        "fontColor":                "#FFFFFF",
        "borderColor":              "#AAAAAA",
        "borderThickness":          "1.5",
        "indicatorActiveColor":     "#FF0000",
        "indicatorInactiveColor":   "#580000",
        "buttonHeight":             "80",
        "buttonFont":               "Futura Std Bold Condensed 34px"
    },
    // Array of pages to load.
    "pages":
    [
        {
            // Datatype of the page. Exact match with the C datatype, or "null" to not load a page.
            "type": <Page 0 Datatype>,

            // The user-friendly name to assign the page. Appears in the button panel.
            "name": <Page 0 Name>,

            // Page-specific style.
            "style":
            {
                // Optional override to the application's base style. Any
                // member valid in the application's base style is valid here.
                "baseStyle":
                {
                    // Overridden fields...
                }
            },

            // Page-specific fields...
        },
        {
            "type":  <Page 1 Datatype>,
            "name":  <Page 1 Name>,
            "style": {},
        },
        ...
        {
            "type":  <Page N-1 Datatype>,
            "name":  <Page N-1 Name>,
            "style": {},
        }
    ]
}
```

### Pages

Each page represents a specific "view" of the application. Like the application itself, pages are loaded in via a configuration.

The `type` field indicates the type of page to instance. The value of this field should be an exact match to the C datatype of a specific page implementation (ex. `pageDrive_t`). This indicates both how the remainder of the configuration will be parsed and how the page will be structured.

The `name` field indicates the user-friendly name of the page instance. This is the text that is placed on the page's entry in the application's button panel.

The `style` field is a page-specific configuration on how to stylize the page. All pages support the `baseStyle` field, which may be used to override enties in the application's `baseStyle` field.

### CAN Widgets

TODO(Barach)...