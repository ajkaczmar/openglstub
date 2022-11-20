# openglstub
OpenGL stub + some snippets based on repo https://github.com/ands/spherical_harmonics_playground 

To create repo like this:

```
git init
git submodule add https://github.com/glfw/glfw.git
cd glfw
git checkout 9017eaee
```

Then copy CMakeLists.txt + code and resources
There is an error on macOS to fix it in cocoa_window line that contains makeFirstResponder should be placed just before return.

Note that it needs to be cloned with ```--recursive``` option because glfw is added as a submodule.

```
git clone --recursive https://github.com/ajkaczmar/openglstub.git
```
