from __future__ import division
import os
import re

#0:[time]log; 1:more detail
log_lvl = 0

target_file = "dictionary"
trace_memory_file = "fw_trace.txt"
trace_memory = []

file_arr = {}
log_arr = {}
exist = []

task_id_dict = {}
msg_id_dict = {}

TRACE_HEADER_LEN = 8
TRACE_FILE_ID_SIZE = 9
TRACE_FILE_ID_MAX = 110
TRACE_FILE_ID_OFT = 24 -TRACE_FILE_ID_SIZE

#read target_file into file_arr[id] and log_arr[id][line]
#log_arr[id][line] to get trace log
def get_file_arr():
    f = open(target_file, "r")
    line = f.readline()
    while line:
        data = line.strip().split()
        if(len(data) < 3):
            break
        file_id = data[0]
        file_name = data[1]
        line_num = data[2]
        log = "TRACE_" + line.strip().split("TRACE_")[1]
        file_arr[file_id] = file_name
        if file_id not in exist:
            exist.append(file_id)
            log_arr[file_id] = {}
        log_arr[file_id][line_num] = log
        line = f.readline()
    f.close()
    #print detail
    if(log_lvl):
        print("===================read from dictionary=====================")
        for a in log_arr:
            for b in log_arr[a]:
                print(a, file_arr[a], b, log_arr[a][b])
        print("===================read dictionary end=====================")

def get_id_from_str(enums_str):
    pattern = re.compile(r'(\w+)\s=\s(\d+)')
    matches = pattern.findall(enums_str)
    result_dict = {key: int(value) for key, value in matches}
    return result_dict

def get_msg_task_id():
    enum_task = 0
    enum_msg = 0
    f = open(target_file, "r")
    line = f.readline()
    while line:
        data = line.strip().split()
        if("enum_task_start" in data):
            enum_task = 1
        elif("enum_msg_start" in data):
            enum_msg = 1
            enum_task = 0

        if(len(data) > 1):
            if(enum_task == 1):
                task_id_dict[int(data[0], 16)] = data[1]
            elif(enum_msg == 1):
                msg_id_dict[int(data[0], 16)] = data[1]
        line = f.readline()

def add_4byte_to_trace_memory(byte_str):
    trace_memory.append(int(byte_str[6:8],16))
    trace_memory.append(int(byte_str[4:6],16))
    trace_memory.append(int(byte_str[2:4],16))
    trace_memory.append(int(byte_str[0:2],16))

#read trace_memory_file into trace_memory
#trace_memory[n] to get No.n byte in memory
def get_trace_memory():
    f = open(trace_memory_file, "r")
    line = f.readline()
    while line:
        data = line.strip().split()
        for byte in data:
            add_4byte_to_trace_memory(byte)
        line = f.readline()
    f.close()

def translate_2byte(arr):
    return (arr[1] << 8) + arr[0]

def translate_4byte(arr):
    high_u16 =(arr[1] << 8) + arr[0]
    low_u16 = (arr[3] << 8) + arr[2]
    return (high_u16 << 16) + low_u16

def translate_6byte(arr):
    mac1 = ("%02X" % arr[0]) + (":%02X" % arr[1])
    mac2 = (":%02X" % arr[2]) + (":%02X" % arr[3])
    mac3 = (":%02X" % arr[4]) + (":%02X" % arr[5])
    return mac1 + mac2 + mac3

def fixed_id_str(id_n):
    return ("[%d]" %(id_n)).rjust(5)

def replace_msg_str(match):
    result = ""
    n = int(match.group(1))
    m_id = n & 0x3ff
    if n in msg_id_dict:
        if log_lvl == 1:
            result = "%s%s(%s)" % (fixed_id_str(m_id), msg_id_dict[n], hex(n))
        else:
            result = "%s" % (msg_id_dict[n])
    else:
        result = "%sMSG_ERROR_ID(%s)" % (fixed_id_str(m_id), hex(n))
    if log_lvl == 1:
        return result.ljust(38)
    else:
        return result.ljust(27)

def replace_task_str(match):
    result = ""
    n = int(match.group(1))
    t_id = n & 0xff
    t_index = n >> 8
    if t_id in task_id_dict:
        if log_lvl == 1:
            result =  "%s%s(%d)" % (fixed_id_str(t_id), task_id_dict[t_id], t_index)
        else:
            result =  "%s(%d)" % (task_id_dict[t_id], t_index)
    else:
        result =  "%sTASK_ERROR_ID(%d)" % (fixed_id_str(t_id), t_index)
    if log_lvl == 1:
        return result.ljust(18)
    else:
        return result.ljust(13)

