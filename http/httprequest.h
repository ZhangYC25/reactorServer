#pragma once
#include <string>
#include <map>

class HttpRequest{
public:
    enum Method{
        kInvalid = 0,
        kGet,
        kPost,
        kPut,
        kDelete,
        kHead
    };
    enum Version{
        kUnknown = 0,
        kHttp10,
        kHttp11
    };
    HttpRequest();
    ~HttpRequest();

    void SetVersion(const std::string &ver);
    Version version() const;
    std::string GetVersionString() const;

    bool SetMethod(const std::string &method);
    Method method() const;
    std::string GetMethodString() const;

    void SetUrl(const std::string &url);
    const std::string& url() const;

    void SetRequestParams(const std::string &key, const std::string &value);
    std::string GetRequestValue(const std::string &key) const;
    const std::map<std::string, std::string>& request_params() const;

    void SetProtocol(const std::string &str);
    const std::string& protocol() const;

    void AddHeader(const std::string &field, const std::string &value);
    std::string GetHeader(const std::string &field) const;
    const std::map<std::string, std::string>& headers() const;

    void SetBody(const std::string &str);
    const std::string& body() const;
private:
    Method method_;
    Version version_;
    std::string url_;
    std::string protocol_;

    std::map<std::string, std::string> headers_;
    std::string body_;
    std::map<std::string, std::string> request_params_;
};