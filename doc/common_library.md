# ZRE-CAN-Tools Common Library - Zips Racing
ZRE-CAN-Tools has a custom library used by all applications in the project. This library contains general code used for debugging, error handling, code portability, and more. Some of the general highlights of this library are listed here.

## Debugging
The `debug.h` header is used for helping both the users and developers to debug the behaviour of this project's applications and libraries. In order to use this module, the `debugInit` function should be called at the very beginning of the application.

The `debugPrintf` function is used to log debug information. Normally this information is not important and can be overwhelming, however it can be vital in determining why an issue occurred. This can be used by both libraries and applications. Normally anything printed to this is ignored, however a destination for this information can be specified by the `debugSetStream` function. Typically, if the verbose option is set, this will be set to `stderr` stream. This can also be set to a hard-coded log file to log everything in the background.

## Error Handling

Most custom functions in this project use `errno` for indicating errors. The project provides an extension to the regular `errno` values indicating custom error types. Upon failure, most functions will set `errno` to the error code and return failure. These codes are defined in the `error_codes.h` header.

The `errorPrintf` function, defined in `debug.h`, is used to print error information to the user, via `stderr`. This function will generate a message based on the value stored in `errno` and postfix it to the user provided message. This function also returns the value retrieved from `errno`, making it effective in one-liners. For example:

```
canDevice_t* device = canInit (deviceName);
if (device == NULL)
	return errorPrintf ("Failed to initialize CAN device '%s'", deviceName);
```

Error-specific behavior can also triggered by testing the value of `errno`. In addition to the set of errors defined in the C standard library's `errno.h`, all custom errors are defined in `error_codes.h`.

## Standard Options

The `options.h` header provides some functions for handling common options that all applications should support. This also implements a standard form for specifying options that should be consistent across all applications. This form is as follows:

`application-name <Options> <Required Arguments>`

Where options may take the following form

`-a`, `-b`, `-c`, etc. for single character options.

`--abc`, `--def`, etc. for string options.

Both character options and string options can be followed by `=<Value>` for any values associated with the option. Note this means that character options need not be a single character, rather the identifier of the option is only a single character.

For instance, `-m=100` is a character option identifed by `m` with a value of `100`.

At the beginning of each program, before validating the number of required arguments, the `handleOptions` function should be used to check for and handle all the program's options. If finer control is required, the `handleOptions` function can be used to individually handle arguments. A simple example of how to use these is provided below:

```
// At top-level scope
void handleA (char option, char* value)
{
	// Only using this once, so we know option == 'a'.
	(void) option;

	// If a value is given, print it, ignore otherwise.
	if (value == NULL)
		printf ("Option A\n");
	else
		printf ("Option A = \"%s\"\n", value);
}

void handleBCD (char option, char* value)
{
	// If a value is given, print it, ignore otherwise.
	if (value == NULL)
		printf ("Option %c\n", option);
	else
		printf ("Option %c = \"%s\"\n", option, value);
}

void handleStr (char* option, char* value)
{
	// If a value is given, print it, ignore otherwise.
	if (value == NULL)
		printf ("Option %s\n", option);
	else
		printf ("Option %s = \"%s\"\n", option, value);
}

// In main...

// Handle program options
// - Note we use a pointer to argc / argv. This function modifies their values so argv only contains the unparsed values on
// exit.
if (handleOptions (&argc, &argv, &(handleOptionsParams_t)
{
	.fprintHelp		= NULL,
	.chars			= (char []) { 'a', 'b', 'c', 'd' },
	.charHandlers	= (optionCharCallback_t* []) { handleA, handleBCD, handleBCD, handleBCD },
	.charCount		= 4,
	.strings		= (char* []) { "str0", "str1", "str2" },
	.stringHandlers	= (optionStringCallback_t* []) { handleStr, handleStr, handleStr },
	.stringCount	= 3
}) != 0)
	return errorPrintf ("Failed to parse options");

// Only the required arguments are left, so do something with them...
for (int index = 0; index < argc; ++index)
	printf ("Arg: \"%s\"\n", argv [index]);
```

```
// Handle program options
for (int index = 1; index < argc; ++index)
{
	switch (handleOption (argv [index], NULL, fprintHelp))
	{
	case OPTION_CHAR:
	case OPTION_STRING:
		fprintf (stderr, "Unknown argument '%s'.\n", argv [index]);
		return -1;

	case OPTION_QUIT:
		return 0;

	default:
		break;
	}
}
```

If the application's custom options can be handled at this time, they should be. If they cannot be handled until after the argument count is handled, then this iteration can be performed again after said validation.

## Custom Compilation Macros
ZRE-CAN-Tools uses several custom macros during compilation. This is a comprehensive list of what they are and how to use them.

```
ZRE_CANTOOLS_OS_<OS Type>           Macro indicating what operating system the
                                    application has been compiled for. Intended
                                    for conditional compilation via
                                    preprocessor directives.
    ZRE_CANTOOLS_OS_linux           The application has been compiled for Linux.
	ZRE_CANTOOLS_OS_windows         The application has been compiled for Windows.

ZRE_CANTOOLS_OS_TYPE				String indicating what operating system the
                                    application has been compiled for.

ZRE_CANTOOLS_ARCH_TYPE              String indicating what architecture the
                                    application has been compiled for.

ZRE_CANTOOLS_VERSION_NUMBER         String indicating the version number of the
                                    application. In the form <YYYY.MM.DD>.

ZRE_CANTOOLS_VERSION_FULL           String indicating the full form of the
									application version. In the form:
	zre_cantools_<OS Type>_<Arch Type>_<Version Number>
```