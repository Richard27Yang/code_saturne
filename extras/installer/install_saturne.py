#!/usr/bin/env python

#-------------------------------------------------------------------------------
# Library modules import
#-------------------------------------------------------------------------------

import sys

if sys.version_info[:2] < (2,3):
    sys.stderr.write("This script needs Python 2.4 at least\n")

import platform

if platform.system == 'Windows':
    sys.stderr.write("This script only works on Unix-like platforms\n")

import os, shutil
import string
import subprocess

#-------------------------------------------------------------------------------
# Global variable
#-------------------------------------------------------------------------------

verbose = 'yes'

#-------------------------------------------------------------------------------
# Global methods
#-------------------------------------------------------------------------------

def run_command(cmd, stage, app, log):
    """
    Run a command via the subprocess module.
    """

    if verbose == 'yes':
        sys.stdout.write("   o " + stage + "...\n")

    p = subprocess.Popen(cmd,
                         shell=True,
                         stdout=subprocess.PIPE,
                         stderr=subprocess.STDOUT)

    output = p.communicate()
    log.write(output[0])

    if p.returncode != 0:
        sys.stderr.write("Error during " + string.lower(stage) +
                         " stage of " + app + ".\n")
        sys.stderr.write("See " + log.name + " for more information.\n")
        sys.exit(1)


def run_test(cmd):
    """
    Run a test for a given command via the subprocess module.
    """

    if verbose == 'yes':
        sys.stdout.write("   o Checking for " + os.path.basename(cmd) + "...  ")

    cmd = "type " + cmd

    p = subprocess.Popen(cmd,
                         shell=True,
                         stdout=subprocess.PIPE,
                         stderr=subprocess.STDOUT)

    output = p.communicate()
    if verbose == 'yes':
        if p.returncode == 0: log_str = output[0].split()[2]
        else: log_str = "not found"
        sys.stdout.write("%s\n" % log_str)

    return p.returncode

#-------------------------------------------------------------------------------
# Class definition for a generic package
#-------------------------------------------------------------------------------

class Package:

    def __init__(self, name, description, package, version, archive, url):

        # Package information
        self.name = name
        self.description = description
        self.package = package
        self.version = version
        self.archive = archive
        self.url = url % self.archive

        # Installation information
        self.use = 'no'
        self.installation = 'no'
        self.installation_pdf = 'no'
        self.source_dir = None
        self.install_dir = None
        self.config_opts = ''
        self.log_file = sys.stdout
        self.cc = None
        self.fc = None
        self.vpath_support = True
        self.create_install_dirs = False


    def info(self):

        sys.stdout.write("\n"
                         "   %(s_name)s (%(l_name)s)\n"
                         "   version: %(vers)s\n"
                         "   url: %(url)s\n"
                         "   package: %(pack)s\n"
                         "   source_dir: %(src)s\n"
                         "   install_dir: %(inst)s\n"
                         "   config_opts: %(opts)s\n\n"
                         % {'s_name':self.name, 'l_name':self.description,
                            'vers':self.version, 'url':self.url,
                            'pack':self.package,
                            'src':self.source_dir, 'inst':self.install_dir,
                            'opts':self.config_opts})


    def download(self):

        import urllib

        try:
            urllib.urlretrieve(self.url, self.archive)
        except:
            sys.stderr.write("Error while retrieving %s\n" % self.url)
            sys.exit(1)


    def extract(self):

        if self.archive[-4:] == '.zip':

            import zipfile

            if not zipfile.is_zipfile(self.archive):
                sys.stderr.write("%s is not a zip archive\n" % self.archive)
                sys.exit(1)

            zip = zipfile.ZipFile(self.archive)

            relative_source_dir = zip.namelist()[0].split(os.path.sep)[0]
            self.source_dir = os.path.abspath(relative_source_dir)

            zip.close()

            # Use external unzip command so as to keep file properties

            p = subprocess.Popen('unzip ' + self.archive,
                                 shell=True,
                                 stdout=sys.stdout,
                                 stderr=sys.stderr)
            output = p.communicate()

            if p.returncode != 0:
                sys.stderr.write("Error unzipping file " + self.archive + ".\n")
                sys.exit(1)

        else:

            import tarfile

            if not tarfile.is_tarfile(self.archive):
                sys.stderr.write("%s is not a tar archive\n" % self.archive)
                sys.exit(1)

            tar = tarfile.open(self.archive)

            first_member = tar.next()
            relative_source_dir = first_member.name.split(os.path.sep)[0]
            self.source_dir = os.path.abspath(relative_source_dir)

            try:
                tar.extractall()
            except AttributeError:
                for tarinfo in tar:
                    tar.extract(tarinfo)

            tar.close()


    def install(self):

        current_dir = os.getcwd()

        build_dir = self.source_dir + '.build'
        if os.path.isdir(build_dir): shutil.rmtree(build_dir)

        # Create some install directories in case install script does not work
        if self.create_install_dirs and self.install_dir is not None:
            inc_dir = os.path.join(self.install_dir, 'include')
            lib_dir = os.path.join(self.install_dir, 'lib')
            for dir in [inc_dir, lib_dir]:
                if not os.path.isdir(dir):
                    os.makedirs(dir)

        # Copy source files in build directory if VPATH feature is unsupported
        if self.vpath_support:
            os.makedirs(build_dir)
        else:
            shutil.copytree(self.source_dir, build_dir)
        os.chdir(build_dir)

        # Set command line for configure pass
        configure = os.path.join(self.source_dir, 'configure')
        if self.install_dir is not None:
            configure = configure + ' --prefix=' + self.install_dir
        configure = configure + ' ' + self.config_opts

        # Add compilers
        if self.cc is not None: configure = configure + ' CC=\"' + self.cc + '\"'
        if self.fc is not None: configure = configure + ' FC=\"' + self.fc + '\"'

        # Install the package
        run_command(configure, "Configure", self.name, self.log_file)
        run_command("make", "Compile", self.name, self.log_file)
        run_command("make install", "Install", self.name, self.log_file)

        # Install pdf documentation if needed
        if self.installation_pdf == 'yes':
            try:
                run_command("make pdf", "Generate documentation",
                            self.name, self.log_file)
                run_command("make install-pdf", "Install documentation",
                            self.name, self.log_file)
            except:
                pass

        # Clean build directory
        run_command("make clean", "Clean", self.name, self.log_file)

        # End of installation
        os.chdir(current_dir)

