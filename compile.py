#!/usr/bin/env python
import sys
import os
import shutil
import subprocess
import platform


def clean_build_artifacts():
    extensions = ['.so', '.pyd'] if platform.system() == 'Windows' else ['.so']
    for ext in extensions:
        for filename in os.listdir('.'):
            if filename.endswith(ext):
                os.remove(filename)


def main(build_mode):
    if build_mode.lower() not in ['debug', 'release']:
        print("Error: Must be called with 'debug' or 'release'!")
        sys.exit(1)

    print(f"Compiling as {build_mode.lower()}...")

    # Clean up
    clean_build_artifacts()

    circle_path = os.path.join('.', 'circle')
    for filename in os.listdir(circle_path):
        if filename.endswith('.cpp'):
            os.remove(os.path.join(circle_path, filename))

    out_folder = 'cmake-build'

    # Create output folder if it doesn't exist
    os.makedirs(out_folder, exist_ok=True)

    # Run CMake and build
    cmake_args = [
        'cmake',
        '-S', 'dbat',
        '-B', out_folder,
        f'-D', f'CMAKE_BUILD_TYPE={build_mode.capitalize()}'
    ]
    build_args = [
        'cmake',
        '--build', out_folder,
        '--', f'-j{os.cpu_count()}'
    ]

    subprocess.run(cmake_args, check=True)
    subprocess.run(build_args, check=True)

    # Build Python extension
    subprocess.run([sys.executable, 'setup.py', 'build_ext', '--inplace', "--parallel"], check=True)


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: python compile.py [debug/release]")
        sys.exit(1)
    main(sys.argv[1])