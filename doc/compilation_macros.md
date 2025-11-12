# Custom Compilation Macros
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