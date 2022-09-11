#pragma once
#include <map>
#include <vector>

enum JsonNodeType
{
    OBJ,
    ARR,
    STR,
    NUM,
    BOL,
    NUL
};

class JsonNode
{
    std::string _name;
    std::string _json;
    JsonNodeType _type;
    std::map<std::string, int> _lookup;
    std::vector<JsonNode *> _children;

public:
    JsonNode(std::string name, std::string json, JsonNodeType type);
    std::string name();
    std::string json();
    JsonNodeType type();
    void addChild(JsonNode *node);
};

class JsonPath
{
    JsonNode *root;

    void init(std::string json);
    JsonNode *parse(std::string name, std::string json);

public:
    JsonPath(std::string json);
    JsonPath(std::ifstream &file);
};