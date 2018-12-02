#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <array>
#include <cstddef>

#include <boost/timer/timer.hpp>

/*
 * Changes assertion behaviour of json.
 * Instead of standard aborting an std::exception is thrown
 */
namespace rapidjson {
class exception {
  public:
    exception(const std::string& text): Text(text) {
    }

    ~exception() {
    }

    const std::string what() {
        return Text;
    };

  private:
    std::string Text;
};
}

#if defined(RAPIDJSON_ASSERT)
#undef RAPIDJSON_ASSERT
#endif //-- #defined(RAPIDJSON_ASSERT)

#define RAPIDJSON_ASSERT(x) if(!(x)) {throw rapidjson::exception("rapidjson exception: " #x);}

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

#include "jsonWrapper.h"

#include "nlohmann/json.hpp"

constexpr int LOOP_CNT = 1000000;
constexpr int MAX_IP = 4;

struct __attribute__((packed, aligned(4))) Dhcp {
    bool active;
    int interface;
};

struct __attribute__((packed, aligned(4))) Numbers {
    int addr;
    int mask;
};

struct __attribute__((packed, aligned(4))) IpCfg {
    int schemaVersion;
    struct Dhcp dhcp;
    int n; // Numbers of valid array entries
    struct Numbers ip[MAX_IP];
};

struct IpCfg myipcfg;

const char json_ipcfg[] = "{\"schemaVersion\":1,\"dhcp\":{\"active\":true,\"interface\":1},\"ip\":[{\"addr\":1234,\"mask\":2345},{\"addr\":3456,\"mask\":4567}]}";
const char json_ipcfg_short[] = "{\"v\":1,\"dhcp\":{\"a\":true,\"i\":1},\"ipV4\":[{\"a\":1234,\"n\":2345},{\"a\":3456,\"n\":4567}]}";
const char json_ipcfg_extended[] = "{\"schemaVersion\":1,\"dynamicHostControlProtocol\":{\"active\":true,\"interface\":1},\"ipVersion4\":[{\"address\":1234,\"netmask\":2345},{\"address\":3456,\"netmask\":4567}]}";

#include "getmember.h"
#include "getvalue.h"

void parseIPCfgWithOriginal() {
    //char abuffer[0x10000];
    char pbuffer[1000];

    for(int i = 0; i < LOOP_CNT; i++) {
        //rapidjson::MemoryPoolAllocator<> mpa(&abuffer[0], sizeof(abuffer));
        rapidjson::Document document; //(&mpa);

        memcpy(pbuffer, json_ipcfg, sizeof(json_ipcfg));

        if(document.ParseInsitu(pbuffer).HasParseError()) {
            std::cout << "Error parsing" << std::endl;
        } else {
            rapidjson::Value::ConstMemberIterator itr_val = document.FindMember("schemaVersion");
            if(itr_val != document.MemberEnd())
                myipcfg.schemaVersion = itr_val->value.GetInt();

            rapidjson::Value::ConstMemberIterator itr_obj = document.FindMember("dhcp");
            if(itr_obj != document.MemberEnd()) {
                itr_val = itr_obj->value.FindMember("active");
                if(itr_val != itr_obj->value.MemberEnd())
                    myipcfg.dhcp.active = itr_val->value.GetBool();
                itr_val = itr_obj->value.FindMember("interface");
                if(itr_val != itr_obj->value.MemberEnd())
                    myipcfg.dhcp.interface = itr_val->value.GetInt();
            }

            itr_obj = document.FindMember("ip");
            if(itr_obj != document.MemberEnd()) {
                int i = 0;
                for(auto itr = itr_obj->value.Begin(); itr != itr_obj->value.End() && i < MAX_IP; ++itr, ++i) {
                    myipcfg.ip[i].addr = itr->FindMember("addr")->value.GetInt();
                    myipcfg.ip[i].mask = itr->FindMember("mask")->value.GetInt();
                }
            }
        }
    }
}