#-------------------------------------------------------------------------------
# Class definition for Code_Saturne setup
#-------------------------------------------------------------------------------

class Setup:

    def __init__(self):

        # Code_Saturne libraries
        self.satlibs = ['bft', 'fvm', 'mei', 'ecs', 'ncs']

        # Optional libraries
        self.optlibs = ['cgns', 'hdf5', 'med', 'mpi', 'libxml2', 'swig']

        self.libs = self.optlibs + self.satlibs

        # Code_Saturne version 
        self.version = '2.0'

        # Logging file
        self.log_file = sys.stdout

        # Download packages
        self.download = 'yes'

        # Code_Saturne language (may be en/fr)
        self.language = 'en'

        # Code_Saturne installation with debugging symbols
        self.debug = 'no'

        # Default compilers
        self.cc = None
        self.fc = None
        self.mpicc = None

        # Disable GUI
        self.disable_gui = 'no'

        # Python interpreter path
        self.python = None

        # BLAS library path
        self.blas = None

        # Metis library path
        self.metis = None

        # Scotch library path
        self.scotch = None

        # SYRTHES path
        self.syrthes = None

        # Architecture name
        self.arch = None

        # Global installation prefix
        # (if None, standard directory "/usr" will be used)
        self.prefix = None

        # Code_Saturne installation prefix
        self.cs_prefix = None


        # Packages definition
        self.packages = {}

        url_cs = "http://innovation.edf.com/fichiers/fckeditor/Commun/Innovation/logiciels/code_saturne/Releases/%s"

        # BFT library

        self.packages['bft'] = \
            Package(name="BFT",
                    description="Basic Functions and Tools",
                    package="bft",
                    version="1.1.5",
                    archive="bft-115.zip",
                    url=url_cs)

        # FVM library

        self.packages['fvm'] = \
            Package(name="FVM",
                    description="Finite Volume Mesh",
                    package="fvm",
                    version="0.15.3",
                    archive="fvm-0153.zip",
                    url=url_cs)

        p = self.packages['fvm']
        p.use_lib_mpi_wrapper = True

        # MEI library

        self.packages['mei'] = \
            Package(name="MEI",
                    description="Mathematical Expressions Interpreter",
                    package="mei",
                    version="1.0.2",
                    archive="mei-102.zip",
                    url=url_cs)

        # Code_Saturne Preprocessor

        self.packages['ecs'] = \
            Package(name="ECS",
                    description="Code_Saturne Preprocessor",
                    package="ecs",
                    version="2.0.2",
                    archive="ecs-202.zip",
                    url=url_cs)

        # Code_Saturne Kernel

        self.packages['ncs'] = \
            Package(name="NCS",
                    description="Code_Saturne Kernel",
                    package="ncs",
                    version="2.0.2",
                    archive="ncs-202.zip",
                    url=url_cs)

        p = self.packages['ncs']
        p.use_lib_mpi_wrapper = True

        # CGNS library

        self.packages['cgns'] = \
            Package(name="CGNS",
                    description="CFD General Notation System",
                    package="cgnslib",
                    version="2.5.5",
                    archive="cgnslib_2.5-5.tar.gz",
                    url="http://sourceforge.net/projects/cgns/files/cgnslib_2.5/Release%%205/%s/download")

        p = self.packages['cgns']
        p.config_opts = "--enable-64bit --enable-lfs"
        p.vpath_support = False
        p.create_install_dirs = True

        # HDF5 library

        self.packages['hdf5'] = \
            Package(name="HDF5",
                    description="Hierarchical Data Format",
                    package="hdf5",
                    version="1.8.7",
                    archive="hdf5-1.8.7.tar.gz",
                    url="http://www.hdfgroup.org/ftp/HDF5/current/src/%s")

        p = self.packages['hdf5']
        p.config_opts = "--enable-production"

        # MED library

        self.packages['med'] = \
            Package(name="MED",
                    description="Model for Exchange of Data",
                    package="med",
                    version="3.0.3",
                    archive="med-3.0.3.tar.gz",
                    url="http://files.salome-platform.org/Salome/other/%s")

        p = self.packages['med']
        p.config_opts = "--with-med_int=int"

        # MPI library

        self.packages['mpi'] = \
            Package(name="MPI",
                    description="Message Passing Interface",
                    package="openmpi",
                    version="1.4.3",
                    archive="openmpi-1.4.3.tar.gz",
                    url="http://www.open-mpi.org/software/ompi/v1.4/downloads/%s")

        # Libxml2 library (official url "ftp://xmlsoft.org/libxml2/%s")

        self.packages['libxml2'] = \
            Package(name="libxml2",
                    description="XML library",
                    package="libxml2",
                    version="2.6.32",
                    archive="libxml2-2.6.32.tar.gz",
                    url="ftp://fr.rpmfind.net/pub/libxml/%s")


        p = self.packages['libxml2']
        p.config_opts = "--with-ftp=no --with-http=no"

        # Swig library

        self.packages['swig'] = \
            Package(name="Swig",
                    description="Wrapper Python",
                    package="swig",
                    version="1.3.40",
                    archive="swig-1.3.40.tar.gz",
                    url="http://sourceforge.net/projects/swig/files/swig/swig-1.3.40/%s/download")

        p = self.packages['swig']
        p.config_opts = "--without-alllang"


    def read_setup(self):
        
        #
        # setup file reading
        #
        try:
            setupFile = file('setup', mode='r')
        except IOError:
            sys.stderr.write('Error: opening setup file\n')
            sys.exit(1)

        shutil.copy('setup','setup_ini')

        while 1:

            line = setupFile.readline()
            if line == '': break

            # skip comments
            if line[0] == '#': continue
            # splitlines necessary to get rid of carriage return on IRIX64
            line = line.splitlines()
            list = line[0].split()
            # skip blank lines
            if len(list) == 0: continue

            key = list[0]
            
            if len(list) > 1:
                if key == 'download': self.download = list[1]
                elif key == 'prefix': self.prefix = list[1]
                elif key == 'debug': self.debug = list[1]
                elif key == 'language': self.language = list[1]
                elif key == 'use_arch': self.use_arch = list[1]
                elif key == 'arch': self.arch = list[1]
                elif key == 'compC': self.cc = list[1]
                elif key == 'compF': self.fc = list[1]
                elif key == 'mpiCompC': self.mpicc = list[1]
                elif key == 'disable_gui': self.disable_gui = list[1]
                elif key == 'python': self.python = list[1]
                elif key == 'blas': self.blas = list[1]
                elif key == 'metis': self.metis = list[1]
                elif key == 'scotch': self.scotch = list[1]
                elif key == 'syrthes': self.syrthes = list[1]
                else:
                    p = self.packages[key]
                    p.use = list[2]
                    p.installation = list[3]
                    if (p.use == 'yes' or p.use == 'auto') \
                            and p.installation == 'no':
                        if list[1] != 'None':
                            p.install_dir = list[1]

        # Specify architecture name
        if self.use_arch == 'yes' and self.arch is None:
            self.arch = os.uname()[0] + '_' + os.uname()[4]

        # Expand user variables
        if self.prefix is not None:
            self.prefix = os.path.expanduser(self.prefix)
            self.prefix = os.path.expandvars(self.prefix)
            self.prefix = os.path.abspath(self.prefix)

        if self.python is not None:
            self.python = os.path.expanduser(self.python)
            self.python = os.path.expandvars(self.python)
            self.python = os.path.abspath(self.python)

        if self.blas is not None:
            self.blas = os.path.expanduser(self.blas)
            self.blas = os.path.expandvars(self.blas)
            self.blas = os.path.abspath(self.blas)

        if self.metis is not None:
            self.metis = os.path.expanduser(self.metis)
            self.metis = os.path.expandvars(self.metis)
            self.metis = os.path.abspath(self.metis)

        if self.scotch is not None:
            self.scotch = os.path.expanduser(self.scotch)
            self.scotch = os.path.expandvars(self.scotch)
            self.scotch = os.path.abspath(self.scotch)

        if self.syrthes is not None:
            self.syrthes = os.path.expanduser(self.syrthes)
            self.syrthes = os.path.expandvars(self.syrthes)
            self.syrthes = os.path.abspath(self.syrthes)

        # Compute Code_Saturne installation prefix
        if self.prefix is not None:
            subdir = os.path.join('cs-' + self.version)
            if self.arch is not None:
                subdir = os.path.join(subdir, 'arch', self.arch)
                if self.debug == 'yes': subdir = subdir + '_dbg'
            self.cs_prefix= os.path.join(self.prefix, subdir)


    def check_setup(self):

        check = """
Check the setup file and some utilities presence.
"""

        sys.stdout.write(check)
        if verbose == 'yes':
            sys.stdout.write("\n")

        # Testing download option
        if self.download not in ['yes', 'no']:
            sys.stderr.write("\n*** Aborting installation:\n"
                             "\'download\' option in the setup file "
                             "should be \'yes\' or \'no\'.\n"
                             "Please check your setup file.\n\n")
            sys.exit(1)

        # Testing debug option
        if self.debug not in ['yes', 'no']:
            sys.stderr.write("\n*** Aborting installation:\n"
                             "\'debug\' option in the setup file "
                             "should be \'yes\' or \'no\'.\n"
                             "Please check your setup file.\n\n")
            sys.exit(1)

        # Testing debug option
        if self.disable_gui not in ['yes', 'no']:
            sys.stderr.write("\n*** Aborting installation:\n"
                             "\'disable_gui\' option in the setup file "
                             "should be \'yes\' or \'no\'.\n"
                             "Please check your setup file.\n\n")
            sys.exit(1)

        # Testing language option
        if self.language not in ['en', 'fr']:
            sys.stderr.write("\n*** Aborting installation:\n"
                             "\'language\' option in the setup file "
                             "should be \'en\' or \'fr'.\n"
                             "Please check your setup file.\n\n")
            sys.exit(1)
            
        # Testing prefix directory
        if self.prefix is not None and not os.path.isdir(self.prefix):
            sys.stderr.write("\n*** Aborting installation:\n"
                             "\'%s\' prefix directory is provided in the setup "
                             "file but is not a directory.\n"
                             "Please check your setup file.\n\n"
                             % self.prefix)
            sys.exit(1)

        # Testing architecture option
        if self.use_arch not in ['yes', 'no']:
            sys.stderr.write("\n*** Aborting installation:\n"
                             "\'use_arch\' option in the setup file "
                             "should be \'yes\' or \'no\'.\n"
                             "Please check your setup file.\n\n")
            sys.exit(1)

        # Looking for compilers provided by the user
        for compiler in [self.cc, self.fc, self.mpicc]:
            if compiler is not None:
                ret = run_test(compiler)
                if ret != 0:
                    sys.stderr.write("\n*** Aborting installation:\n"
                                     "\'%s\' compiler is provided in the setup "
                                     "file but cannot be found.\n"
                                     "Please check your setup file.\n\n"
                                     % compiler)
                    sys.exit(1)
        
        # Looking for Python executable provided by the user
        if self.python is not None:
            python_exe = os.path.join(self.python, 'python')
        else:
            python_exe = "python"

        ret = run_test(python_exe)
        if ret != 0:
            if self.python is not None:
                sys.stderr.write("\n*** Aborting installation:\n"
                                 "\'%s\' Python path is provided in the setup "
                                 "file but no executable can be found.\n"
                                 "Please check your setup file.\n\n"
                                 % self.python)
            sys.exit(1)
        else:
            cmd = python_exe + " -c \'import sys; print sys.version[:3]\'"
            if verbose == 'yes':
                sys.stdout.write("     Python version is ")
            p = subprocess.Popen(cmd,
                                 shell=True,
                                 stdout=subprocess.PIPE,
                                 stderr=subprocess.STDOUT)

            output = p.communicate()
            if verbose == 'yes':
                if p.returncode == 0:
                    sys.stdout.write(output[0])

        # Checking libraries options
        for lib in self.libs:
            p = self.packages[lib]
            if p.use not in ['yes', 'no', 'auto']:
                sys.stderr.write("\n*** Aborting installation:\n"
                                 "\'%s\' use option in the setup file "
                                 "should be \'yes\', \'no' or \'auto\'.\n"
                                 "Please check your setup file.\n\n"
                                 % lib)
                sys.exit(1)
            if p.installation not in ['yes', 'no']:
                sys.stderr.write("\n*** Aborting installation:\n"
                                 "\'%s\' install option in the setup file "
                                 "should be \'yes\' or \'no'.\n"
                                 "Please check your setup file.\n\n"
                                 % lib)
                sys.exit(1)
            if p.installation == 'no' and p.use == 'yes':
                if not os.path.isdir(p.install_dir):
                    sys.stderr.write("\n*** Aborting installation:\n"
                                     "\'%(path)s\' path is provided for "
                                     "\'%(lib)s\' in the setup "
                                     "file but is not a directory.\n"
                                     "Please check your setup file.\n\n"
                                     % {'path':p.install_dir, 'lib':lib})
                    sys.exit(1)

        # Looking for BLAS path probided by the user
        if self.blas is not None and not os.path.isdir(self.blas):
            sys.stderr.write("\n*** Aborting installation:\n"
                             "\'%s\' BLAS directory is provided in the setup "
                             "file but is not a directory.\n"
                             "Please check your setup file.\n\n"
                             % self.blas)
            sys.exit(1)
            
        # Looking for Metis path probided by the user
        if self.metis is not None and not os.path.isdir(self.metis):
            sys.stderr.write("\n*** Aborting installation:\n"
                             "\'%s\' Metis directory is provided in the setup "
                             "file but is not a directory.\n"
                             "Please check your setup file.\n\n"
                             % self.metis)
            sys.exit(1)
            
        # Looking for Scotch path probided by the user
        if self.scotch is not None and not os.path.isdir(self.scotch):
            sys.stderr.write("\n*** Aborting installation:\n"
                             "\'%s\' Scotch directory is provided in the setup "
                             "file but is not a directory.\n"
                             "Please check your setup file.\n\n"
                             % self.scotch)
            sys.exit(1)
            
        # Looking for SYRTHES path probided by the user
        if self.syrthes is not None and not os.path.isdir(self.syrthes):
            sys.stderr.write("\n*** Aborting installation:\n"
                             "\'%s\' BLAS directory is provided in the setup "
                             "file but is not a directory.\n"
                             "Please check your setup file.\n\n"
                             % self.syrthes)
            sys.exit(1)

        # Looking for make utility
        ret = run_test("make")
        if ret != 0:
            sys.stderr.write("\n*** Aborting installation:\n"
                             "\'make\' utility is mandatory for Code_Saturne "
                             "compilation.\n"
                             "Please install development tools.\n\n")
            sys.exit(1)

        # Looking for pdflatex for potential documentation compilation
        ret = run_test("pdflatex")
        if ret != 0:
            self.packages['ncs'].installation_pdf = 'no'
            sys.stderr.write("\n*** Warning: "
                             "pdflatex utility cannot be found\n"
                             "Code_Saturne documentation will not be "
                             "installed\n")
        else:
            self.packages['ncs'].installation_pdf = 'yes'

        # Looking for fig2dev for potential documentation compilation
        ret = run_test("fig2dev")
        if ret != 0:
            self.packages['ncs'].installation_pdf = 'no'
            sys.stderr.write("\n*** Warning: "
                             "fig2dev utility cannot be found\n"
                             "Code_Saturne documentation will not be "
                             "installed\n")
        else:
            self.packages['ncs'].installation_pdf = 'yes'

        if verbose == 'yes':
            sys.stdout.write("\n")


    def update_package_opts(self):

        # Update log file, installation directory and compilers
        for lib in self.libs:
            p = self.packages[lib]
            # Update logging file
            p.log_file = self.log_file
            # Installation directory
            if p.installation == 'yes':
                if lib in self.satlibs:
                    p.install_dir = self.cs_prefix
                else:
                    subdir = os.path.join(p.package + '-' + p.version)
                    if self.arch is not None:
                        subdir = os.path.join(subdir, 'arch', self.arch)
                    p.install_dir = os.path.join(self.prefix, subdir)
            # Compilers
            if lib in ['fvm', 'ncs'] and self.mpicc is not None:
                p.cc = self.mpicc
            else:
                p.cc = self.cc
            if lib in ['med', 'ncs']:
                p.fc = self.fc

        # Update configuration options

        if self.debug == 'yes':
            for lib in self.satlibs:
                p = self.packages[lib]
                p.config_opts = p.config_opts + " --enable-debug"

        bft = self.packages['bft']
        fvm = self.packages['fvm']
        mei = self.packages['mei']
        ecs = self.packages['ecs']
        ncs = self.packages['ncs']

        cgns = self.packages['cgns']
        hdf5 = self.packages['hdf5']
        med= self.packages['med']
        mpi = self.packages['mpi']
        libxml2 = self.packages['libxml2']
        swig = self.packages['swig']

        # Disable GUI (and MEI Python bindings too)

        if self.disable_gui == 'yes':
            swig.use = 'no'
            ncs.config_opts = ncs.config_opts + " --disable-gui"

        # BFT

        for p in [fvm, mei, ecs, ncs]:
            p.config_opts = p.config_opts + " --with-bft=" + bft.install_dir

        # FVM

        ncs.config_opts = ncs.config_opts + " --with-fvm=" + fvm.install_dir

        # MEI

        if mei.use == 'yes':
            ncs.config_opts = ncs.config_opts + " --with-mei=" + mei.install_dir

        # Preprocessor

        ncs.config_opts = ncs.config_opts + " --with-prepro=" + ecs.install_dir

        # CGNS

        for p in [fvm, ecs]:
            if cgns.use == 'no':
                p.config_opts = p.config_opts + " --without-cgns"
            else:
                if cgns.install_dir is not None:
                    p.config_opts = p.config_opts + \
                        " --with-cgns=" + cgns.install_dir

        # HDF5

        if med.use == 'yes':
            med.config_opts = med.config_opts + " --with-hdf5=" + hdf5.install_dir

        for p in [fvm, ecs]:
            if hdf5.use == 'no':
                p.config_opts = p.config_opts + " --without-hdf5"
            else:
                if hdf5.install_dir is not None:
                    p.config_opts = p.config_opts + \
                        " --with-hdf5=" + hdf5.install_dir

        # MED

        for p in [fvm, ecs]:
            if med.use == 'no':
                p.config_opts = p.config_opts + " --without-med"
            else:
                if med.install_dir is not None:
                    p.config_opts = p.config_opts + \
                        " --with-med=" + med.install_dir

        # MPI

        for p in [fvm, ncs]:
            if mpi.use == 'no' and self.mpicc is None:
                p.config_opts = p.config_opts + " --without-mpi"
            else:
                if mpi.install_dir is not None:
                    p.config_opts = p.config_opts + \
                        " --with-mpi=" + mpi.install_dir

        if mpi.use == 'yes' or self.mpicc is not None:
            fvm.config_opts = fvm.config_opts + " --disable-mpi-io"

        # Metis

        if self.metis is not None:
            ecs.config_opts = ecs.config_opts + " --with-metis=" + self.metis

        # Scotch

        if self.scotch is not None:
            ecs.config_opts = ecs.config_opts + " --with-scotch=" + self.scotch

        # Libxml2

        if libxml2.use == 'no':
            ncs.config_opts = ncs.config_opts + " --without-libxml2"
        else:
            if libxml2.install_dir is not None:
                ncs.config_opts = ncs.config_opts + \
                    " --with-libxml2=" + libxml2.install_dir
            
        # Swig

        if swig.use == 'no':
            mei.config_opts = mei.config_opts + " --disable-python-bindings"
        else:
            if swig.install_dir is not None:
                mei.config_opts = mei.config_opts + \
                    " --with-swig-exec=" + os.path.join(swig.install_dir, 'bin')

        # Python

        for p in [mei, ncs]:
            if self.python is not None:
                p.config_opts = p.config_opts + \
                    " --with-python-exec=" + self.python

        if self.python is not None:
            swig.config_opts = swig.config_opts + \
                " --with-python=" + os.path.dirname(self.python)
        else:
            swig.config_opts = swig.config_opts + " --with-python"

        # BLAS

        if self.blas is not None:
            ncs.config_opts = ncs.config_opts + " --with-blas=" + self.blas

        # SYRTHES

        if self.syrthes is not None:
            ncs.config_opts = ncs.config_opts + " --with-syrthes=" + self.syrthes

        # Language

        if self.language == 'fr':
            ncs.config_opts = ncs.config_opts + " --enable-french"


    def install(self):

        for lib in self.libs:
            p = self.packages[lib]
            if p.installation == 'yes':
                sys.stdout.write("Installation of %s\n" % p.name)
                if self.download == 'yes':
                    p.download()
                p.extract()
                if verbose == 'yes':
                    p.info()
                p.install()
                p.installation = 'no'
                self.write_setup()
                if verbose == 'yes':
                    sys.stdout.write("\n")


    def write_setup(self):
        #
        # setup file update
        #
        sf = file(os.path.join(os.getcwd(), "setup"), mode='w')

        setupMain = \
