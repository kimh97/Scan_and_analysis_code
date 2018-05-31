import argparse
import os

def parsing():
    parser = argparse.ArgumentParser()
    parser.add_argument("SCANS", type=int, help="Number of scans")
    parser.add_argument("-F", "--filename", type=str, default="/home/lab-cpt03/LabData/Linus/180508/waveform_2pulses_", help="Name of files to be saved; do not add .dat")

    args = parser.parse_args()
    return args



if __name__ == '__main__':

    # args = parsing()
    # print args
    # N_scans = args.SCANS
    # name = args.filename


    for x in range(0, 40):

        os.system("/home/lab-cpt03/programs/TimingDAQ/DRSclDat2Root --input_file=/home/lab-cpt03/LabData/Linus/180508/waveform_2pulses_" + str(x)+".dat --config=./config/CPTLab/DRS_trial180504.config --save_meas") # Run root conversion code
