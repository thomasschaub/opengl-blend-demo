# opengl-blend-demo

Small demo to quickly toy around with OpenGL's blending.

# Usage

    $ opengl-blend-demo a.png b.png

This will render a.png, then b.png. 

* Set the source factor using q, w, ..., f
* Set the destination factor using Shift + q, Shift + w, ..., Shift + f
* Set the RGB blend equation using z, x, ..., b
* Set the alpha blend equation using Shift + z, Shift + x, ..., Shift + b

Note that you can pass more than two images.

# Compiling

    $ cmake /path/to/opengl-blend-demo && cmake --build .
