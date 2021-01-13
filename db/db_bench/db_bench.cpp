#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <assert.h>
#include <string>
#include <set>
#include <algorithm>
#include <numeric>
#include "random.h"
#include "histogram.h"
using namespace leveldb;
#if defined(__linux)
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif
#include "db.h"

namespace {

//柱形统计
bool FLAGS_histogram = false;
//压缩率, 暂无用
double FLAGS_compression_ratio = 0.5;
//数据量
int FLAGS_num = 1000000;
//读操作次数, 默认与FLAGS_num相同
int FLAGS_reads = -1;
//value大小
int FLAGS_value_size = 100;
//cache_low取cache_high一半
int FLAGS_cache_high = 10240;
//默认数据库文件名
const char* FLAGS_db = "/tmp/wdbbench";
//随机数初始化种子
const int rand_seed = 1000003;
//stderr是否重定向了
bool err_redirect = false;
//新插入权重
int FLAGS_weight_w = 1;
//读权重
int FLAGS_weight_r = 1;
//更新权重
int FLAGS_weight_u = 1;
//热点数据数量
int FLAGS_hot_size = 10000;

const char *FLAGS_benchmarks = 
	"fillseq,"
	"fillrandom,"
	"overwrite,"
	"readrandom,"
	"wru,"
	;

uint64_t NowMicros()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return static_cast<uint64_t>(tv.tv_sec) * 1000000 + tv.tv_usec;
}
void AppendWithSpace(std::string *str, const std::string &msg)
{
	if (msg.empty()) return;
	if (!str->empty()) {
		str->push_back(' ');
	}
	str->append(msg);
}
int ChooseItem(Random &rand, int *weight_vec, int vec_size)
{
	int sum = std::accumulate(weight_vec, weight_vec + vec_size, 0);
	int r = rand.Uniform(sum);
	for(int i=0; i<vec_size; ++i)
	{
		if(r < weight_vec[i]) return i;
		r -= weight_vec[i];
	}
	return vec_size - 1;
}

void RandomString(Random* rnd, int len, std::string* dst)
{
	dst->resize(len);
	for (int i = 0; i < len; i++) {
		(*dst)[i] = static_cast<char>(' ' + rnd->Uniform(95));   // ' ' .. '~'
	}
}
void CompressibleString(Random* rnd, double compressed_fraction, size_t len, std::string* dst)
{
	int raw = static_cast<int>(len * compressed_fraction);
	if (raw < 1) raw = 1;
	std::string raw_data;
	RandomString(rnd, raw, &raw_data);

	// Duplicate the random data until we have filled "len" bytes
	dst->clear();
	while (dst->size() < len) {
		dst->append(raw_data);
	}
	dst->resize(len);
}
// Helper for quickly generating random data.
class RandomGenerator
{
private:
	std::string data_;
	size_t pos_;

public:
	RandomGenerator() {
		// We use a limited amount of data over and over again and ensure
		// that it is larger than the compression window (32KB), and also
		// large enough to serve all typical value sizes we want to write.
		Random rnd(301);
		std::string piece;
		while (data_.size() < 1048576) {
			// Add a short fragment that is as compressible as specified
			// by FLAGS_compression_ratio.
			CompressibleString(&rnd, FLAGS_compression_ratio, 100, &piece);
			data_.append(piece);
		}
		pos_ = 0;
	}

	const char* Generate(size_t len) {
		if (pos_ + len > data_.size()) {
			pos_ = 0;
			assert(len < data_.size());
		}
		pos_ += len;
		return data_.data() + pos_ - len;
	}
};

void TestRandomNext() {
	Random rand(997);
	std::set<int> sets;
	sets.insert(rand.Next());
	for(;;) {
		if(sets.size() % 1000000 == 0)
			printf("current set size: %d millon\n", (int)sets.size()/1000000);
		if(sets.size() >= 20000000) {
			printf("end: current set size: %d\n", (int)sets.size());
			return;
		}
		if(!sets.insert(rand.Next()).second) {
			printf("repeat: current set size: %d\n", (int)sets.size());
			return;
		}
	}
}

class Stats
{
private:
	double start_;
	double finish_;
	double seconds_;
	int done_;
	int next_report_;
	int64_t bytes_;
	double last_op_finish_;
	Histogram hist_;
	std::string message_;

public:
	Stats() { Start(); }

	void Start() {
		next_report_ = 100;
		hist_.Clear();
		done_ = 0;
		bytes_ = 0;
		seconds_ = 0;
		start_ = NowMicros();
		last_op_finish_ = start_;
		finish_ = start_;
		message_.clear();
	}

