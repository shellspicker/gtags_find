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
typedef vector<string> bash_ret;

// 运行linux命令并返回
void
exec(const char *cmd, bash_ret &vec)
{
	//printf("%s cmd is like below:\n"
	//		"%s\n"
	//		"exec cmd end\n" , __func__, cmd);
	FILE *pipe = popen(cmd, "r");
	char *buf = NULL;
	int nread;
	size_t bsize;
	string line;
	vec.clear();

	if (!pipe) {
		return;
	}

	while ((nread = getline(&buf, &bsize, pipe)) != -1) {
		buf[nread - 1] = 0;
		line = buf;
		vec.push_back(line);
	}

	free(buf);
	pclose(pipe);
}

char *print(bash_ret &vec, bool to_stdout)
{
	//if (to_stdout)
	//	printf("%s stdout:\n", __func__);
	char *ret = NULL;
	string scp;
	for (auto s : vec) {
		if (to_stdout)
			cout << s << '\n';
		else {
			scp += s + '\n';
		}
	}
	if (!to_stdout)
		ret = strdup(scp.c_str());
	return ret;
}

vector<string> path;
map<string, int> mmp;
double clk1, clk2;

void dfs(char *pattern)
{
	bash_ret bret, file, line, func;
	string cmd, qs = pattern;
	char *tmp, buf[10233];

	if (mmp[qs] == 1) {
		return;
	}
	mmp[qs] = 1;

	sprintf(buf, "./find.sh \'%s\'", pattern);
	cmd = buf;
	exec(cmd.c_str(), bret);

	if (!bret.size()) {
		print(path, 1);
		putchar(10);
		return;
	}
	for (long i = 0; i < bret.size(); ++i) {
		char *nxt_query;
		string nxt;
		tmp = print(bret, 0);
		sprintf(buf, "echo \"%s\" | sed -n %dp | awk '{print $3}'",
				tmp, i + 1);
		free(tmp);
		cmd = buf;
		exec(cmd.c_str(), func);

		tmp = print(bret, 0);
		sprintf(buf, "echo \"%s\" | sed -n %dp | awk '{print $4}'",
				tmp, i + 1);
		free(tmp);
		cmd = buf;
		exec(cmd.c_str(), file);
		tmp = print(bret, 0);
		sprintf(buf, "echo \"%s\" | sed -n %dp | awk '{print $5}'",
				tmp, i + 1);
		free(tmp);
		cmd = buf;
		exec(cmd.c_str(), line);

		tmp = print(func, 0);
		nxt += func[0] + ':';
		nxt_query = strdup(tmp);
		free(tmp);
		tmp = print(file, 0);
		nxt += file[0] + ':';
		free(tmp);
		tmp = print(line, 0);
		nxt += line[0];
		free(tmp);

		path.push_back(nxt);
		dfs(nxt_query);
		path.pop_back();
		free(nxt_query);
	}
	mmp[qs] = 0;
}

void query(char *qstr)
{
	string qs = qstr;
	mmp.clear();
	path.push_back(qs);
	dfs(qstr);
}

bool arg_parse(int argc, char **argv)
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
