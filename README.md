[![Build Status](https://travis-ci.org/Caspinol/tosamo.svg?branch=master)](https://travis-ci.org/Caspinol/tosamo)

TOSAMO - syncronise parts of configuration files across linux and/or OSX machines.

When you need to synchronize "parts" of a config file look no further...

WHY:
	Imagine you have a file.conf on server A and file.conf on server B.
	The configuration file contains parts that are common to both servers but also
	configuration bits specific to machine its running on (i.e bind interface).

HOW:
	Simply envelop the common parts of the file with a tag i.e

	#%% as opening tag
	    and
	%%# as closing tag

	(You can specify different tags in the config file.)

and start the program with:

	"/etc/init.d/tosamod start" - on a Linux system
	"launchctl load -w /Library/LaunchDaemons/net.catdamnit.tosamod.plist" - on OSX

The program works in master/slave model so you need to specify the master machine and slave.
Master always uploads to slave.

