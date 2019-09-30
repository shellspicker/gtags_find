#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

#include "leveldb/db.h"
#include "dsm_db.h"

using std::ios;
using std::cout;
using std::cin;
using std::string;
using std::stringstream;
using std::fstream;;
using std::vector;
using std::map;
using std::pair;
typedef pair<string, int> bash_ret;

// 运行linux命令并返回.
// 默认原样输出, 如果需要单行的字符串就开启trim_endline.
bash_ret
exec(const string cmd, bool trim_endline = false)
{
	FILE *pipe = popen(cmd.data(), "r");
	char *buf = NULL;
	size_t bsize;
	string ret;
	int line = 0;

	if (!pipe) {
		return make_pair(ret, line);
	}

	// c的getline是包含delim的.
	while (getline(&buf, &bsize, pipe) != -1) {
		ret += buf;
		if (trim_endline) {
			ret.pop_back();
		}
		line++;
	}

	pclose(pipe);
	return make_pair(ret, line);
}

// 原本没有换行符的字符串列表, 输出时加上换行符.
string
print(vector<string> &vec, bool to_stdout)
{
	string scp;

	for (auto s : vec) {
		if (to_stdout) {
			cout << s << '\n';
		} else {
			scp += s + '\n';
		}
	}

	return scp;
}

// 代替sprintf的输出流到字符串的转换.
void ss_to_s(stringstream &ss, string &s, char delim = '\n')
{
	s.clear();
	getline(ss, s, delim);
	ss.clear();
}

vector<string> path;
map<string, int> mmp;
map<string, bash_ret> table;
double clk1, clk2;

void
dfs(const string pattern, string *fa)
{
	bash_ret bret, file, line, func;
	string cmd;

	// have visit?
	if (mmp[pattern] == 1) {
		return;
	}

	// set visit tag.
	mmp[pattern] = 1;

	// set road tag.
	if (!fa) {
		path.push_back(pattern);
	} else {
		path.push_back(*fa);
	}

	// extend next point.
	auto it = table.find(pattern);

	if (it != table.end()) {
		bret = it->second;
	} else {
		stringstream ss;

		ss << "./find.sh '" << pattern << "'";
		ss_to_s(ss, cmd);
		bret = exec(cmd);
		table[pattern] = bret;
	}

	// is leaf?
	if (!bret.second) {
		print(path, 1);
		cout << '\n';
		goto dfs_end;
	}

	for (long i = 0; i < bret.second; ++i) {
		string tonxt, nxt_query;
		stringstream ss;

		ss << "echo \"" << bret.first << "\" | sed -n '" <<
			(i + 1) << "p' | awk '{print $3}'";
		ss_to_s(ss, cmd, '\0');
		func = exec(cmd, true);

		ss << "echo \"" << bret.first << "\" | sed -n '" <<
			(i + 1) << "p' | awk '{print $4}'";
		ss_to_s(ss, cmd, '\0');
		file = exec(cmd, true);

		ss << "echo \"" << bret.first << "\" | sed -n '" <<
			(i + 1) << "p' | awk '{print $5}'";
		ss_to_s(ss, cmd, '\0');
		line = exec(cmd, true);

		nxt_query = func.first;
		tonxt += func.first + ':';
		tonxt += file.first + ':';
		tonxt += line.first;
		dfs(nxt_query, &tonxt);
	}

dfs_end:
	//clear visit tag.
	mmp[pattern] = 0;
	//clear road tag.
	path.pop_back();
}

void
query(const string &query_pattern)
{
	mmp.clear();
	dfs(query_pattern, NULL);
}

bool
arg_parse(int argc, char **argv)
{
	if (argc != 2) {
		return false;
	}

	return true;
}

int
main(int argc, char **argv)
{
	ios::sync_with_stdio(false);
	cin.tie(0);

	if (!arg_parse(argc, argv)) {
		cout << "arg error\n";
		return -1;
	}

	char *input = argv[1];
	query(input);
	return 0;
}
