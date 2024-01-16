import os
import sys
from setuptools import setup, find_packages, Extension
from Cython.Build import cythonize

os.chdir(os.path.dirname(os.path.realpath(__file__)))

OS_WINDOWS = os.name == "nt"

CMAKE = dict()
with open("cmake-build/CMakeCache.txt") as f:
    for line in f.readlines():
        if line.startswith("#") or line.startswith("//"):
            continue
        if ":" in line:
            key, value = line.split(":", 1)
            line_type, val = value.split("=", 1)
            CMAKE[key] = val.strip()

def get_requirements():
    """
    To update the requirements for advent, edit the requirements.txt file.
    """
    with open("requirements.txt", "r") as f:
        req_lines = f.readlines()
    reqs = []
    for line in req_lines:
        # Avoid adding comments.
        line = line.split("#")[0].strip()
        if line:
            reqs.append(line)
    return reqs


def package_data():
    """
    By default, the distribution tools ignore all non-python files.
    Make sure we get everything.
    """
    file_set = []
    for root, dirs, files in os.walk("adventkai"):
        for f in files:
            if ".git" in f.split(os.path.normpath(os.path.join(root, f))):
                # Prevent the repo from being added.
                continue
            file_name = os.path.relpath(os.path.join(root, f), "adventkai")
            file_set.append(file_name)
    return file_set


from os import path

this_directory = path.abspath(path.dirname(__file__))
with open(path.join(this_directory, "README.md"), encoding="utf-8") as f:
    long_description = f.read()

INCLUDE_PATHS = [
    "dbat/include",
    "dbat/include/dbat",
    CMAKE["Boost_INCLUDE_DIR"],
    CMAKE["spdlog_SOURCE_DIR"] + "/include",
    CMAKE["FMT_SOURCE_DIR"] + "/include",
    CMAKE["effolkronium_random_SOURCE_DIR"] + "/include",
    CMAKE["SQLiteCpp_SOURCE_DIR"] + "/include",
    CMAKE["sodium_SOURCE_DIR"] + "/libsodium/src/libsodium/include",
    CMAKE["nlohmann_json_SOURCE_DIR"] + "/single_include"
]

LIBRARY_DIRS = [
    "dbat/bin",
    CMAKE["SQLiteCpp_BINARY_DIR"],
    CMAKE["effolkronium_random_BINARY_DIR"],
    CMAKE["sodium_BINARY_DIR"],
    CMAKE["FMT_BINARY_DIR"]

]

extensions = [
    Extension(
        "circlemud",
        sources=["circle/circlemud.pyx"],
        include_dirs=INCLUDE_PATHS,
        library_dirs=LIBRARY_DIRS,
        libraries=["circlemud", "SQLiteCpp", "fmtd", "sodium",
                   "sqlite3"],
        language="c++"
    )
]

# setup the package
setup(
    name="adventkai",
    version="0.1.0",
    author="VolundMush",
    maintainer="VolundMush",
    url="https://github.com/volundmush/adventkai",
    description="",
    license="MIT",
    long_description=long_description,
    long_description_content_type="text/markdown",
    packages=find_packages(),
    package_data={"": package_data()},
    install_requires=get_requirements(),
    zip_safe=False,
    classifiers=[
        "Programming Language :: Python :: 3 :: Only",
        "Programming Language :: Python :: 3.12",
        "Intended Audience :: Developers",
        "Topic :: Games/Entertainment :: Multi-User Dungeons (MUD)",
        "Topic :: Games/Entertainment :: Puzzle Games",
        "Topic :: Games/Entertainment :: Role-Playing",
        "Topic :: Games/Entertainment :: Simulation",
        "Topic :: Software Development :: Libraries :: Application Frameworks",
    ],
    python_requires=">=3.12",
    project_urls={
        "Source": "https://github.com/volundmush/adventkai",
        "Issue tracker": "https://github.com/volundmush/adventkai/issues",
    },
    ext_modules=cythonize(extensions, include_path=INCLUDE_PATHS)
)
