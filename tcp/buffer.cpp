#include "buffer.h"
#include <string.h>
#include <iostream>
#include <assert.h>

Buffer::Buffer() : buffer_(kInitialSize), read_index_(kPrePendIndex), write_index_(kPrePendIndex){}
Buffer::~Buffer(){}

char *Buffer::begin() { return &*buffer_.begin(); }
const char *Buffer::begin() const { return &*buffer_.begin(); }
char* Buffer::beginread() { return begin() + read_index_; } 
const char* Buffer::beginread() const { return begin() + read_index_; }
char* Buffer::beginwrite() { return begin() + write_index_; }
const char* Buffer::beginwrite() const { return begin() + write_index_; }



void Buffer::Append(const char* message){
    Append(message, static_cast<int>(strlen(message)));
}
void Buffer::Append(const std::string &message){
    Append(message.data(), static_cast<int>(message.size()));
}
void Buffer::Append(const char* message, int len) {
    //reset write_index_ and read_index_
    EnsureWritableBytes(len);
    std::copy(message, message + len, beginwrite());
    write_index_ += len;
}

int Buffer::readablebytes() const {return write_index_ - read_index_;}
int Buffer::writablebytes() const {return static_cast<int>(buffer_.size()) - write_index_;}
int Buffer::prependablebytes() const{return read_index_;}



void Buffer::EnsureWritableBytes(int len){
    if(writablebytes() >= len) return;
    if(writablebytes() + prependablebytes() >= kPrePendIndex + len){
        std::copy(beginread(), beginwrite(), begin()+kPrePendIndex);
        write_index_ = kPrePendIndex + readablebytes();
        read_index_ = kPrePendIndex;
    } else {
        buffer_.resize(write_index_ + len);
    }
}

char *Buffer::Peek() { return beginread(); }
const char *Buffer::Peek() const { return beginread(); }

std::string Buffer::PeekAsString(int len){
    return std::string(beginread(), beginread() + len);
}

std::string Buffer::PeekAllAsString(){
    return std::string(beginread(), beginwrite());
}

void Buffer::UpdataIndex(int len){
    assert(readablebytes() >= len);
    if (len + read_index_ < write_index_){
        read_index_ += len;
    } else {
        ReSetIndex();
    }
}

void Buffer::ReSetIndex(){
    write_index_ = kPrePendIndex;
    read_index_ = write_index_;
}
std::string Buffer::RetrieveAsString(int len){
    assert(read_index_ + len <= write_index_);

    std::string ret = std::move(PeekAsString(len));
    UpdataIndex(len);
    return ret;
}

std::string Buffer::RetrieveAllAsString(){
    assert(readablebytes() > 0);
    std::string ret = std::move(PeekAllAsString());
    ReSetIndex();
    return ret;
}

//void Buffer::RetrieveUtil(const char* end){
//    assert(beginwrite() >= end);
//    read_index_ += static_cast<int>(end - beginread());
//}

std::string Buffer::RetrieveToEndAsString(const char *end){
    assert(beginwrite() >= end);
    std::string ret = std::move(PeekAsString(static_cast<int>(end - beginread())));
    read_index_ += static_cast<int>(end - beginread());
    return ret;
}