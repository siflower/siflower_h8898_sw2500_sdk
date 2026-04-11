if [ ! $# -eq 1  ];then
        FILE=opt_addr
else
        FILE=$1
fi

cmp_data() {
        local val_1=0x$1
        local val_2=$2
        local str=$3
        if [ ! "X$val_1" = "X$val_2" ];then
                printf "Test fail <$str> [$val_1 != $val_2]\n"
        else
                #printf "Test Success <$str>\n"
                echo "" > /dev/null
        fi
}

wr_cmp_32w_32r_64r() {
        local addr=$1
        local val_1=$2
        local val_2=$3
        local addr_2=$(( 0x$addr + 4 ))
        local addr_2=`printf "%x" $addr_2`
        local val
        local val32_low
        local val32_high
        local val64_low
        local val64_high
        local val32_low_tmp
        local val32_high_tmp

        #read low 4Bytes
        cmd=`echo "devmem 0x$addr 32"`;
        val32_low_tmp=`env $cmd`

        #read high 4Bytes
        cmd=`echo "devmem 0x$addr_2 32"`;
        val32_high_tmp=`env $cmd`

        #write low 4Bytes
        cmd=`echo "devmem 0x$addr 32 0x$val_1"`;
        env $cmd

        #write high 4Bytes
        cmd=`echo "devmem 0x$addr_2 32 0x$val_2"`;
        env $cmd

        #compare high 4Bytes between 32read and 64read
        cmd=`echo "devmem 0x$addr 64"`;
        val=`env $cmd`
        val64_high="${val:2:8}"
        cmd=`echo "devmem 0x$addr_2 32"`;
        val32_high=`env $cmd`
        cmp_data $val64_high $val32_high "32Write: 64Read 32Read(high 4Bytes) --> $cmd"

        #compare low 4Bytes between 32read and 64read
        cmd=`echo "devmem 0x$addr 64"`;
        val=`env $cmd`
        val64_low="${val:10:8}"
        cmd=`echo "devmem 0x$addr 32"`;
        val32_low=`env $cmd`
        cmp_data $val64_low $val32_low "32Write: 64Read 32Read(low 4Bytes) --> $cmd"

        #reset low 4Bytes
        cmd=`echo "devmem 0x$addr 32 $val32_low_tmp"`;
        env $cmd

        #reset high 4Bytes
        cmd=`echo "devmem 0x$addr_2 32 $val32_high_tmp"`;
        env $cmd
}

wr_cmp_64w_64r_32r() {
        local addr=$1
        local val_64=$2
        local val_1="${val_64:8:8}"
        local val_2="${val_64:0:8}"
        local addr_2=$(( 0x$addr + 4 ))
        local addr_2=`printf "%x" $addr_2`
        local val
        local val64_tmp

        #read initial 8Bytes
        cmd=`echo "devmem 0x$addr 64"`;
        val64_tmp=`env $cmd`

        #write 8Bytes
        cmd=`echo "devmem 0x$addr 64 0x$val_64"`;
        env $cmd

        #compare high 4Bytes between 32read and 64read
        cmd=`echo "devmem 0x$addr 64"`;
        val=`env $cmd`
        val64_high="${val:2:8}"
        cmd=`echo "devmem 0x$addr_2 32"`;
        val32_high=`env $cmd`
        cmp_data $val64_high $val32_high "64Write: 64Read 32Read(high 4Bytes) --> $cmd"

        #compare low 4Bytes between 32read and 64read
        cmd=`echo "devmem 0x$addr 64"`;
        val=`env $cmd`
        val64_low="${val:10:8}"
        cmd=`echo "devmem 0x$addr 32"`;
        val32_low=`env $cmd`
        cmp_data $val64_low $val32_low "64Write: 64Read 32Read(low 4Bytes) --> $cmd"

        #reset 8Bytes
        cmd=`echo "devmem 0x$addr 64 $val64_tmp"`;
        env $cmd
}


cat $FILE | while read line
do
        if [ "${line:0:1}" = "#" ];then
                echo "$line"
        elif [ -z "${line}" ];then
                continue
        else
                g_addr=`echo $line | awk '{print $1}'`
                cmd=`echo "devmem 0x$g_addr 64"`;
                val=`env $cmd`
                g_w_val32_1=`echo $line | awk '{print $2}'`
                g_w_val32_2=`echo $line | awk '{print $3}'`
                g_w_val64=`echo $line | awk '{print $4}'`
                if [ -z "$val" ];then
		        echo "Skip"
		else
                        wr_cmp_32w_32r_64r $g_addr $g_w_val32_1 $g_w_val32_2
                        wr_cmp_64w_64r_32r $g_addr $g_w_val64
                fi
        fi
done
