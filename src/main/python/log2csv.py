#!/usr/bin/python

"""
Create .csv files from logfiles produced by the simulator.

"""

import sys
import csv
import random
import os

def prepare_csv(log_file_path):
    """
    From logfile with all contacts or transmissions logged in the following format:
    [PART] local_id part_age part_gender
    [CONT] local_id part_age cnt_age cnt_home cnt_school cnt_work cnt_other sim_day
    [TRAN] local_id start_infection

    Create csv-files participants.csv, contacts.csv and transmissions.csv
    """

    participants_file = log_file_path + '_participants.csv'
    contacts_file     = log_file_path + '_contacts.csv'
    transmission_file = log_file_path + '_transmissions.csv'

    # Open csv files to write to.
    with open(participants_file, 'w') as p, open(contacts_file, 'w') as c, open(transmission_file,'w') as t:

        # Write headers for csv files
        p_fieldnames = ['local_id', 'part_age', 'part_gender']
        p_writer = csv.DictWriter(p, fieldnames=p_fieldnames)
        p_writer.writeheader()

        c_fieldnames = ['local_id', 'part_age', 'cnt_age', 'cnt_home', 'cnt_school', 'cnt_work', 'cnt_prim_comm', 'cnt_sec_comm', 'sim_day']
        c_writer = csv.DictWriter(c, fieldnames=c_fieldnames)
        c_writer.writeheader()

        t_fieldnames  = ['local_id', 'new_infected_id', 'cnt_location','sim_day']
        t_writer = csv.DictWriter(t, fieldnames=t_fieldnames)
        t_writer.writeheader()

        flag_p = 0
        flag_c = 0
        flag_t = 0

        with open (log_file_path+'_logfile.txt', 'r') as f:
            for line in f:
                identifier = line[:6]
                line = line[7:]
                line = line.split()
                if identifier == "[PART]":
                    flag_p = 1
                    dic = {}
                    for i in range(len(p_fieldnames)):
                        value = line[i]
                        dic[p_fieldnames[i]] = value
                    p_writer.writerow(dic)
                # else for Contacts.csv
                if identifier == "[CONT]":
                    flag_c = 1
                    dic = {}
                    for i in range(len(c_fieldnames)):
                        value = line[i]
                        dic[c_fieldnames[i]] = value
                    c_writer.writerow(dic)
                # else for transmissions
                if identifier == "[TRAN]":
                    flag_t = 1
                    dic = {}
                    for i in range(len(t_fieldnames)):
                        value = line[i]
                        dic[t_fieldnames[i]] = value
                    t_writer.writerow(dic)


        if(flag_p==0):
            os.remove(participants_file)
        if(flag_c==0):
            os.remove(contacts_file)
        if(flag_t==0):
            os.remove(transmission_file)
        os.remove(log_file_path+'_logfile.txt')


def main(argv):
    if len(argv) == 1:
        prepare_csv(argv[0])
    else:
        print ("Usage: python prepare_csv.py <run_file_path>")


if __name__ == "__main__":
    main(sys.argv[1:])

