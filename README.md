# DataInspector
## Dependencies
* cmake
* arrow
* libmongoc
* root
* boost
* rapidjson
* [httplib](https://github.com/yhirose/cpp-httplib)

All dependencies can be installed by in simillar manner by following the example installation on Ubuntu 20.04 in [Dockerfile](builder/Dockerfile).

## Parameters
Program requires those environment variables to be declared:
* WORK_DIR - path to alibuild work dir (i.e. ~/alice/sw)
* ALIBUILD_ARCH_PREFIX - nam of the folder inside WORK_DIR where builds are stored (i.e. ubuntu2004_x86-64)
* DI_DATASETS - path to folder, where available datasets for runs are stored (must be created - can be empty)
* RUNS_PATH - points to directory in which runs will be started (and their output stored) (must be created)
* MONGO_URL - mongo connection string (mongo can be started by exposing port 27017 in [docker-compose.yaml](docker-compose.yaml) and running ```sudo docker-compose up mongo```)

Proxy expects two arguments to be provided:
1. path to bash script used to execute workflows
2. path to folder containing files which available O2/O2Physics builds:
    1. Filename: \<name\>.build
    2. Contest:
        1. Build name
        2. GitHub url (required - but not used at the moment)
        3. Branch name (required - but not used at the moment)
        4. Relative path from ALIBUILD_WORK_DIR for specific architecture (i.e. ~/alice/sw/$ALIBUILD_ARCH_PREFIX) to chosen build (same as in alienv, i.e. O2/data-inspector-cleaned-local1)
    
