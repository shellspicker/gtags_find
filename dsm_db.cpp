#include "dsm_db.h"

dsm_db::dsm_db()
{
	db = nullptr;
	opt.create_if_missing = true;
}

void
dsm_db::open(string name)
{
	fake = ldb::Open(opt, name, &db);
	assert(fake.ok());
}

int
dsm_db::put(string key, string value)
{
	assert(db);
	fake = db->Put(opt_w, key, value);
	return fake.ok();
}

int
dsm_db::get(string key, string *value)
{
	assert(db);
	fake = db->Get(opt_r, key, value);
	return fake.ok();
}

int
dsm_db::del(string key)
{
	assert(db);
	fake = db->Delete(opt_w, key);
	return fake.ok();
}
