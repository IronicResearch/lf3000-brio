#==============================================================================
# Proprietary & Confidential
# Copyright (c) 2005,2006 LeapFrog Enterprises, Inc.
# All Rights Reserved
#==============================================================================
# File:
#		Makefile
#
# Description:
#		Makefile for Nucleus kernel code.	   
#==============================================================================

#==============================================================================
# Makefile includes
#==============================================================================
include				../../Etc/BuildSystem/LFBuild.mk				# Global makefile defines

#==============================================================================
# Initialize directory build
#
# Parameters:
#		$(1) - current directory name
#		$(2) - default target
#
# Description:
#		Inits all the standard top-level targets for help, builds, cleans,
#		depends, installs, releases, etc.
#==============================================================================
LF_MAKE_DIR = Kernel

$(eval $(call LFStartMakeDir, $(LF_MAKE_DIR), help))

#==============================================================================
# LFBuildSettings.Target.$(LF_MAKE_DIR).Common
#
# Description:
#		Common sources, libraries & sub-makes for all builds in this directory.
#==============================================================================
define LFBuildSettings.Target.$(LF_MAKE_DIR).Common
	# Source files 
	# FIXME/dg: have .s--not platform agnostic
	#LF_TARGET_SRCS.Common		+= 	./KernelMPI.cpp		\
	#								./Kernel.cpp		\
	#								./KernelModule.cpp

	# Define build path for the common source files and the binding tag
	# that links the common sources to their custom build path
	LF_TARGET_SRCS_BUILD_PATH.Common  	+= $$(LF_TARGET_COMMON_SRCS_BUILD_PATH)
	LF_TARGET_BINDINGS					+= Common

	# List additional paths to header include directories
	LF_TARGET_INC_PATHS					+= 	./Nucleus				\
											./Nucleus/plus			\
											./Nucleus/plus_specific

	#LF_TARGET_MAKES				+= all@./MQX2.50  

	# List additional object/library files
	# FIXME/dg: not platform-agnostic
	#LF_TARGET_OBJS				+= ./MQX2.50/$$(LF_TARGET_BUILD_DIR)/$$(LF_PLATFORM)/MQX.lib

	# get all the MQX preprocessor defines for a build of MQX--defines MQX_STANDARD_CPPFLAGS
	# FIXME/dg: temp -- not platform-agnostic
	#include					./MQX2.50/configcpp.mk
	#LF_TARGET_AS_FLAGS +=	$$(MQX_STANDARD_CPPFLAGS) -D__MET__ -DMQX_CPU=0xACA5
	#LF_TARGET_C_FLAGS +=	$$(MQX_STANDARD_CPPFLAGS) -D__MET__ -DMQX_CPU=0xACA5
	#LF_TARGET_CC_FLAGS +=	$$(MQX_STANDARD_CPPFLAGS) -D__MET__ -DMQX_CPU=0xACA5

	# Additional symbols that should be extracted from MQX.lib
	#
	# These symbols are only referenced in the Kernel.(CHIP).lib
	# So the code for these symbols will not be transfered from MQX.lib to Kernel.lib
	# unless we inform the build that these symbols are needed.
	# FIXME/dg: still need these?

	#LF_TARGET_LD_REL_FLAGS	+=  -u _psp_set_int_level	\
	#							-u _vuart_serial_polled_install	\
	#						 	-u _vuart_serial_int_install


endef

#==============================================================================
# LFBuildSettings.Target.$(LF_MAKE_DIR).$(LF_TARGET)
#
# Description:
#		Defines target-specific sources, libraries, sub-makes, tool flags & targets
#
# For help:
#		make help@<target>		- for a list of the possible LF_TARGET_xxx settings 
#								  for the given target
#==============================================================================

#------------------------------------------------------------------------------
# Target:		Nucleus
#------------------------------------------------------------------------------
LF_TARGET := Nucleus

define LFBuildSettings.Target.$(LF_MAKE_DIR).$(LF_TARGET)
	LF_TARGET_ALIASES		= # none

	LF_TARGET_INSTALL_PATHS		= $$(LF_SYSTEM_LIB_PATH)

	LF_TARGET_MAKES		+= 	all@./Nucleus
endef

$(eval $(call LFDefineArchiveLibraryTarget, $(LF_TARGET), 				\
				$(LF_ALL_PLATFORM_TARGET_BUILD_SETTINGS)				\
				LFBuildSettings.Target.$(LF_MAKE_DIR).Common			\
				LFBuildSettings.Target.$(LF_MAKE_DIR).$(LF_TARGET)		\
				))

#------------------------------------------------------------------------------
# Target:		KernelImpl
#------------------------------------------------------------------------------
LF_TARGET := KernelImpl

define LFBuildSettings.Target.$(LF_MAKE_DIR).$(LF_TARGET)
	LF_TARGET_ALIASES		= # none

	LF_TARGET_INSTALL_PATHS		= $$(LF_SYSTEM_LIB_PATH)

	LF_TARGET_SRCS		+= 	./Kernel.cpp		\
							./KernelModule.cpp
endef

$(eval $(call LFDefineArchiveLibraryTarget, $(LF_TARGET), 				\
				$(LF_ALL_PLATFORM_TARGET_BUILD_SETTINGS)				\
				LFBuildSettings.Target.$(LF_MAKE_DIR).Common			\
				LFBuildSettings.Target.$(LF_MAKE_DIR).$(LF_TARGET)		\
				))

#==============================================================================
# LFBuildSettings.Target.$(LF_MAKE_DIR).$(LF_TARGET)
#
# Description:
#		Defines target-specific sources, libraries, sub-makes, tool flags & targets
#
# For help:
#		make help@<target>		- for a list of the possible LF_TARGET_xxx settings 
#								  for the given target
#==============================================================================

#------------------------------------------------------------------------------
# Target:		KernelMPI
#------------------------------------------------------------------------------
LF_TARGET := KernelMPI

define LFBuildSettings.Target.$(LF_MAKE_DIR).$(LF_TARGET)
	LF_TARGET_ALIASES		= # none

	LF_TARGET_INSTALL_PATHS	= $$(LF_SYSTEM_LIB_PATH)

	LF_TARGET_SRCS			+= ./KernelMPI.cpp
endef

$(eval $(call LFDefineArchiveLibraryTarget, $(LF_TARGET), 				\
				$(LF_ALL_PLATFORM_TARGET_BUILD_SETTINGS)				\
				LFBuildSettings.Target.$(LF_MAKE_DIR).Common			\
				LFBuildSettings.Target.$(LF_MAKE_DIR).$(LF_TARGET)		\
				))

#==============================================================================
# Done
#==============================================================================
$(eval $(call LFEndMakeDir, $(LF_MAKE_DIR)))

# EOF


	