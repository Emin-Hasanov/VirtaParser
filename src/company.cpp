#include "company.h"


int process_unit (fstream *target, fs::path path, vector<string> fields_to_retrieve, string kind)
{
    fstream buf;
    if(!try_open_file(&buf, path, READ, "Error! opening file to read unit info."))
    {
        return 12;
    }
    if (buf.peek() == ifstream::traits_type::eof())
    {
        cerr << "Error: nothing to parse." << std::endl;
        fs::remove(path);
        return 13;
    }
    json js_buf = json::parse(buf);
    buf.close();
    if (path.stem() == "summary")
    {
        json data_res;
        if (kind == "sawmill" || kind == "farm" || kind == "orchard")
        {
            data_res = js_buf.at("city_culture");
            //cout << data_res.dump(4) << endl;
        }
        if (kind == "mine")
        {
            data_res = js_buf.at("city_deposit");
            //cout << data_res.dump(2) << endl;
        }
        for (string field : fields_to_retrieve)
        {
            if (data_res.contains(field))
            {
                (*target) << data_res.value(field, "0");
            }
            else if(valnut(js_buf, field, "0") == "null")
            {
                (*target) << "0";
            }
            else
            {
                (*target) << js_buf.value(field, "0");
            }
            if (field != fields_to_retrieve.back())
            {
                (*target) << ";";
            }
        }
    }
    if (path.stem() == "artefact")
    {
        vector<bool> artefacts_status = {false, false, false};
        //bool energy = false, wear = false, feeder = false;
        for (auto &artefact : js_buf.items())
        {
            json slot = artefact.value();
            //cout << "Size is " << slot.size() << endl;
            if (slot.size() != 0)
            {
                //cout << slot.dump(4);
                int art_id = stoi( slot.value("id", "0"));
                switch (art_id)
                {
                case 300804:
                    artefacts_status[0] = true;
                    //energy = true;
                    break;
                case 300990:
                case 300997:
                case 301008:
                    artefacts_status[1] = true;
                    //wear = true;
                    break;
                case 301043:
                    artefacts_status[2] = true;
                    break;
                default:
                    break;
                    //cout << "None" << endl;
                }
            }
        }
        for (int i = 0; i < fields_to_retrieve.size(); i++)
        {
            (*target) << artefacts_status[i];
            if (i != (fields_to_retrieve.size() - 1))
            {
                (*target) << ";";
            }
        }
        /*for (string field : fields_to_retrieve)
        {
            //cout << "Current field is " << field << endl;

        }*/
        //cout << "Artefacts: " << energy << "; " << wear << "; " << feeder << endl;
    }
    if(path.stem() == "extension")
    {
        cout << "Info about extension in progress..." << endl;
        if (js_buf.size() > 1)
        {
            cout << js_buf.dump(4) << endl;

        }

    }

    return 0;
}

