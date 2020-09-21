# name of your application
APPLICATION = saran-nb-iot-example
APPLICATION_VERSION = 0.1.0

# platform for which the application will be built
PLATFORM ?= octa

LOW_POWER ?= 1

DEBUG = 1

# NB-IoT
SHIELDS += NB-IoT
NB_IOT_CONNECTOR = 1
# Available providers: Orange, Proximus, Telenet
NB_IOT_PROVIDER = Orange
# DYAMAND_TESTING_STAGING, DYAMAND_TESTING_PRODUCTION, IOW_STAGING, IOW_PRODUCTION, IDLAB_TESTING
NB_IOT_SERVER = IDLAB_TESTING

# Set PRINT_FLOATS to 1 to enable linker flags enabling float printf & sprintf
PRINT_FLOATS = 1

BOOTLOADER ?= 1
# Path to the octa-stack base directory:
STACKBASE ?= ../..
include $(STACKBASE)/core/Makefile.core
