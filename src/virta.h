#ifndef _VIRTA_H_
#define _VIRTA_H_

#include <iostream>
#include <string>
#include <curl/curl.h>
#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono>


#include "json.hpp"

using namespace std;
namespace fs = std::filesystem;
using json = nlohmann::json;

extern struct curl_slist *log_info;
extern fs::path exp_data;
extern fs::path ext_data;

extern string server;
extern string url_base;
extern string url;

typedef enum  {READ, WRITE, REWRITE, RW} FileOpType;

extern size_t write_callback_file(char *ptr, size_t size, size_t nmemb, void *userdata);

extern size_t write_callback_str(char *ptr, size_t size, size_t nmemb, void *userdata);


template <typename T>
extern int connect(string link, bool cookies_exist, T *WriteTo, string post_params = "") //cookies exist: false - create, true - use
{
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if(curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, link.c_str());
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
        curl_easy_setopt(curl, CURLOPT_CA_CACHE_TIMEOUT, 604800L);
        if (post_params != "")
        {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_params.c_str());
        }
        if (cookies_exist) //use cookies for extended info
        {
            struct curl_slist *each = log_info;
            while (each)
            {
                // Use CURLOPT_COOKIELIST to inject the cookie string
                res = curl_easy_setopt(curl, CURLOPT_COOKIELIST, each->data);
                if (res != CURLE_OK)
                {
                    fprintf(stderr, "curl_easy_setopt(CURLOPT_COOKIELIST) failed: %s\n", curl_easy_strerror(res));
                }
                each = each->next;
            }
        }
        else
        {
            curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
        }
        if constexpr (std::is_same_v<T, string>)
        {
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback_str);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, WriteTo);
        }
        else if constexpr (std::is_same_v<T, fstream>)
        {
            if (WriteTo->is_open())
            {
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback_file);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, WriteTo);
            }
        }
        //cout << "Type used for this connect:" << typeid(T).name() << endl;

        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

        /* Perform the request, result gets the return code */
        res = curl_easy_perform(curl);
        /* Check for errors */
        if(res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
            return 1;
        }


        if (!cookies_exist) //retrieve cookies after log-in
        {
            res = curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &log_info);
            if(res != CURLE_OK)
            {
                fprintf(stderr, "curl curl_easy_getinfo failed: %s\n",
                        curl_easy_strerror(res));
                return 1;
            }
        }

        /* always cleanup */
        curl_easy_cleanup(curl);
        //
        this_thread::sleep_for(20ms);
    }

    return 0;
}
//extern int connect(string link, bool cookies_exist, string *strr, string post_params = "");

//extern int connect(string link, bool cookies_exist, fstream *file, string post_params = "");

extern string valnut (json js, string key, string def);

extern bool try_open_file(fstream *file, string filename, FileOpType type, string err_message);


extern bool check_access(string serv, int company);

extern void fill_vs(string str, vector<string> *v);

extern bool DownloadParseFormat(string link, json* json_entry, fstream *target, string pathway);

#endif //_VIRTA_H_
