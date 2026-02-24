#include "company.h"
#include "unit.h"

//#define setjso(prop) set##prop ()
#define setjsv(prop,json,key,default) set##prop(valnut(json,key,default))



int process_unit(Unit *current, fs::path filepath)
{
    fstream buf;
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
    if(filepath.stem() == "summary")
    {
        json data_res;
        if (current->getKind() == "sawmill" || current->getKind() == "farm" || current->getKind() == "orchard")
        {
            data_res = js_buf.at("city_culture");
            current->setjsv(Fertility, data_res, "fertility", "0");
            current->setjsv(CultQuality, data_res, "quality", "0");
        }
        if (current->getKind() == "mine")
        {
            data_res = js_buf.at("city_deposit");
            current->setjsv(Hardness, data_res, "extract_cost", "0");
            current->setjsv(CultQuality, data_res, "quality", "0");
        }
        current->setjsv(Name, js_buf, "name", "UNKNOWN");
        current->setjsv(RegOfficeID, js_buf, "office_id", "0");
        current->setjsv(CityID, js_buf, "city_id", "0");
        current->setjsv(Type, js_buf, "unit_type_produce_id", "0");
        current->setjsv(UnitSize, js_buf, "size", "0");
        current->setjsv(Tech, js_buf, "technology_level", "0");
        current->setjsv(Employees, js_buf, "employee_count", "0");
        current->setjsv(Salary, js_buf, "employee_salary", "0");
        current->setjsv(Qualification, js_buf, "employee_level", "0");
        current->setjsv(Equip, js_buf, "equipment_count", "0");
        current->setjsv(EquipQual, js_buf, "equipment_quality", "0");
        current->setjsv(Wear, js_buf, "equipment_wear", "0");
        current->setjsv(Holidays, js_buf, "on_holiday", "t");
        current->setjsv(Productivity, js_buf, "productivity", "0.0");
    }
    if(filepath.stem() == "artefact")
    {
        for (auto &artefact : js_buf.items())
        {
            json slot = artefact.value();
            if (slot.size() != 0)
            {
                int art_id = stoi( slot.value("id", "0"));
                switch (art_id)
                {
                case 300804:
                    current->setArtefactEnergy("t");
                    break;
                case 300990:
                case 300997:
                case 301008:
                    current->setArtefactWear("t");
                    break;
                case 301043:
                    current->setArtefactFeeder("t");
                    break;
                default:
                    break;
                }
            }
        }
    }
    if(filepath.stem() == "extension")
    {
        if (js_buf.size() > 1)
        {
            js_buf = js_buf.at("effects");
            js_buf = js_buf.at(0);
            current->setExtEstVal(to_string(js_buf.value("estimated_value", 0.0)));
            current->setjsv(ExtName, js_buf, "name", "UNKNOWN");
        }
    }
    if(filepath.stem() == "supply")
    {
        //Yellog::Debug("Sup info is being processed...");
        cout << "Kind is " << current->getKind() << endl;
        if((current->getKind() == "workshop") || (current->getKind() == "animalfarm") || (current->getKind() == "mill") || (current->getKind() == "power"))
        {
            for (auto& el : js_buf.items())
            {
                json SupElem = el.value();
                //cout << SupElem.dump(4)<<endl;
                if(SupElem == 406)
                {
                    cout << "No supplies needed!" << endl;
                    break;
                }
                if(SupElem == 401)
                {
                    cout << "Access limited!" << endl;
                    break;
                }
                int ProdID = stoi(SupElem.value("product_id", "0"));
                float ProdQual = stof(valnut(SupElem, "quality", "0"));
                int ProdReq = stoi(valnut(SupElem, "required", "0"));
                float ProdPrime = stof(valnut(SupElem, "prime_cost", "0"));

                auto it = std::find(Unit::ProdNeedIDs.begin(), Unit::ProdNeedIDs.end(), ProdID);
                if (it == Unit::ProdNeedIDs.end())
                {
                    Unit::ProdNeedIDs.push_back(ProdID);
                }
                current->ProdNeedVec.push_back(make_tuple(ProdID, ProdQual, ProdReq, ProdPrime));
                cout << ProdID << "; " << ProdQual << "; " << ProdReq << "; " << ProdPrime << endl;
            }
        }
        cout << "Size of ProdNeedVec is " << current->ProdNeedVec.size() << endl;
        //cout << js_buf.dump(4) << endl;
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
        auto start = std::chrono::steady_clock::now();
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
        auto stop = std::chrono::steady_clock::now();

        auto durationOne = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        auto durationApprox = durationOne.count() * (num_units - 1);
        Yellog::Debug("Unit %s processed in %d  microseconds", curr_id.c_str(), durationOne.count());
        if(!i)
        {
            Yellog::Info("Approximate time to execute: %f  seconds", durationApprox / 1000000.0);
        }
        //cout << "Kind is " << kind << endl;

    }

    cout << "Size of Vector: " << Unit::ProdNeedIDs.size() << endl;


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

