#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/mid_ivr.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L/usr/lib64 -lpjsua2 -lpjsua -lpjsip-ua -lpjsip-simple -lpjsip -lpjmedia-codec -lpjnath -lpj -lpjlib-util -lpjmedia -lcrypto

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/mid-ivr

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/mid-ivr: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/mid-ivr ${OBJECTFILES} ${LDLIBSOPTIONS} -lm -lpthread -ldl -lcrypto

${OBJECTDIR}/mid_ivr.o: mid_ivr.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/usr/include -I/usr/include/pjsua2 -I/usr/include/pjsip -I/usr/lib64 -I../../programs/pgsip/pjsip/include -I../../programs/pgsip/pjlib/include -I../../programs/pgsip/pjlib-util/include -I/home/hamza/programs/pgsip/pjmedia/include -I../../programs/pgsip/pjnath/include -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/mid_ivr.o mid_ivr.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
