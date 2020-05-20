#####
# Specify another source file here if needed and add prefix $(SRCS_DIR)\, for example 
# $(SRCS_DIR)\MyUsart.cpp, to add a source file named MyGpio.c. And add a backslash to the 
# source file item before it. And the result should look like this:
#
# SRCS				= \
#					  $(SRCS_DIR)\Main.cpp\
#					  $(SRCS_DIR)\MyUsart.cpp
#
# Note: Visual Studio do not add additional source file to this file when you add a source
#       file to Visual Studio project, you have to manually add the source file here. The same
#		thing apply when you rename the source file(s) on Visual Studio, so you need to change
#		or rename source file(s) in this macro.
#
 
SRCS				= \
					  $(SRCS_DIR)\Main.cpp