void parseIPCfgWithTemplate() {
    char abuffer[0x10000];
    char pbuffer[1000];

    for(int i = 0; i < LOOP_CNT; i++) {
        rapidjson::MemoryPoolAllocator<> mpa(&abuffer[0], sizeof(abuffer));
        rapidjson::Document document(&mpa);

        memcpy(pbuffer, json_ipcfg, sizeof(json_ipcfg));

        if(document.ParseInsitu(pbuffer).HasParseError()) {
            std::cout << "Error parsing" << std::endl;
        } else {
            rapidjson::Value obj;

            GetMember(document, "schemaVersion", &myipcfg.schemaVersion);
            if(GetMember(document, "dhcp", obj)) {
                GetMember(obj, "active", &myipcfg.dhcp.active);
                GetMember(obj, "interface", &myipcfg.dhcp.interface);
            }

            rapidjson::SizeType n;
            rapidjson::Value array[MAX_IP];
            if(GetMember(document, "ip", &array[0], MAX_IP, n)) {
                for(rapidjson::SizeType i = 0; i < n; ++i) {
                    GetMember(array[i], "addr", &myipcfg.ip[i].addr);
                    GetMember(array[i], "mask", &myipcfg.ip[i].mask);
                }
            }
        }
    }
}

void parseIPCfgWithOverload() {
    char abuffer[0x10000];
    char pbuffer[1000];

    for(int i = 0; i < LOOP_CNT; i++) {
        rapidjson::MemoryPoolAllocator<> mpa(&abuffer[0], sizeof(abuffer));
        rapidjson::Document document(&mpa);

        memcpy(pbuffer, json_ipcfg, sizeof(json_ipcfg));

        if(document.ParseInsitu(pbuffer).HasParseError()) {
            std::cout << "Error parsing" << std::endl;
        } else {
            rapidjson::Value obj;

            GetValue(document, "schemaVersion", &myipcfg.schemaVersion);
            if(GetValue(document, "dhcp", obj)) {
                GetValue(obj, "active", &myipcfg.dhcp.active);
                GetValue(obj, "interface", &myipcfg.dhcp.interface);
            }

            rapidjson::SizeType n;
            rapidjson::Value array[MAX_IP];
            if(GetValue(document, "ip", &array[0], MAX_IP, n)) {
                for(rapidjson::SizeType i = 0; i < n; ++i) {
                    GetValue(array[i], "addr", &myipcfg.ip[i].addr);
                    GetValue(array[i], "mask", &myipcfg.ip[i].mask);
                }
            }
        }
    }
}

void parseIPCfgWithOverloadExt() {
    char abuffer[0x10000];
    char pbuffer[1000];

    for(int i = 0; i < LOOP_CNT; i++) {
        rapidjson::MemoryPoolAllocator<> mpa(&abuffer[0], sizeof(abuffer));
        rapidjson::Document document(&mpa);

        memcpy(pbuffer, json_ipcfg_extended, sizeof(json_ipcfg_extended));

        if(document.ParseInsitu(pbuffer).HasParseError()) {
            std::cout << "Error parsing" << std::endl;
        } else {
            rapidjson::Value obj;

            GetValue(document, "schemaVersion", &myipcfg.schemaVersion);
            if(GetValue(document, "dynamicHostControlProtocol", obj)) {
                GetValue(obj, "active", &myipcfg.dhcp.active);
                GetValue(obj, "interface", &myipcfg.dhcp.interface);
            }

            rapidjson::SizeType n;
            rapidjson::Value array[MAX_IP];
            if(GetValue(document, "ipV4", &array[0], MAX_IP, n)) {
                for(rapidjson::SizeType i = 0; i < n; ++i) {
                    GetValue(array[i], "address", &myipcfg.ip[i].addr);
                    GetValue(array[i], "netmask", &myipcfg.ip[i].mask);
                }
            }
        }
    }
}

void parseIPCfgWithOverloadShort() {
    char abuffer[0x10000];
    char pbuffer[1000];

    for(int i = 0; i < LOOP_CNT; i++) {
        rapidjson::MemoryPoolAllocator<> mpa(&abuffer[0], sizeof(abuffer));
        rapidjson::Document document(&mpa);

        memcpy(pbuffer, json_ipcfg_short, sizeof(json_ipcfg_short));

        if(document.ParseInsitu(pbuffer).HasParseError()) {
            std::cout << "Error parsing" << std::endl;
        } else {
            rapidjson::Value obj;

            GetValue(document, "v", &myipcfg.schemaVersion);
            if(GetValue(document, "dhcp", obj)) {
                GetValue(obj, "a", &myipcfg.dhcp.active);
                GetValue(obj, "i", &myipcfg.dhcp.interface);
            }

            rapidjson::SizeType n;
            rapidjson::Value array[MAX_IP];
            if(GetValue(document, "ipV4", &array[0], MAX_IP, n)) {
                for(rapidjson::SizeType i = 0; i < n; ++i) {
                    GetValue(array[i], "a", &myipcfg.ip[i].addr);
                    GetValue(array[i], "n", &myipcfg.ip[i].mask);
                }
            }
        }
    }
}

