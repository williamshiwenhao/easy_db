#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ctype.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <ctype.h>

#include "database.h"
#include "server.h"

static const string DATABASELIST = "database.list";

int Server::InitServer(const string dir)
{
	workingName.clear();
	this->dir = dir;
	string configFile = dir + "//" + DATABASELIST;
	std::fstream fd(configFile.data());
	if (!fd.is_open())
	{
		fprintf(stderr, "[Server init]Error, can not open database list %s\n",
				configFile.data());
		fprintf(stderr, "[Server init]Try to create database list\n");
		std::ofstream fd2(configFile.data(), std::ofstream::app);
		if (fd2.is_open())
		{
			printf("[Server init]Create database list succeed\n");
			fd2.close();
			return 0;
		}
		else
		{
			fprintf(stderr, "[Server init]Create failed, exit");
			return -1;
		}
	}
	string databaseName;
	while (fd >> databaseName)
	{
		database.emplace(databaseName, Database());
	}
	fd.close();
	printf("[Server init]Success\n");
	ListDatabase();
	return 0;
}

int Server::Work()
{
	while (true)
	{
		using std::cin;
		string oldCmd = rowCmd;
		printf("> ");
		getline(cin, rowCmd);
		rowCmd = oldCmd + rowCmd;
		if (rowCmd.find(';') == string::npos)
		{
			// can not find ';' a sql must end up with ';'
			continue;
		}
		if (SplitCmd())
		{
			// Syntax error
			rowCmd.clear();
			cmd.clear();
			continue;
		}
		int handleResult = HandleSql();
		if (handleResult == ServerExit)
		{
			printf("Bye~\n");
			return 0;
		}
		cmd.clear();
		rowCmd.clear();
	}
	return 0;
}

int Server::HandleSql()
{
	int size = static_cast<int>(cmd.size());
	if (cmd[0] == "exit" || cmd[0] == "bye")
	{
		return ServerExit;
	}
	if (size < 2)
	{
		fprintf(stderr, "Syntax error\n");
		return 0;
	}
	//using database
	if (cmd[0] == "using")
	{
		return ChangeDatabase(cmd[1]);
	}
	//create database xxx
	if (size >= 3 && cmd[0] == "create" && cmd[1] == "database")
	{
		return CreateDatabase(cmd[2]);
	}
	//other sql
	if (workingName.empty())
	{
		fprintf(stderr, "Working database must be set\n");
		return 0;
	}
	auto it = database.find(workingName);
	if (it == database.end())
	{
		fprintf(stderr, "Server error, cannot find working database %s\n", workingName.data());
		return -1;
	}
	it->second.HandleSql(cmd);
	return 0;
}

int Server::ChangeDatabase(const string &name)
{
	auto it = database.find(name);
	if (it == database.end())
	{
		fprintf(stderr, "Database %s do not exist\n", name.data());
		return -1;
	}
	auto &nextDb = it->second;
	if (!workingName.empty())
	{
		auto workingIt = database.find(workingName);
		if (workingIt == database.end())
		{
			fprintf(stderr, "ChangeDatabase Warning: old working database do not exist\n");
		}
		else
			database[workingName].Stop();
	}
	//Change database
	if (!nextDb.isInited())
	{
		if (nextDb.Init(dir, name))
		{
			fprintf(stderr, "[Change Database]Init database %s failed\n", name.data());
			return -1;
		}
	}
	workingName = name;
	printf("[Change Database]Working database: %s\n", name.data());
	return 0;
}

