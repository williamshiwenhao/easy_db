#ifndef EASYDB_SERVER_H
#define EASYDB_SERVER_H

#include <map>
#include <string>
#include <vector>

#include "database.h"

using std::map;
using std::string;
using std::vector;

class Server
{
public:
  int InitServer(const string dir);
  int CreateDatabase(const string name);
  // help function
  void ListDatabase();
  int Work();

private:
  int SplitCmd();
  int HandleSql();
  int ChangeDatabase(const string &name);
  map<string, Database> database;
  string dir;
  string workingName;
  string rowCmd;
  vector<string> cmd;
  enum
  {
    ServerExit = 1
  };
};

void PrintList(const string &title, const vector<string> &names);
void PrintTable(const vector<string> &title,
                const vector<vector<string>> &names);

#endif