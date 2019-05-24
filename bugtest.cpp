//
// leveldb compaction bug test
//

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <deque>
#include <map>
#include <string>
#include <sstream>
#include <tr1/memory>

#include <leveldb/db.h>
#include <leveldb/write_batch.h>

const std::size_t KEYS_COUNT = 10000;
const std::size_t VALUE_SIZE = 1000;

class Snapshot {
public:
	explicit Snapshot(std::tr1::shared_ptr<leveldb::DB> db)
		: Db(db), Handle(db->GetSnapshot())	{}

	~Snapshot() {
		Db->ReleaseSnapshot(Handle);
	}

private:
	std::tr1::shared_ptr<leveldb::DB> Db;
	const leveldb::Snapshot* Handle;
};

std::string makeKey(std::size_t index) {
	std::stringstream s;
	s << index;
	return s.str();
}

std::string makeRandomValue() {
	static const std::string alphabet("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");

	std::string value;
	value.resize(VALUE_SIZE);
	for (std::size_t i = 0; i < value.size(); ++i) {
		value[i] = alphabet[rand() % alphabet.size()];
	}

	return value;
}

int main(int argc, char* argv[]) {
	srand(time(0));

	std::string dbName = "testdb";
	std::size_t maxSnapshots = 10;
	if (argc > 1) {
		dbName = argv[1];
	}
	if (argc > 2) {
		maxSnapshots = atoi(argv[2]);
	}

	leveldb::Options opts;
	leveldb::DestroyDB(dbName, opts);

	leveldb::DB* rawDb = 0;
	opts.create_if_missing = true;
	leveldb::Status s = leveldb::DB::Open(opts, dbName, &rawDb);
	if (!s.ok()) {
		std::cerr << "Failed to open DB " << dbName << std::endl;
		return 1;
	}

	std::tr1::shared_ptr<leveldb::DB> db(rawDb);
	leveldb::ReadOptions rOpts;
	leveldb::WriteOptions wOpts;

	typedef std::map<std::string, std::string> KeyValueStore;
	KeyValueStore store;
	for (std::size_t i = 0; i < KEYS_COUNT; ++i) {
		const std::string& key = makeKey(i);
		const std::string& value = makeRandomValue();
		store[key] = value;
		s = db->Put(wOpts, key, value);
		if (!s.ok()) {
			std::cerr << "Failed to write key=" << key << std::endl;
			return 1;
		}
	}

	std::deque<std::tr1::shared_ptr<Snapshot> > snapshots;
	while (true) {
		std::size_t modifyCount = (rand() % KEYS_COUNT) * 0.05;
		for (std::size_t i = 0; i < modifyCount; ++i) {
			std::size_t index = rand() % KEYS_COUNT;
			const std::string& key = makeKey(index);

			std::string dbValue;
			s = db->Get(rOpts, key, &dbValue);
			if (s.IsNotFound()) {
				std::cerr << "Value MISSING key=" << key << std::endl;
				return 1;
			}
			if (!s.ok()) {
				std::cerr << "Failed to read key=" << key << std::endl;
				return 1;
			}

			if (dbValue != store[key]) {
				std::cerr << "Value MISMATCH key=" << key << std::endl;
				return 1;
			}

			const std::string& newValue = makeRandomValue();
			store[key] = newValue;
			s = db->Put(wOpts, key, newValue);
			if (!s.ok()) {
				std::cerr << "Failed to write key=" << key << std::endl;
				return 1;
			}
		}

		if (maxSnapshots && rand() % 100 < 10) {
			if (snapshots.size() >= maxSnapshots) {
				snapshots.pop_front();
			}
			snapshots.push_back(std::tr1::shared_ptr<Snapshot>(new Snapshot(db)));
		}

		std::cout << "modified=" << modifyCount << std::endl;
	}

	return 0;
}

// find the largest key in a vector of files. returns true if files it not empty
bool FindLargestKey(const InternalKeyComparator & icmp, const std::vector<FileMetaData*> & files, InternalKey *largestKey) {
  if (files.empty()) {
    return false;
  }
  *largestKey = files[0]->largest;
  for (size_t i = 1; i < files.size(); ++i) {
    FileMetaData* f = files[i];
    if (icmp.Compare(f->largest, *largestKey) > 0) {
      *largestKey = f->largest;
    }
  }
  return true;
}

// find minimum file b2=(l2, u2) in level file for which l2 > u1 and user_key(l2) = user_key(u1)
FileMetaData* FindSmallestBoundaryFile(const InternalKeyComparator & icmp,
                                       const std::vector<FileMetaData*> & levelFiles,
                                       const InternalKey & largestKey) {
  const Comparator* user_cmp = icmp.user_comparator();
  FileMetaData* smallestBoundaryFile = NULL;
  for (size_t i = 0; i < levelFiles.size(); ++i) {
    FileMetaData* f = levelFiles[i];
    if (icmp.Compare(f->smallest, largestKey) > 0 &&
        user_cmp->Compare(f->smallest.user_key(), largestKey.user_key()) == 0) {
      if (smallestBoundaryFile == NULL ||
          icmp.Compare(f->smallest, smallestBoundaryFile->smallest) < 0) {
        smallestBoundaryFile = f;
      }
    }
  }
  return smallestBoundaryFile;
}

// If there are two blocks, b1=(l1, u1) and b2=(l2, u2) and
// user_key(u1) = user_key(l2), and if we compact b1 but not
// b2 then a subsequent get operation will yield an incorrect
// result because it will return the record from b2 in level
// i rather than from b1 because it searches level by level
// for records matching the supplied user key.
//
// This function extracts the largest file b1 from compactionFiles
// and then searches for a b2 in levelFiles for which user_key(u1) =
// user_key(l2). If it finds such a file b2 (known as a boundary file)
// it adds it to compactionFiles and then searches again using this
// new upper bound.
//
// parameters:
//   in     levelFiles:      list of files to search for boundary files
//   in/out compactionFiles: list of files to extend by adding boundary files
void AddBoundaryInputs(const InternalKeyComparator& icmp,
                       const std::vector<FileMetaData*>& levelFiles,
                       std::vector<FileMetaData*>* compactionFiles) {
  InternalKey largestKey;

  // find largestKey in compactionFiles, quick return if compactionFiles is
  // empty
  if (!FindLargestKey(icmp, *compactionFiles, &largestKey)) {
    return;
  }

  bool continueSearching = true;
  while (continueSearching) {
    FileMetaData* smallestBoundaryFile =
        FindSmallestBoundaryFile(icmp, levelFiles, largestKey);

    // if a boundary file was found advance largestKey, otherwise we're done
    if (smallestBoundaryFile != NULL) {
      compactionFiles->push_back(smallestBoundaryFile);
      largestKey = smallestBoundaryFile->largest;
    } else {
      continueSearching = false;
    }
  }
}
