/*#include <iostream>
#include <limits>
#include <vector>
#include <string>*/
#include <limits>
#include "retail.h"
#include "company.h"


int server_check(string str)
{
    if (str == "vera")
        return 1;
    if (str == "olga")
        return 2;
    if(str == "mary")
        return 3;
    if(str == "lien")
        return 4;
    if(str == "nika")
        return 5;
    if(str == "fast")
        return 6;
    return 0;
}


void menu()
{
    cout << "Wellcome to VirtaParser!" << endl;
    cout << endl;
    cout << "Choose needed parse:" << endl;
    cout << "----------------------" << endl;
    cout << "1 - Company and units" << endl;
    cout << "----------------------" << endl;
    cout << "2 - Markets in cities" << endl;
    cout << "----------------------" << endl;
    cout << "3 - Exit from program" << endl;
    cout << "----------------------" << endl;
    cout << "4 - Exit and export" << endl;
    cout << "----------------------" << endl;
}

bool automat = false;
int option = 0;
int company_id = 0;
bool git_exp = false;
fstream LocalIDs;
vector<int> cities;
vector<int> products;

/*
    Arguments:
    -o - option (1 - Company; 2 - Markets);
    -s - server (vera, olga, mary,lien, nika)
    -c - id of company (number)
    -g - geocodes of (city) markets **possible to use countries/regions names (aggregate all city markets of geo) / "World" to get all markets / "Local" to ger ids from file "cities.data"
    (countries with 1 region using same name for country and region)
    -p - codes of products **possible to use category names (aggregate all products from category) / "All" to get all products  / "Local" to ger ids from file "products.data"
    -h - add history
    -ms - add major shops
    -e - export to repo
    */
