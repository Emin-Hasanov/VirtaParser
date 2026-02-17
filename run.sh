#!/bin/sh

export vma_login="secret1981-2014@mail.ua"
export vma_password="699076"

dayOfMonth=$(date +%d)
Month=$(date +%m)
Year=$(date +%Y)

Houre=$(date +%H)
Minut=$(date +%M)
Secys=$(date +%S)

#export folder=${realm_need}
export data=${Year}-${Month}-${dayOfMonth}-${Houre}_${Minut}_${Secys}
#export filenm=${Year}-${Month}-${dayOfMonth}-${Houre}_${Minut}_${Secys}-${realm_need}-${company_id}.json

#export finalname=${realm_need}-${company_id}.csv

export fields_sum="name city_id unit_class_kind unit_type_produce_id size technology_level employee_count employee_salary employee_level equipment_count equipment_quality equipment_wear district_id fertility quality extract_cost on_holiday productivity"
export fields_art="artefact_energy artefact_wear artefact_feeder"
export fields_ext="estimated_value name"
export fields_mark="avg_price local_market_size  index_min"
#export fields_arr="id name city_id unit_class_kind unit_type_produce_id size technology_level employee_count employee_salary employee_level equipment_count equipment_quality district_id "
#export fields_ext="estimated_value name"

echo "Starting data collect..."
#echo "Server is $realm_need, company is $company_id"'
date --iso-8601=seconds

time ./VirtaParse

ReturnParse=$?

echo "Return code for data reciver: "${ReturnParse}

if [ $ReturnParse -eq 0 ]; then
    cp -r ./data/* ../VirtaProj
    cd ../VirtaProj
    git add ./companies/* ./markets/*
    git commit -m "Update from ${data}"
    git push origin master
    else
    echo "Error during reciving"
fi


#echo "Data collected!"
date --iso-8601=seconds


# main ->   (realm-company.csv)
#      ->   realm-company ->    date -> (company/units)
#                                       units -> (unit/summary)


#