def translate_id_to_str(log_str):
    tmp = re.sub(r'\[(\d+)\]\[MSG_ERROR_ID\]', replace_msg_str, log_str)
    result = re.sub(r'\[(\d+)\]\[TASK_ERROR_ID\]', replace_task_str, tmp)
    return result

def translate_param(log_str, param):
    full_log = re.sub(r'"\s*"', '', log_str) + "END"
    full_log = re.findall(r'TRACE[^(]+\((.*)\)END', full_log)[0]
    pattern = re.compile(r'\(([^)]*)\)')
    full_log = pattern.sub('()', full_log)
    #log =  re.findall(r'"(.*?)"', full_log)[0].replace("%kM","%d").replace("%kT","%d").replace("%p","%x").replace("%pM","%s").replace("%F","%d")
    log =  re.findall(r'"(.*?)"', full_log)[0].replace("%kM","[%d][MSG_ERROR_ID]").replace("%kT","[%d][TASK_ERROR_ID]").replace("%pM","%s").replace("%p","%x").replace("%F","%d")
    if len(param) == 0:
        return log
    li = re.sub(r'"[^"]*"', '""', full_log).split(",")
    if(len(li) <= 2):
        return full_log
    arg = []
    pn = 0
    for n in range(2,len(li)):
        if(pn >= len(param)):
            print("error: get more than %d param in log %s" %(pn, full_log))
            exit()
        tmp = li[n]
        if("TR_32" in tmp):
            arg.append(translate_4byte(param[pn:pn+4]))
            pn = pn + 4
        elif("TR_MAC" in tmp):
            arg.append(translate_6byte(param[pn:pn+6]))
            pn = pn + 6
        else:
            arg.append(translate_2byte(param[pn:pn+2]))
            pn = pn + 2
    return log % tuple(arg[:log.count('%')])

def get_trace_info(start):
    if(start > len(trace_memory) - 8 or start < 0):
        print("error: trace_memory has %d bytes, start at %d bytes" % (len(trace_memory), start))
        return

    nb_param = trace_memory[start+1]
    if(start+TRACE_HEADER_LEN+nb_param*2 > len(trace_memory)):
        print("error:get wrong param number %d from %d bytes. need %d bytes, longer than %d bytes" % (nb_param, start, start+TRACE_HEADER_LEN+nb_param*2, len(trace_memory)))
        return

    trace_id = (trace_memory[start] << 16) + (trace_memory[start + 3] << 8) + trace_memory[start + 2]
    trace_file_id = trace_id >> TRACE_FILE_ID_OFT
    if(trace_file_id > TRACE_FILE_ID_MAX):
        print(trace_file_id, "> TRACE_FILE_ID_MAX")
        return

    if(str(trace_file_id) not in log_arr):
        print("get wrong id %d, it's not in dictionary" % (trace_file_id))
        print("trace stop, maybe you should update your dictionary")
        return

    line = trace_id - (trace_file_id << TRACE_FILE_ID_OFT);

    if(str(line) not in log_arr[str(trace_file_id)]):
        print("get wrong line %d, it's not in id %d" % (line, trace_file_id))
        print("trace stop, maybe you should update your dictionary")
        return

    ts = (trace_memory[start + 5] << 24) + (trace_memory[start + 4] << 16) + (trace_memory[start + 7] << 8) + trace_memory[start + 6]
    param = trace_memory[start+TRACE_HEADER_LEN:start+TRACE_HEADER_LEN+nb_param*2]
    if(len(param) != nb_param*2):
        return

    if(log_lvl > 0):
        print("num,id,file,line,log:",nb_param, trace_file_id, file_arr[str(trace_file_id)], str(line), log_arr[str(trace_file_id)][str(line)],param)

    log_str = translate_param(log_arr[str(trace_file_id)][str(line)], param)
    log_str = translate_id_to_str(log_str)
    print("[%.6f]%s" % (ts/1000000, log_str))

    next_start = start + TRACE_HEADER_LEN + nb_param * 2;
    if(next_start > len(trace_memory) - 8):
        return -1
    else:
        return next_start

def main():
    get_file_arr()
    get_trace_memory()
    get_msg_task_id()
    if log_lvl == 1:
        arr = sorted(msg_id_dict.items(), key=lambda x: x[0])
        for n in arr:
            print (n)
    start = 0
    while True:
        if(log_lvl > 0):
            print("start:", start)
        start = get_trace_info(start)
        if(start < 0):
            break;
main()
