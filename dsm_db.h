#include <leveldb/db.h>

using ldb = leveldb::DB;
using ldb_opt = leveldb::Options;
using ldb_sta = leveldb::Status;
using ldb_opt_r = leveldb::ReadOptions;
using ldb_opt_w = leveldb::WriteOptions;
using std::string;

class dsm_db
{
	ldb *db;
	ldb_opt opt;
	ldb_sta fake;
	ldb_opt_r opt_r;
	ldb_opt_w opt_w;
  public:
	dsm_db();
	void
	open(string);
	int
	put(string, string);
	int
	get(string, string *);
	int
	del(string);
};
