#include "commom.h"

#include <string>
#include <vector>
#include <memory>
#include <cstring>

static const int kPrePendIndex = 8;
static const int kInitialSize = 8;

class Buffer{
private:
    std::vector<char>  buffer_;
    int read_index_;
    int write_index_;
public:
    DISALLOW_COPY_AND_MOVE(Buffer);
    Buffer();
    ~Buffer();

    char* begin();
    const char* begin() const;

    char* beginread();
    const char* beginread() const;

    char *beginwrite();
    const char *beginwrite() const;

    // 添加数据
    void Append(const char *message);
    void Append(const char *message, int len);
    void Append(const std::string &message);


    // 获得可读大小等
    int readablebytes() const;
    int writablebytes() const;
    int prependablebytes() const;

    void EnsureWritableBytes(int);

    //check data but not updata read_index_
    char* Peek();
    const char* Peek() const;
    std::string PeekAsString(int len);
    std::string PeekAllAsString();

    // 取数据，取出后更新read_index,相当于不可重复取
    // 定长
    void UpdataIndex(int len);
    void ReSetIndex();
    std::string RetrieveAsString(int len);

    // 全部
    //void RetrieveAll();
    //void UpdataIndex();
    std::string RetrieveAllAsString();

    // 某个索引之前
    //void RetrieveUtil(const char *end);
    std::string RetrieveToEndAsString(const char *end);

};