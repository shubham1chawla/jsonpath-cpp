#include <iostream>
#include <fstream>
#include <stack>
#include "JsonPath.h"

// -----------------------------------------------------------
// PRE-PROCESSORS BELOW
// -----------------------------------------------------------

#define CURLO '{'
#define CURLC '}'
#define SQURO '['
#define SQURC ']'

#define QUOTE '"'
#define COLON ':'
#define COMMA ','

#define HYPHN '-'
#define ZEROV '0'
#define ONE_V '1'
#define TWO_V '2'
#define THREV '3'
#define FOURV '4'
#define FIVEV '5'
#define SIX_V '6'
#define SEVNV '7'
#define EGHTV '8'
#define NINEV '9'

#define NULLV "null"
#define TRUEV "true"
#define FALSV "false"
#define ROOTV "$"

#define EMPTY_JSON_ERROR_MESSAGE "Empty json string provided!"
#define MALFORMED_JSON_ERROR_MESSAGE "Malformed JSON string provided!"

#define WHITESPACE " \n\r\t\f\v"

// -----------------------------------------------------------
// UTILITY METHODS BELOW
// -----------------------------------------------------------

std::string ltrim(const std::string &s)
{
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

std::string rtrim(const std::string &s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

std::string trim(const std::string &s)
{
    return rtrim(ltrim(s));
}

std::string extractKey(std::string keyValue)
{
    int cidx = keyValue.find(COLON);
    if (cidx == std::string::npos || cidx < 2)
        throw std::invalid_argument(MALFORMED_JSON_ERROR_MESSAGE);

    std::string left = keyValue.substr(keyValue.find(QUOTE) + 1, cidx - 1);
    return trim(left.substr(0, left.rfind(QUOTE)));
}

std::string extractVal(std::string keyValue)
{
    int cidx = keyValue.find(COLON);
    if (cidx == std::string::npos || cidx < 2)
        throw std::invalid_argument(MALFORMED_JSON_ERROR_MESSAGE);

    return trim(keyValue.substr(cidx + 1));
}

// -----------------------------------------------------------
// IMPLEMENTATIONs BELOW
// -----------------------------------------------------------

JsonNode::JsonNode(std::string name, std::string json, JsonNodeType type)
{
    this->_name = name;
    this->_json = json;
    this->_type = type;
}

std::string JsonNode::name()
{
    return this->_name;
}

std::string JsonNode::json()
{
    return this->_json;
}

JsonNodeType JsonNode::type()
{
    return this->_type;
}

void JsonNode::addChild(JsonNode *node)
{
    this->_lookup[node->name()] = this->_children.size();
    this->_children.push_back(node);
}

void JsonPath::init(std::string json)
{
    try
    {
        json = trim(json);
        if (json.size() == 0)
            throw std::invalid_argument(EMPTY_JSON_ERROR_MESSAGE);
        this->root = this->parse(ROOTV, json);
    }
    catch (std::invalid_argument e)
    {
        std::cout << e.what() << std::endl;
        throw e;
    }
}

JsonPath::JsonPath(std::string json)
{
    this->init(json);
}

JsonPath::JsonPath(std::ifstream &file)
{
    std::string json = "";
    if (!file.is_open())
        throw std::runtime_error("Unable to open file!");
    std::string line;
    while (std::getline(file, line))
        json += line;
    file.close();
    this->init(json);
}

JsonNode *JsonPath::parse(std::string name, std::string json)
{
    char front = json.front();
    char back = json.back();
    switch (front)
    {

    case CURLO:
    case SQURO:
    {
        if ((front == CURLO && back != CURLC) || (front == SQURO && back != SQURC))
            throw std::invalid_argument(MALFORMED_JSON_ERROR_MESSAGE);

        JsonNodeType type = front == CURLO ? OBJ : ARR;
        JsonNode *node = new JsonNode(name, json, type);
        std::stack<char> stack;
        std::string segment = "", key = "", val = "";
        int len = 0;

        for (int i = 1; i < json.size() - 1; i++)
        {
            char c = json[i];
            switch (c)
            {
            case CURLO:
            case SQURO:
                stack.push(c);
                segment += c;
                break;
            case CURLC:
            case SQURC:
                if ((c == CURLC && stack.top() != CURLO) || (c == SQURC && stack.top() != SQURO))
                    throw std::invalid_argument(MALFORMED_JSON_ERROR_MESSAGE);
                stack.pop();
                segment += c;
                break;
            case COMMA:
                if (stack.empty())
                {
                    segment = trim(segment);
                    key = type == OBJ ? extractKey(segment) : SQURO + std::to_string(len) + SQURC;
                    val = type == OBJ ? extractVal(segment) : segment;
                    node->addChild(this->parse(key, val));
                    len++;
                    segment = "";
                }
                else
                    segment += c;
                break;
            default:
                segment += c;
            }
        }
        segment = trim(segment);
        key = type == OBJ ? extractKey(segment) : SQURO + std::to_string(len) + SQURC;
        val = type == OBJ ? extractVal(segment) : segment;
        node->addChild(this->parse(key, val));

        return node;
    }

    // JSON segment starts with " indicates STRING
    case QUOTE:
        if (json.back() != QUOTE)
            throw std::invalid_argument(MALFORMED_JSON_ERROR_MESSAGE);
        return new JsonNode(name, json, STR);

    // JSON segment starts with either - or 0-9 indicates NUMBER
    case HYPHN:
    case ZEROV:
    case ONE_V:
    case TWO_V:
    case THREV:
    case FOURV:
    case FIVEV:
    case SIX_V:
    case SEVNV:
    case EGHTV:
    case NINEV:
        return new JsonNode(name, json, NUM);
    }

    // JSON segment equals either "true" or "false" indicates BOOLEAN
    if ((json == TRUEV || json == FALSV))
        return new JsonNode(name, json, BOL);

    // JSON segment equals "null" indicates NULL
    else if (json == NULLV)
        return new JsonNode(name, json, NUL);

    // Probably invalid JSON segment
    else
        throw std::invalid_argument(MALFORMED_JSON_ERROR_MESSAGE);
}
