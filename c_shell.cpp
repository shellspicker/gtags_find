#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <vector>

// 运行linux命令并返回
void
exec(const char *cmd, std::vector<std::string> &vec)
{
	printf("%s cmd is like below:\n%s\n", __func__, cmd);
	FILE *pipe = popen(cmd, "r");
	char *buf = NULL;
	int nread;
	size_t bsize;
	std::string line;

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

int
main()
{
	std::ios::sync_with_stdio(false);
	std::cin.tie(0);

	char *cmd = strdup("echo $(ls)");
	std::vector<std::string> vec_str;
	exec(cmd, vec_str);

	for (auto s : vec_str) {
		std::cout << s << '\n';
	}
	return 0;
}
