# Introdution to the CAN-Bus Communication Protocol - Zips Racing
CAN-bus (or more specifically, CAN 2.0a) is a communication protocol used for connecting multiple control units spread over a large area. Due to its flexibility and reliability, CAN-Bus is a common choice for connecting the various subsystems of a vehicle. For an explanation of the general concepts of CAN-bus, see the below video. Note that parts about OBD2, J1939, NMEA, and other higher-level protocols aren't relevant in our applications.

[CSS-Electronics - CAN-Bus Explained - A Simple Intro (https://www.youtube.com/watch?v=oYps7vT708E)](https://www.youtube.com/watch?v=oYps7vT708E)

## The CAN Device Library
In ZRE-CAN-Tools, CAN bus is exposed through the CAN device interface. The majority of computer cannot interface with a CAN bus without the use of a specific adapter. The CAN device (aka the `canDevice_t` struct) is an interface representing such an adapter. As there are many different types of adapters with equally many implementations, this interface simply defines what a CAN device *looks* like, rather that how it is *implemented*.

The CAN device library provides implementations for 2 specific classes of CAN adapters:
- SocketCAN devices
- SLCAN devices

For documentation on the objects and functions involved in the CAN device library, see the library's header:

[../lib/can_device/can_device.h](../lib/can_device/can_device.h)

## The CAN Database Library
A common abstraction to apply to a CAN bus is a CAN database. A CAN database is a relational database that aggregates all information present in a CAN bus for random access. The CAN database library is an implementation of this exact concept.

A CAN database is constructed from a CAN database file. For an introduction to the CAN database file, or just for a refresher, see the provided introduction: [can_database_file_introduction.md](can_database_file_introduction.md).

For documentation on the objects and functions involved in the CAN database library, see the library's header:

[../lib/can_database/can_database.h](../lib/can_database/can_database.h)
