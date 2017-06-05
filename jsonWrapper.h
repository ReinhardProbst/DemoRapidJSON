/*
 * jsonWrapper.h
 *
 *  Created on: Jul 31, 2014
 *      Author: dirk
 */

#ifndef JSONWRAPPER_H_
#define JSONWRAPPER_H_


#include <unordered_map>
#include <vector>
#include <string>

enum JsonDataType {JSON_STRING, JSON_INT, JSON_UINT, JSON_DOUBLE, JSON_BOOL, JSON_OBJECT,
                   JSON_STRINGARRAY, JSON_INTARRAY, JSON_UINTARRAY, JSON_DOUBLEARRAY, JSON_BOOLARRAY, JSON_OBJECTARRAY
                  };

struct JsonBinaryStructMapInfo {
    JsonDataType	jsonDataType;
    uint32_t        offsetInBinaryStruct;
    uint32_t		sizeInBinaryStruct;
};

typedef void* ParserHandle;
typedef void* ValueHandle;
typedef void* InterpreterObjectHandle;


ParserHandle JSON_parserNew();

ParserHandle JSON_parserNew(std::unordered_map<std::string, std::unordered_map<std::string, JsonBinaryStructMapInfo> >*	pInterpreter);

bool JSON_parse(ParserHandle docHandle, char* jsonString);

void JSON_parserDelete(ParserHandle hDoc);

ValueHandle 		JSON_getMemberValue(ParserHandle hDoc, const char* jsonMemberName);

const char*			JSON_getOutString(ParserHandle hDoc);



bool				JSON_isInt(ValueHandle hVal);

int 				JSON_getIntValue(ValueHandle hVal);

void				JSON_setIntValue(ValueHandle hVal, int anInt);



bool				JSON_isUint(ValueHandle hVal);

unsigned int		JSON_getUintValue(ValueHandle hVal);

void				JSON_setUintValue(ValueHandle hVal, unsigned int aUint);



bool				JSON_isString(ValueHandle hVal);

char* 				JSON_getStringValue(ValueHandle hVal);

void 				JSON_setStringValue(ValueHandle hVal, char* aString);



bool				JSON_isDouble(ValueHandle hVal);

double				JSON_getDoubleValue(ValueHandle hVal);

void				JSON_setDoubleValue(ValueHandle hVal, double aDouble);



bool				JSON_isBool(ValueHandle hVal);

bool				JSON_getBoolValue(ValueHandle hVal);

void				JSON_setBoolValue(ValueHandle hVal, bool aBool);



bool				JSON_isArray(ValueHandle hVal);

unsigned int		JSON_getArraySize(ValueHandle hVal);

ValueHandle			JSON_getArrayElement(ValueHandle hVal, unsigned int idx);



bool				JSON_isObject(ValueHandle hVal);






// create a new object-interpreter for objects of a given name (in the JSON tree)
InterpreterObjectHandle JSON_parserNewObject(ParserHandle hDoc, const char* jsonObjectName);

// add a member to an object
bool JSON_parserObjectAddMember(InterpreterObjectHandle interpreterObjectHandle, const char* member, JsonDataType dataType, uint32_t offset, uint32_t size);

// apply an interpreter to a parsed document to produce binary data
uint32_t JSON_TextToBin(ParserHandle hDoc, char* jsonString, unsigned char* binBuffer, uint32_t binBufferSize);
// reverse
uint32_t JSON_BinToText(ParserHandle hDoc, unsigned char* binBuffer);


#endif /* JSONWRAPPER_H_ */
