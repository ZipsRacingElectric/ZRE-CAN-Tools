#ifndef CJSON_UTIL_H
#define CJSON_UTIL_H

// Includes -------------------------------------------------------------------------------------------------------------------

// cJSON
#include "cjson.h"

// C Standard Library
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

// Functions ------------------------------------------------------------------------------------------------------------------

cJSON* jsonLoadFile (const char* path);

cJSON* jsonPrompt (FILE* stream);

// TODO(Barach): Not a fan of this

bool jsonGetObject (cJSON* json, const char* key, cJSON** object);

bool jsonGetString (cJSON* json, const char* key, char** value);

bool jsonGetUint16_t (cJSON* json, const char* key, uint16_t* value);

bool jsonGetUint32_t (cJSON* json, const char* key, uint32_t* value);

#endif // CJSON_UTIL_H