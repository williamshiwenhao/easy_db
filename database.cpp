#include <iostream>
#include <stdio.h>
#include <string>

#include "database.h"

//TODO:init
int Database::Init(const string &dir, const string &name)
{
    using std::fstream;
    using std::ifstream;
    tables.clear();
    this->name = name;
    dataBaseDir = dir;
    string configFile = dir + "//" + name + ".db"; //database file name
    fd.open(configFile.data(), fstream::in);
    if (!fd.is_open())
    {
    }
    return 0;
}

//TODO:finish stop;
void Database::Stop()
{
    printf("Stop\n");
}

//TODO:finish HandleSql
int Database::HandleSql(const vector<string> &cmd)
{
    using std::cout;
    using std::endl;
    for (auto &str : cmd)
    {
        cout << str << endl;
    }
    return 0;
}