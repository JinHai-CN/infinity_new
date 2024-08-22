#! /usr/bin/env python3
import csv
import functools
import os
import errno
import subprocess
import time
import traceback
from shutil import copyfile
import numpy as np

from common import common_values

current_path = os.getcwd()
data_dir = current_path + common_values.TEST_DATA_DIR
copy_dir = common_values.TEST_TMP_DIR


# catch the expected exception
def trace_expected_exceptions(func):
    @functools.wraps(func)
    def wrapped_func(*args, **kwargs):
        try:
            func(*args, **kwargs)
        except:
            # print("Expected exception in " + func.__name__)
            traceback.print_exc()

    return wrapped_func


def get_project_path():
    current_file = os.path.abspath(__file__)
    index = current_file.index("infinity")
    desired_path = current_file[: index + len("infinity")]
    return str(desired_path)


# read fvecs file
def read_fvecs_file(filename):
    file_int32 = np.fromfile(filename, dtype="int32")
    d = file_int32[0]
    ivecs = file_int32.reshape(-1, d + 1)[:, 1:].copy()
    fvecs = ivecs.view("float32")
    return fvecs


def start_infinity_service_in_subporcess():
    shell = get_project_path() + "/cmake-build-debug/src/infinity"
    with open("./tmp.txt", "w") as f:
        infinity = subprocess.Popen(shell, stdout=f)
    time.sleep(1)
    return infinity

def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exc: # Python >2.5
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else: raise

def copy_data(file_name, dest_file_name):
    data_dir = get_project_path() + common_values.TEST_DATA_DIR
    for dirpath, dirnames, filenames in os.walk(data_dir):
        for filename in filenames:
            if filename == file_name:
                src_path = os.path.join(dirpath, filename)
                dest_path = os.path.join(common_values.TEST_TMP_DIR, dest_file_name)
                if not os.path.exists(common_values.TEST_TMP_DIR):
                    mkdir_p(common_values.TEST_TMP_DIR)
                    copyfile(src_path, dest_path)
                copyfile(src_path, dest_path)


def generate_big_int_csv(num, filename):
    with open(
        get_project_path() + common_values.TEST_DATA_DIR + "csv/" + filename, "w"
    ) as f:
        for i in range(num):
            f.write(str(i) + "," + str(i) + "\n")
    f.close()


def generate_big_rows_csv(num, filename):
    with open(
        get_project_path() + common_values.TEST_DATA_DIR + "csv/" + filename, "w"
    ) as f:
        for i in range(num):
            f.write(str(i) + ",asdasdlk中fjio@!#!@asd #$%$23\n")
    f.close()


def generate_big_columns_csv(num, filename):
    with open(
        get_project_path() + common_values.TEST_DATA_DIR + "csv/" + filename, "w"
    ) as f:
        data = "".join(str(i) + "," for i in range(num - 1))
        data += str(num - 1)
        f.write(data)
    f.close()


def generate_fvecs(num, dim, filename):
    with open(
        get_project_path() + common_values.TEST_DATA_DIR + "fvecs/" + filename, "wb"
    ) as fvecs_file:
        for _ in range(num):
            fvecs_file.write((dim).to_bytes(4, byteorder="little"))
            fvec = np.random.random(dim).astype(np.float32)
            fvec.tofile(fvecs_file)
    fvecs_file.close()


def generate_commas_enwiki(in_filename, out_filename, is_embedding):
    with open(
        get_project_path() + common_values.TEST_DATA_DIR + "csv/" + in_filename, "r"
    ) as infile, open(
        get_project_path() + common_values.TEST_DATA_DIR + "csv/" + out_filename, "w"
    ) as outfile:
        reader = csv.reader(infile, delimiter="\t")
        writer = csv.writer(outfile, delimiter=",")

        if is_embedding:
            i = 0
            for row in reader:
                suffix = [i, "[{},{},{},{}]".format(i, i, i, i)]
                writer.writerow(row + suffix)
                i += 1
        else:
            for row in reader:
                writer.writerow(row)


def check_data(data_dir):
    # path not exists
    if not os.path.exists(data_dir):
        os.makedirs(data_dir)
        return False