	void Stop() {
		finish_ = NowMicros();
		seconds_ = (finish_ - start_) * 1e-6;
	}

	void AddMessage(const std::string &msg) {
		AppendWithSpace(&message_, msg);
	}

	void FinishedSingleOp() {
		if (FLAGS_histogram) {
			double now = NowMicros();
			double micros = now - last_op_finish_;
			hist_.Add(micros);
			if (micros > 20000) {
				if(!err_redirect)
					fprintf(stderr, "long op: %.1f micros%30s\r", micros, "");
				else
					fprintf(stderr, "long op: %.1f micros\n", micros);
				fflush(stderr);
			}
			last_op_finish_ = now;
		}

		done_++;
		if (done_ >= next_report_) {
			if      (next_report_ < 1000)   next_report_ += 100;
			else if (next_report_ < 5000)   next_report_ += 500;
			else if (next_report_ < 10000)  next_report_ += 1000;
			else if (next_report_ < 50000)  next_report_ += 5000;
			else if (next_report_ < 100000) next_report_ += 10000;
			else if (next_report_ < 500000) next_report_ += 50000;
			else                            next_report_ += 100000;
			if(!err_redirect) {
				fprintf(stderr, "... finished %d ops%30s\r", done_, "");
				fflush(stderr);
			}
		}
	}

	void AddBytes(int64_t n) {
		bytes_ += n;
	}

	void Report(const std::string& name){
		// Pretend at least one op was done in case we are running a benchmark
		// that does not call FinishedSingleOp().
		if (done_ < 1) done_ = 1;

		std::string extra;
		if (bytes_ > 0) {
			// Rate is computed on actual elapsed time, not the sum of per-thread
			// elapsed times.
			double elapsed = (finish_ - start_) * 1e-6;
			char rate[100];
			snprintf(rate, sizeof(rate), "%6.1f MB/s",
					(bytes_ / 1048576.0) / elapsed);
			extra = rate;
		}
		AppendWithSpace(&extra, message_);

		fprintf(stdout, "%-12s : %11.3f micros/op;%s%s\n",
				name.c_str(),
				seconds_ * 1e6 / done_,
				(extra.empty() ? "" : " "),
				extra.c_str());
		if (FLAGS_histogram) {
			fprintf(stdout, "Microseconds per op:\n%s\n", hist_.ToString().c_str());
		}
		fflush(stdout);
	}
};

}

class Benchmark
{
	int num_;
	int reads_;
	int value_size_;
	Stats stats_;
	lcore::DB *db_;

	void PrintHeader() {
		const int kKeySize = 16;
		PrintEnvironment();
		fprintf(stdout, "Keys:       %d bytes each\n", kKeySize);
		fprintf(stdout, "Values:     %d bytes each (%d bytes after compression)\n",
				FLAGS_value_size, static_cast<int>(FLAGS_value_size * FLAGS_compression_ratio + 0.5));
		fprintf(stdout, "Entries:    %d\n", num_);
		fprintf(stdout, "RawSize:    %.1f MB (estimated)\n",
				((static_cast<int64_t>(kKeySize + FLAGS_value_size) * num_) / 1048576.0));
		//fprintf(stdout, "FileSize:   %.1f MB (estimated)\n",
				//(((kKeySize + FLAGS_value_size * FLAGS_compression_ratio) * num_) / 1048576.0));
		PrintWarnings();
		fprintf(stdout, "------------------------------------------------\n");
		fflush(stdout);
	}

	void PrintWarnings() {
#if defined(__GNUC__) && !defined(__OPTIMIZE__)
		fprintf(stdout, "WARNING: Optimization is disabled: benchmarks unnecessarily slow\n");
#endif
	}

