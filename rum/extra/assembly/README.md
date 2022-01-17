# Rum Assembly

Utilizing rum's auto messaging system, we create this utility to allow assembling software components freely across processes.  
The idea is: instead of creating executables and writing main functions, we write a rum component for every module and compile it into a library; then we use `rumassemble`or `rum assemble` command to launch a selection of these components for each process. This way, we have the choice to achieve best communication performance by putting components in same process, or better isolation by putting them in different processes.

