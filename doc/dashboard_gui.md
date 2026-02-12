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

Every page of the GUI is made up of widgets. As most information displayed on the dashboard is sourced from the vehicle's CAN bus(ses), it is convenient to have a set of widgets specialized for such. This is the purpose of the family of CAN widgets.

Functionally a CAN widget is just a normal GTK widget that has a specialized `update` function. The `update` function is responsible for fetching any required information from the application's `canDatabase_t` and updating the visual component of the widget based on this.

The dashboard's CAN widgets are polymorphic, meaning they all 'look the same' to users. The appeal of this is that CAN widgets can be loaded in from configurations in the same way GUI pages are.

In the configuration of a page, a certain fields may be expect a CAN widget's configuration as the datatype. Inside this configuration, same as with the page configurations, the `type` field indicates the C datatype of the CAN widget to be instanced. This type indicates how the rest of the configuration is to be interpreted.

For example (inside of a page configuration):
```
{
	// This page has an array of CAN widgets it renders in a specific location.
	// The configs are specified as elements of this array.
	"widgets":
	[
		// This is the configuration of the first CAN widget.
		{
			// The datatype of the CAN widget. Here we are instancing a canLabelFloat_t.
			"type": "canLabelFloat_t",

			// Widget-specific configuration starts here...

			// Name of the CAN database signal to bind to.
			"signalName": "SPEED",

			// The format string to use for valid data.
			"formatValue": "%02.0f",

			// The format string to use for invalid data.
			"formatInvalid": "%s"
		},

		// Other widget configurations here...
	]
}
```