#ifndef _UNIT_H
#define _UNIT_H

#include "virta.h"
#include <tuple>

#define defvar(type,name,default) type name = default
#define defset(name) void set##name (string value)
#define defget(type,name) type get##name ()

/*Class of typical unit

members:
ID, Name, RegOfficeID, CityID, Kind;
*/

//#define UnitP16 Productivity

class Unit
{
    defvar(int, ID, 0);
    defvar(string, Name, "");
    defvar(int, RegOfficeID, 0);
    defvar(int, CityID, 0);
    defvar(string, Kind, "");
    defvar(int, Type, 0);
    defvar(int, UnitSize, 0);
    defvar(int, Tech, 0);
    defvar(int, Employees, 0);
    defvar(float, Salary, 0.0);
    defvar(float, Qualification, 0.0);
    defvar(int, Equip, 0);
    defvar(float, EquipQual, 0.0);
    defvar(float, Wear, 0.0);
    defvar(bool, Holidays, false);
    defvar(int, Fertility, 0);
    defvar(int, CultQuality, 0);
    defvar(float, Hardness, 0);
    defvar(float, Productivity, 0.0);
    defvar(bool, ArtefactEnergy, false);
    defvar(bool, ArtefactWear, false);
    defvar(bool, ArtefactFeeder, false);
    defvar(string, ExtName, "");
    defvar(float, ExtEstVal, 0.0);



public:
    defset(ID);
    defset(Name);
    defset(RegOfficeID);
    defset(CityID);
    defset(Kind);
    defset(Type);
    defset(UnitSize);
    defset(Tech);
    defset(Employees);
    defset(Salary);
    defset(Qualification);
    defset(Equip);
    defset(EquipQual);
    defset(Wear);
    defset(Holidays);
    defset(Fertility);
    defset(CultQuality);
    defset(Hardness);
    defset(Productivity);
    defset(ArtefactEnergy);
    defset(ArtefactWear);
    defset(ArtefactFeeder);
    defset(ExtName);
    defset(ExtEstVal);

    defget(int, ID);
    defget(string, Name);
    defget(int, RegOfficeID);
    defget(int, CityID);
    defget(string, Kind);
    defget(int, Type);
    defget(int, UnitSize);
    defget(int, Tech);
    defget(int, Employees);
    defget(float, Salary);
    defget(float, Qualification);
    defget(int, Equip);
    defget(float, EquipQual);
    defget(float, Wear);
    defget(bool, Holidays);
    defget(int, Fertility);
    defget(int, CultQuality);
    defget(float, Hardness);
    defget(float, Productivity);
    defget(bool, ArtefactEnergy);
    defget(bool, ArtefactWear);
    defget(bool, ArtefactFeeder);
    defget(string, ExtName);
    defget(float, ExtEstVal);

    static vector<int> ProdNeedIDs;
    vector<tuple<int, float, int, float>> ProdNeedVec;

    void printFields(fstream *file = nullptr);
    void printValues(fstream *file = nullptr);

};

#endif //_UNIT_H
