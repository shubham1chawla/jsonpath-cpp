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
    std::string _key;
    std::string _val;
    JsonNodeType _type;

    std::map<std::string, int> _lookup;
    std::vector<JsonNode *> _children;

public:
    JsonNode(const std::string &key, const std::string &val, const JsonNodeType type);

    std::string key();
    std::string val();
    JsonNodeType type();

    void addChild(JsonNode *node);
};

class JsonPath
{
    JsonNode *root;

    void init(const std::string &json);
    JsonNode *parse(const std::string &key, const std::string &val);

public:
    JsonPath(const std::string &json);
    JsonPath(std::ifstream &file);
};