#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <map>

using std::ios;
using std::cout;
using std::cin;
using std::string;
using std::vector;
using std::map;
using std::pair;
typedef pair<string, int> bash_ret;

// 运行linux命令并返回
bash_ret
exec(const char *cmd, bool trim_endline)
{
	//printf("%s cmd is like below:\n"
	//		"%s\n"
	//		"exec cmd end\n" , __func__, cmd);
	FILE *pipe = popen(cmd, "r");
	char *buf = NULL;
	int nread, line = 0;
	size_t bsize;
	string ret;

	if (!pipe) {
		pclose(pipe);
		return make_pair(ret, line);
	}

	while ((nread = getline(&buf, &bsize, pipe)) != -1) {
		if (trim_endline) {
			buf[nread - 1] = 0;
		}

		ret += buf;
		line++;
	}

	free(buf);
	pclose(pipe);
	return make_pair(ret, line);
}

char *
print(vector<string> &vec, bool to_stdout)
{
	//if (to_stdout)
	//	printf("%s stdout:\n", __func__);
	char *ret = NULL;
	string scp;

	for (auto s : vec) {
		if (to_stdout) {
			cout << s << '\n';
		} else {
			scp += s + '\n';
		}
	}

	if (!to_stdout) {
		ret = strdup(scp.c_str());
	}

	return ret;
}

vector<string> path;
map<string, int> mmp;
map<string, bash_ret> table;
double clk1, clk2;

void
dfs(const char *pattern)
{
	bash_ret bret, file, line, func;
	string cmd, qs = pattern;
	char buf[10233];

	// have visit?
	if (mmp[qs] == 1) {
		return;
	}

	// set road tag.
	// set visit tag.
	mmp[qs] = 1;

	// extend next point.
	auto it = table.find(qs);

	if (it != table.end()) {
		bret = it->second;
	} else {
		sprintf(buf, "./find.sh \'%s\'", pattern);
		cmd = buf;
		bret = exec(cmd.c_str(), false);
		table[qs] = bret;
	}

	// is leaf?
	if (!bret.second) {
		print(path, 1);
		cout << '\n';
		goto dfs_end;
	}

	for (long i = 0; i < bret.second; ++i) {
		string nxt, nxt_query;
		sprintf(buf, "echo \"%s\" | sed -n %dp | awk '{print $3}'",
				bret.first.c_str(), i + 1);
		cmd = buf;
		func = exec(cmd.c_str(), true);

		sprintf(buf, "echo \"%s\" | sed -n %dp | awk '{print $4}'",
				bret.first.c_str(), i + 1);
		cmd = buf;
		file = exec(cmd.c_str(), true);

		sprintf(buf, "echo \"%s\" | sed -n %dp | awk '{print $5}'",
				bret.first.c_str(), i + 1);
		cmd = buf;
		line = exec(cmd.c_str(), true);

		nxt += func.first + ':';
		nxt_query = func.first;
		nxt += file.first + ':';
		nxt += line.first;

		path.push_back(nxt);
		dfs(nxt_query.c_str());
		path.pop_back();
	}

dfs_end:
	//clear visit tag.
	mmp[qs] = 0;
	//clear road tag.
}

void
query(char *qstr)
{
	string qs = qstr;
	mmp.clear();
	path.push_back(qs);
	dfs(qstr);
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
