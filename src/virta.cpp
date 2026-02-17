#include "virta.h"

struct curl_slist *log_info;

fs::path exp_data = "data";
fs::path ext_data = "ext_data";
string server;
string url_base = "https://virtonomics.com/api/" + server + "/main/";
string url = url_base;

//typedef enum  {READ, WRITE, REWRITE, RW} FileOpType;

size_t write_callback_file(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    ofstream *file = static_cast<ofstream*>(userdata);
    size_t written = size * nmemb;
    file->write(static_cast<const char*>(ptr), written);
    return written;

}

size_t write_callback_str(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    size_t written = size * nmemb;
    // Cast userp back to your std::string
    ((string*)userdata)->append((char*)ptr, written);
    return written;

}

string valnut (json js, string key, string def)
{
    try
    {
        return (to_string(js.at(key)) == "") ? def :
               (to_string(js.at(key)));
    }
    catch (json::out_of_range)
    {
        return def;
    }
}

bool try_open_file(fstream *file, string filename, FileOpType type, string err_message) //type: read, write, rewrite; return 1 if opened success, 0 if failed
{
    switch(type)
    {
    case READ:
    {
        file->open(filename, fstream::in);
        if (!file->is_open())
        {
            cerr << err_message << endl;
            return false;
        }
        else
        {
            return true;
        }
    }
    case WRITE:
    {
        file->open(filename, fstream::out);
        if (!file->is_open())
        {
            cerr << err_message << endl;
            return false;
        }
        else
        {
            return true;
        }
    }
    case REWRITE:
    {
        file->open(filename, fstream::out | fstream::trunc);
        if (!file->is_open())
        {
            cerr << err_message << endl;
            return false;
        }
        else
        {
            return true;
        }
    }
    case RW:
    {
        file->open(filename, fstream::in | fstream::out);
        if (!file->is_open())
        {
            cerr << err_message << endl;
            return false;
        }
        else
        {
            return true;
        }
    }
    default:
        return false;
    }
}


bool check_access(string serv, int company)
{
    int user_id_company = 0;
    int user_id_cookie = 0;
    struct curl_slist *each = log_info;
    while (each)
    {
        string line(each->data);

        int start_pos = line.find("_mm_user_");
        if (start_pos != string::npos)
        {
            //cout << "data of this cookie line is " << line << endl;
            user_id_cookie = stoi( line.substr(start_pos + 10));
            //cout << "userid by cookie: " << user_id_cookie << endl;
            break;
        }
        each = each->next;
    }

    string str = "https://virtonomics.com/api/" + serv + "/main/company/info?id=" + to_string( company);
    cout << "URL is " << str << endl;

    string compdata;
    if(connect<string>(str, true, &compdata))
    {
        cout << "Connection to check access failed!" << endl;
        return true;
    }
    json data = json::parse(compdata);
    if(valnut(data, "president_user_id", "0") == "null")
    {
        throw "Error! company not exist";
    }
    user_id_company = stoi(data.value("president_user_id", "0"));
    // cout << "intended user id:" << user_id_company << endl;
    // cout << data.dump(4) << endl;

    return !(user_id_company == user_id_cookie);
}

void fill_vs(string str, vector<string> *v)
{
    stringstream ss(str);
    string tmp;
    while (getline(ss, tmp, ' '))
    {
        v->push_back(tmp);
    }
}

bool DownloadParseFormat(string link, json* json_entry, fstream *target, string pathway)
{
    string tmp;
    if (connect<string>(link, true, &tmp))
    {
        cout << "Retrieving info failed!" << endl;
        return false;
    }
    if (tmp == "")
    {
        cout << "Result is empty!" << endl;
        return false;
    }
    try
    {
        (*json_entry) = json::parse(tmp);
        cout << "Size of current json is " << json_entry->size() << endl;
        try_open_file(target, pathway, WRITE, "Error opening file for write.");
        if (target->is_open())
        {
            (*target) << json_entry->dump(4);
        }
        else
        {
            cout << "Res is " << json_entry->dump(4);
        }
    }
    catch(const json::exception& e)
    {
        cerr << "message: " << e.what() << endl;
        if(target->is_open())
        {
            target->close();
        }
        return true;
    }
    target->close();
    return true;
}