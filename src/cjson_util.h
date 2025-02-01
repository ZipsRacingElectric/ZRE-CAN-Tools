#ifndef CJSON_UTIL_H
#define CJSON_UTIL_H

// Includes -------------------------------------------------------------------------------------------------------------------

// cJSON
#include <cjson.h>

// C Standard Library
#include <stdio.h>
#include <stdint.h>

// Functions ------------------------------------------------------------------------------------------------------------------

cJSON* jsonLoad (const char* path);

cJSON* jsonRead (FILE* stream);

int jsonGetObject (cJSON* json, const char* key, cJSON** object);

int jsonGetString (cJSON* json, const char* key, char** value);

int jsonGetUint16_t (cJSON* json, const char* key, uint16_t* value);

int jsonGetUint32_t (cJSON* json, const char* key, uint32_t* value);

#endif // CJSON_UTIL_H