"""#========================================================
#  Setup file for Code_Saturne installation
#========================================================
#
#--------------------------------------------------------
# Download packages
#--------------------------------------------------------
download  %(download)s
#
#--------------------------------------------------------
# Language
#    default: "en" english
#    others:  "fr" french
#--------------------------------------------------------
language  %(lang)s
#
#--------------------------------------------------------
# Install Code_Saturne with debugging symbols
#--------------------------------------------------------
debug     %(debug)s
#
#--------------------------------------------------------
# Installation directory
#--------------------------------------------------------
prefix    %(prefix)s
#
#--------------------------------------------------------
# Architecture Name
#--------------------------------------------------------
use_arch  %(use_arch)s
arch      %(arch)s
#
#--------------------------------------------------------
# C compiler
#--------------------------------------------------------
compC     %(cc)s
#
#--------------------------------------------------------
# Fortran compiler
#--------------------------------------------------------
compF     %(fc)s
#
#--------------------------------------------------------
# MPI wrapper for C compiler
#--------------------------------------------------------
mpiCompC  %(mpicc)s
#
#--------------------------------------------------------
# Disable Graphical user Interface
#--------------------------------------------------------
disable_gui  %(disable_gui)s
#
#--------------------------------------------------------
# Python is mandatory to launch the Graphical User
# Interface and to use Code_Saturne scripts.
# It has to be compiled with PyQt 4 support.
#
# It is highly recommended to use the Python provided
# by the distribution and to install PyQt through
# the package manager if needed.
#
# If you need to provide your own Python, just set
# the following variable to the bin directory of Python
#--------------------------------------------------------
python    %(python)s
#
#--------------------------------------------------------
# BLAS For hardware-optimized Basic Linear Algebra
# Subroutines. If no system BLAS is used, one reverts
# to an internal BLAS emulation, which may be somewhat
# slower.
#
# ATLAS (or another BLAS) should be available for most
# platforms through the package manager. If using the
# Intel or IBM compilers, IMKL or ESSL may be used in
# place of ATLAS respectively.
# For a fine-tuning of BLAS library support, it may
# be necessary to install Code_Saturne Kernel manually.
#--------------------------------------------------------
blas      %(blas)s
#
#--------------------------------------------------------
# Metis is more rarely found in Linux distributions,
# but may already be installed on massively parallel
# machines and on clusters. For good parallel
# performance, it is highly recommended.
# For meshes larger than 15 million cells, Metis 5.0
# beta is recommended, as Metis 4 may fail above
# the 20-35 million cells.
#
# Scotch can be use as an alternative.
#
# If both are present, Metis will be the default.
# If none are present, a space-filling-curve algorithm
# will be used.
#--------------------------------------------------------
metis     %(metis)s
scotch    %(scotch)s
#
#--------------------------------------------------------
# SYRTHES installation path for an optional coupling.
#
# Only coupling with the SYRTHES thermal code version 3
# is handled at the moment.
#
# SYRTHES has to be installed before Code_Saturne for
# a correct detection. However, it is still possible to
# update the scripts after Code_Saturne installation.
#--------------------------------------------------------
syrthes   %(syrthes)s
#
#========================================================
#  Name     Path                      Use         Install
#========================================================
"""
        setupLibSat = \
