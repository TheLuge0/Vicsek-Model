# Vicsek Model

This project is based on the vicsek's model which I've modified slightly

**Information :**
- To compile the .c you can use this command : gcc -o vicsek.exe -lm vicsek.c $(sdl2-config --cflags --libs)
- You need to install the SDL2 library with this command on linux : sudo apt-get install libsdl2-2.0-0
- To lunch the simulation use this command : ./vicsek.exe

You can download the .exe, execute them and just skip all the previous steps.
You can change the values of variables inside the vicsek.h to see the reaction of the group. <br>
**Important :** To apply the changes, you must recompile the vicsek.c.
