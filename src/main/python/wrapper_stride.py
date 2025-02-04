#!/usr/bin/python

import sys
import os
import argparse
import subprocess
import json
import itertools
import datetime
import shutil
import csv
import random


import xml.etree.cElementTree as ET

# --------------------------------
# Function that runs the simulator.
# --------------------------------
def runSimulator(binary_command, num_days, rng_seed, seeding_rate, r0, population_file, immunity_rate, output_prefix, disease_config_file, generate_person_file, num_participants_survey, start_date, holidays_file, age_contact_matrix_file, log_level):

    # Write configuration file
    root = ET.Element("run")

    ET.SubElement(root, "num_days").text = str(num_days)
    ET.SubElement(root, "rng_seed").text = str(rng_seed)
    ET.SubElement(root, "seeding_rate").text = str(seeding_rate)
    ET.SubElement(root, "r0").text = str(r0)
    ET.SubElement(root, "population_file").text = population_file
    ET.SubElement(root, "immunity_rate").text = str(immunity_rate)
    ET.SubElement(root, "output_prefix").text = str(str(output_prefix))
    ET.SubElement(root, "disease_config_file").text = str(str(disease_config_file))
    ET.SubElement(root, "generate_person_file").text = str(str(generate_person_file))
    ET.SubElement(root, "num_participants_survey").text = str(str(num_participants_survey))

    ET.SubElement(root, "start_date").text = str(str(start_date))
    ET.SubElement(root, "holidays_file").text = str(str(holidays_file))
    ET.SubElement(root, "age_contact_matrix_file").text = str(str(age_contact_matrix_file))
    ET.SubElement(root, "log_level").text = str(str(log_level))

    tree = ET.ElementTree(root)
    tree.write(str(output_prefix)+ ".xml")


    # Execute the call
    cmd_stride = "".join([str(binary_command), " --config ", str(output_prefix)+ ".xml"])
    os.system(cmd_stride)


# -----------------------------------------------------
# Function that uses an R script to process the results
# -----------------------------------------------------
def processOutput(output_dir,file_tag):

    work_dir = os.getcwd()
    r_file = open(os.path.join(output_dir,'plot_timings.R'), 'w')
    r_file.write('# R file to plot the output' + '\n')

    r_file.write('\n' + '## SET R WORK DIR' + '\n')
    r_file.write('setwd("' + os.path.join(work_dir,output_dir) + '")' + '\n')

    r_file.write('\n' + '## LOAD HELP FUNCTIONS' + '\n')
    r_file.write('source("' + work_dir + '/lib/plot_results_lib.R")' + '\n')
    r_file.write('source("' + work_dir + '/lib/plot_social_contacts_lib.R")' + '\n')

    r_file.write('\n' + '## GET DATA' + '\n')
    r_file.write('data_tag <- "' + file_tag + '"' + '\n')
    r_file.write('project_dir <- "' + work_dir + '"' + '\n')

    r_file.write('\n## PLOT ALL RESULTS' + '\n')
    r_file.write('plot_results(data_tag)' + '\n')
    r_file.write('plot_social_contacts(data_tag,project_dir)' + '\n')


    r_file.close()

    # RUN Rscript
    cmd_r = 'Rscript ' + output_dir + '/plot_timings.R'
    os.system(cmd_r)


def main(argv):

    # Arguments parser
    parser = argparse.ArgumentParser(description='Script to execute multiple runs of the simulator.')
    parser.add_argument('--config', help = 'A config file describing the experiments to run.', default = './config/wrapper_default.json', type=str)

    args = vars(parser.parse_args())

    # Load the json file
    config = json.load(open(args['config'], 'r'))

    # Load the experiment configurations from the json
    experiments = [config['threads'], config['rng_seed'], config['seeding_rate'], config['r0'], config['population_file'],config['immunity_rate']]

    # Check the config 'output'... if not empty, use separator in names
    name_sep = ''
    if len(config['output_tag']) > 0:
        name_sep='_'

    # Create (summary) output dir with subdir for experiment output
    time_stamp  = datetime.datetime.now().strftime("%m%d%H%M%S") # add '%Y to add year and '%f' for milisec
    file_tag    = time_stamp + name_sep + config['output_tag']

    output_dir        = os.path.join('output', time_stamp + name_sep + config['output_tag'])
    experiments_dirs  = os.path.join(output_dir, 'experiments')
    if not os.path.isdir(experiments_dirs):
        os.makedirs(experiments_dirs)

    # Open the aggregated output files
    summary_file  = open(os.path.join(output_dir, file_tag + '_summary.csv'), 'w')
    cases_file     = open(os.path.join(output_dir, file_tag + '_cases.csv'), 'w')

    # Copy the json configuration file
    config_file  = open(os.path.join(experiments_dirs, 'exp_config.json'), 'w')
    config_file.write(open(args['config'], 'r').read())
    config_file.close()

    # Create a copy of the environment variables to modify and pass to simulator
    env = os.environ.copy()

    is_first=True # needed to have only one header in the aggregated output

    # RUN ALL EXPERIMENTS
    # note: 'itertools.product' makes all combinations within 'experiments'
    # note: (a,b) is a tuple... enumerate(vect) gives ((1, vect_n1) (2,vect_n2) ...)

    for (index, experiment) in enumerate(itertools.product(*experiments)):

        # Create a output_prefix for the experiment output
        output_prefix = os.path.join(experiments_dirs, 'exp' + str(index))

        # Set the OpenMP environment
        os.putenv('OMP_NUM_THREADS',str(experiment[0]))
        os.putenv('OMP_SCHEDULE' , str(config['omp_schedule']))

        # Run the simulator     ('experiment[0]' has been used for the OMP_NUM_THREADS)
        runSimulator(config['binary_command'], config['num_days'], experiment[1], experiment[2], experiment[3], experiment[4], experiment[5], output_prefix, config['disease_config_file'],config['generate_person_file'],config['num_participants_survey'], config['start_date'], config['holidays_file'], config['age_contact_matrix_file'], config['log_level'])

        # Append the aggregated outputs
        if is_first:
            summary_file.write(open(output_prefix+'_summary.csv','r').read())
            is_first = False
        else:
            lines = open(output_prefix+'_summary.csv','r').readlines()
            for line in lines[1:]:
                summary_file.write(line)

        cases_file.write(open(output_prefix + '_cases.csv', 'r').read())
        cases_file.flush()
        summary_file.flush()

        # Remove copied files
        os.remove(output_prefix + '_summary.csv')
        os.remove(output_prefix + '_cases.csv')

        # get participant, contact and transmission files from the 'logfile'.
        cmd_parse = './lib/log2csv.py ' + output_prefix
        os.system(cmd_parse)

        # Remove configuration file
        os.remove(output_prefix + '.xml')

    summary_file.close()
    cases_file.close()

    ## process the output
    processOutput(output_dir, file_tag)



if __name__ == "__main__":
    main(sys.argv)
