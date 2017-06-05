//============================================================================
// Name        : jsonWrapper.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Simple wrapper for parsing JSON strings, Ansi-style
//============================================================================

#include <iostream>
#include <string>
#include <cassert>

#include <stdint.h>

#if defined(OL91)
#include <el/osal/logger.h>
#endif

#include "jsonWrapper.h"

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

using namespace rapidjson;

// the standard malloc/free allocator crashes when used within the execution context of the softPLC
// Here's my own Allocator based on new and delete
/*
class MyAllocator_Orig {
public:
    static const bool kNeedFree = true;
    void* Malloc(size_t size) { return new unsigned char[size]; }
    void* Realloc(void* originalPtr, size_t originalSize, size_t newSize) {
        void* newPtr = Malloc(newSize);
        memcpy(newPtr, originalPtr, originalSize);
        delete[] originalPtr;
        return newPtr; }
    static void Free(void *ptr) { delete[] ptr; }
};
*/
class MyAllocator_New {
  public:
    static const bool kNeedFree = true;
    void* Malloc(size_t size) {
        return new unsigned char[size];
    }
    void* Realloc(void* originalPtr, size_t originalSize, size_t newSize) {
        void* newPtr = Malloc(newSize);
        memcpy(newPtr, originalPtr, originalSize);
        Free(originalPtr);
        return newPtr;
    }
    static void Free(void *ptr) {
        delete[] (unsigned char*)ptr;
    }
};
/////Use memory leak free allocator
typedef MyAllocator_New MyAllocator;

/////Back to standard allocator
//typedef MemoryPoolAllocator<> MyAllocator;

// define types for document and value based on the new Allocator
typedef GenericDocument<UTF8<>, MyAllocator > MyDocument;
typedef GenericValue<UTF8<>, MyAllocator > MyValue;

#include <unordered_map>
#include <vector>

struct RW_Parser {
    MyDocument* 			pDocument;
    StringBuffer* 			pBuffer;
    Writer<StringBuffer>* 	pWriter;
    // outer(object-name    inner (member-name  type/offset/size       ) )  interpreter
    std::unordered_map<std::string, std::unordered_map<std::string, JsonBinaryStructMapInfo> >*	pInterpreter;
    bool					interpreterAllocated;
};

// A JSON object consists of a list of JSON-members. Each member has a name and
// 	some binary mapping info consisting of datatype, offset & size.
// The inner map below stores the members of an object.
// The outer map below stores the object of an interpreter.

// use this to create interpreter step by step
ParserHandle JSON_parserNew() {
    RW_Parser* pDocStrBufWriter = new RW_Parser();

    pDocStrBufWriter->pDocument = new MyDocument();
    pDocStrBufWriter->pBuffer = new StringBuffer();
    pDocStrBufWriter->pWriter = new Writer<StringBuffer>(*(pDocStrBufWriter->pBuffer));

    pDocStrBufWriter->pInterpreter = new std::unordered_map<std::string, std::unordered_map<std::string, JsonBinaryStructMapInfo> >;
    pDocStrBufWriter->interpreterAllocated = true;

    return pDocStrBufWriter;
}

// use this to use a pre-initialized interpreter
ParserHandle JSON_parserNew(std::unordered_map<std::string, std::unordered_map<std::string, JsonBinaryStructMapInfo> >*	pInterpreter) {
    RW_Parser* pDocStrBufWriter = new RW_Parser();

    pDocStrBufWriter->pDocument = new MyDocument();
    pDocStrBufWriter->pBuffer = new StringBuffer();
    pDocStrBufWriter->pWriter = new Writer<StringBuffer>(*(pDocStrBufWriter->pBuffer));

    pDocStrBufWriter->pInterpreter = pInterpreter;
    pDocStrBufWriter->interpreterAllocated = false;

    return pDocStrBufWriter;
}

bool JSON_parse(ParserHandle docHandle, char* jsonString) {
    // 1. Parse a JSON string into DOM.

    RW_Parser* pDocStrBufWriter = (RW_Parser*)docHandle;

    pDocStrBufWriter->pDocument->ParseInsitu(jsonString);

    if (pDocStrBufWriter->pDocument->HasParseError()) {
        // if there's a parsing error do not return a document
        return false;
    }

    return true;
}

void JSON_parserDelete(ParserHandle hDoc) {
    // also clean up a possibly attached writer and buffer
    if (((RW_Parser*)hDoc)->pDocument)
        delete ((RW_Parser*)hDoc)->pDocument;

    if (((RW_Parser*)hDoc)->pBuffer)
        delete ((RW_Parser*)hDoc)->pBuffer;

    if (((RW_Parser*)hDoc)->pWriter)
        delete ((RW_Parser*)hDoc)->pWriter;

    if ((((RW_Parser*)hDoc)->pInterpreter)
            && (((RW_Parser*)hDoc)->interpreterAllocated))
        delete ((RW_Parser*)hDoc)->pInterpreter;

    delete ((RW_Parser*)hDoc);
}

