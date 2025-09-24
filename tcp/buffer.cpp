#include "buffer.h"
#include <string.h>
#include <iostream>

const std::string& Buffer::buf() const {return buf_;}

const char* Buffer::c_str() const {return buf_.c_str();}

void Buffer::set_buf(const char* buf) {
    std::string new_buf(buf);
    buf_.swap(new_buf);
}

size_t Buffer::Size() const { return buf_.size();}
void Buffer::Append(const char* _str, int _size){
    for(int i = 0;i < _size;++i) {
        //这里注意""和''的区别
        if (_str[i] == '\0'){break;}
        buf_.push_back(_str[i]);
    }
}


void Buffer::Clear(){
    buf_.clear();
}