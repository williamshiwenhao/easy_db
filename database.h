#ifndef EASYDB_DATABASE_H
#define EASYDB_DATABASE_H

#include <string>
#include <vector>
#include <map>
#include <fstream>

#include "types.h"
#include "table.h"

using std::fstream;
using std::map;
using std::string;
using std::vector;

class Database
{
public:
  Database() : inited(false)
  {
  }
  int Init(const string &dir, const string &name);
  int CreateTable(const std::string &tableName,
                  const std::vector<string>,
                  const std::vector<Type>);
  int HandleSql(const vector<string> &cmd);
  bool isInited() { return inited; }
  void Stop();

private:
  map<std::string, Table> tables;
  string dataBaseDir;
  string name;
  bool inited;
  fstream fd;
};

#endif
