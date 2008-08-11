#!/bin/sh

for x in $(seq 1 20); do

row=$((20 - $x))

cat <<__EOF >../23.23.23.$x/opt/mcu/etage.setup
<?xml version="1.0" encoding="utf-8"?>                                          
                                                                                
<mcu-setup height="20" width="26" channels="1" pixels="26">                     
        <lamp id="0"  row="${row}" column="0"/>
        <lamp id="1"  row="${row}" column="1"/>
        <lamp id="2"  row="${row}" column="2"/>
        <lamp id="3"  row="${row}" column="3"/>
        <lamp id="4"  row="${row}" column="4"/>
        <lamp id="5"  row="${row}" column="5"/>
        <lamp id="6"  row="${row}" column="6"/>
        <lamp id="7"  row="${row}" column="7"/>
        <lamp id="8"  row="${row}" column="8"/>
        <lamp id="9"  row="${row}" column="9"/>
        <lamp id="10" row="${row}" column="10"/>
        <lamp id="11" row="${row}" column="11"/>
        <lamp id="12" row="${row}" column="12"/>
        <lamp id="13" row="${row}" column="13"/>
        <lamp id="14" row="${row}" column="14"/>
        <lamp id="15" row="${row}" column="15"/>
        <lamp id="16" row="${row}" column="16"/>
        <lamp id="17" row="${row}" column="17"/>
        <lamp id="18" row="${row}" column="18"/>
        <lamp id="19" row="${row}" column="19"/>
        <lamp id="20" row="${row}" column="20"/>
        <lamp id="21" row="${row}" column="21"/>
        <lamp id="22" row="${row}" column="22"/>
        <lamp id="23" row="${row}" column="23"/>
        <lamp id="24" row="${row}" column="24"/>
        <lamp id="25" row="${row}" column="25"/>
</mcu-setup>
__EOF

done
