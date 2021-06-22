

#Sunrise Stage system

Description: an abstract system for dependancy traking both for execution and resources

usecase: scene update systems, scene rener coordinator (configurable render command encoding)

Components:

stages: item which performs the action. there are many of them and their execution and usage of shared resources allows for parallelisation of work
without running into write conflicts

Dispatcher: holds stages and incharge of calling stages and performing dependancy managment

execution cycle: started by the dispatcher


design criteria:

dispatcher graph of stages should be updateable but in the begignnign it will not be

needs to support "reosurce transitions" (for vulkan render cooridnator images have to transtion layouts and have to transtion in and out of render passes)

have options which can be passed to the run funtion of stages
subclass stages for gpu stages which are subclassed for gpu render and gpu compute passes

help pages:

https://www.youtube.com/watch?v=D4hz0wEB978&list=PLlrATfBNZ98dC-V-N3m0Go4deliWHPFwT&index=77 at right before 10:26 for churno about templates and importing and dlls

more template stuff
https://medium.com/@ivan.matmati/advanced-c-templates-fdfc48ea3d8a