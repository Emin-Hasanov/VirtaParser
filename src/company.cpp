#include "company.h"

#define def(a,b,c) a b = c
#define setint(a) void set##a(string value) { this->a = stoi(value);}
#define setflt(a) void set##a(string value) { this->a = stof(value);}
#define setstr(a) void set##a(string value) { this->a = value;}
#define get(a,b) a get##b() {return this->b;}
#define ref(a) this->a
#define refout(a) << this->a << ";"
//#define jspar(a,b,c,d) set##a(b.value(c,d))
#define jsparn(a,b,c,d) set##a(valnut(b,c,d))

class Unit
{
    int id = 0;
    string name = "";
    int regOfficeID = 0;
    int cityID = 0;
    string kind = "";
    int type = 0;
    int unitSize = 0;
    int tech = 0;
    def(int, employees, 0);
    def(float, salary, 0.0);
    def(float, qualification, 0.0);
    def(int, equip, 0);
    def(float, equip_qual,0.0);

public:

    void setID(string value)
    {
        this->id = stoi(value);
    }
    void setName(string value)
    {
        this->name = value;
    }
    void setRegID(string value)
    {
        this->regOfficeID = stoi(value);
    }
    void setCityID(string value)
    {
        this->cityID = stoi(value);
    }
    void setKind(string value)
    {
        this->kind = value;
    }
    void setType(string value)
    {
        this->type = stoi(value);
    }
    void setSize(string value)
    {
        this->unitSize = stoi(value);
    }
    void setTech(string value)
    {
        this->tech = stoi(value);
    }
    setint(employees)
    setflt(salary)
    setflt(qualification)
    setint(equip)
    setflt(equip_qual)

    int getID()
    {
        return this->id;
    }
    string getName()
    {
        return this->name;
    }
    int getRegID()
    {
        return this->regOfficeID;
    }
    int getCityID()
    {
        return this->cityID;
    }
    string getKind()
    {
        return this->kind;
    }
    int getType()
    {
        return this->type;
    }
    int getSize()
    {
        return this->unitSize;
    }
    int getTech()
    {
        return this->tech;
    }
    get(int, employees)
    get(float, salary)
    get(float, qualification)
    get(int, equip)
    get(float,equip_qual)

    void printFields(fstream *file = nullptr)
    {
        auto cout_buff = cout.rdbuf();
        if (file)
        {
            cout.rdbuf(file->rdbuf());
        }
        cout << "id; name; office; city; kind; type; size; technology_level; employees; salary; qualification; equip; equip_qual; wear;" << endl;
        cout.rdbuf(cout_buff);
    }
    void printValues(fstream *file = nullptr)
    {
        auto cout_buff = cout.rdbuf();
        if (file)
        {
            cout.rdbuf(file->rdbuf());
        }
        cout << this->id << ";" << this->name << ";" << this->regOfficeID << ";" << this->cityID << ";" << this->kind << ";";
        cout << this->type << ";" << this->unitSize << ";" << this->tech << ";" refout(employees) refout(salary) refout(qualification) refout(equip);
        cout refout(equip_qual) << endl;
        cout.rdbuf(cout_buff);
    }
};

int process_unit(Unit *current, fs::path filepath)
{
    fstream buf;
    //cout << "Params are: " << path << endl;
    if(!try_open_file(&buf, filepath, READ, "Error! opening file to read unit info."))
    {
        return 12;
    }
    if (buf.peek() == ifstream::traits_type::eof())
    {
        cerr << "Error: nothing to parse." << std::endl;
        fs::remove(filepath);
        return 13;
    }
    json js_buf = json::parse(buf);
    buf.close();
    if (filepath.stem() == "summary")
    {
        json data_res;
        if (current->getKind() == "sawmill" || current->getKind() == "farm" || current->getKind() == "orchard")
        {
            data_res = js_buf.at("city_culture");
            //cout << data_res.dump(4) << endl;
        }
        if (current->getKind() == "mine")
        {
            data_res = js_buf.at("city_deposit");
            //cout << data_res.dump(2) << endl;
        }

        current->setName(js_buf.value("name", "UNKNOWN"));
        current->setRegID(js_buf.value("office_id", "0"));
        current->setCityID(js_buf.value("city_id", "0"));
        current->setType(js_buf.value("unit_type_produce_id", "0"));
        current->setSize(js_buf.value("size", "0"));
        current->setTech(js_buf.value("technology_level", "0"));
        current->jsparn(employees, js_buf, "employee_count", "0");
        current->jsparn(salary, js_buf, "employee_salary", "0");
        current->jsparn(qualification, js_buf, "employee_level", "0");
        current->jsparn(equip, js_buf, "equipment_count", "0");
        current->jsparn(equip_qual,js_buf,"equipment_quality","0");
        //employee_count; employee_salary; employee_level; equipment_count	equipment_quality	equipment_wear

        //current->set

        //cout << js_buf.dump(4);
    }
    return 0;
}


