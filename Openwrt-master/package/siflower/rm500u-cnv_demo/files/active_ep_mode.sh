#!bin/ash
#set as EP mode
echo -e "AT+QCFG="pcie/mode",0\r" > /dev/stty_nr31
#reset the module
echo -e "AT+CFUN=1,1\r" > /dev/stty_nr31
#wait the module to reset done