ValueHandle JSON_getMemberValue(ParserHandle hDoc, const char* jsonMemberName) {
    // NOTE: "s" does not need to be cleaned up because it's a reference to a member
    // in the DOM model of the document
    MyValue* s = &(*((RW_Parser*)hDoc)->pDocument)[jsonMemberName];

    return s;
}

const char* JSON_getOutString(ParserHandle hDoc) {

    // Attach writer and stringbuffer if they have not been created for this document yet.
    // The user of this wrapper does not want to know about the existence of a stringbuffer nor writer.

    if (!(((RW_Parser*)hDoc)->pBuffer))
        ((RW_Parser*)hDoc)->pBuffer = new StringBuffer();

    ((RW_Parser*)hDoc)->pBuffer->Clear();

    if (!(((RW_Parser*)hDoc)->pWriter))
        ((RW_Parser*)hDoc)->pWriter = new Writer<StringBuffer>(*(((RW_Parser*)hDoc)->pBuffer));

    ((RW_Parser*)hDoc)->pWriter->Reset(*(((RW_Parser*)hDoc)->pBuffer));

    (((RW_Parser*)hDoc)->pDocument)->Accept(*(((RW_Parser*)hDoc)->pWriter));

    return ((RW_Parser*)hDoc)->pBuffer->GetString();
}

bool JSON_isInt(ValueHandle hVal) {
    return ((MyValue*)hVal)->IsInt();
}

int JSON_getIntValue(ValueHandle hVal) {
    return ((MyValue*)hVal)->GetInt();
}

void JSON_setIntValue(ValueHandle hVal, int anInt) {
    ((MyValue*)hVal)->SetInt(anInt);
}




bool JSON_isUint(ValueHandle hVal) {
    return ((MyValue*)hVal)->IsUint();
}

unsigned int JSON_getUintValue(ValueHandle hVal) {
    return ((MyValue*)hVal)->GetUint();
}

void JSON_setUintValue(ValueHandle hVal, unsigned int aUint) {
    ((MyValue*)hVal)->SetUint(aUint);
}




bool JSON_isString(ValueHandle hVal) {
    return ((MyValue*)hVal)->IsString();
}

char* JSON_getStringValue(ValueHandle hVal) {
    return (char*)((MyValue*)hVal)->GetString();
}

void JSON_setStringValue(ValueHandle hVal, char* aString) {
    ((MyValue*)hVal)->SetString(aString, strlen(aString));
}




bool JSON_isDouble(ValueHandle hVal) {
    return ((MyValue*)hVal)->IsDouble();
}

double JSON_getDoubleValue(ValueHandle hVal) {
    return ((MyValue*)hVal)->GetDouble();
}

void JSON_setDoubleValue(ValueHandle hVal, double aDouble) {
    ((MyValue*)hVal)->SetDouble(aDouble);
}



bool JSON_isBool(ValueHandle hVal) {
    return ((MyValue*)hVal)->IsBool();
}

bool JSON_getBoolValue(ValueHandle hVal) {
    return ((MyValue*)hVal)->GetBool();
}

void JSON_setBoolValue(ValueHandle hVal, bool aBool) {
    ((MyValue*)hVal)->SetBool(aBool);
}




bool JSON_isArray(ValueHandle hVal) {
    return ((MyValue*)hVal)->IsArray();
}

unsigned int JSON_getArraySize(ValueHandle hVal) {
    return ((MyValue*)hVal)->Size();
}

ValueHandle	JSON_getArrayElement(ValueHandle hVal, unsigned int idx) {
    MyValue* s = &(((MyValue*)hVal)[idx]);

    return s;
}

// NOTE: "s" does not need to be cleaned up because it's a reference to a member
// in the DOM model of the document


bool JSON_isObject(ValueHandle hVal) {
    return ((Value*)hVal)->IsObject();
}







uint32_t RecurseInterpret(
    GenericValue<UTF8<char>, MyAllocator>& jsonObject,
    std::unordered_map<std::string, std::unordered_map<std::string, JsonBinaryStructMapInfo> >* pInterpreter,
    std::unordered_map<std::string, JsonBinaryStructMapInfo>& jsonObjectMapping,
    unsigned char* binBuffer,
    uint32_t binBufferSize);

