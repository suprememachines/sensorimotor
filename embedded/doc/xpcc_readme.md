# Getting started with xpcc

Hi there. This is the starting point for using [the xpcc microcontroller framework](http://xpcc.io) in your project.

We created a couple of simple programs to show-case different parts of xpcc on as many development boards as we could find.
You can change the development board for which you want to compile the example for in the `project.cfg` file.
Note that not all examples compile with all boards, so make sure to check the example specific Readme first.

Here is an overview of the examples:

- [`hello-world`](hello-world): LED blinking and Serial output.

xpcc's [API documentation can be found here][api].

## Get the code

```sh
git clone --recursive https://github.com/roboterclubaachen/getting-started-with-xpcc.git
```

The example contains the xpcc framework as a git submodule, a `Vagrantfile`
to enable use of [our virtual machine](http://xpcc.io/installation/#virtual-machine),
a `SConstruct` file for [our build system](http://xpcc.io/reference/build-system/#build-commands),
a project configuration file and of course the source code.

## Your own project

When you create you own project, you need to adapt the `xpccpath` inside the
`SConstruct` to point to the location of the xpcc framework.
Note that this allows you to use different versions of the xpcc frameworks
(your own fork?) for your projects.

```python
# path to the xpcc root directory (modify path as needed, but do not rename!)
xpccpath = '../xpcc'
# execute the common SConstruct file
execfile(xpccpath + '/scons/SConstruct')
```


[api]: http://xpcc.io/reference/api/
[examples]: https://github.com/roboterclubaachen/xpcc/tree/develop/examples