int CompanyParse(string serv, int company_id)
{
    //url_base = "https://virtonomics.com/api/" + server + "/main/";

    fstream file;
    bool work_local = false;

    // Initialize libcurl
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != 0)
    {
        fprintf(stderr, "curl_global_init() failed\n");
        return 1;
    }
    cout << "Initializing connection..." << endl;

    url = "https://virtonomics.com/" + serv + "/main/user/login";
    cout << "URL is " << url << endl;
    string params = "userData[login]=" + (string)getenv("vma_login") + "&userData[password]=" + (string)getenv("vma_password");

    if (connect<fstream>(url, false, &file, params))
    {
        cout << "Connection to login failed! Working in local mode..." << endl;
        work_local = true;
        cout << url_base << endl;
        //return 1;
    }
    else
    {
        cout << "Connection to login successful..." << endl;
        url = url_base + "game/processing";
        cout << "URL is " << url << endl;
        params = "";
        connect<string>(url, true, &params);
        json data_turn =  json::parse(params);
        //cout << data_turn.dump(2);
        if(data_turn.value("started", true) == true)
        {
            cout << "There is turn update now! Unable to recieve info! Local mode..." << endl;
            work_local = true;
        }
        else
        {
            cout << "Connection to server successful! Working in internet mode..." << endl;
        }
    }

    json data_units, data_ext;
    //fs::path base = "ext-data";
    fs::path data_folder = ext_data / "companies" / (serv + "-" + to_string(company_id)); // / getenv("datafolder");
    fs::path company_units_info = data_folder / "company_units.json";
    fs::path company_ext_info = data_folder / "company_ext.json";
    fs::path unit_info_fold = data_folder / "units";


    if (!work_local)
    {
        bool limited;
        try
        {
            limited = check_access(serv, company_id);
        }
        catch(char const*  except)
        {
            cout << except << endl;
            return 2;
        }


        cout << "access is limited? " << limited << endl;
        cout << "Getting list of units of company " << company_id << " on server " << serv << "..." << endl;
        url = url_base + "company/units?id=" + to_string(company_id) + "&pagesize=100000&lang=en&unit_class_id=0";
        cout << "URL is " << url << endl;


        fs::create_directories(data_folder);

        if (!DownloadParseFormat(url, &data_units, &file, company_units_info))
        {
            cerr << "Error while getting info about units in company" << endl;
            return 3;
        }
        cout << "Separating data by files..." << endl;
        try
        {
            try_open_file(&file, company_units_info, REWRITE, "Error! opening file for filtering info about units.");
            file << data_units.at("data").dump(4);
            file.close();
            if (!limited)
            {
                try_open_file(&file, company_ext_info, REWRITE, "Error! opening file for filtering info about unit extensions.");
                file << data_units.at("extensions").dump(4);
                file.close();
            }
        }
        catch (...)
        {
            cout << "Error! Data not separated" << endl;
            return 6;

        }
        cout << "Data are separated!" << endl;
    }
    if (!exists(data_folder))
    {
        cerr << "That company never checked before. Exiting as no data to process..." << endl;
        return 7;
    }
    try_open_file(&file, company_units_info, READ, "Error! Opening file for getting info about units.");
    data_units = json::parse(file);
    file.close();
    if (try_open_file(&file, company_ext_info, READ, "Error! Opening file for getting info about unit extensions."))
    {
        data_ext = json::parse(file);
        file.close();
    }
    cout << "There are " << data_units.size() << " units and " << data_ext.size() << " known extensions in company " << company_id << " on server " << serv  << endl;

    cout << "Making an array of Unit IDs to collect data..." << endl;
    size_t num_units;
    vector<string> id_list;
    try
    {
        num_units = data_units.size();
        for (auto it : data_units)
        {
            id_list.push_back(it.value("id", "0"));
        }
    }
    catch(...)
    {
        cout << "Error while getting clear Unit_IDs!"  << endl;
        return 8;
    }
    cout << "Unit_IDs are cleared!" << endl;

    string finalFile = serv + "-" + to_string(company_id) + ".csv";
    fs::path place = exp_data / "companies" / finalFile;
    //fs::path place = getenv("finalname");
    if (!try_open_file(&file, place, REWRITE, "Error! Opening final file"))
    {
        return 9;
    }

    vector<string> fields;
    cout << "Fields are: ";
    file << "id;";
    //vector<string> unit_fields = {"sum"};
    vector<string> unit_fields = {"sum", "art", "ext"};//, "sup"};
    for (string fields_val : unit_fields)
    {
        fields.clear();
        string f = "fields_" + fields_val;
        fill_vs(getenv(f.c_str()), &fields);
        for (string field : fields)
        {
            file << field;
            cout << field;
            if (field != fields.back())
            {
                file << ";";
                cout << "; ";
            }
        }
        if (fields_val != unit_fields.back())
        {
            file << ";";
            cout << "; ";
        }
    }

    file << endl;
    cout << endl;

    for (int i = 0; i < num_units; i++)
    {
        string curr_id = id_list[i];
        cout << "Unit " << curr_id << " (" << i + 1 << " / " << num_units << ")" << endl;
        fs::path curr_id_fold = unit_info_fold / curr_id;
        fs::path unit_info;// = curr_id_fold / "summary.json";
        vector<string> unit_files = {"summary.json", "artefact.json", "extension.json"};//, "supply.json"};
        vector<string> unit_links = {"unit/summary?id=", "unit/artefact/attached?id=", "unit/extension/current?id="};//, "unit/supply/summary?id="};
        fstream temp;

        if (!work_local)
        {
            fs::create_directories(curr_id_fold);
            cout << "Amount of files for each unit: " << unit_files.size() << endl;
            for (int i = 0; i < unit_links.size(); i++)
            {
                url = url_base + unit_links[i] + curr_id;
                cout << "URL is " << url << endl;
                unit_info = curr_id_fold / unit_files[i];
                if (!DownloadParseFormat(url, &data_units, &temp, unit_info))
                {
                    if (unit_links[i] == "unit/extension/current?id=")
                    {
                        url = url_base + "unit/extension/construction?id=" + curr_id;
                        if (!DownloadParseFormat(url, &data_units, &temp, unit_info))
                        {
                            cerr << "Error while getting info about unit " << curr_id  << endl;
                            return 10;
                        }
                    }
                    else
                    {
                        cerr << "Error while getting info about unit " << curr_id  << endl;
                        return 10;
                    }


                }
                else
                    cout <<  unit_files[i] << " of unit " << curr_id << " retrieved!" << endl;
            }

        }
        if (!fs::exists(curr_id_fold))
        {
            cerr << "Error! No info about unit " << curr_id  << endl;
            return 11;
        }
        unit_info = curr_id_fold / unit_files[0];
        try_open_file(&temp, unit_info, READ, "Error! opening file for filtering info about units.");
        string kind = json::parse(temp).value("unit_class_kind", "0");
        temp.close();
        cout << "Kind is " << kind << endl;
        file << curr_id << ";";
        for (int i = 0; i < unit_files.size(); i++)
        {
            fields.clear();
            unit_info = curr_id_fold / unit_files[i];
            cout << "Current path is " << unit_info << endl;
            string field_arg = "fields_" + unit_fields[i];
            fill_vs(getenv(field_arg.c_str()), &fields);

            int ProcStatus = process_unit(&file, unit_info, fields, kind);
            if(ProcStatus)
            {
                cerr << "Error while processing unit summary!" << endl;
                return ProcStatus;
            }
            file << ";";
        }
        file << endl;
        //temp.close();
    }
    file.close();
    curl_global_cleanup();
    curl_slist_free_all(log_info);

    cout << "Data collection done! File is " << finalFile << endl;
    return 0;
}
