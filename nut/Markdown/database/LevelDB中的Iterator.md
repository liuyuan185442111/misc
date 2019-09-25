## Iterator
void Iterator::seek(const Slice& target); // 定位到第一个key大于等于target的位置
以下的Key指的是InternalKey。
## MemTableIterator
用于对MemTable进行遍历和查找。
在内部转调用MemTable::Table::Iterator， MemTable::Table是SkipList的typedef ，MemTable::Table::Iterator是一个内部类，没有继承自Iterator，避免了一些虚函数调用开销。
key():Key
value():Value

MemTable::NewIterator()返回指向MemTableIterator的指针。
## Block::Iter
封装了block内部的前缀压缩和重启点细节，用于对block中的key-value遍历和查找。
因为Data 	Block和Index Block使用了相同的格式，二者的Iterator所表示的key和value的意义不同。
对于Data Block
key():Key
value():Value
对于Index Block
key():对应Data Block的last_key的ShortestSeparator（可以简单理解为Data Block的largest key）
value(): 对应Data Block的BlockHandle的变长编码

Block::NewIterator()返回指向Block::Iter的指针。
## Version::LevelFileNumIterator
非level 0的sst元信息集合的Iterator，用来遍历和查找sst文件。
key():FileMetaData的largest
value():FileMetaData的number和file_size的fixed64编码

做为TwoLevelIterator的index_iter使用。
## TwoLevelIterator
考虑在一个sst文件中进行查找，可以先加载Index Block，并用其Iterator找到特定的Data Block，然后通过Data Block的Iterator找到具体的data。
对于类似这种需要定位index，然后根据index定位到具体data的使用方式，封装成TwoLevelIterator使用。
TwoLevelIterator包含index_iter和data_iter，另外需要一个block_function用来根据index_iter的value()初始化data_iter。

Table::NewIterator()返回的是指向TwoLevelIterator的指针，其index_iter是Index Block的Iterator，block_function是Table::BlockReader()，它的功能是根据index_iter的value()找到（此处有block cache）并返回该Data Block的Iterator。
于是Table::NewIterator()就能用来遍历和查找整个sst文件中的key-value了。

========================================
Version::LevelFileNumIterator可以根据一个key找到一个文件的number和file_size，于是可以依靠该文件的number和file_size去FindTable()（此处有table cache），然后就可以使用Table::NewIterator()去查找、遍历了。
TableCache::NewIterator()就用来FindTable()后获得Table::NewIterator()。GetFileIterator()用来调用TableCache::NewIterator()。
于是可以将Version::LevelFileNumIterator做为index_iter，将GetFileIterator()做为block_function又构造出一个TwoLevelIterator，用来对非level 0的sst文件集合中的key-value进行遍历和查找。
## MergingIterator
内部有多个child Iterator，以类似归并的方式进行遍历和查找。
用处一：
compact的时候，所有输入文件形成iterator数组，level 0的一个sst对应一个Iterator（Table::NewIterator()），其他level整个level对应一个Iterator（Version::LevelFileNumIterator和GetFileIterator()构造出的TwoLevelIterator）。
用处二：
参见DBIter。

NewMergingIterator()返回指向MergingIterator的指针。
## DBIter
DBImpl::NewIterator()返回指向DBIter的指针。
DBIter内部实际通过一个MergingIterator工作，NewInternalIterator()返回该MergingIterator，这个MergingIterator的children包括mem一个迭代器、imm一个迭代器、level 0每个sst一个迭代器、其他level每个level一个迭代器。

DBImpl::NewIterator()先通过NewInternalIterator()获取了一个MergingIterator，另外获取了一个sequence number（用户指定或取当前值），然后调用NewDBIterator()返回指向DBIter的指针。

因为是面向用户的，DBIter::key()返回的是user key，DBIter处理了内部多个相同user key的问题。
