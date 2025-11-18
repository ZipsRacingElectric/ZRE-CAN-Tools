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