int CompanyParse(string serv, int company_id)
{
    locality();

    json data_units, data_ext;
    //Extended data about company and units
    fs::path data_folder = ext_data / "companies" / (serv + "-" + to_string(company_id));
    fs::path company_units_info = data_folder / "company_units.json";
    fs::path company_ext_info = data_folder / "company_ext.json";
    fs::path unit_info_fold = data_folder / "units";

    bool limited = false;
    if (!work_local)
    {
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

        fs::create_directories(data_folder);
        cout << "Getting list of units of company " << company_id << " on server " << serv << "..." << endl;
        url = url_base + "company/units?id=" + to_string(company_id) + "&pagesize=100000&lang=en&unit_class_id=0";
        Yellog::Debug("URL is %s", url.c_str());
        if (!DownloadParseFormat(url, &data_units, &file, company_units_info))
        {
            cerr << "Error while getting info about units in company" << endl;
            return 3;
        }
        //cout << "Separating data by files..." << endl;
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
            //cout << "Error! Data not separated" << endl;
            return 6;
        }
        //cout << "Data are separated!" << endl;

    }
    if (!exists(data_folder))
    {
        Yellog::Error("That company never checked before. Exiting as no data to process...");
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
    size_t num_units = data_units.size();
    Yellog::Info("There are %d units and %d known extensions in company %d on server %s", num_units, data_ext.size(), company_id, serv.c_str());

    cout << "Making an array of Unit IDs to collect data..." << endl;
    vector<string> id_list;
    try
    {
        for (auto it : data_units)
        {
            id_list.push_back(it.value("id", "0"));
        }
    }
    catch(...)
    {
        Yellog::Error("Error while getting clear Unit_IDs!");
        return 8;
    }
    cout << "Unit_IDs are cleared!" << endl;

    Unit units[num_units];
    vector<string> unit_files = {"summary.json", "artefact.json", "extension.json", "supply.json"};
    vector<string> unit_links = {"unit/summary?id=", "unit/artefact/attached?id=", "unit/extension/current?id=", "unit/supply/summary?id="};
    for (size_t i = 0; i < num_units; i++)
    {
        string curr_id = id_list[i];
        units[i].setID(curr_id);
        Yellog::Info("Unit %s (%d/%d)", curr_id.c_str(), i + 1, num_units);
        fs::path curr_id_fold = unit_info_fold / curr_id;
        fs::path unit_info;
        fstream temp;

        if (!work_local)
        {
            fs::create_directories(curr_id_fold);
            Yellog::Info("Getting info about unit %s from server...", curr_id.c_str());
            //cout << "Amount of files for each unit: " << unit_files.size() << endl;
            for (unsigned i = 0; i < unit_links.size(); i++)
            {
                url = url_base + unit_links[i] + curr_id;
                Yellog::Debug("URL is %s", url.c_str());
                unit_info = curr_id_fold / unit_files[i];
                if (!DownloadParseFormat(url, &data_units, &temp, unit_info))
                {
                    if (unit_links[i] == "unit/extension/current?id=")
                    {
                        if (!limited) //local extension info
                        {
                            json curr_ext = data_ext.value(curr_id, false);
                            //cout << curr_ext.dump(4) << endl;
                            try_open_file(&temp, unit_info, WRITE, "Unable to write ext file!");
                            temp << curr_ext.dump(4) << endl;
                            temp.close();
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
        units[i].setKind(kind);
        temp.close();
        Yellog::Info("Processing retrieved info about unit %s...", curr_id.c_str());
        for (unsigned j = 0; j < unit_files.size(); j++)
        {
            unit_info = curr_id_fold / unit_files[j];

            Yellog::Debug("Current path is %s", unit_info.c_str());
            int ProcStatus = process_unit(&units[i], unit_info);
            if(ProcStatus)
            {
                Yellog::Error("Error while parsing unit info!");
                return ProcStatus;
            }
        }

        //cout << "Kind is " << kind << endl;

    }

    Yellog::Info("Making summary file about company...");
    fs::path place = exp_data / "companies";
    fs::create_directories(place);
    string finalFile = serv + "-" + to_string(company_id) + ".csv";
    place = place / finalFile;
    if (!try_open_file(&file, place, REWRITE, "Error! Opening final file"))
    {
        return 9;
    }
    //cout << "fields..." << endl;
    units[0].printFields(&file);
    //cout << "values..." << endl;
    for (size_t i = 0; i < num_units; i++)
    {
        units[i].printValues(&file);
    }
    file.close();
    Yellog::Info("Data collection done! File is %s", finalFile.c_str());
    //cerr.rdbuf(cerr_buff);
    curl_global_cleanup();
    curl_slist_free_all(log_info);
    return 0;
}

