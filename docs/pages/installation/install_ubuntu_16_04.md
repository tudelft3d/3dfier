---
title: Install 3dfier on Ubuntu
keywords: install ubuntu
sidebar: 3dfier_sidebar
permalink: install_ubuntu.html
---

## 1. Adding *ubuntugis* repositories
To install *GDAL* on Ubuntu 16.04 LTS it is probably the easiest to add one of the *ubuntugis* repositories, either [*ubuntugis-stable*](https://launchpad.net/~ubuntugis/+archive/ubuntu/ppa?field.series_filter=xenial) or [*ubuntugis-unstable*](https://launchpad.net/~ubuntugis/+archive/ubuntu/ubuntugis-unstable?field.series_filter=xenial). Both contains *GDAL* >= 2.1.

E.g. add the *ubuntugis-stable* repository by running:
```
sudo add-apt-repository ppa:ubuntugis/ppa
sudo apt-get update
```

## 2. Install dependencies
*CGAL* (`libcgal-dev`), *Boost* (`libboost-all-dev`) and *yaml-cpp* (`libyaml-cpp0.5v5`) are part of the *Ubuntu Universe* repository.

Once you have all the repos added, you can use a package manager, e.g. `apt` or *Synaptic* to install them. 

E.g. using apt-get:
```
sudo apt-get install libcgal-dev libboost-all-dev libyaml-cpp0.5v5
```

## 3. Run the build script 
*LASzip*, *libLAS* and *3dfier* need to be built manually in this order, and this is what the [build script](https://github.com/tudelft3d/3dfier/blob/build_ubuntu_2/ressources/build_ubuntu1604.sh) can do for you. It downloads and compiles these three packages, and takes care that *libLAS* is compiled with *LASzip* support and that *3dfier* can find the executables of both. We try to take care that the download links are up to date with the upcoming releases. However, if you notice that the links are outdated, just update the version number in the script.

To download and run the [build script](https://github.com/tudelft3d/3dfier/blob/build_ubuntu_2/ressources/build_ubuntu1604.sh) do:
```
wget https://raw.githubusercontent.com/tudelft3d/3dfier/build_ubuntu_2/ressources/build_ubuntu1604.sh
sudo build_ubuntu1604.sh /opt
```

Where `/opt` is the directory where *3dfier* will be installed and *LASzip*, *libLAS* downloaded and compiled. *LASzip*, *libLAS* is installed into `/usr/local`, hence you'll probably need root privilage to run the script.

**A note on GRASS GIS and libLAS**\
If you already have GRASS installed from the *ubuntugis-(un)stable* PPA, you probably also have libLAS (`liblas-c3, liblas3`) installed, as GRASS depends on them. However, this *libLAS* install is without *LASzip* support as *LASzip* is not part of the *ubuntugis* PPA. Therefore, you will need to remove the GRASS and *libLAS* libraries first, then compile *libLAS* with *LASzip* support (with this script). Then you can install GRASS again from the *ubuntugis* PPA, it will be compiled with *libLAS* that now supports `.laz`.
