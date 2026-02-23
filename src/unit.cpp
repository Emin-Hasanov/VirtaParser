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
setflt(Productivity)

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
get(float, Productivity);

void Unit::printFields(fstream *file)
{
    auto cout_buff = cout.rdbuf();
    if (file)
    {
        cout.rdbuf(file->rdbuf());
    }
    cout << "id; name; office; city; kind; type; size; technology_level; employees; salary; qualification; equip; equip_qual; wear; on_holiday; productivity" << endl;
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
    cout << refout(Tech) << refout(Employees) << refout(Salary) <<refout(Qualification) <<refout(Equip) <<refout(EquipQual) <<refout(Wear);
    cout << refout(Holidays) <<refout(Productivity) << endl;
    cout.rdbuf(cout_buff);
}