void launch_args(int args_num, char const *args_vals[])
{
    if (args_num > 1)
    {
        cout << "Working in (semi)-automatic mode..." << endl;
        automat = true;
    }
    for (int i = 0; i < args_num; i++)
    {
        string current_arg = args_vals[i];
        cout << "Current is " << current_arg << endl;
        if (current_arg == "-o")
        {
            option = stoi(args_vals[i + 1]);
        }
        if (current_arg == "-s")
        {
            server = args_vals[i + 1];
            if(!server_check(server))
            {
                server.clear();
            }
            url_base.insert(28, server);
        }
        if(current_arg == "-c")
        {
            try
            {
                company_id = stoi(args_vals[i + 1]);
            }
            catch (const invalid_argument& ia)
            {
                company_id = 0;
            }
        }
        if (current_arg == "-g")
        {
            string geo = args_vals[i + 1];
            //cout << "Chosen geo is " << geo << endl;
            if (geo == "Local")
            {
                cout << "Using local file to retrieve ids of cities..." << endl;
                if(!try_open_file(&LocalIDs, "cities.data", READ, "Error! File for cites not opened"))
                {
                    cout << "Error! No cities are loaded! Exiting..." << endl;
                    break;
                }
                string tmp;
                while(getline(LocalIDs, tmp, '\n'))
                {
                    cities.push_back(stoi(tmp));
                }
                LocalIDs.close();
            }
            else if (geo == "World")
            {
                cout << "All cities chosen!" << endl;
                url = url_base + "geo/city/browse";
                cout << "URL is " << url << endl;
                string GeoBuf;
                connect(url, false, &GeoBuf);
                json GeoJSON = json::parse(GeoBuf);
                cout << "Cities:" << GeoJSON.size() << endl;
                for (auto& el : GeoJSON.items())
                {
                    json GeoInfo = el.value();
                    cities.push_back(stoi(GeoInfo.value("city_id", "0")));
                }
            }
            else
            {
                string FilterID = "";
                string GeoTypeURL = "country";
                url = url_base + "geo/" + GeoTypeURL + "/browse";
                //geo/country/browse
                cout << "URL is " << url << endl;
                string GeoBuf;
                connect(url, false, &GeoBuf);
                json GeoJSON = json::parse(GeoBuf);
                for (auto& el : GeoJSON.items())
                {
                    json GeoInfo = el.value();
                    if (GeoInfo.value("name", "Unknown") == geo)
                    {
                        FilterID = GeoInfo.value("country_id", "0");
                        //cout << GeoInfo.value("country_id", "0") << endl;
                    }
                    //cout << GeoInfo.value("country_name", "Unknown") << endl;
                }
                cout << "FilterID [country] is " << FilterID << endl;
                if (FilterID == "")
                {
                    GeoBuf.clear();
                    GeoTypeURL = "region";
                    url = url_base + "geo/" + GeoTypeURL + "/browse";
                    //geo/country/browse
                    cout << "URL is " << url << endl;
                    connect(url, false, &GeoBuf);
                    GeoJSON = json::parse(GeoBuf);
                    for (auto& el : GeoJSON.items())
                    {
                        json GeoInfo = el.value();
                        if (GeoInfo.value("name", "Unknown") == geo)
                        {
                            FilterID = GeoInfo.value("region_id", "0");
                            //cout << GeoInfo.value("country_id", "0") << endl;
                        }
                        //cout << GeoInfo.value("name", "Unknown") << endl;
                    }
                    cout << "FilterID [region] is " << FilterID << endl;
                }
                if(FilterID == "")
                    cout << "FilterID not found! "  << endl;
                else
                {
                    GeoBuf.clear();
                    url = url_base + "geo/city/browse";
                    GeoTypeURL += "_id";
                    cout << "Type is " << GeoTypeURL << endl;
                    //geo/country/browse
                    cout << "URL is " << url << endl;
                    connect(url, false, &GeoBuf);
                    GeoJSON = json::parse(GeoBuf);
                    cout << "Cities:" << GeoJSON.size() << endl;
                    for (auto& el : GeoJSON.items())
                    {
                        json GeoInfo = el.value();

                        if (GeoInfo.value(GeoTypeURL, "0") == FilterID)
                        {
                            cities.push_back(stoi(GeoInfo.value("city_id", "0")));
                            //FilterID = GeoInfo.value("region_id", "0");
                            //cout << GeoInfo.value("country_id", "0") << endl;
                        }
                    }
                }
                //cout << GeoInfo.dump(4) << endl;
            }
            if(cities.size() != 0)
                cout << "Cities IDs added sucessully!" << endl;
            // cout << "City ids are:" << endl;
            // for (int city : cities)
            // {
            //     cout << city << "; ";
            // }
            // cout << endl;
        }
        if (current_arg == "-p")
        {
            string ProdCat = args_vals[i + 1];
            if (ProdCat == "Local")
            {
                cout << "Using local file to retrieve ids of products..." << endl;
                if(!try_open_file(&LocalIDs, "products.data", READ, "Error! File for products not opened"))
                {
                    cout << "Error! No cities are loaded! Exiting..." << endl;
                    break;
                }
                string tmp;
                while(getline(LocalIDs, tmp, '\n'))
                {
                    products.push_back(stoi(tmp));
                }
                LocalIDs.close();
            }
            else if (ProdCat == "All")
            {
                string ProdBuf;
                url = url_base + "product/goods";
                cout << "URL is " << url << endl;
                connect(url, false, &ProdBuf);
                json ProdJSON = json::parse(ProdBuf);
                for (auto& el : ProdJSON.items())
                {
                    json ProdInfo = el.value();
                    products.push_back(stoi(ProdInfo.value("id", "0")));
                }
            }
            else
            {
                string ProdBuf;
                string FilterID = "";
                url = url_base + "product/goodscategories";
                cout << "URL is " << url << endl;
                connect(url, false, &ProdBuf);
                json ProdJSON = json::parse(ProdBuf);
                for (auto& el : ProdJSON.items())
                {
                    json ProdInfo = el.value();
                    if (ProdInfo.value("name", "Unknown") == ProdCat)
                    {
                        FilterID = ProdInfo.value("id", "0");
                        //cout << GeoInfo.value("country_id", "0") << endl;
                    }
                    //cout << GeoInfo.value("name", "Unknown") << endl;
                }
                cout << "FilterID [ProdCat] is " << FilterID << endl;
                ProdBuf.clear();
                url = url_base + "product/goods";
                cout << "URL is " << url << endl;
                connect(url, false, &ProdBuf);
                ProdJSON = json::parse(ProdBuf);
                for (auto& el : ProdJSON.items())
                {
                    json ProdInfo = el.value();
                    if (ProdInfo.value("category_id", "0") == FilterID)
                    {
                        products.push_back(stoi(ProdInfo.value("id", "0")));
                        //cout << GeoInfo.value("country_id", "0") << endl;
                    }
                    //cout << GeoInfo.value("name", "Unknown") << endl;
                }
                //cout << ProdJSON.dump(4) << endl;

            }
            if(products.size() != 0)
                cout << "Product IDs added sucessully!" << endl;
            /*cout << "Product ids are:" << endl;
            for (int prods : products)
            {
                cout << prods << "; ";
            }
            cout << endl;*/
        }
        if (current_arg == "-e")
        {
            git_exp = true;
        }
    }
}