	void PrintEnvironment() {
#if defined(__linux)
		time_t now = time(NULL);
		fprintf(stdout, "Date:       %s", ctime(&now));  // ctime() adds newline

		FILE* cpuinfo = fopen("/proc/cpuinfo", "r");
		if (cpuinfo != NULL) {
			char line[1000];
			int num_cpus = 0;
			std::string cpu_type;
			std::string cache_size;
			while (fgets(line, sizeof(line), cpuinfo) != NULL) {
				const char* sep = strchr(line, ':');
				if (sep == NULL) {
					continue;
				}
				std::string key(line, sep - 1 - line);
				std::string val(sep + 2);
				if (strstr(key.c_str(), "model name")) {
					++num_cpus;
					cpu_type = val;
				} else if (strstr(key.c_str(), "cache size")) {
					cache_size = val;
				}
			}
			fclose(cpuinfo);
			fprintf(stdout, "CPU:        %d * %s", num_cpus, cpu_type.c_str());
			fprintf(stdout, "CPUCache:   %s", cache_size.c_str());
		}
#endif
	}

public:
	Benchmark() : db_(NULL) { }
	~Benchmark() { delete db_; }
	void Reopen(bool reset)
	{
		delete db_;
		if (reset) system((std::string("rm -rf ") + FLAGS_db + "*").c_str());
		db_ = new lcore::DB(FLAGS_db, FLAGS_cache_high, FLAGS_cache_high / 2);
		if(!db_->init()) abort();
	}
	void Checkpoint()
	{
		uint64_t a = NowMicros();
		db_->checkpoint();
		uint64_t b = NowMicros();
		fprintf(stdout, "%13s checkpoint use %.2f msecs\n", "", (b-a)*1e-3);
	}
	bool TryCheckpoint()
	{
		if (db_->hashsize() <= (size_t)FLAGS_cache_high) return false;
		Checkpoint();
		return true;
	}

	void DoWrite(bool seq)
	{
		Random rand(rand_seed);
		RandomGenerator gen;
		int64_t bytes = 0;
		stats_.Start();
		if (num_ != FLAGS_num)
		{
			char msg[100];
			snprintf(msg, sizeof(msg), "(%d ops)", num_);
			stats_.AddMessage(msg);
		}
		for (int i = 0; i < num_; ++i)
		{
			const int k = seq ? i : rand.Next();
			char key[100];
			snprintf(key, sizeof(key), "%016d", k);
			db_->put(key, strlen(key), gen.Generate(value_size_), value_size_, true);
			bytes += value_size_ + strlen(key);
			stats_.FinishedSingleOp();
		}
		stats_.AddBytes(bytes);
	}

	void WriteSeq() { DoWrite(true); }
	void WriteRandom() { DoWrite(false); }

	void ReadRandom()
	{
		Random rand(rand_seed);
		RandomGenerator gen;
		stats_.Start();
		if (num_ != FLAGS_num)
		{
			char msg[100];
			snprintf(msg, sizeof(msg), "(%d ops)", num_);
			stats_.AddMessage(msg);
		}
		int found = 0;
		char val_buf[102400];
		for (int i = 0; i < reads_; ++i)
		{
			const int k = rand.Next();
			char key[100];
			snprintf(key, sizeof(key), "%016d", k);
			size_t val_len = sizeof(val_buf);
			if (db_->find(key, strlen(key), val_buf, val_len))
				++found;
			stats_.FinishedSingleOp();
		}
		char msg[100];
		snprintf(msg, sizeof(msg), "(%d of %d found)", found, num_);
		stats_.AddMessage(msg);
	}

	void WriteReadUpdate()
	{
		RandomGenerator gen;
		std::vector<int> keys_vec;
		Random r1(5939), r2(5399), r3(5333);
		int weight_vec[3] = {FLAGS_weight_w, FLAGS_weight_r, FLAGS_weight_u};
		char key_buf[100];
		char val_buf[102400];
		int n_find = 0, n_update = 0;
		stats_.Start();
		{
			const int k = r2.Next();
			snprintf(key_buf, sizeof(key_buf), "%016d", k);
			int value_size = value_size_/2 + r3.Uniform(value_size_/2);
			db_->put(key_buf, strlen(key_buf), gen.Generate(value_size), value_size, false);
			keys_vec.push_back(k);
		}
		while(keys_vec.size() < (size_t)num_)
		{
			switch(ChooseItem(r1,weight_vec,3))
			{
				case 0:
					{
						const int k = r2.Next();
						snprintf(key_buf, sizeof(key_buf), "%016d", k);
						int value_size = value_size_/2 + r3.Uniform(value_size_/2);
						db_->put(key_buf, strlen(key_buf), gen.Generate(value_size), value_size, false);
						keys_vec.push_back(k);
					}
					break;
				case 1:
					{
						const int k = keys_vec[r3.Uniform(keys_vec.size()) % FLAGS_hot_size];
						snprintf(key_buf, sizeof(key_buf), "%016d", k);
						size_t val_len = sizeof(val_buf);
						if(!db_->find(key_buf, strlen(key_buf), val_buf, val_len))
							abort();
						++n_find;
					}
					break;
				case 2:
					{
						const int k = keys_vec[r3.Uniform(keys_vec.size()) % FLAGS_hot_size];
						snprintf(key_buf, sizeof(key_buf), "%016d", k);
						int value_size = value_size_/2 + r3.Uniform(value_size_/2);
						db_->put(key_buf, strlen(key_buf), gen.Generate(value_size), value_size, true);
						++n_update;
					}
					break;
			}
			stats_.FinishedSingleOp();
		}
		char msg[100];
		snprintf(msg, sizeof(msg), "(w:%d r:%d u:%d)", (int)keys_vec.size(), n_find, n_update);
		stats_.AddMessage(msg);
	}

