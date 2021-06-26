#!/usr/bin/env python3

from os import listdir
import os
import pandas as pd

config_output_dir = 'output'
config_parsed_output_dir = 'parsed_output'


def cmake_parser(file_name):
    content = read_all_lines_file(file_name)
    features = []
    for line in content:
        if ':BOOL=' in line and not line.startswith('CMAKE_'):
            #print line
            feature_name = line.split(':')[0]
            features.append({'feature': feature_name,
                             'enable': '{}:BOOL=ON'.format(feature_name),
                             'disable': '{}:BOOL=OFF'.format(feature_name)
                             })
    save_features(file_name, features)


def save_features(file_name, features):
    df = pd.DataFrame(features)
    result_file = os.path.join(config_parsed_output_dir, file_name.split('.')[0]+'.csv')
    df.to_csv(result_file, index=False)


def read_all_lines_file(file_name):
    file_full_path = os.path.join(config_output_dir, file_name)
    with open(file_full_path) as f:
        content = f.readlines()
    content = [x.strip() for x in content]
    return content


def auto_parser(file_name):
    content = read_all_lines_file(file_name)
    features = []
    for line in content:
        if '--enable-' in line or '--disable-' in line:
            feature_name = line.split()[0].split('[')[0].split('able-')[1]
            exceptions = ['FEATURE', 'option-checking', 'documentation', 'doxygen',
                          'manpages', 'tests', 'examples']
            if feature_name not in exceptions:
                features.append({'feature': feature_name,
                                 'enable': '--enable-{}'.format(feature_name),
                                 'disable': '--disable-{}'.format(feature_name)
                                 })
    save_features(file_name, features)


def main():
    output_files = listdir(config_output_dir)
    for file_name in output_files:
        if '.cmake' in file_name:
            cmake_parser(file_name)
        elif '.auto' in file_name:
            auto_parser(file_name)
        else:
            print(f"No valid filename: {file_name}")
            print(f"Filename should contain .cmake or .auto")


if __name__ == '__main__':
    main()
