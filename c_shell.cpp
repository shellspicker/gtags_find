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
#include <set>

#include "idref.h"
#include "dsm_db.h"

using std::ios;
using std::cout;
using std::cin;
using std::string;
using std::stringstream;
using std::fstream;;
using std::vector;
using std::map;
using std::set;
using std::pair;
typedef string bash_ret;

// 运行linux命令并返回.
// 默认原样输出, 如果需要单行的字符串就开启trim_endline.
bash_ret
exec(const string cmd, bool trim_endline = false)
{
	FILE *pipe = popen(cmd.data(), "r");
	char *buf = NULL;
	size_t bsize;
	string ret;

	if (!pipe)
		return ret;

	// c的getline是包含delim的.
	while (getline(&buf, &bsize, pipe) != -1) {
		ret += buf;
		if (trim_endline)
			ret.pop_back();
	}

	pclose(pipe);
	return ret;
}

// 原本没有换行符的字符串列表, 输出时加上换行符.
string
print(vector<string> &vec, bool to_stdout)
{
	string scp;

	for (auto s : vec) {
		if (to_stdout)
			cout << s << '\n';
		else
			scp += s + '\n';
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
set<string> sst;
dsm_db db;

void
dfs(const string pattern, string *fa)
{
	bash_ret bret, file, line, func;
	string cmd;

	// have visit?
	if (sst.count(pattern))
		return;

	// set visit tag.
	sst.insert(pattern);

	// set road tag.
	if (!fa)
		path.push_back(pattern);
	else
		path.push_back(*fa);

	// extend next point.
	db.get(pattern, &bret);

	if (!bret.length()) {
		stringstream ss;

		ss << "./find.sh '" << pattern << "'";
		ss_to_s(ss, cmd);
		bret = exec(cmd);
		if (!bret.length())
			bret = string("notfoundshit");
		db.put(pattern, bret);
	}

	// is leaf?
	if (bret == "notfoundshit") {
		print(path, 1);
		cout << '\n';
		goto dfs_end;
	}

	{
		stringstream line_cp(bret);
		string one_line;
		for (int i = 0; getline(line_cp, one_line, '\n'); ++i) {
			string tonxt, nxt_query;
			stringstream ss;

			ss << "echo \"" << one_line << "\" | awk '{print $3}'";
			ss_to_s(ss, cmd, '\0');
			func = exec(cmd, true);

			ss << "echo \"" << one_line << "\" | awk '{print $4}'";
			ss_to_s(ss, cmd, '\0');
			file = exec(cmd, true);

			ss << "echo \"" << one_line << "\" | awk '{print $5}'";
			ss_to_s(ss, cmd, '\0');
			line = exec(cmd, true);

			nxt_query = func;
			tonxt += func + ':';
			tonxt += file + ':';
			tonxt += line;
			dfs(nxt_query, &tonxt);
		}
	}

dfs_end:
	//clear visit tag.
	sst.erase(pattern);
	//clear road tag.
	path.pop_back();
}

void
query(const string &query_pattern)
{
	db.open("road_table");
	string ques(query_pattern), cmd, qcount, qall;
	stringstream ss;
	ss << "global -x " << ques << " | wc -l";
	ss_to_s(ss, cmd);
	qcount = exec(cmd, true);
	ss << "global -x " << ques << " | awk {'print $1'}";
	ss_to_s(ss, cmd);
	qall = exec(cmd);
	if (qcount != "1") {
		cout << "just input exactly 1 string, if use regex, must ensure that "
			"query string is only one, thus find.sh can't deal.\n";
		cout << "now query string count: " << qcount << '\n';
		cout << qall;
		return;
	}
	ss << "global -x " << ques << " | awk {'print $1'}";
	ss_to_s(ss, cmd);
	ques = exec(cmd, true);
	dfs(ques, NULL);
}

bool
arg_parse(int argc, char **argv)
{
	if (argc != 2)
		return false;

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
