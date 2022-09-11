#include <iostream>
#include <fstream>
#include "JsonPath.h"

int main()
{
    try
    {
        std::ifstream file("../example.json");
        JsonPath jsonPath(file);
    }
    catch (...)
    {
        std::cout << "Unexpectd error occured!";
        return 1;
    }
    return 0;
}