uint32_t RecurseWrite(
    MyAllocator& myAlloc,
    GenericValue<UTF8<char>, MyAllocator>& jsonObject,
    std::unordered_map<std::string, std::unordered_map<std::string, JsonBinaryStructMapInfo> >* pInterpreter,
    std::unordered_map<std::string, JsonBinaryStructMapInfo>& jsonObjectMapping,
    unsigned char* binBuffer);


InterpreterObjectHandle JSON_parserNewObject(ParserHandle hDoc, const char* jsonObjectName) {
    // create a new (empty) map entry for the object
    std::unordered_map<std::string, JsonBinaryStructMapInfo> jsonMemberDescrVect;

    // add it to the map
    (*(((RW_Parser*)hDoc)->pInterpreter))[std::string(jsonObjectName)] = jsonMemberDescrVect;

    return (InterpreterObjectHandle)&((*(((RW_Parser*)hDoc)->pInterpreter))[std::string(jsonObjectName)]);
}

bool JSON_parserObjectAddMember(InterpreterObjectHandle interpreterObjectHandle, const char* member, JsonDataType dataType, uint32_t offset, uint32_t size) {
    std::unordered_map<std::string, JsonBinaryStructMapInfo> * pJsonMemberDescrVect = (std::unordered_map<std::string, JsonBinaryStructMapInfo> *)interpreterObjectHandle;

    (*pJsonMemberDescrVect)[std::string(member)] = {dataType, offset, size};

    return true;
}


/*
	// diagnostic output to check table-to-map-conversion
	for (auto& object : g_jsonObjectMapping) {

		std::cout << "\tDescription for object: " << object.first << std::endl;

		for (auto& member : object.second) {
			std::cout << "\tMember: " << member.first << std::endl;
			std::cout << "\t\tDatatype: " << member.second.jsonDataType << std::endl;
			std::cout << "\t\tOffset to Struct: " << member.second.offsetInBinaryStruct << std::endl;
			std::cout << "\t\tSize: " << member.second.sizeInBinaryStruct << std::endl;
		}

	}
*/

uint32_t JSON_TextToBin(ParserHandle hDoc, char* jsonString, unsigned char* binBuffer, uint32_t binBufferSize) {

    assert(hDoc != NULL);

    // cast to pInterpreter
    std::unordered_map<std::string, std::unordered_map<std::string, JsonBinaryStructMapInfo> >* pInterpreter = (((RW_Parser*)hDoc)->pInterpreter);

    MyDocument* pDoc = ((RW_Parser*)hDoc)->pDocument;

    // 1. Parse a JSON string into DOM.
    bool bResult = JSON_parse(hDoc, jsonString);
    if (!bResult) {
        std::cout << "JSON parsing error\n";
        return 10;
    }

    // Do a standard interpretation, pass the GenericDocument as the GenericValue
    // (1st param, a GenercDocument is derived from GenericValue)
    return RecurseInterpret(*pDoc,
                            pInterpreter,
                            (*pInterpreter)[""],
                            binBuffer,
                            binBufferSize);
}

uint32_t JSON_BinToText(ParserHandle hDoc, unsigned char* binBuffer) {

    assert(hDoc != NULL);

    // cast to pInterpreter
    std::unordered_map<std::string, std::unordered_map<std::string, JsonBinaryStructMapInfo> >* pInterpreter = (((RW_Parser*)hDoc)->pInterpreter);

    // reuse the same document that may have been used for parsing
    // in any case the output string is created new from scratch
    MyDocument* pDoc = ((RW_Parser*)hDoc)->pDocument;

    // initially the document is an empty object ...
    pDoc->SetObject();

    // ... that gets filled recursively

    // Do a standard writing, pass the GenericDocument as the GenericValue
    // (2nd param, a GenercDocument is derived from GenericValue)
    // (for writing an allocator is needed, too)
    uint32_t retval = RecurseWrite(
                          pDoc->GetAllocator(),
                          *pDoc,
                          pInterpreter,
                          (*pInterpreter)[""],
                          binBuffer);

    return retval;
}

