# name of your application
APPLICATION = lora-gps-example
APPLICATION_VERSION = 0.1.0

# platform for which the application will be built
PLATFORM ?= octa

# Set PRINT_FLOATS to 1 to enable linker flags enabling float printf & sprintf
PRINT_FLOATS = 1

# Add shields used in application
# Murata
SHIELDS += Murata-dualstack
MURATA_CONNECTOR = 3
LORAWAN_APPLICATION = portforward

# GPS
SHIELDS += gps-firefly
FIREFLY_CONNECTOR = 1

# Path to the octa-stack base directory:
STACKBASE ?= ../..

include $(STACKBASE)/core/Makefile.core