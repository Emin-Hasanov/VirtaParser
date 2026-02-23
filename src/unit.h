#ifndef _UNIT_H
#define _UNIT_H

#include "virta.h"

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
    defvar(int,ID,0);
    defvar(string,Name,"");
    defvar(int,RegOfficeID,0);
    defvar(int,CityID,0);
    defvar(string,Kind,"");
    defvar(int,Type,0);
    defvar(int,UnitSize,0);
    defvar(int,Tech,0);
    defvar(int, Employees, 0);
    defvar(float, Salary, 0.0);
    defvar(float, Qualification, 0.0);
    defvar(int, Equip, 0);
    defvar(float, EquipQual, 0.0);
    defvar(float, Wear, 0.0);
    defvar(bool, Holidays, false);
    defvar(float,Productivity,0.0);

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
    defset(Productivity);

    defget(int,ID);
    defget(string,Name);
    defget(int,RegOfficeID);
    defget(int,CityID);
    defget(string,Kind);
    defget(int,Type);
    defget(int,UnitSize);
    defget(int,Tech);
    defget(int, Employees);
    defget(float, Salary);
    defget(float, Qualification);
    defget(int, Equip);
    defget(float, EquipQual);
    defget(float, Wear);
    defget(bool, Holidays);
    defget(float,Productivity);

    void printFields(fstream *file = nullptr);
    void printValues(fstream *file = nullptr);

};

#endif //_UNIT_H
