############################################################################
# This file is part of ProcessLib, a submodule of LImA project the
# Library for Image Acquisition
#
# Copyright (C) : 2009-2011
# European Synchrotron Radiation Facility
# BP 220, Grenoble 38043
# FRANCE
#
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.
############################################################################
import os
import glob
import sipconfig
import shutil
import numpy
import platform

shutil.copyfile("processlib.sip","processlib_tmp.sip")

##Append Tasks sip
sipFile = file("processlib_tmp.sip","a")
sipFile.write('\n')
for filename in glob.glob(os.path.join("..","tasks","sip","*.sip")) :
    sipFile.write('%%Include %s\n' % filename.replace('\\','/'))
sipFile.close()


# The name of the SIP build file generated by SIP and used by the build
# system.
build_file = "processlib.sbf"
config = sipconfig.Configuration()

# Run SIP to generate the code.
# module's specification files using the -I flag.
cmd = " ".join([config.sip_bin,"-g", "-e","-c", '.',
                "-b", build_file,"processlib_tmp.sip"])
print cmd
os.system(cmd)

# We are going to install the SIP specification file for this module and
# its configuration module.
installs = []

installs.append(["processlib.sip", os.path.join(config.default_sip_dir, "processlib")])

installs.append(["processlibconfig.py", config.default_mod_dir])

# Create the Makefile.  The QtModuleMakefile class provided by the
# pyqtconfig module takes care of all the extra preprocessor, compiler and
# linker flags needed by the Qt library.
makefile = sipconfig.ModuleMakefile(
    configuration=config,
    build_file=build_file,
    installs=installs,
  )
makefile.extra_include_dirs = [os.path.join("..","core","include")]
if platform.system() == 'Windows':
    makefile.extra_cxxflags = ['/I..\core\include','/I..\\tasks\include',
                               '/I..\core\include\WindowSpecific',
                               '/I' + config.sip_inc_dir,
                               '/I' + numpy.get_include(),
                               '/EHsc']
    makefile.extra_libs = ['libprocesslib']
    #makefile.extra_lib_dirs = ['..\\build']
    #makefile.extra_lib_dirs = ['..\\build\\msvc\\9.0\\libprocesslib\\Debug']
    makefile.extra_lib_dirs = ['..\\build\\msvc\\9.0\\libprocesslib\\Release']
else:
    makefile.extra_cxxflags = ['-pthread']
    makefile.extra_include_dirs = ['../core/include','../tasks/include',config.sip_inc_dir,numpy.get_include()]
    makefile.extra_libs = ['pthread','processlib']
    makefile.extra_lib_dirs = ['../build']

# Add the library we are wrapping.  The name doesn't include any platform
# specific prefixes or extensions (e.g. the "lib" prefix on UNIX, or the
# ".dll" extension on Windows).
# None (for me)

# Generate the Makefile itself.
makefile.generate()

# Now we create the configuration module.  This is done by merging a Python
# dictionary (whose values are normally determined dynamically) with a
# (static) template.
content = {
    # Publish where the SIP specifications for this module will be
    # installed.
    "processlib_sip_dir":    config.default_sip_dir
}

# This creates the pixmaptoolsconfig.py module from the pixmaptoolsconfig.py.in
# template and the dictionary.
sipconfig.create_config_module("processlibconfig.py", "processlibconfig.py.in", content)

