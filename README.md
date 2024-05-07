# openglstub
OpenGL stub + some snippets based on repo https://github.com/ands/spherical_harmonics_playground 

To create repo like this:

```
git init
git submodule add https://github.com/glfw/glfw.git
cd glfw
git checkout 7b6aead9
```

commit 7b6aead9 points to version 3.4

Then copy CMakeLists.txt + code and resources

Note that it needs to be cloned with ```--recursive``` option because glfw is added as a submodule.

```
git clone --recursive https://github.com/ajkaczmar/openglstub.git
```