uint32_t RecurseInterpret(GenericValue<UTF8<char>, MyAllocator>& jsonObject, std::unordered_map<std::string, std::unordered_map<std::string, JsonBinaryStructMapInfo> >* pInterpreter, std::unordered_map<std::string, JsonBinaryStructMapInfo>& jsonObjectMapping,
                          unsigned char* binBuffer, uint32_t binBufferSize) {


    uint32_t returnCode = 0;

    // iterate over the member description array
    for (auto& member : jsonObjectMapping) {

        const char* memberName = member.first.c_str();

        // does the member exist?
        if (!jsonObject.HasMember(memberName)) {
            // indicate error to console & logfile
            std::cout << R"(JSON for PLC: ")" << memberName << R"(" not found.)" << std::endl;
#if defined(OL91)
            el_logff(LOG_NOTICE, "JSON for PLC: \"%s\ not found.\n", memberName);
#endif
            return 1;
        }

        // if it is of the expected type extract it
        switch (member.second.jsonDataType) {
        case JSON_STRING:
            if (!(jsonObject[memberName]).IsString()) {
                // indicate error to console & logfile
                std::cout << R"(JSON for PLC: ")" << memberName << R"(" is not a string.)" << std::endl;
#if defined(OL91)
                el_logff(LOG_NOTICE, "JSON for PLC: \"%s\ is not a string.\n", memberName);
#endif
                return 2; // wrong type
            }

            {
                const char* string = (jsonObject[memberName]).GetString();

                SizeType strLength = (jsonObject[memberName]).GetStringLength();

                if (strLength > member.second.sizeInBinaryStruct-1) {
                    // indicate error to console & logfile
                    std::cout << R"(JSON for PLC: ")" << memberName << R"(" insufficient binSize for string -> truncated.)" << std::endl;
#if defined(OL91)
                    el_logff(LOG_NOTICE, "JSON for PLC: \"%s\ insufficient binSize for string -> truncated.\n", memberName);
#endif
                    returnCode = 3; // string truncated

                    // truncate stringLength
                    strLength = member.second.sizeInBinaryStruct-1;
                }

                char* destStr = (char*)binBuffer + member.second.offsetInBinaryStruct;
                strncpy(destStr, string, strLength);
                //set terminating 0
                destStr[strLength] = (char)0;

            }
            break;

        case JSON_INT:
            if (!(jsonObject[memberName]).IsInt()) {
                // indicate error to console & logfile
                std::cout << R"(JSON for PLC: ")" << memberName << R"(" is not an int.)" << std::endl;
#if defined(OL91)
                el_logff(LOG_NOTICE, "JSON for PLC: \"%s\ is not an int.\n", memberName);
#endif
                return 2; // wrong type
            }

            if (member.second.sizeInBinaryStruct < sizeof(int32_t)) {
                // indicate error to console & logfile
                std::cout << R"(JSON for PLC: ")" << memberName << R"(" insufficient binSize for dataType.)" << std::endl;
#if defined(OL91)
                el_logff(LOG_NOTICE, "JSON for PLC: \"%s\ insufficient binSize for dataType.\n", memberName);
#endif
                return 4; // insufficient binSize for dataType
            }

            if (member.second.sizeInBinaryStruct > sizeof(int32_t)) {
                // indicate error to console & logfile
                std::cout << R"(JSON for PLC: ")" << memberName << R"(" too large binSize for dataType.)" << std::endl;
#if defined(OL91)
                el_logff(LOG_NOTICE, "JSON for PLC: \"%s\ too large binSize for dataType.\n", memberName);
#endif
                return 5; // too large binSize for dataType
            }

            {
                int32_t* pInt = (int32_t*)(binBuffer + member.second.offsetInBinaryStruct);

                *pInt = (jsonObject[memberName]).GetInt();
            }
            break;

        case JSON_UINT:
            if (!(jsonObject[memberName]).IsUint()) {
                // indicate error to console & logfile
                std::cout << R"(JSON for PLC: ")" << memberName << R"(" is not a uint.)" << std::endl;
#if defined(OL91)
                el_logff(LOG_NOTICE, "JSON for PLC: \"%s\ is not a uint.\n", memberName);
#endif
                return 2; // wrong type
            }

            if (member.second.sizeInBinaryStruct < sizeof(uint32_t)) {
                // indicate error to console & logfile
                std::cout << R"(JSON for PLC: ")" << memberName << R"(" insufficient binSize for dataType.)" << std::endl;
#if defined(OL91)
                el_logff(LOG_NOTICE, "JSON for PLC: \"%s\ insufficient binSize for dataType.\n", memberName);
#endif
                return 4; // insufficient binSize for dataType
            }

            if (member.second.sizeInBinaryStruct > sizeof(uint32_t)) {
                // indicate error to console & logfile
                std::cout << R"(JSON for PLC: ")" << memberName << R"(" too large binSize for dataType.)" << std::endl;
#if defined(OL91)
                el_logff(LOG_NOTICE, "JSON for PLC: \"%s\ too large binSize for dataType.\n", memberName);
#endif
                return 5; // too large binSize for dataType
            }

            {
                uint32_t* pUint = (uint32_t*)(binBuffer + member.second.offsetInBinaryStruct);

                *pUint = (jsonObject[memberName]).GetUint();
            }
            break;

        case JSON_DOUBLE:
            if (!(jsonObject[memberName]).IsDouble()) {
                // indicate error to console & logfile
                std::cout << R"(JSON for PLC: ")" << memberName << R"(" is not a double.)" << std::endl;
#if defined(OL91)
                el_logff(LOG_NOTICE, "JSON for PLC: \"%s\ is not a double.\n", memberName);
#endif
                return 2; // wrong type
            }

            if (member.second.sizeInBinaryStruct < sizeof(double)) {
                // indicate error to console & logfile
                std::cout << R"(JSON for PLC: ")" << memberName << R"(" insufficient binSize for dataType.)" << std::endl;
#if defined(OL91)
                el_logff(LOG_NOTICE, "JSON for PLC: \"%s\ insufficient binSize for dataType.\n", memberName);
#endif
                return 4; // insufficient binSize for dataType
            }

            if (member.second.sizeInBinaryStruct > sizeof(double)) {
                // indicate error to console & logfile
                std::cout << R"(JSON for PLC: ")" << memberName << R"(" too large binSize for dataType.)" << std::endl;
#if defined(OL91)
                el_logff(LOG_NOTICE, "JSON for PLC: \"%s\ too large binSize for dataType.\n", memberName);
#endif
                return 5; // too large binSize for dataType
            }

            {
                double* pDouble = (double*)(binBuffer + member.second.offsetInBinaryStruct);

                *pDouble = (jsonObject[memberName]).GetDouble();
            }
            break;

        case JSON_BOOL:
            if (!(jsonObject[memberName]).IsBool()) {
                // indicate error to console & logfile
                std::cout << R"(JSON for PLC: ")" << memberName << R"(" is not a bool.)" << std::endl;
#if defined(OL91)
                el_logff(LOG_NOTICE, "JSON for PLC: \"%s\ is not a bool.\n", memberName);
#endif
                return 2; // wrong type
            }

            // in IEC the size of a CODESYS-BOOL is one byte
            if (member.second.sizeInBinaryStruct < sizeof(char)) {
                // indicate error to console & logfile
                std::cout << R"(JSON for PLC: ")" << memberName << R"(" insufficient binSize for dataType.)" << std::endl;
#if defined(OL91)
                el_logff(LOG_NOTICE, "JSON for PLC: \"%s\ insufficient binSize for dataType.\n", memberName);
#endif
                return 4; // insufficient binSize for dataType
            }

            if (member.second.sizeInBinaryStruct > sizeof(char)) {
                // indicate error to console & logfile
                std::cout << R"(JSON for PLC: ")" << memberName << R"(" too large binSize for dataType.)" << std::endl;
#if defined(OL91)
                el_logff(LOG_NOTICE, "JSON for PLC: \"%s\ too large binSize for dataType.\n", memberName);
#endif
                return 5; // too large binSize for dataType
            }

            {
                char* pIecBool = (char*)(binBuffer + member.second.offsetInBinaryStruct);

                if ((jsonObject[memberName]).GetBool())
                    *pIecBool = 1;
                else
                    *pIecBool = 0;
            }
            break;

        case JSON_OBJECT:
            if (!(jsonObject[memberName]).IsObject()) {
                // indicate error to console & logfile
                std::cout << R"(JSON for PLC: ")" << memberName << R"(" is not an object.)" << std::endl;
#if defined(OL91)
                el_logff(LOG_NOTICE, "JSON for PLC: \"%s\ is not an object.\n", memberName);
#endif
                return 2; // wrong type
            }

            // no size check for an object, this is done for each object member

            returnCode = RecurseInterpret(jsonObject[memberName],
                                          pInterpreter,
                                          (*pInterpreter)[memberName],
                                          binBuffer + member.second.offsetInBinaryStruct,
                                          member.second.sizeInBinaryStruct);

            if (returnCode != 0)
                return returnCode;

            break;

        case JSON_STRINGARRAY:
        case JSON_INTARRAY:
        case JSON_UINTARRAY:
        case JSON_DOUBLEARRAY:
        case JSON_BOOLARRAY:
        case JSON_OBJECTARRAY:
            if (!(jsonObject[memberName]).IsArray()) {
                // indicate error to console & logfile
                std::cout << R"(JSON for PLC: ")" << memberName << R"(" is not an array.)" << std::endl;
#if defined(OL91)
                el_logff(LOG_NOTICE, "JSON for PLC: \"%s\ is not an array.\n", memberName);
#endif
                return 2; // wrong type
            }


            {
                SizeType jsonArraySize = jsonObject[memberName].Size();
                // write UsedArraySize to a required 'int' just before the array

                int32_t* pInt = (int32_t*)(binBuffer + member.second.offsetInBinaryStruct - sizeof(int));

                SizeType maxArraySize = *pInt;

                // limit the arraysize to the maximum
                if (maxArraySize < jsonArraySize) {
                    // indicate error to console & logfile
                    std::cout << R"(JSON for PLC - WARNING: binary arraySize ()" << maxArraySize << R"() for ")" << memberName << R"(" is less than JSON no of array elements ()" << jsonArraySize << ")" << std::endl;
#if defined(OL91)
                    el_logff(LOG_NOTICE, "JSON for PLC - WARNING: binary arraySize (%d) for \"%s\" is less than JSON no of array elements (%d)\n", maxArraySize, memberName, jsonArraySize);
#endif
                    jsonArraySize = maxArraySize;
                }

                *pInt = jsonArraySize;

                switch (member.second.jsonDataType) {

                case JSON_STRINGARRAY:
                    for (SizeType arrayIdx = 0; arrayIdx < jsonArraySize; arrayIdx++) {

                        if (!(jsonObject[memberName][arrayIdx]).IsString()) {
                            // indicate error to console & logfile
                            std::cout << R"(JSON for PLC: ")" << memberName << R"([)" << arrayIdx << R"(])" << R"(" is not a string.)" << std::endl;
#if defined(OL91)
                            el_logff(LOG_NOTICE, "JSON for PLC: \"%s\[%d] is not a string.\n", memberName, arrayIdx);
#endif
                            return 2; // wrong type
                        }

                        {
                            const char* string = (jsonObject[memberName][arrayIdx]).GetString();

                            SizeType strLength = (jsonObject[memberName][arrayIdx]).GetStringLength();

                            if (strLength > member.second.sizeInBinaryStruct-1) {
                                // indicate error to console & logfile
                                std::cout << R"(JSON for PLC: ")" << memberName << R"([)" << arrayIdx << R"(])" << R"(" insufficient binSize for string -> truncated.)" << std::endl;
#if defined(OL91)
                                el_logff(LOG_NOTICE, "JSON for PLC: \"%s\[%d] insufficient binSize for string -> truncated.\n", memberName, arrayIdx);
#endif
                                returnCode = 3; // string truncated

                                // truncate stringLength
                                strLength = member.second.sizeInBinaryStruct-1;
                            }

                            char* destStr = (char*)binBuffer + member.second.offsetInBinaryStruct + arrayIdx * member.second.sizeInBinaryStruct;
                            strncpy(destStr, string, strLength);
                            //set terminating 0
                            destStr[strLength] = (char)0;
                        }

                    }
                    break;

                case JSON_INTARRAY:
                    for (SizeType arrayIdx = 0; arrayIdx < jsonArraySize; arrayIdx++) {
                        if (!(jsonObject[memberName][arrayIdx]).IsInt()) {
                            // indicate error to console & logfile
                            std::cout << R"(JSON for PLC: ")" << memberName << R"([)" << arrayIdx << R"(])" << R"(" is not an int.)" << std::endl;
#if defined(OL91)
                            el_logff(LOG_NOTICE, "JSON for PLC: \"%s\[%d] is not an int.\n", memberName, arrayIdx);
#endif
                        }

                        int32_t* pInt = (int32_t*)(binBuffer + member.second.offsetInBinaryStruct + arrayIdx * member.second.sizeInBinaryStruct);
                        *pInt = (jsonObject[memberName][arrayIdx]).GetInt();
                    }
                    break;

                case JSON_UINTARRAY:
                    for (SizeType arrayIdx = 0; arrayIdx < jsonArraySize; arrayIdx++) {
                        if (!(jsonObject[memberName][arrayIdx]).IsUint()) {
                            // indicate error to console & logfile
                            std::cout << R"(JSON for PLC: ")" << memberName << R"([)" << arrayIdx << R"(])" << R"(" is not a uint.)" << std::endl;
#if defined(OL91)
                            el_logff(LOG_NOTICE, "JSON for PLC: \"%s\[%d] is not a uint.\n", memberName, arrayIdx);
#endif
                            return 2; // wrong type
                        }

                        uint32_t* pUint = (uint32_t*)(binBuffer + member.second.offsetInBinaryStruct + arrayIdx * member.second.sizeInBinaryStruct);
                        *pUint = (jsonObject[memberName][arrayIdx]).GetUint();
                    }
                    break;

                case JSON_DOUBLEARRAY:
                    for (SizeType arrayIdx = 0; arrayIdx < jsonArraySize; arrayIdx++) {
                        if (!(jsonObject[memberName][arrayIdx]).IsDouble()) {
                            // indicate error to console & logfile
                            std::cout << R"(JSON for PLC: ")" << memberName << R"([)" << arrayIdx << R"(])" << R"(" is not a double.)" << std::endl;
#if defined(OL91)
                            el_logff(LOG_NOTICE, "JSON for PLC: \"%s\[%d] is not a double.\n", memberName, arrayIdx);
#endif
                            return 2; // wrong type
                        }

                        double* pDouble = (double*)(binBuffer + member.second.offsetInBinaryStruct + arrayIdx * member.second.sizeInBinaryStruct);
                        *pDouble = (jsonObject[memberName][arrayIdx]).GetDouble();
                    }
                    break;

                case JSON_BOOLARRAY:
                    for (SizeType arrayIdx = 0; arrayIdx < jsonArraySize; arrayIdx++) {
                        if (!(jsonObject[memberName][arrayIdx]).IsBool()) {
                            // indicate error to console & logfile
                            std::cout << R"(JSON for PLC: ")" << memberName << R"([)" << arrayIdx << R"(])" << R"(" is not a bool.)" << std::endl;
#if defined(OL91)
                            el_logff(LOG_NOTICE, "JSON for PLC: \"%s\[%d] is not a bool.\n", memberName, arrayIdx);
#endif
                            return 2; // wrong type
                        }

                        char* pIecBool = (char*)(binBuffer + member.second.offsetInBinaryStruct + arrayIdx * member.second.sizeInBinaryStruct);

                        if ((jsonObject[memberName][arrayIdx]).GetBool())
                            *pIecBool = 1;
                        else
                            *pIecBool = 0;
                    }
                    break;

                case JSON_OBJECTARRAY:
                    for (SizeType arrayIdx = 0; arrayIdx < jsonArraySize; arrayIdx++) {
                        if (!(jsonObject[memberName][arrayIdx]).IsObject()) {
                            // indicate error to console & logfile
                            std::cout << R"(JSON for PLC: ")" << memberName << R"([)" << arrayIdx << R"(])" << R"(" is not an object.)" << std::endl;
#if defined(OL91)
                            el_logff(LOG_NOTICE, "JSON for PLC: \"%s\[%d] is not an object.\n", memberName, arrayIdx);
#endif
                            return 2; // wrong type
                        }

                        returnCode = RecurseInterpret(jsonObject[memberName][arrayIdx],
                                                      pInterpreter,
                                                      (*pInterpreter)[memberName],
                                                      binBuffer + member.second.offsetInBinaryStruct + arrayIdx * member.second.sizeInBinaryStruct,
                                                      member.second.sizeInBinaryStruct);

                        if (returnCode != 0)
                            return returnCode;
                    }
                    break;

                default:
                    break;	// only to avoid the not-handled-in-switch warning
                }
            }

            break;

        default:
            std::cout << "unknown JSON Type " << member.second.jsonDataType << ". I don't know how to interpret it" << std::endl;
            return 5; // unknown object - TODO: for all error returns check if continuation is possible / makes sense
            break;
        }
    }

    return returnCode;
}

