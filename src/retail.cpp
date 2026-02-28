#include "retail.h"

//string url;
mutex file_lock;
string market_link;

bool MarketsToday(fstream *today_file, string city, string product)
{
    lock_guard<mutex> lock(file_lock);
    url = market_link + "metrics?lang=en&geo=0/0/" + city + "&product_id=" + product;
    cout << "Today market..." << "\t";
    Yellog::Debug("URL (today): %s", url.c_str());
    string tmp;
    if(connect(url, true, &tmp) )
    {
        cerr << "Error! Connection failed" << endl;
        return false;
    }
    try
    {
        json data_prod = json::parse(tmp);
        //cout << data_prod.dump(4);
        (*today_file) << product << ";" << data_prod.value("avg_price", "0") << ";" << data_prod.value("local_market_size", "0") << ";";
        //cout << "part 1..." ;
        (*today_file) << stoi(data_prod.value("avg_price", "0"))*stoi(data_prod.value("local_market_size", "0")) << ";" << data_prod.value("index_min", "0") << endl;
        //cout << "part 2..." ;
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

bool LookLatest(vector <tuple<int, float, int, float, int>> &info, fs::path pathway, bool historic)
{
    json js_tmp;
    int id;
    float avg_price;
    int total_amount;
    float market;
    int index;
    if(historic)
    {
        lock_guard<mutex> lock(file_lock);
        fs::path lookup = pathway / "history.json";
        fstream hist;
        try_open_file(&hist, lookup, READ, "Opps!");
        js_tmp = json::parse(hist);
        auto last = js_tmp.rbegin();
        js_tmp = last.value();
        id = stoi(js_tmp.value("goods_id", "0"));
        avg_price = stof(js_tmp.value("avg_price", "0.0"));
        total_amount = stoi(js_tmp.value("local_market_size", "0"));
        market = total_amount * avg_price;
        index = stoi(js_tmp.value("index_min", "0"));
        info.push_back(make_tuple(id, avg_price, total_amount, market, index));
    }
    else
    {
        lock_guard<mutex> lock(file_lock);
        url = market_link + "metrics?lang=en&geo=0/0/" + pathway.parent_path().stem().c_str() + "&product_id=" + pathway.stem().c_str();
        cout << "Today market..." << "\t";
        //Yellog::Debug("URL (today): %s", url.c_str());
        string tmp;
        if(connect(url, true, &tmp) )
        {
            cerr << "Error! Connection failed" << endl;
            return false;
        }
        try
        {
            json js_tmp = json::parse(tmp);
            //cout << js_tmp.dump(4);
            id = stoi(pathway.stem().c_str());
            avg_price = stof(js_tmp.value("avg_price", "0.0"));
            total_amount = stoi(js_tmp.value("local_market_size", "0"));
            market = total_amount * avg_price;
            index = stoi(js_tmp.value("index_min", "0"));

            info.push_back(make_tuple(id, avg_price, total_amount, market, index));
        }
        catch(...)
        {
            cerr << "Error! " << endl;
            return false;
        }
        return true;
    }

    //cout <<"Info: "<< id << ";" << avg_price << ";" << total_amount << ";" << market << ";" << index << endl;
    return true;
}

int threadsMax;
fs::path ext_base;
fs::path markets;

void cityCheck(int start_city, vector<int> cities, vector<int> products, bool HistoryNeeded, bool MajorsNeeded)
{

    int num_cities = cities.size(), num_prods = products.size();
    for (int i = start_city - 1; i < num_cities; i = i + threadsMax)
    {
        vector <tuple<int, float, int, float, int>> today_info;
        //Yellog::Info("Thread %d\tCity %d (%d/%d)", start_city, cities[i], i + 1, num_cities);
        fs::path ext_city = ext_base / to_string (cities[i]);
        if(HistoryNeeded || MajorsNeeded)
            fs::create_directories(ext_city);
        //try_open_file(&file, location, WRITE, "Error! File for city not opened");
        //file << "product_id;avg_price;local_market_size;total_market;index_min" << endl;

        for (int j = 0; j < num_prods; j++)
        {
            //Yellog::Info("Thread %d\tProduct %d (%d/%d)", start_city, products[j], j + 1, num_prods);
            fs::path ext_prod = ext_city / to_string (products[j]);
            if(HistoryNeeded || MajorsNeeded)
                fs::create_directories(ext_prod);
            //MarketsToday(&file, to_string(cities[i]), to_string(products[j]));

            if (HistoryNeeded)
            {
                MarketsHistory(ext_city, to_string(cities[i]), to_string(products[j]));
            }
            LookLatest(today_info, ext_prod, HistoryNeeded);
            if(MajorsNeeded)
                MarketsMajorShops(ext_city, to_string(cities[i]), to_string(products[j]));
            //cout << endl;
        }
        //semaphore.acquire();
        file_lock.lock();
        //Yellog::Info("Thread %d\t Forming today file for %d", start_city, cities[i]);
        fs::path location = markets / ("markets-" + server + + "-" + to_string (cities[i]) + ".csv");
        try_open_file(&file, location, WRITE, "Error! File for city not opened");
        file << "product_id;avg_price;local_market_size;total_market;index_min" << endl;
        //cout << "Size of vec is" << today_info.size() << endl;
        file << fixed;
        for(const auto& [id, avg_price, total_amount, market, index] : today_info)
        {

            file << id << ";" << avg_price << ";" << total_amount << ";" << market << ";" << index << endl;
        }
        file.close();
        //semaphore.release();
        file_lock.unlock();

    }

    return;
}


int MarketsParse(string realm, vector<int> cities, vector<int> products, bool HistoryNeeded, bool MajorsNeeded)
{
    locality(realm);

    market_link  = url_base + "marketing/report/retail/";
    ext_base = ext_data / "markets";
    markets = exp_data / "markets";

    fs::create_directories(markets);

    //vector<string> fields;
    //fill_vs(getenv("fields_mark"), &fields);
    //Yellog::Info("Parsing retail markets...");
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
    //cout << "th " << threadsMax << endl;
    vector<thread> threads;

    int num_downloads = 1;
    if (HistoryNeeded)
        num_downloads++;
    if(MajorsNeeded)
        num_downloads++;
//    float estimates = num_cities * num_prods * num_downloads * wait_time / 1000.0;
    //cout  << "www " <<wait_time << " r " << 1000ms << " i " << (1000ms/wait_time) <<endl;
    //Yellog::Info("Estimated time for executing is %f", estimates);

    for (int i = 1; i <= threadsMax; i++)
    {
        threads.emplace_back(cityCheck, i, cities, products, HistoryNeeded, MajorsNeeded);
    }

    for (auto& t : threads)
    {
        if (t.joinable())
        {
            t.join(); // Blocks the current thread until t finishes
        }
    }

    //Yellog::Info("Parsing retail markets done!");

// file_struc: prod_id, avg_price, local_market_size, (Col2*Col3), index_max

    return 0;
}