"""#
#--------------------------------------------------------
# Code_Saturne kernel and module libraries
#--------------------------------------------------------
#
"""
        setupLibOpt = \
"""#
#--------------------------------------------------------
# Optional packages:
# ------------------
#
# MED / HDF5  For MED file format support
#             (used by SALOME and now by Gmsh)
#
# CGNS        For CGNS file support
#             (used by many meshers)
#
# Open MPI (or MPICH2)
#
#   For Linux workstations, MPI, HDF5, and even MED
# packages may be available through the package manager.
# HDF5 is also often available on large systems such as
# IBM Blue Gene or Cray XT.
#
#   For massively parallel architectures, it is
# recommended to use the system's default MPI library.
#
#   Libxml2 is needed to read xml files output by the
# Graphical User Interface, and swig is needed by the
# Graphical User Interface to handle mathematical
# expressions. Both should be installed by you package
# manager.
#--------------------------------------------------------
#
"""
        setupLib= \
"""%(lib)s    %(dir)s    %(use)s  %(install)s
"""
        setupEnd= \
"""#
#========================================================
"""

        # Clean some potentially not-defined variables for output
        if self.prefix is None: self.prefix = ''
        if self.arch is None: self.arch = ''
        if self.cc is None: self.cc = ''
        if self.fc is None: self.fc = ''
        if self.mpicc is None: self.mpicc = ''
        if self.python is None: self.python = ''
        if self.blas is None: self.blas = ''
        if self.metis is None: self.metis = ''
        if self.scotch is None: self.scotch = ''
        if self.syrthes is None: self.syrthes = ''

        sf.write(setupMain
                 % { 'download':self.download, 'prefix':self.prefix,
                     'lang':self.language, 'debug':self.debug,
                     'use_arch':self.use_arch, 'arch':self.arch,
                     'cc':self.cc, 'fc':self.fc, 'mpicc':self.mpicc,
                     'disable_gui':self.disable_gui,
                     'python':self.python, 'blas':self.blas,
                     'metis':self.metis, 'scotch': self.scotch,
                     'syrthes':self.syrthes })

        sf.write(setupLibSat)
        for lib in self.satlibs:
            p = self.packages[lib]
            sf.write(setupLib % { 'lib':lib, 'dir':p.install_dir,
                                  'use':p.use, 'install':p.installation })

        sf.write(setupLibOpt)
        for lib in self.optlibs:
            p = self.packages[lib]
            sf.write(setupLib % { 'lib':lib, 'dir':p.install_dir,
                                  'use':p.use, 'install':p.installation })

        sf.write(setupEnd)
        sf.close()

#-------------------------------------------------------------------------------
# Main
#-------------------------------------------------------------------------------

if __name__ == "__main__":

    # Messages
    # --------
    welcome = \
        """
        Installation of Code_Saturne
        ____________________________
        
The process will take several minutes.
You can have a look at the log file meanwhile.
"""

    finalize = \
"""
Before using Code_Saturne, please update your path with:

  cspath=%(cspath)s
  export PATH=$cspath:$PATH

The documentation should then be available through the commands:
  code_saturne info -g refcard
  code_saturne info -g user

"""

    thanks = \
"""
Thank you for choosing Code_Saturne!

"""

    # Setup process
    # -------------
    sys.stdout.write(welcome)

    setup = Setup()

    setup.log_file = open('install_saturne.log', mode='w')

    setup.read_setup()
    setup.check_setup()
    setup.update_package_opts()
    setup.install()
    
    setup.log_file.close()

    sys.stdout.write(finalize % {'cspath':os.path.join(setup.cs_prefix, 'bin')})
    sys.stdout.write(thanks)