void parseIPCfgWithIndexException() {
    char abuffer[0x10000];
    char pbuffer[1000];

    for(int i = 0; i < LOOP_CNT; i++) {
        rapidjson::MemoryPoolAllocator<> mpa(&abuffer[0], sizeof(abuffer));
        rapidjson::Document document(&mpa);

        memcpy(pbuffer, json_ipcfg, sizeof(json_ipcfg));

        if(document.ParseInsitu(pbuffer).HasParseError()) {
            std::cout << "Error parsing" << std::endl;
        } else {
            try {
                myipcfg.schemaVersion = document["schemaVersion"].GetInt();

                rapidjson::Value &v = document["dhcp"];
                myipcfg.dhcp.active = v["active"].GetBool();
                myipcfg.dhcp.interface = v["interface"].GetInt();

                v = document["ip"];
                for(rapidjson::SizeType i = 0; i < std::min(static_cast<rapidjson::SizeType>(MAX_IP), v.Size()); i++) {
                    myipcfg.ip[i].addr = v[i]["addr"].GetInt();
                    myipcfg.ip[i].mask = v[i]["mask"].GetInt();
                }
            } catch(rapidjson::exception &ex) {
                std::cout << ex.what(); // To much overhead in case of JSON fault
            }
        }
    }
}

void parseIPCfgWithFindException() {
    char abuffer[0x10000];
    char pbuffer[1000];

    for(int i = 0; i < LOOP_CNT; i++) {
        rapidjson::MemoryPoolAllocator<> mpa(&abuffer[0], sizeof(abuffer));
        rapidjson::Document document(&mpa);

        memcpy(pbuffer, json_ipcfg, sizeof(json_ipcfg));

        if(document.ParseInsitu(pbuffer).HasParseError()) {
            std::cout << "Error parsing" << std::endl;
        } else {
            try {
                myipcfg.schemaVersion  = document.FindMember("schemaVersion")->value.GetInt();
                myipcfg.dhcp.active    = document.FindMember("dhcp")->value.FindMember("active")->value.GetBool();
                myipcfg.dhcp.interface = document.FindMember("dhcp")->value.FindMember("interface")->value.GetInt();

                int i = 0;
                for(rapidjson::Value::ConstValueIterator itv = document.FindMember("ip")->value.Begin(); itv != document.FindMember("ip")->value.End() && i < MAX_IP; ++itv) {
                    myipcfg.ip[i].addr  = itv->FindMember("addr")->value.GetInt();
                    myipcfg.ip[i].mask = itv->FindMember("mask")->value.GetInt();
                    ++i;
                }
            } catch(rapidjson::exception &ex) {
                std::cout << ex.what(); // To much overhead in case of JSON fault
            }
        }
    }
}

void parseIPCfgWithTable() {
    char pbuffer[1000];

    ParserHandle jsonParserHandle = JSON_parserNew();

    if (jsonParserHandle == NULL) {
        std::cout << "JSON_documentNew failed\n";
        return;
    }

    InterpreterObjectHandle objHandleIp = JSON_parserNewObject(jsonParserHandle, "ip");

    JSON_parserObjectAddMember(objHandleIp, "addr", JSON_INT, offsetof(Numbers, addr), sizeof(Numbers::addr));
    JSON_parserObjectAddMember(objHandleIp, "mask", JSON_INT, offsetof(Numbers, mask), sizeof(Numbers::mask));

    InterpreterObjectHandle objHandleDhcp = JSON_parserNewObject(jsonParserHandle, "dhcp");

    JSON_parserObjectAddMember(objHandleDhcp, "active",    JSON_BOOL, offsetof(Dhcp, active), 	 sizeof(Dhcp::active));
    JSON_parserObjectAddMember(objHandleDhcp, "interface", JSON_INT,  offsetof(Dhcp, interface), sizeof(Dhcp::interface));

    InterpreterObjectHandle objHandleRoot = JSON_parserNewObject(jsonParserHandle, "");

    JSON_parserObjectAddMember(objHandleRoot, "schemaVersion", JSON_INT,         offsetof(IpCfg, schemaVersion), sizeof(IpCfg::schemaVersion));
    JSON_parserObjectAddMember(objHandleRoot, "dhcp",          JSON_OBJECT,      offsetof(IpCfg, dhcp),          sizeof(IpCfg::dhcp));
    JSON_parserObjectAddMember(objHandleRoot, "ip",            JSON_OBJECTARRAY, offsetof(IpCfg, ip),            sizeof(IpCfg::ip[0]));

    myipcfg.n = MAX_IP;  // Set usable element count

    for(auto i = 0; i < LOOP_CNT; i++) {
        memcpy(pbuffer, json_ipcfg, sizeof(json_ipcfg));

        JSON_TextToBin(jsonParserHandle, pbuffer, (unsigned char*)&myipcfg, sizeof(myipcfg));
    }

    JSON_parserDelete(jsonParserHandle);
}

