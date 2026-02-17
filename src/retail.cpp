#include "retail.h"

//string url;
string market_link;

bool MarketsToday(fstream *today_file, string city, string product)
{
    url = market_link + "metrics?lang=en&geo=0/0/" + city + "&product_id=" + product;
    cout << "Today market..." << "\t";
    //cout << "URL: " << url << endl;
    string tmp;
    if(connect<string>(url, true, &tmp) )
    {
        cerr << "Error! Connection failed" << endl;
        return false;
    }
    try
    {
        json data_prod = json::parse(tmp);
        (*today_file) << product << ";" << data_prod.value("avg_price", "0") << ";" << data_prod.value("local_market_size", "0") << ";";
        (*today_file) << stoi(data_prod.value("avg_price", "0"))*stoi(data_prod.value("local_market_size", "0")) << ";" << data_prod.value("index_min", "0") << endl;
    }
    catch(...)
    {
        cerr << "Error! " << endl;
        return false;
    }
    return true;
}

bool MarketsHistory(fs::path pathway, string city, string product)
{
    url = market_link + "history?geo=0/0/" + city + "&product_id=" + product;
    cout << "History of market..." << "\t";
    //cout << "URL (history): " << url << endl;
    string tmp;
    if(connect<string>(url, true, &tmp))
    {
        cerr << "Error! Connection failed" << endl;
        return false;
    }

    fs::path history_path = pathway / product / "history.json";
    fstream history_file;
    json data_prod;
    if (exists(history_path))
    {
        try_open_file(&history_file, history_path, READ, "Error! File for historical data not opened");
        data_prod = json::parse(history_file);
        history_file.close();
        int last_turn_local = 0, last_turn_net = 0;
        //cout << data_prod.dump(4);
        for (auto& el : data_prod.items())
        {
            json turn_info = el.value();
            //cout << turn_info.dump(3);
            last_turn_local = stoi(turn_info.value("turn_id", "0")) ; // endl;
            //std::cout << "key: " << el.key() << ", value:" << el.value() << '\n';
        }
        cout << "Last local turn is " << last_turn_local << endl;
        json js_tmp = json::parse(tmp);
        for (auto& el : js_tmp.items())
        {
            json turn_info = el.value();
            //cout << turn_info.dump(3);
            last_turn_net = stoi(turn_info.value("turn_id", "0")) ; // endl;
            //std::cout << "key: " << el.key() << ", value:" << el.value() << '\n';
        }
        cout << "Last net turn is " << last_turn_net << endl;
        int new_turns = last_turn_net - last_turn_local;
        cout << "Turn needed to add is " << new_turns << endl;
        if (new_turns > 61)
            new_turns = 61;
        if(new_turns > 0)
            for(int i = 61 - new_turns; i < 61; i++)
            {
                json new_info = js_tmp.at(i);
                cout << "New turn info: " << endl << new_info.dump(4) << endl;
                data_prod.emplace_back(new_info);
                //data_prod.insert(data_prod.end(), js_tmp.begin() + i);
                try_open_file(&history_file, history_path, REWRITE, "Error! File for historical data not opened");
                history_file << data_prod.dump(4);
                history_file.close();
            }
    }
    else
    {
        DownloadParseFormat(url, &data_prod, &history_file, history_path);
    }
    //history_file.close();
    return true;
}

bool MarketsMajorShops(fs::path pathway, string city, string product)
{
    url = market_link + "units?geo=0/0/" + city + "&product_id=" + product;
    cout << "Major Shops..." << "\t";
    //cout << "URL (shops): " << url << endl;
    /*string tmp;
    if(connect(url, true, &tmp))
    {
        cerr << "Error! Connection failed" << endl;
        return false;
    }*/
    fs::path shops_path = pathway / product / "majors.json";
    fstream shops_file;
    json data_shops;
    DownloadParseFormat(url, &data_shops, &shops_file, shops_path);
    return true;
}

int MarketsParse(string realm, vector<int> cities, vector<int> products)
{
    //url_base = "https://virtonomics.com/api/" + server + "/main/";
    market_link  = url_base + "marketing/report/retail/";

    fstream file;
    //bool work_local = false;

    // Initialize libcurl
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != 0)
    {
        fprintf(stderr, "curl_global_init() failed\n");
        return 1;
    }
    cout << "Initializing connection..." << endl;

    url = "https://virtonomics.com/" + realm + "/main/user/login";
    cout << "URL is " << url << endl;
    string params = "userData[login]=" + (string)getenv("vma_login") + "&userData[password]=" + (string)getenv("vma_password");

    if (connect<fstream>(url, false, &file, params))
    {
        cout << "Connection to login failed! Working in local mode..." << endl;
        //work_local = true;
        return 1;
    }
    else
    {
        cout << "Connection to server successful! Working in internet mode..." << endl;
    }

    fs::path ext_base = ext_data / "markets";
    fs::path markets = exp_data / "markets";

    fs::create_directories(markets);

    vector<string> fields;
    fill_vs(getenv("fields_mark"), &fields);


    int num_cities = cities.size(), num_prods = products.size();
    for (int i = 0; i < num_cities; i++)
    {
        cout << "City " << cities[i] << " (" << i + 1 << " / " << num_cities << ")" << endl;
        fs::path location = markets / ("markets-" + server + + "-" + to_string (cities[i]) + ".csv");
        fs::path ext_city = ext_base / to_string (cities[i]);
        fs::create_directories(ext_city);
        try_open_file(&file, location, WRITE, "Error! File for city not opened");
        file << "product_id;avg_price;local_market_size;total_market;index_min" << endl;

        for (int j = 0; j < num_prods; j++)
        {
            cout << "Product " << products[j] << " (" << j + 1 << " / " << num_prods << ")" << endl;
            fs::path ext_prod = ext_city / to_string (products[j]);
            fs::create_directories(ext_prod);
            MarketsToday(&file, to_string(cities[i]), to_string(products[j]));
            MarketsHistory(ext_city, to_string(cities[i]), to_string(products[j]));
            MarketsMajorShops(ext_city, to_string(cities[i]), to_string(products[j]));
            cout << endl;
        }
        file.close();
    }

// file_struc: prod_id, avg_price, local_market_size, (Col2*Col3), index_max

    return 0;
}
