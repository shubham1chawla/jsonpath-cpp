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

std::string &trim(std::string &s)
{
    // Erasing whitespaces from the start of the string
    const size_t sidx = s.find_first_not_of(WHITESPACE);
    if (sidx != 0 && sidx != std::string::npos)
        s.erase(s.begin(), s.begin() + s.find_first_not_of(WHITESPACE));

    // Erasing whitespaces from the end of the string
    const size_t eidx = s.find_last_not_of(WHITESPACE);
    if (eidx != s.size() - 1 && eidx != std::string::npos)
        s.resize(eidx + 1);

    return s;
}

std::string extractKey(const std::string &prop)
{
    int cidx = prop.find(COLON);
    if (cidx == std::string::npos || cidx < 2)
        throw std::invalid_argument(MALFORMED_JSON_ERROR_MESSAGE);

    const size_t s = prop.find(QUOTE) + 1;
    const size_t e = prop.rfind(QUOTE, cidx - 1);
    return prop.substr(s, e - s);
}

std::string extractVal(const std::string &prop)
{
    int cidx = prop.find(COLON);
    if (cidx == std::string::npos || cidx < 2)
        throw std::invalid_argument(MALFORMED_JSON_ERROR_MESSAGE);

    const size_t s = prop.find_first_not_of(WHITESPACE, cidx + 1);
    const size_t e = prop.find_last_not_of(WHITESPACE) + 1;
    return prop.substr(s, e - s);
}

// -----------------------------------------------------------
// IMPLEMENTATIONs BELOW
// -----------------------------------------------------------

JsonNode::JsonNode(const std::string &key, const std::string &val, const JsonNodeType type)
{
    this->_key = key;
    this->_val = val;
    this->_type = type;
}

std::string JsonNode::key()
{
    return this->_key;
}

std::string JsonNode::val()
{
    return this->_val;
}

JsonNodeType JsonNode::type()
{
    return this->_type;
}

void JsonNode::addChild(JsonNode *node)
{
    this->_lookup[node->key()] = this->_children.size();
    this->_children.push_back(node);
}

void JsonPath::init(const std::string &json)
{
    try
    {
        if (json.find_first_not_of(WHITESPACE) == std::string::npos)
            throw std::invalid_argument(EMPTY_JSON_ERROR_MESSAGE);
        this->root = this->parse(ROOTV, json);
    }
    catch (std::invalid_argument e)
    {
        std::cout << e.what() << std::endl;
        throw e;
    }
}

JsonPath::JsonPath(const std::string &json)
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

JsonNode *JsonPath::parse(const std::string &key, const std::string &val)
{
    const unsigned int fidx = val.find_first_not_of(WHITESPACE);
    const unsigned int bidx = val.find_last_not_of(WHITESPACE);
    const char front = val[fidx];
    const char back = val[bidx];

    switch (front)
    {

    case CURLO:
    case SQURO:
    {
        if ((front == CURLO && back != CURLC) || (front == SQURO && back != SQURC))
            throw std::invalid_argument(MALFORMED_JSON_ERROR_MESSAGE);

        JsonNodeType type = front == CURLO ? OBJ : ARR;
        JsonNode *node = new JsonNode(key, val, type);
        std::stack<char> stack;

        std::string _seg = "", _key = "", _val = "";
        int itemIdx = 0;

        for (int i = fidx + 1; i < bidx; i++)
        {
            char c = val[i];
            switch (c)
            {
            case CURLO:
            case SQURO:
                stack.push(c);
                _seg += c;
                break;
            case CURLC:
            case SQURC:
                if ((c == CURLC && stack.top() != CURLO) || (c == SQURC && stack.top() != SQURO))
                    throw std::invalid_argument(MALFORMED_JSON_ERROR_MESSAGE);
                stack.pop();
                _seg += c;
                break;
            case COMMA:
                if (stack.empty())
                {
                    _key = type == OBJ ? extractKey(_seg) : std::to_string(itemIdx);
                    _val = type == OBJ ? extractVal(_seg) : trim(_seg);
                    node->addChild(this->parse(_key, _val));
                    itemIdx++;
                    _seg = "";
                }
                else
                    _seg += c;
                break;
            default:
                _seg += c;
            }
        }
        _key = type == OBJ ? extractKey(_seg) : std::to_string(itemIdx);
        _val = type == OBJ ? extractVal(_seg) : trim(_seg);
        node->addChild(this->parse(_key, _val));

        return node;
    }

    // JSON segment starts with " indicates STRING
    case QUOTE:
        if (back != QUOTE)
            throw std::invalid_argument(MALFORMED_JSON_ERROR_MESSAGE);
        return new JsonNode(key, val, STR);

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
        return new JsonNode(key, val, NUM);
    }

    // JSON segment equals either "true" or "false" indicates BOOLEAN
    if ((val == TRUEV || val == FALSV))
        return new JsonNode(key, val, BOL);

    // JSON segment equals "null" indicates NULL
    else if (val == NULLV)
        return new JsonNode(key, val, NUL);

    // Probably invalid JSON segment
    else
        throw std::invalid_argument(MALFORMED_JSON_ERROR_MESSAGE);
}
