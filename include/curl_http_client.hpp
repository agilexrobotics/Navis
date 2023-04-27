
#pragma once

#include <curl/curl.h>
#include <string>
#include <iostream>
#include "json.hpp"
using namespace nlohmann;
using namespace std;


class HttpClient {
public:
    HttpClient() {
        curl_global_init(CURL_GLOBAL_ALL);
        curl = curl_easy_init();
    }

    ~HttpClient() {

        curl_global_cleanup();
    }

    const char * get_token(){
        if (!m_token.empty()) {
           return  m_token.c_str();
        } else {
           return  nullptr;
        }
    }
    void set_token(std::string _token){
        m_token = "Authorization:"+ _token;
    }
    // 登陆函数函数，主要为了设置请求头中的auth token。
    bool login_()
    {

        std::string login_str,login_url,response,_auth;
        login_str = "{\"username\":\"agilex\",\"password\":\"NTdhMTE5NGNiMDczY2U4YjNiYjM2NWU0YjgwNWE5YWU=\"}";

        login_url = "/user/passport/login";
        response = this->post(login_url, login_str, false);

        json res = json::parse(response);
        if ( 0 != res.at("code"))
            return  false;
        _auth = res.at("data");

        this->set_token(_auth);
        return  true;
    }

    std::string get(const std::string& url) {
        CURLcode res;
        std::string readBuffer;
        struct curl_slist* headers_ = NULL;

        //headers_ = curl_slist_append(headers_, "content-type:application/json");
        headers_ = curl_slist_append(headers_,  get_token());//设置请求头中的Authorization，否则会请求失败

        if (headers_ == NULL){
            return "headers = null";
        }
        std::string temp_utl = m_url + url;
        curl = curl_easy_init();
        if (curl) {

            curl_easy_setopt(curl, CURLOPT_URL, temp_utl.c_str());
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20);
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers_);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            res = curl_easy_perform(curl);

            if (res != CURLE_OK) {
                std::cerr << "ERROR: " << curl_easy_strerror(res) << std::endl;
            }

        }
        curl_slist_free_all(headers_);
        curl_easy_cleanup(curl);
        return readBuffer;
    }
    //post 请求
    std::string post(const std::string& url, const std::string& data, bool is_token = true) {
        CURLcode res;
        std::string readBuffer;
        struct curl_slist* headers = NULL;

        headers = curl_slist_append(headers, "content-type:application/json");
        if (is_token) {
            headers = curl_slist_append(headers,  get_token());
        }
        if (!headers){
            return "headers = null";
        }
        std::string temp_utl = m_url + url;
        curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, temp_utl.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());//post 请求

            res = curl_easy_perform(curl);

            if (res != CURLE_OK) {
                std::cerr << "ERROR: " << curl_easy_strerror(res) << std::endl;
            }

        }
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return readBuffer;
    }

    //del 请求
    std::string del(const std::string& url) {
        CURLcode res;
        std::string readBuffer;
        struct curl_slist* headers = NULL;
        curl = curl_easy_init();
        //headers = curl_slist_append(headers, "content-type:application/json");
        headers = curl_slist_append(headers,  get_token());
        std::string temp_utl = m_url + url;
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, temp_utl.c_str());
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            res = curl_easy_perform(curl);

            if (res != CURLE_OK) {
                std::cerr << "ERROR: " << curl_easy_strerror(res) << std::endl;
            }
        }
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return readBuffer;
    }
    void set_url(std::string url_)
    {
        m_url = url_;
    }
    //下载文件，并且保存 请求
    int download_file(const std::string & url, const char* outfilename){

        FILE *fp;
        CURLcode res;
        /*   调用curl_global_init()初始化libcurl  */
        res = curl_global_init(CURL_GLOBAL_ALL);
        if (CURLE_OK != res)
        {
            printf("init libcurl failed.");
            return -1;
        }
        /*  调用curl_easy_init()函数得到 easy interface型指针  */
        curl = curl_easy_init();
        if (curl) {
            fp = fopen(outfilename,"wb+");

            std::string temp_utl = m_url + url;
            /*  调用curl_easy_setopt()设置传输选项 */
            res = curl_easy_setopt(curl, CURLOPT_URL, temp_utl.c_str());
            if (res != CURLE_OK)
            {
                fclose(fp);
                curl_easy_cleanup(curl);
                return -1;
            }
            /*  根据curl_easy_setopt()设置的传输选项，实现回调函数以完成用户特定任务  */
            res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writedata2file);
            if (res != CURLE_OK){
                fclose(fp);
                curl_easy_cleanup(curl);
                return -1;
            }
            /*  根据curl_easy_setopt()设置的传输选项，实现回调函数以完成用户特定任务  */
            res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
            if (res != CURLE_OK)
            {
                fclose(fp);
                curl_easy_cleanup(curl);
                return -1;
            }

            res = curl_easy_perform(curl);
            // 调用curl_easy_perform()函数完成传输任务
            fclose(fp);
            /* Check for errors */
            if(res != CURLE_OK){
                fprintf(stderr, "curl_easy_perform() failed: %s\n",curl_easy_strerror(res));
                curl_easy_cleanup(curl);
                return -1;
            }

            /* always cleanup */
            curl_easy_cleanup(curl);
            // 调用curl_easy_cleanup()释放内存

        }
        return 0;
    }


private:
    CURL* curl;
    std::string m_url = "";//ip + port
    std::string m_token = "";//

    //请求成功的回调函数
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }
    //请求写数据成功的回调函数
    static size_t writedata2file(void *ptr, size_t size, size_t nmemb, FILE *stream) {
        size_t written = fwrite(ptr, size, nmemb, stream);
        return written;
    }

};

