#include "retail.h"

mutex file_lock;
string market_link;

bool MarketsHistory(fs::path pathway, string city, string product)
{
    lock_guard<mutex> lock(file_lock);
    url = market_link + "history?geo=0/0/" + city + "&product_id=" + product;
    //cout << "History of market..." << "\t";
    //Yellog::Debug("URL (history): %s", url.c_str());
    string tmp;
    if(connect(url, true, &tmp))
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


        auto last = data_prod.rbegin();
        json turn_info = last.value();
        last_turn_local = stoi(turn_info.value("turn_id", "0")) ;
        //cout << "Last local turn is " << last_turn_local << endl;

        json js_tmp = json::parse(tmp);
        last = js_tmp.rbegin();
        turn_info = last.value();
        last_turn_net = stoi(turn_info.value("turn_id", "0")) ;
        //cout << "Last net turn is " << last_turn_net << endl;

        int new_turns = last_turn_net - last_turn_local;
        //cout << "Turn needed to add is " << new_turns << endl;
        int net_size = js_tmp.size();
        if (new_turns > net_size)
            new_turns = net_size;
        if(new_turns > 0)
        {
            for(int i = net_size - new_turns; i < net_size; i++)
            {
                json new_info = js_tmp.at(i);
                //cout << "New turn info: " << endl << new_info.dump(4) << endl;
                data_prod.emplace_back(new_info);
                //data_prod.insert(data_prod.end(), js_tmp.begin() + i);

            }
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
    lock_guard<mutex> lock(file_lock);
    url = market_link + "units?geo=0/0/" + city + "&product_id=" + product;
    //Yellog::Debug("URL (majors): %s", url.c_str());
    fs::path shops_path = pathway / product / "majors.json";
    fstream shops_file;
    json data_shops;
    DownloadParseFormat(url, &data_shops, &shops_file, shops_path);
    return true;
}

int threadsMax;
fs::path ext_base;
fs::path markets;

void cityCheck(int start_city, vector<int> cities, vector<int> products, bool HistoryNeeded, bool MajorsNeeded)
{
    int num_cities = cities.size(), num_prods = products.size();
    for (int i = start_city; i < num_cities; i = i + threadsMax)
    {
        fs::path ext_city = ext_base / to_string (cities[i]);
        fs::create_directories(ext_city);
        for (int j = 0; j < num_prods; j++)
        {
            Yellog::Info("Thread %d\tCity %d (%d/%d)\tProduct %d (%d/%d)", start_city, cities[i], i + 1, num_cities, products[j], j + 1, num_prods);
            fs::path ext_prod = ext_city / to_string (products[j]);
            if(HistoryNeeded || MajorsNeeded)
                fs::create_directories(ext_prod);
            if (HistoryNeeded)
                MarketsHistory(ext_city, to_string(cities[i]), to_string(products[j]));
            if(MajorsNeeded)
                MarketsMajorShops(ext_city, to_string(cities[i]), to_string(products[j]));
        }
    }
    return;
}

void prodCheck(vector<int> products)
{
    int num_prods = products.size();
    vector <tuple<string, string, string, int, int, float, int, float, int>> Prodinfo;
    fs::path ProdBase = markets / "Products";
    fs::create_directories(ProdBase);
    for (int j = 0; j < num_prods; j++)
    {
        Yellog::Info("Thread M\tProduct %d (%d/%d)", products[j], j + 1, num_prods);
        url = market_link + "cities?pagesize=500&product_id=" + to_string(products[j]);
        float worldMarket = 0;
        string tmp;
        connect(url, true, &tmp);
        json retailInfo = json::parse(tmp);
        retailInfo = retailInfo.at("data");
        for (auto &item : retailInfo.items())
        {
            json city = item.value();
            string country = city.value("country_name", "UNKNOWN");
            string region = city.value("region_name", "UNKNOWN");
            string cityName = city.value("city_name", "UNKNOWN");
            int cityID = stoi(city.value("city_id", "0"));
            float avg_price = stof(city.value("avg_price", "0.0"));
            int total_amount = stoi(city.value("local_market_size", "0"));
            float market = total_amount * avg_price;
            int index = stoi(city.value("cologne_index", "0"));
            Prodinfo.push_back(make_tuple(country, region, cityName, cityID, products[j], avg_price, total_amount, market, index));
            worldMarket += market;
            //cout << city.dump(4);
        }
        //cout << retailInfo.dump(4);
        file_lock.lock();
        fs::path location = ProdBase /  ("products-" + to_string (products[j]) + ".csv");
        try_open_file(&file, location, WRITE, "Error! File for product not opened");
        file << "country;region;city;avg_price;local_market_size;total_market;percentage;index_min" << endl;
        for(const auto& [country, region, cityName, cityID, prodID, avg_price, total_amount, market, index] : Prodinfo)
        {
            file << country << ";" << region << ";" << cityName << ";" << avg_price << ";" << total_amount << ";" << fixed << setprecision(2) << market << ";" << market / worldMarket * 100.0  << ";" << index << endl;
        }
        file.close();
        file_lock.unlock();
    }
    //TODO: integrate generating files for 'needed' cities of used products

}


int MarketsParse(string realm, vector<int> cities, vector<int> products, bool HistoryNeeded, bool MajorsNeeded)
{
    locality(realm);

    market_link  = url_base + "marketing/report/retail/";
    ext_base = ext_data / "markets" / realm;
    markets = exp_data / "markets" / realm;
    fs::create_directories(markets);

    Yellog::Info("Parsing retail markets...");

    threadsMax = thread::hardware_concurrency();
    //cout << "th " << threadsMax <<endl;
    if (threadsMax < 4)
    {
        threadsMax = 1;
    }
    else
    {
        threadsMax = threadsMax / 2;
    }
    Yellog::Info("For parsing %d threads can be used...", threadsMax);
    vector<thread> threads;
    if(HistoryNeeded || MajorsNeeded)
    {
        for (int i = 0; i < threadsMax; i++)
        {
            threads.emplace_back(cityCheck, i, cities, products, HistoryNeeded, MajorsNeeded);
        }
    }
    prodCheck(products);
    for (auto& t : threads)
    {
        if (t.joinable())
        {
            t.join(); // Blocks the current thread until t finishes
        }
    }

    Yellog::Info("Parsing retail markets done!");
    threads.clear();
    curl_slist_free_all(log_info);

    return 0;
}