void parsen_nl_json() {
    char pbuffer[1000];

    for(auto i = 0; i < LOOP_CNT; i++) {
        memcpy(pbuffer, json_ipcfg, sizeof(json_ipcfg));

        try {
            auto js = nlohmann::json::parse(pbuffer);

            myipcfg.schemaVersion  = js["schemaVersion"];
            myipcfg.dhcp.active    = js["dhcp"]["active"];
            myipcfg.dhcp.interface = js["dhcp"]["interface"];
            myipcfg.ip[0].addr     = js["ip"][0]["addr"];
            myipcfg.ip[0].mask     = js["ip"][0]["mask"];
            myipcfg.ip[1].addr     = js["ip"][1]["addr"];
            myipcfg.ip[1].mask     = js["ip"][1]["mask"];
        } catch(const std::exception& ex) {
            std::cout << ex.what(); // To much overhead in case of JSON fault
        }
    }
}

void output(const char *title) {
    std::cout << title << "schemaVersion:" << myipcfg.schemaVersion
              << " dhcp.active:" << myipcfg.dhcp.active << " dhcp.interface:" << myipcfg.dhcp.interface
              << " ip[0].addr:" << myipcfg.ip[0].addr << " ip[0].mask:" << myipcfg.ip[0].mask
              << " ip[1].addr:" << myipcfg.ip[1].addr << " ip[1].mask:" << myipcfg.ip[1].mask
              << std::endl << "+++" << std::endl;
}

int main(int, char**) {
    std::cout << "JSON string to parse: " << &json_ipcfg[0]
              << std::endl << "+++" << std::endl;

    {
        memset(&myipcfg, 0, sizeof(myipcfg));

        boost::timer::auto_cpu_timer act;

        parseIPCfgWithOriginal();
    }

    output("Original - ");

    {
        memset(&myipcfg, 0, sizeof(myipcfg));

        boost::timer::auto_cpu_timer act;

        parseIPCfgWithTemplate();
    }

    output("Template - ");

    {
        memset(&myipcfg, 0, sizeof(myipcfg));

        boost::timer::auto_cpu_timer act;

        parseIPCfgWithOverload();
    }

    output("Overload - ");

    {
        memset(&myipcfg, 0, sizeof(myipcfg));

        boost::timer::auto_cpu_timer act;

        parseIPCfgWithIndexException();
    }

    output("IndexException - ");

    {
        memset(&myipcfg, 0, sizeof(myipcfg));

        boost::timer::auto_cpu_timer act;

        parseIPCfgWithFindException();
    }

    output("FindException - ");

    {
        memset(&myipcfg, 0, sizeof(myipcfg));

        boost::timer::auto_cpu_timer act;

        parseIPCfgWithTable();
    }

    output("Table - ");

    {
        memset(&myipcfg, 0, sizeof(myipcfg));

        boost::timer::auto_cpu_timer act;

        parsen_nl_json();
    }

    output("NL-Json - ");

    std::cout << "JSON long string to parse: " << &json_ipcfg_extended[0] << std::endl;

    {
        memset(&myipcfg, 0, sizeof(myipcfg));

        boost::timer::auto_cpu_timer act;

        parseIPCfgWithOverloadExt();
    }

    output("Overload - ");

    std::cout << "JSON short string to parse: " << &json_ipcfg_short[0] << std::endl;

    {
        memset(&myipcfg, 0, sizeof(myipcfg));

        boost::timer::auto_cpu_timer act;

        parseIPCfgWithOverloadShort();
    }

    output("Overload - ");

    return 0;
}
