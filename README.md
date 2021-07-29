# A pong clone made with raylib(-cpp) with an ECS system
Started as an attempt to make pong with raylib, evolved into me trying to use raylib-cpp because (somehow) I forgot that I can just complie raylib's C code with a C++ compiler to use C++ features, me discovering how raylib-cpp can help, then me experimenting with different kinds of systems and C++ features. (Quadtrees, ECS architecture, C++ templates, variadics, and double dispatching). It's been a fun trip up until now (although not without it's fair share of compiler and linker errors...). 

Feel free to use this however you like, or to improve on it/suggest improvements.

# What's currently implmented?
- An ECS system. Not really based off any research, just based off my requirements and my experience with the Unity Engine.
- An immature collision system. The only colliders right now are circles and rectangles, and I'm sure performance could be improved greatly if given to an experienced programmer.
- A delayed function calling system. Currently allows passing functions and lambdas with a delay.
- I probably forgot something...