int main(int argc, char const *argv[])
{

    int server_type;
    //string server;

    //cout << "Arguments (" << argc << ") of prog is: " << endl;

    launch_args(argc, argv);


    int result;
    do
    {
        if(option == 0)
        {
            menu();
            if (!(cin >> option))
            {
                cout << "Invalid input! Please enter a number." << endl;
                cin.clear(); // Clear error flags
                cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Discard bad input
                option = 0; // Force loop to continue
                continue;
            }
        }
        switch (option)
        {
        case 1:
            if(server.empty())
            {
                cout << "Enter server..." << endl;
                cin >> server;
                server_type = server_check(server);
                while(!server_type)
                {
                    cout << "Invalid input! Please enter correct server." << endl;
                    cin >> server;
                    server_type = server_check(server);
                }
                url_base.insert(28, server);
            }
            if (company_id == 0)
            {
                cout << "Enter id of needed company..." << endl;
                cin >> company_id;
            }

            cout << "Server is " << server << " and id is " << company_id << endl;
            result = CompanyParse(server, company_id);
            if (result)
            {
                cout << "Error happened during company parsing..." << result << endl;
                if (automat)
                {
                    return result;
                }
            }
            else
            {
                cout << "Company parsing ended sucessully!" << endl;
                if (automat)
                {
                    return -1;
                }
                if (git_exp)
                {
                    return 0;
                }
            }
            break;
        case 2:

            if(server.empty())
            {
                cout << "Enter server..." << endl;
                cin >> server;
                server_type = server_check(server);
                while(!server_type)
                {
                    cout << "Invalid input! Please enter correct server." << endl;
                    cin >> server;
                    server_type = server_check(server);
                }
            }
            if (cities.size() == 0)
            {
                cout << "Enter ids of cities..." << endl;
                int city_id;
                cin >> city_id;
                while (city_id != 0)
                {
                    cities.push_back(city_id);
                    cout << "Enter next city or press 0 to end list..." << endl;
                    cin >> city_id;
                }
                if (cities.size() == 0)
                {
                    cout << "Using local file to retrieve ids of cities..." << endl;
                    if(!try_open_file(&LocalIDs, "cities.data", READ, "Error! File for cites not opened"))
                    {
                        cout << "Error! No cities are loaded! Exiting..." << endl;
                        break;
                    }
                    string tmp;
                    while(getline(LocalIDs, tmp, '\n'))
                    {
                        cities.push_back(stoi(tmp));
                    }
                    LocalIDs.close();
                    //getline()
                }
            }
            if (products.size() == 0)
            {
                cout << "Enter ids of products..." << endl;
                int product_id;
                cin >> product_id;
                while (product_id != 0)
                {
                    products.push_back(product_id);
                    cout << "Enter next product or press 0 to end list..." << endl;
                    cin >> product_id;
                }
                if (products.size() == 0)
                {
                    cout << "Using local file to retrieve ids of products..." << endl;
                    if(!try_open_file(&LocalIDs, "products.data", READ, "Error! File for products not opened"))
                    {
                        cout << "Error! No products are loaded! Exiting..." << endl;
                        break;
                    }
                    string tmp;
                    while(getline(LocalIDs, tmp, '\n'))
                    {
                        products.push_back(stoi(tmp));
                    }
                    LocalIDs.close();
                }
            }

            cout << "Server is " << server << ";" << endl;
            cout << "City ids are:" << endl;
            for (int city : cities)
            {
                cout << city << "; ";
            }
            cout << endl;
            cout << "Product ids are:" << endl;
            for (int prods : products)
            {
                cout << prods << "; ";
            }
            cout << endl;
            result = MarketsParse(server, cities, products);
            if (result)
            {
                cout << "Error happened during markets parsing..." << result << endl;
                if (automat)
                {
                    return result;
                }
            }
            else
            {
                cout << "Markets parsing ended sucessully!" << endl;
                if (automat)
                {
                    return -1;
                }
            }
            break;
        case 3:
            cout << "Exiting program. Goodbye!" << endl;
            cout << "Files will NOT be sent to repo" << endl;
            return -1;
            break;
        case 4:
            cout << "Exiting program. Goodbye!" << endl;
            cout << "Files will be sent to repo" << endl;
            return 0;
            break;
        default:
            cout << "Invalid choice. Please try again.\n\n";
        }
        option = 0;
    }
    while ((option != 3) || (option != 4));


}