	void Run()
	{
		num_ = FLAGS_num;
		PrintHeader();

		const char *benchmarks = FLAGS_benchmarks;
		while (benchmarks)
		{
			const char *sep = strchr(benchmarks, ',');
			const char *name;
			size_t len;
			if(sep == NULL)
			{
				name = benchmarks;
				len = strlen(name);
				if (len == 0) break;
				benchmarks = NULL;
			}
			else
			{
				name = benchmarks;
				len = sep - benchmarks;
				benchmarks = sep + 1;
			}

			num_ = FLAGS_num;
			reads_ = FLAGS_reads;
			value_size_ = FLAGS_value_size;

			void (Benchmark::*method)() = NULL;
			bool fresh_db = true;

			if (strncmp(name, "fillseq", len) == 0)
			{
				method = &Benchmark::WriteSeq;
			}
			else if (strncmp(name, "fillrandom", len) == 0)
			{
				method = &Benchmark::WriteRandom;
			}
			else if (strncmp(name, "fill100K", len) == 0)
			{
				num_ /= 1000;
				value_size_ = 100 * 1024;
				method = &Benchmark::WriteRandom;
			}
			else if (strncmp(name, "overwrite", len) == 0)
			{
				fresh_db = false;
				method = &Benchmark::WriteRandom;
			}
			else if (strncmp(name, "readrandom", len) == 0)
			{
				fresh_db = false;
				method = &Benchmark::ReadRandom;
			}
			else if (strncmp(name, "randomnext", len) == 0)
			{
				TestRandomNext();
			}
			else if (strncmp(name, "wru", len) == 0)
			{
				method = &Benchmark::WriteReadUpdate;
			}

			if (method)
			{
				Reopen(fresh_db);
				(this->*method)();
				stats_.Stop();
				stats_.Report(std::string(name, len));
				Checkpoint();
			}
		}
	}
};

int main(int argc, char** argv)
{
#if defined(__linux)
	{
		struct stat buf;
		fstat(fileno(stderr), &buf);
		if(S_ISREG(buf.st_mode))
			err_redirect = true;
	}
#endif

	for (int i = 0; i < argc; i++) {
		printf("%s ", argv[i]);
	}
	printf("\n\n");
	fflush(stdout);

	for (int i = 1; i < argc; i++)
	{
		double d;
		int n;
		char junk;
		if (strncmp(argv[i], "--benchmarks=", strlen("--benchmarks=")) == 0) {
			FLAGS_benchmarks = argv[i] + strlen("--benchmarks=");
		} else if (sscanf(argv[i], "--compression_ratio=%lf%c", &d, &junk) == 1) {
			FLAGS_compression_ratio = d;
		} else if (sscanf(argv[i], "--histogram=%d%c", &n, &junk) == 1 && (n == 0 || n == 1)) {
			FLAGS_histogram = n;
		} else if (sscanf(argv[i], "--num=%d%c", &n, &junk) == 1) {
			FLAGS_num = n;
		} else if (sscanf(argv[i], "--reads=%d%c", &n, &junk) == 1) {
			FLAGS_reads = n;
		} else if (sscanf(argv[i], "--value_size=%d%c", &n, &junk) == 1) {
			FLAGS_value_size = n;
		} else if (strncmp(argv[i], "--db=", 5) == 0) {
			FLAGS_db = argv[i] + 5;
		} else if (sscanf(argv[i], "--cache_high=%d%c", &n, &junk) == 1) {
			FLAGS_cache_high = n;
		} else if (sscanf(argv[i], "--weight_r=%d%c", &n, &junk) == 1) {
			FLAGS_weight_r = n;
		} else if (sscanf(argv[i], "--weight_u=%d%c", &n, &junk) == 1) {
			FLAGS_weight_u = n;
		} else if (sscanf(argv[i], "--hot_size=%d%c", &n, &junk) == 1) {
			FLAGS_hot_size = n;
		} else {
			fprintf(stderr, "Invalid flag '%s'\n", argv[i]);
			fprintf(stderr, "--benchmarks=%s\n", FLAGS_benchmarks);
			fprintf(stderr, "--histogram, --num, --reads, --value_size, --cache_high\n");
			exit(1);
		}
	}
	if(FLAGS_reads == -1)
		FLAGS_reads = FLAGS_num;

	Benchmark().Run();
	return 0;
}

