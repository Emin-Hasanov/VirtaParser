#include "virta.h"

int wait_time = 1;
struct curl_slist *log_info;

fs::path exp_data = "data";
fs::path ext_data = "ext_data";
fstream file;
string server;
string url_base = "https://virtonomics.com/api/" + server + "/main/";
string url = url_base;
bool work_local = false;

string VmaLogin;
string VmaPass;

//typedef enum  {READ, WRITE, REWRITE, RW} FileOpType;

size_t write_callback_file(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    ofstream *temp = static_cast<ofstream*>(userdata);
    size_t written = size * nmemb;
    temp->write(static_cast<const char*>(ptr), written);
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
        auto temp = js.at(key);
        //cout << temp << endl;
        json emp;
        if (temp == emp)
            return def;
        else
        {
            return js.value(key, def);
        }
    }
    catch(json::out_of_range)
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
        size_t start_pos = line.find("_mm_user_");
        if (start_pos != string::npos)
        {
            //cout << line << endl;
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
    if(connect(str, true, &compdata))
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
        //cout << "Size of current json is " << json_entry->size() << endl;
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

void locality()
{
    if(!work_local)
    {
        // Initialize libcurl
        if (curl_global_init(CURL_GLOBAL_DEFAULT) != 0)
        {
            Yellog::Critical("curl_global_init() failed");
            //return 1;
        }
        Yellog::Debug("Initializing connection...");
        url = "https://virtonomics.com/olga/main/user/login";
        Yellog::Debug("URL is %s", url.c_str());
        string params = "userData[login]=" + VmaLogin + "&userData[password]=" + VmaPass;

        if (connect(url, false, &file, params))
        {
            Yellog::Warn("Connection to login failed! Working in local mode..." );
            work_local = true;
            //cout << url_base << endl;
            //return 1;
        }
        else
        {
            Yellog::Info("Connection to login successful...");
            url = url_base + "game/processing";
            Yellog::Debug("URL is %s", url.c_str());
            params = "";
            connect(url, true, &params);
            json data_turn =  json::parse(params);
            //cout << data_turn.dump(2);
            if(data_turn.value("started", true) == true)
            {
                Yellog::Warn("There is turn update now! Unable to receive info! Local mode...");
                work_local = true;
            }
            else
            {
                Yellog::Info("Connection to server successful! Working in internet mode...");
            }
        }
    }
}