uint32_t RecurseWrite(MyAllocator& myAlloc, GenericValue<UTF8<char>, MyAllocator>& jsonObject, std::unordered_map<std::string, std::unordered_map<std::string, JsonBinaryStructMapInfo> >* pInterpreter, std::unordered_map<std::string, JsonBinaryStructMapInfo>& jsonObjectMapping,
                      unsigned char* binBuffer) {

    uint32_t returnCode = 0;

    // iterate over the member description array
    for (auto& member : jsonObjectMapping) {

        const char* memberName = member.first.c_str();

        MyValue newJsonValue;

        // if it is of the expected type extract it
        switch (member.second.jsonDataType) {
        case JSON_STRING: {
            const char* string = (char*)binBuffer + member.second.offsetInBinaryStruct;

            newJsonValue.SetString((char*)string, strlen(string));
        }
        break;

        case JSON_INT: {
            int32_t* pInt = (int32_t*)(binBuffer + member.second.offsetInBinaryStruct);

            newJsonValue.SetInt(*pInt);
        }
        break;

        case JSON_UINT: {
            uint32_t* pUint = (uint32_t*)(binBuffer + member.second.offsetInBinaryStruct);

            newJsonValue.SetUint(*pUint);
        }
        break;

        case JSON_DOUBLE: {
            double* pDouble = (double*)(binBuffer + member.second.offsetInBinaryStruct);

            newJsonValue.SetDouble(*pDouble);
        }
        break;

        case JSON_BOOL: {
            char* pIecBool = (char*)(binBuffer + member.second.offsetInBinaryStruct);

            if (*pIecBool)
                newJsonValue.SetBool(true);
            else
                newJsonValue.SetBool(false);
        }
        break;

        case JSON_OBJECT:
            newJsonValue.SetObject();
            returnCode = RecurseWrite(
                             myAlloc,
                             newJsonValue,
                             pInterpreter,
                             (*pInterpreter)[memberName],
                             binBuffer + member.second.offsetInBinaryStruct);

            if (returnCode != 0)
                return returnCode;

            break;

        case JSON_STRINGARRAY:
        case JSON_INTARRAY:
        case JSON_UINTARRAY:
        case JSON_DOUBLEARRAY:
        case JSON_BOOLARRAY:
        case JSON_OBJECTARRAY:

            newJsonValue.SetArray();

            // get UsedArraySize from the 'int' (required) just before the array
            {
                int32_t* pInt = (int32_t*)(binBuffer + member.second.offsetInBinaryStruct - sizeof(int32_t));
                SizeType jsonArraySize = *pInt;

                switch (member.second.jsonDataType) {

                case JSON_STRINGARRAY:
                    for (SizeType arrayIdx = 0; arrayIdx < jsonArraySize; arrayIdx++) {
                        // the given size is the the size of a single array element
                        const char* string = (char*)binBuffer + member.second.offsetInBinaryStruct + arrayIdx * member.second.sizeInBinaryStruct;
                        newJsonValue.PushBack(MyValue().SetString((char*)string, strlen(string)), myAlloc);
                    }
                    break;

                case JSON_INTARRAY:
                    for (SizeType arrayIdx = 0; arrayIdx < jsonArraySize; arrayIdx++) {
                        int32_t* pInt = (int32_t*)(binBuffer + member.second.offsetInBinaryStruct + arrayIdx * member.second.sizeInBinaryStruct);
                        newJsonValue.PushBack(MyValue().SetInt(*pInt), myAlloc);
                    }
                    break;

                case JSON_UINTARRAY:
                    for (SizeType arrayIdx = 0; arrayIdx < jsonArraySize; arrayIdx++) {
                        uint32_t* pUint = (uint32_t*)(binBuffer + member.second.offsetInBinaryStruct + arrayIdx * member.second.sizeInBinaryStruct);
                        newJsonValue.PushBack(MyValue().SetUint(*pUint), myAlloc);
                    }
                    break;

                case JSON_DOUBLEARRAY:
                    for (SizeType arrayIdx = 0; arrayIdx < jsonArraySize; arrayIdx++) {
                        double* pDouble = (double*)(binBuffer + member.second.offsetInBinaryStruct + arrayIdx * member.second.sizeInBinaryStruct);
                        newJsonValue.PushBack(MyValue().SetDouble(*pDouble), myAlloc);
                    }
                    break;

                case JSON_BOOLARRAY:
                    for (SizeType arrayIdx = 0; arrayIdx < jsonArraySize; arrayIdx++) {
                        char* pIecBool = (char*)(binBuffer + member.second.offsetInBinaryStruct + arrayIdx * member.second.sizeInBinaryStruct);

                        if (*pIecBool == 1)
                            newJsonValue.PushBack(MyValue().SetBool(true), myAlloc);
                        else
                            newJsonValue.PushBack(MyValue().SetBool(false), myAlloc);
                    }
                    break;

                case JSON_OBJECTARRAY:
                    for (SizeType arrayIdx = 0; arrayIdx < jsonArraySize; arrayIdx++) {
                        MyValue myVal;
                        myVal.SetObject();

                        returnCode = RecurseWrite(
                                         myAlloc,
                                         myVal,
                                         pInterpreter,
                                         (*pInterpreter)[memberName],
                                         binBuffer + member.second.offsetInBinaryStruct + arrayIdx * member.second.sizeInBinaryStruct);

                        newJsonValue.PushBack(myVal, myAlloc);

                        if (returnCode != 0)
                            return returnCode;
                    }
                    break;

                default:
                    break;	// only to avoid the not-handled-in-switch warning

                }
            }

            break;

        default:
            std::cout << "unknown JSON Type " << member.second.jsonDataType << ". I don't know how to write it" << std::endl;
            return 5; // unknown object - TODO: for all error returns check if continuation is possible / makes sense
            break;
        }

        // add the member
        MyValue& myVal = jsonObject.AddMember(GenericStringRef<char>(memberName), newJsonValue, myAlloc);
        if (myVal.IsNull()) {
            // indicate error to console & logfile
            std::cout << R"(JSON for PLC: ")" << memberName << R"(" not added.)" << std::endl;
#if defined(OL91)
            el_logff(LOG_NOTICE, "JSON for PLC: \"%s\ not added.\n", memberName);
#endif
            return 1;
        }

    }

    return returnCode;
}