int Server::SplitCmd()
{
	using std::transform;
	transform(rowCmd.begin(), rowCmd.end(), rowCmd.begin(), tolower);
	int length = static_cast<int>(rowCmd.length());
	int beginIndex = 0;
	int endIndex = 0;
	auto newCmd = [&] {
		auto begin = rowCmd.begin();
		cmd.emplace_back(begin + beginIndex, begin + endIndex);
	};
	for (int i = 0; i < length; ++i)
	{
		//unmatch bracket
		if (rowCmd[i] == ')')
		{
			fprintf(stderr, "Syntax error, bracket unmatch");
			return -1;
		}
		if (rowCmd[i] == ';')
		{
			endIndex = i;
			newCmd();
			return 0;
		}
		if (isspace(rowCmd[i]))
		{
			endIndex = i;
			if (beginIndex >= endIndex)
			{
				beginIndex = i + 1;
				continue;
			}
			newCmd();
			beginIndex = i + 1;
		}
		else if (rowCmd[i] == '(')
		{
			beginIndex = i;
			while (rowCmd[++i] != ')')
			{
				if (rowCmd[i] == ';')
				{
					fprintf(stderr, "Syntex error, bracket unmatch");
					return -1;
				}
			}
			endIndex = i;
			newCmd();
			beginIndex = i + 1;
		}
	}
	return 0;
}

int Server::CreateDatabase(const string name)
{
	using std::ofstream;
	//a new name?
	if (database.find(name) != database.end())
	{
		fprintf(stderr, "[Create Database]Database %s allready exist\n", name.data());
		return -1;
	}
	ofstream fd;
	// touch database file
	string configFile = dir + "//" + name + ".db";
	fd.open(configFile.data());
	if (!fd.is_open())
	{
		fprintf(stderr, "Error, can not create database config file\n");
		return -1;
	}
	fd.close();
	// add name to config file
	configFile = dir + "//" + DATABASELIST;
	fd.open(configFile.data(), ofstream::app);
	if (!fd.is_open())
	{
		fprintf(stderr, "Error, can not open databaselist file\n");
		return -1;
	}
	fd << name << std::endl;
	fd.close();
	// add to map<string, Database> database
	database.emplace(name, Database());
	printf("[Create Database]Succeed\n");
	return 0;
}

// help function
void Server::ListDatabase()
{
	vector<string> names;

	for (auto &v : database)
	{
		names.emplace_back(v.first);
	}
	std::sort(names.begin(), names.end());
	string title("Database");
	PrintList(title, names);
}

void PrintList(const string &title, const vector<string> &names)
{
	using std::max;
	int length = static_cast<int>(title.length());
	for (auto &v : names)
	{
		length = max(length, static_cast<int>(v.length()));
	}
	auto DrawLine = [&] {
		printf("+");
		for (int i = 0; i < length; i++)
			printf("-");
		printf("+");
		printf("\n");
	};
	auto PrintStr = [&](const string &str) {
		printf("|");
		printf("%s", str.data());
		for (int i = 0; i < length - static_cast<int>(str.length()); ++i)
			printf(" ");
		printf("|\n");
	};
	DrawLine();
	PrintStr(title);
	DrawLine();
	for (auto &str : names)
	{
		PrintStr(str);
	}
	DrawLine();
}

void PrintTable(const vector<string> &title,
				const vector<vector<string>> &names)
{
	using std::max;
	int rowNum = title.size();
	vector<int> rowLength(rowNum);
	// get echo row length
	for (auto &line : names)
	{
		if (static_cast<int>(line.size()) < rowNum)
		{
			fprintf(stderr, "Print Error\n");
			return;
		}
		for (int i = 0; i < rowNum; ++i)
		{
			rowLength[i] = max(rowLength[i], static_cast<int>(line[i].length()));
		}
	}
	for (int i = 0; i < rowNum; ++i)
	{
		rowLength[i] = max(rowLength[i], static_cast<int>(title[i].length()));
	}

	auto DrawLine = [&] {
		for (auto &i : rowLength)
		{
			printf("+");
			for (int j = 0; j < i; ++j)
				printf("-");
		}
		printf("+\n");
	};

	auto PrintLine = [&](const vector<string> &line) {
		for (int i = 0; i < rowNum; ++i)
		{
			printf("|");
			printf("%s", line[i].data());
			for (int j = 0; j < rowLength[i] - static_cast<int>(line[i].length());
				 j++)
				printf(" ");
		}
		printf("|\n");
	};

	DrawLine();
	PrintLine(title);
	DrawLine();
	for (auto &line : names)
	{
		PrintLine(line);
	}
	DrawLine();
}