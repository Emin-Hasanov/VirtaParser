#include "unit.h"

#define setint(name) void Unit::set##name(string value) { this->name = stoi(value);}
#define setflt(name) void Unit::set##name(string value) { this->name = stof(value);}
#define setstr(name) void Unit::set##name(string value) { this->name = value;}
#define setbool(name) void Unit::set##name(string value) \
                   {if (value == "t") \
                       this->name = true; \
                    else \
                       this->name = false;}
#define get(type,name) type Unit::get##name() {return this->name;}
#define refout(name) this->name << ";"

vector<int> Unit::ProdNeedIDs;
vector<int> Unit::ProdIDs;

setint(ID)
setstr(Name)
setint(RegOfficeID)
setint(CityID);
setstr(Kind);
setint(Type);
setint(UnitSize)
setint(Tech)
setint(Employees)
setflt(Salary)
setflt(Qualification)
setint(Equip)
setflt(EquipQual)
setflt(Wear)
setbool(Holidays)
setint(Fertility)
setint(CultQuality)
setflt(Hardness)
setflt(Productivity)
setbool(ArtefactEnergy)
setbool(ArtefactWear)
setbool(ArtefactFeeder)
setstr(ExtName)
setflt(ExtEstVal)

get(int, ID);
get(string, Name);
get(int, RegOfficeID);
get(int, CityID);
get(string, Kind);
get(int, Type);
get(int, UnitSize);
get(int, Tech);
get(int, Employees);
get(float, Salary);
get(float, Qualification);
get(int, Equip);
get(float, EquipQual);
get(float, Wear);
get(bool, Holidays);
get(int, Fertility);
get(int, CultQuality);
get(float, Hardness);
get(float, Productivity);
get(bool, ArtefactEnergy);
get(bool, ArtefactWear);
get(bool, ArtefactFeeder);
get(string, ExtName);
get(float, ExtEstVal)


void Unit::printFields(fstream *file)
{
    auto cout_buff = cout.rdbuf();
    if (file)
    {
        cout.rdbuf(file->rdbuf());
    }
    cout << "id; name; office; city; kind; type; size; technology_level; employees; salary; qualification; equip; equip_qual; wear; on_holiday; fertility; quality; hardness; productivity; ";
    cout << "artefactEnergy; artefactWear; artefactFeeder; extValue; extType; ";
    for (int ProdID : ProdNeedIDs)
    {
        cout << (to_string(ProdID) + "_Need") << "; ";
    }
    for (int ProdID : ProdIDs)
    {
        cout << (to_string(ProdID) + "_Made") << "; ";
    }
    cout << "TotalMade; ";
    for (int ProdID : ProdNeedIDs)
    {
        cout << (to_string(ProdID) + "_Cost") << "; ";
    }
    cout << "TotalRawCost; ";
    cout << endl;
    cout.rdbuf(cout_buff);
}

void Unit::printValues(fstream *file)
{
    auto cout_buff = cout.rdbuf();
    if (file)
    {
        cout.rdbuf(file->rdbuf());
    }
    cout << refout(ID) << refout(Name) << refout(RegOfficeID) << refout(CityID) << refout(Kind) << refout(Type) << refout(UnitSize);
    cout << refout(Tech) << refout(Employees) << refout(Salary) << refout(Qualification) << refout(Equip) << refout(EquipQual) << refout(Wear);
    cout << refout(Holidays) << refout(Fertility) << refout(CultQuality) << refout(Hardness) << refout(Productivity) << refout(ArtefactEnergy);
    cout << refout(ArtefactWear) << refout(ArtefactFeeder) << refout(ExtEstVal) << refout(ExtName);
    int totals = 0;
    for (int ProdID : ProdNeedIDs)
    {
        auto Prod = find_if(ProdNeedVec.begin(), ProdNeedVec.end(), [ProdID]
                            (const tuple<int, float, int, float>& element)
        {
            return get<0>(element) == ProdID;
        });
        if (Prod == ProdNeedVec.end())
        {
            cout << "0";
        }
        else
        {
            cout << get<2>(*Prod);
            //totals+= get<2>(*Prod);
        }
        cout << ";";
        //cout <<
    }
    //cout << totals << ";";
    for (int ProdID : ProdIDs)
    {
        auto Prod = find_if(ProdVec.begin(), ProdVec.end(), [ProdID]
                            (const tuple<int, float, int, float>& element)
        {
            return get<0>(element) == ProdID;
        });
        if (Prod == ProdVec.end())
        {
            cout << "0";
        }
        else
        {
            cout << get<2>(*Prod);
            totals += get<2>(*Prod);
        }
        cout << ";";
        //cout <<
    }
    cout << totals << ";";
    totals = 0;
    for (int ProdID : ProdNeedIDs)
    {
        auto Prod = find_if(ProdNeedVec.begin(), ProdNeedVec.end(), [ProdID]
                            (const tuple<int, float, int, float>& element)
        {
            return get<0>(element) == ProdID;
        });
        if (Prod == ProdNeedVec.end())
        {
            cout << "0";
        }
        else
        {
            cout << get<3>(*Prod)*get<2>(*Prod);
            totals += get<3>(*Prod) * get<2>(*Prod);
        }
        cout << ";";
        //cout <<
    }
    cout << totals << ";";
    cout << endl;
    cout.rdbuf(cout_buff);
}

void Unit::addPNV(int ID, float Qual, int Req, float Prime)
{
    this->ProdNeedVec.push_back(make_tuple(ID, Qual, Req, Prime));
}

void Unit::addPV(int ID, float Qual, int Qnty, float Prime)
{
    this->ProdVec.push_back(make_tuple(ID, Qual, Qnty, Prime));
}

size_t Unit::sizePNV()
{
    return this->ProdNeedVec.size();
}
